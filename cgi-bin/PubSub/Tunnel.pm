package PubSub::Tunnel;

# Copyright 2000-2002 KnowNow, Inc.  All Rights Reserved.
#
# @KNOWNOW_LICENSE_START@
# 
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 
# 1. Redistributions of source code must retain the above copyright
# notice, this list of conditions and the following disclaimer.
# 
# 2. Redistributions in binary form must reproduce the above copyright
# notice, this list of conditions and the following disclaimer in
# the documentation and/or other materials provided with the
# distribution.
# 
# 3. The name "KnowNow" is a trademark of KnowNow, Inc. and may not
# be used to endorse or promote any product without prior written
# permission from KnowNow, Inc.
# 
# THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESSED OR IMPLIED
# WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
# IN NO EVENT SHALL KNOWNOW, INC. OR ITS CONTRIBUTORS BE LIABLE FOR ANY
# DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
# GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
# IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
# OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
# ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
# 
# @KNOWNOW_LICENSE_END@
#
# $Id: Tunnel.pm,v 1.2 2003/12/23 00:57:09 cobbr2 Exp $

# NAME
# PubSub::Tunnel - Module for managing tunnels to mod_pubsub.

# SYNOPSIS
# use PubSub::Tunnel;
# my $tunnel = new PubSub::Tunnel('http://someserver/kn/foo/kn_journal');
# my @events = $tunnel->get_events;
# my $r = '';
# vec($r, $tunnel->fileno, 1) = 1;
# select($r, undef, undef, 1);
# if (vec($r, $tunnel->fileno, 1)) {
#     process_more_events $tunnel->get_events;
# } else {
#     print "no data on tunnel for one second\n";
# }
# $tunnel->close;

# DESCRIPTION
# Given the URL of the kn_journal topic to route from, returns a
# PubSub::Tunnel.  When you want to receive events over a tunnel from
# mod_pubsub, you can use this module.

# FIXME: Tunnel.pm is not event-driven.  This package is not suitable
# for using in event-loop-driven programs because the constructor blocks.

# FIXME: Can't tell when a tunnel has gotten cut off.

# FIXME: No auth info yet.

use strict;
use PubSub::EventFormat 'from_rfc_822_format';
use PubSub::WebHelper 'http_request';
use IO::Socket;
use Fcntl; # Because IO::Socket doesn't provide setblocking(), how dumb.
use Errno;
use URI;

sub new
{
    my ($class, $url) = @_;
    my $self = bless {}, $class;
    my $uri = new URI($url);

    $self->{uri} = $uri;
    $self->{buf} = '';

    $self->{tunnel_established} = 0;#$self->establish_tunnel();

    return $self;
}

sub get_line
{
    my ($self) = @_;
    my $line;
    while (1)
    {
        # For whatever reason, sticking this pattern match in the
        # conditional doesn't work.  Is $1 scoped?
        if ($self->{buf} =~ s/\A(.*\n)//) 
        {
            $line = $1;
            last;
        }

        if (!$self->read_pending_data())
        {
            return undef;
        }
        select(undef, undef, undef, 0.05);
    }
    return $line;
}


sub get_events
{
    my ($self) = @_;
    if (! $self->{tunnel_established}) {
        $self->establish_tunnel();
    }
    if ($self->{tunnel_established}) {
        $self->read_pending_data();
        return $self->parse_events();
    }
    return;
}

sub read_pending_data
{
    my ($self) = @_;
    my $buf = '';
    my $fileno = fileno($self->{sock});
    while (1)
    {
        my $rfds = '';
        vec($rfds, $fileno, 1) = 1;
        select($rfds, undef, undef, 0);
        if (vec($rfds, $fileno, 1) == 0) 
        {
            # No more data to read.
            return 1;
        }
        my $rv = sysread $self->{sock}, $buf, 4096;
        $self->{buf} .= $buf if $rv;
        if (not $rv) 
        {
            if (defined $rv)
            {
                die "EOF on tunnel " . $self->{uri};
                # EOF
                $self->close();
                return length($self->{buf}) != 0;
            } 
            else
            {
                die "Read error on tunnel " . $self->{uri} . ": $!";
            }
        }
    }
}

sub parse_events
{
    my ($self) = @_;
    # We munch on our local copy of $self->{buf}.
    my $buf = $self->{buf};
    my $len;
    for ($buf)
    {
        s/\A\s+//;    # Remove leading whitespace.
        unless (s/\A(\d+)\n//)  # remove leading count and colon
        {
            if ($buf =~ /\A\D/ or $buf =~ /\A\d+[^\d\n]/)
            {
                die ("Malformed data on tunnel " . $self->{uri} . 
                     ": " . $self->{buf});
            }
            elsif ($buf =~ /\A\d*\z/)   # Incomplete count.
            {
                return ();
            }
            else
            {
                die "Internal error: didn't match regexps: '$buf'";
            }
        }
        $len = $1;
    }
    # Don't do anything if the event is incomplete.
    return () if length($buf) < $len + 1;

    my $event_string = substr($buf, 0, $len);
    my $linefeed = substr($buf, $len, 1);
    my $ordlinefeed = ord $linefeed;
    if ($linefeed ne "\n")
    {
        $self->close();
        $self->{error_msg} =  "Malformed data on tunnel: " . $self->{uri} .
            ": got '$linefeed' ($ordlinefeed) instead of linefeed after '$event_string' (len $len) in buf $buf";
        $self->{tunnel_established} = 0;
        return ();
    }
    my $event = from_rfc_822_format($event_string);
    $self->{buf} = substr($buf, $len+1);
    return ($event, $self->parse_events());
}

sub establish_tunnel
{
    my ($self) = @_;
    if ($self->{uri}->scheme() ne 'http') {
        $self->{error_msg} = "Only HTTP tunnels supported";
        $self->{tunnel_established} = 0;
        return 0;
    }
    $self->{uri}->query_form(do_method => 'route',
                             kn_response_format => 'simple');

    my $sock = new IO::Socket::INET($self->{uri}->host_port());
    if (not $sock) {
		$self->{error_msg} = "Couldn't open socket to " . $self->{uri}->host_port() 
	. ": $!";
        $self->{tunnel_established} = 0;
        return 0;
    }
    $self->{sock} = $sock;

    # HTTP request.
    $sock->print(http_request("GET", $self->{uri}));

    # Here we read the status line.
    # Note no HTTP/0.9 or redirect support, sorry.
    my $status_line = $self->get_line();
    if ($status_line =~ m|\AHTTP/1\.\d+ (\d\d\d) .*\r?$|)
    {
        my $status = $1;
        if ($status ne '200')
        {
            # Wow, I wish there was a better way to report errors.
            $sock->close();
            $self->{error_msg} = "Got non-200 status $status: $status_line (for 
$self->{uri})";
            $self->{tunnel_established} = 0;
            return 0;
        }
    }
    else
    {
        $status_line =~ s/([\x00-\x1f])/sprintf "\\x%02x", ord($1)/ge;
        $self->{error_msg} = "Couldn't parse HTTP status line $status_line";
        $self->{tunnel_established} = 0;
        return 0;
    }

    # And ignore the headers.
    local $_;
    header: while (defined ($_ = $self->get_line()))
    {
	  last header if $_ eq "\r\n" or $_ eq "\n";
    }
    $self->{tunnel_established} = 1;
    return 1;
}

sub close
{
    my ($self) = @_;
    $self->{sock}->close();
}

sub fileno
{
    my ($self) = @_;
    return $self->{sock}->fileno if defined($self) && defined($self->{sock});
    return undef ;
}

1;

# End of Tunnel.pm
