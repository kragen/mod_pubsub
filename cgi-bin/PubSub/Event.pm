package PubSub::Event;

# Copyright 2000-2004 KnowNow, Inc.  All rights reserved.
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
# notice, this list of conditions and the following disclaimer in the
# documentation and/or other materials provided with the distribution.
# 
# 3. Neither the name of the KnowNow, Inc., nor the names of its
# contributors may be used to endorse or promote products derived from
# this software without specific prior written permission.
# 
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
# 
# @KNOWNOW_LICENSE_END@
# 
#
# $Id: Event.pm,v 1.2 2004/04/19 05:39:09 bsittler Exp $

use LWP::UserAgent;
use HTTP::Request::Common 'POST';
use PubSub::WebHelper 'encode_form', 'http_request';
use URI;
use IO::Socket;
use strict;

my $ua = new LWP::UserAgent;
my $url;

sub kn_url_is
{
    $url = $_[0];
}

sub new
{
    my ($class, %vals) = @_;
    my ($package, $filename, $line) = caller;
    return bless { "event" => \%vals }, $class;
}

sub get_header
{
    my ($self, $header) = @_;
    return $self->{"event"}->{$header};
}

sub set_header
{
    my ($self, $header, $value) = @_;
    $self->{'event'}->{$header} = $value;
}

sub kn_id
{
    my ($self) = @_;
    return $self->get_header("kn_id");
}

sub kn_payload
{
    my ($self) = @_;
    return $self->get_header("kn_payload");
}

sub as_form
{
    my ($self, $topic, $debug) = @_;
    return (%{$self->{event}}, 'kn_to' => $topic,
            ($debug ? (kn_debug => 1) : ()));
}

sub post
{
    my ($self, $topic, $debug) = @_;
    my $result = $ua->request(POST $url, [ $self->as_form($topic, $debug) ]);
    die "Bad result posting to ${url}'s $topic: " . $result->message 
        unless $result->code =~ /^2/;
    warn $result->as_string if $debug;
    # warn "Posting to $topic\n";
    return $result;
}

# Post without waiting for reply.

sub send
{
    my ($self, $topic, $debug) = @_;
    my $uri = new URI($url);
    my $host_port = $uri->host_port();
    my $sock = new IO::Socket::INET($host_port);
    die "Couldn't open socket: $!" if not $sock;
    print $sock http_request("POST", $uri, 
                             encode_form($self->as_form($topic, $debug)));
    # If I wanted this to work on broken TCP/IP implementations, I'd
    # have to do an Apache-style lingering_close here.
    # But apparently, on recent Linux talking to recent Linux or
    # recent Solaris, we're OK.
    close $sock;
}

1;

# End of Event.pm
