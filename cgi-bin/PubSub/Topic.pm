package PubSub::Topic;

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
# $Id: Topic.pm,v 1.1 2002/11/07 07:08:00 troutgirl Exp $

# All of pubsub.cgi's output, except for things like do_method=help and
# do_method=lib, is handled by this module.

# It takes a CGI object and a reference to an event-posting function
# as constructor parameters.  Then you feed it a stream of events.
# The first event is expected to be a status event, with a "status"
# attribute.

# Depending on the CGI parameters, the events may be posted to a topic
# (using the event-posting function) or sent over the HTTP connection.

use strict;
use URI ();
use PubSub::Htmlsafe;
use PubSub::Status;
use PubSub::EventFormat ();

sub new
{
    my ($class, $q, $post) = @_;
    my $self = bless {}, $class;
    $self->{'kn_status_to'} = $q->param('kn_status_to');
    $self->{'kn_response_format'} = $q->param('kn_response_format');
    if (not defined $self->{'kn_response_format'})
    {
        $self->{'kn_response_format'}='js';
    }
    $self->{'need_cgi_prologue'} = 1; # true until we print CGI headers
    $self->{'watching'} = 0; # true when we need an onload handler
    $self->{'serial'} = 0; # serial number for form indexes
    $self->{'cgi'} = $q;
    if ($self->is_forwarded())
    {
        $self->{'post'} = $post;
        $self->{'need_cgi_prologue'} = 0;
        print "Status: 204 Content forwarded\n\n";
        return $self;
    }
    elsif (not ($self->is_javascript() || $self->is_simple()))
    {
        $self->{'kn_response_format'} = 'js';
        my $event = $self->status_event();
        $event->status('403 Unsupported kn_response_format value');
        $event->log('This server supports the values "js" and "simple" ' .
                    'for the kn_response_format parameter.');
        $event->send_to($self);
    }
    else
    {
        return $self;
    }
}

sub send
{
    my ($self, $event) = @_;
    my $eventtype = ref($event);
    die "Attempt to send non-event on topic: $eventtype\n"
        if $eventtype ne ref({});
    # perhaps we should replace these ifs with polymorphism?
    if ($self->is_forwarded())
    {
        $self->{'post'}->($self->{'cgi'}, new PubSub::Status(),
                          $self->{'kn_status_to'}, $event);
        return ();
    }
    elsif ($self->is_simple())
    {
        if ($self->{'need_cgi_prologue'})
        {
            die "Status event had no status." if (not $event->{'status'});
            my $q = $self->{'cgi'};
             print($q->header(-type => 'text/plain; charset=utf-8',
                              -status => $event->{'status'}));
            $self->{'need_cgi_prologue'} = 0;
        }
        my $formatted = PubSub::EventFormat::to_rfc_822_format($event);
        print length($formatted) . "\n" . $formatted . "\n";
        return ();
    }
    elsif ($self->is_javascript()) # JavaScript+HTML output
    {
        my $cgi_url = $self->{'cgi'}->url();
        
        if ($self->{'need_cgi_prologue'})
        {
            die "Status event had no status." if (not $event->{'status'});
            my $q = $self->{'cgi'};
            my $status = PubSub::Htmlsafe::entify($event->{'status'});
            my $kn_payload = PubSub::Htmlsafe::entify($event->{'kn_payload'});
            print($q->header(-type => 'text/html; charset=utf-8',
                             -status => $event->{'status'}),
                  $q->start_html(-title => $status,
                                 ($self->{'watching'} ?
                                  (-onLoad => 
                                   'if(parent.kn_tunnelLoadCallback)' . 
                                   ' parent.kn_tunnelLoadCallback(window)'
                                  ) : ())),
                  $q->h1($status),
                  $q->pre($kn_payload)
                 );
            # If there was a client error, link to the help file
            if ($status =~ m/^4/)
            {
                # Build URI for the help file
                my $help_url = URI->new("../kn_docs/");
                $help_url = $help_url->abs($cgi_url);
                
                print $q->p(
                            "Some aspect of your request was not understood." .
                            " Please see the <a href=\"$help_url\">mod_pubsub" .
                            " documentation</a> for more information."
                           );
            }
            print "<!--";
            $self->{'need_cgi_prologue'} = 0;
        }
        my $id = $self->{'serial'};
        print <<eos;
--><script type="text/javascript"><!--
if (parent.kn_sendCallback) parent.kn_sendCallback({elements:[
eos
        my $first = 1;
        for my $key (keys %$event)
        {
            my $value = $event->{$key};

            # if any UTF-8 sequences are actually present, tell the
            # client to decode them by appending U to the property
            # name
            my $kkey = "name";
            my $vkey = "value";
            if ($key =~ /[\x80-\xff]/)
            {
                $kkey .= 'U';
            }
            if ($value =~ /[\x80-\xff]/)
            {
                $vkey .= 'U';
            }

            # escape characters to be placed in JavaScript string literals
            for ($key, $value)
            {
                s/([\\""])/\\$1/g;
                s/\f/\\f/g;
                s/\n/\\n/g;
                s/\r/\\r/g;
                s/\t/\\t/g;
                s/(<|[^ -~])/sprintf("\\x%2.2x",ord($1))/ge;
            }
            if ($first)
            {
                $first = 0;
            }
            else
            {
                print ",";
            }
            print <<eos;
{$kkey:"$key",$vkey:"$value"}
eos
        }
        print <<eos;
]}, window);
// -->
</script><!--
eos
        $self->{'serial'} = $id + 1;
        return ();
    }
}

sub is_forwarded
{
    my ($self) = @_;

    return defined $self->{'kn_status_to'};
}

sub is_javascript
{
    my ($self) = @_;
    return $self->{'kn_response_format'} eq 'js';
}

sub is_simple
{
    my ($self) = @_;
    return $self->{'kn_response_format'} eq 'simple';
}

sub is_watching
{
    my ($self) = @_;
    # this only makes sense for JavaScript output
    $self->{'watching'} = 1;
}

# Some Web browsers won't render newly-received information without
# first receiving a certain number of bytes; for all cases we know of,
# 4096 is enough.  So we call this routine after sending anything we
# want the browser to notice.

sub tickle_renderer
{
    my ($self) = @_;
    if ($self->is_javascript() or not exists $ENV{"MOD_PERL"})
    {
        print " " x 4096, "\n";
    }
}

sub close
{
    my ($self) = @_;
    if ($self->is_forwarded())
    {
        return;
    }
    if ($self->is_javascript() || $self->is_simple())
    {
        if ($self->{'need_cgi_prologue'})
        {
            my $event = $self->status_event();
            $event->status('500 Internal error, no status to report');
            $event->log('No HTTP status was reported.');
            $event->send_to($self);
        }
        if ($self->is_javascript())
        {
            print "-->";
            print $self->{'cgi'}->end_html(), "\n";
        }
    }
}

# Returns a status event you can later send to this topic

sub status_event
{
    my ($self) = @_;
    return new PubSub::Status($self->{'cgi'}->param('kn_status_from'));
}

sub verify_aliveness
{
    my ($self) = @_;
    if (not ($self->is_javascript() || $self->is_simple()))
    {
        die "Can't verify aliveness of a topic!";
    }
    return print " ";
}

1;

# End of Topic.pm
