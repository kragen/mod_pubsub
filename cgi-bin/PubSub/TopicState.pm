# package PubSub::TopicState;

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
# $Id: TopicState.pm,v 1.2 2004/04/19 05:39:09 bsittler Exp $

package PubSub::TopicState::Journal;
use base 'PubSub::TopicState';
use strict;

sub new
{
    my ($class, $topic) = @_;
    return bless { 'tunnel' => PubSub::TopicState::open_tunnel($topic), 
                   'topic' => $topic }, $class;
}

sub new_events
{
    my ($self) = @_;
    # Heuristic so that events posted before we enter this routine
    # will be more likely to show up when we call get_events().
    select undef, undef, undef, .5;
    my @events = $self->{'tunnel'}->get_events();
    return map { new PubSub::Event(%$_) } @events;
}

package PubSub::TopicState::Ordinary;
use PubSub::EventFormat 'build_path';
use PubSub::UUID 'uuid';
use base 'PubSub::TopicState';
use strict;

# We keep a hash of events that currently exist on the topic; when
# we're asked for new events, we get all the events from the topic
# (using a nonce tunnel) and return the ones that weren't there before.

sub new
{
    my ($class, $topic) = @_;
    my $self = bless { 'topic' => $topic, 'events' => {} }, $class;
    $self->new_events();  # initialize $self->{'events'}
    return $self;
}

# Return events in @$b that aren't in %$a (indexed by kn_id).

sub _event_list_difference
{
    my ($a, $b) = @_;
    my @rv = ();
    event: for my $event (@$b)
    {
        my $id = $event->{'kn_id'};
        die "Event without id: @{[keys %$event]}" if not defined $id;
        if (not exists $a->{$id})
        {
            push @rv, $event;
            next event;
        }
        else
        {
            for my $key (keys (%$event), keys (%{$a->{$id}}))
            {
                # XXX: This is the same as the list in pubsub.cgi's 
                # is_duplicate().  Having to keep both lists in sync is
                # guaranteed to introduce bugs in the future.
                next if (($key =~ m|\Akn_route_|) or
                         $key eq 'kn_routed_from');
                if (not defined $a->{$id}->{$key} or 
                    not defined $event->{$key} or
                    $a->{$id}->{$key} ne $event->{$key})
                {
                    push @rv, $event;
                    next event;
                }
            }
        }
    }
    return @rv;
}

sub new_events
{
    my ($self) = @_;
    my $tunnel_topic = PubSub::TopicState::get_journal();
    my $status_from = build_path($tunnel_topic, uuid());
    my $tunnel = PubSub::TopicState::open_tunnel($tunnel_topic);
    PubSub::TopicState::create_route($self->topic(), $tunnel_topic, 
                                 'do_max_age' => 'infinity',
                                 'kn_status_to' => $tunnel_topic,
                                 'kn_status_from' => $status_from);
    my @requested_events = ();
    initial_route_population: while (1)
    {
        my @events = $tunnel->get_events();
        for my $event (@events)
        {
            if (exists $event->{'kn_route_location'} and 
                $event->{'kn_route_location'} eq $status_from)
            {
                last initial_route_population;
            }
            else
            {
                push @requested_events, $event;
            }
        }
        select undef, undef, undef, 0.05;
    }

    $tunnel->close();

    my @new_events = map { new PubSub::Event(%$_) } 
        _event_list_difference($self->{'events'}, \@requested_events);
    $self->{'events'} = { map { $_->{'kn_id'} => $_ } @requested_events };
    return @new_events;
}

package PubSub::TopicState;

use PubSub::EventFormat qw(is_journal build_path);
use PubSub::Tunnel;
use LWP::UserAgent;
use PubSub::UUID 'uuid';
use HTTP::Request::Common 'POST';
use strict;
use vars '$url';

sub kn_url_is 
{
    ($url) = @_;
}

sub get_journal
{
    return build_path($url, "what/apps/topicstate", uuid(), "kn_journal");
}

sub discard_status_event
{
    my ($tunnel) = @_;
    while (1)
    {
        my @events = $tunnel->get_events();
        if (@events > 1)
        {
            die "Just-opened tunnel got two events\n";
        }
        elsif (@events == 1)
        {
            last;
        }
        else
        {
            # Wait a twentieth of a second.
            select undef, undef, undef, .05;
        }
    }
}

sub open_tunnel
{
    my ($topic) = @_;
    my $tunnel = new PubSub::Tunnel($topic);
    die "Failed to init tunnel" if not defined $tunnel;
    discard_status_event($tunnel);
    return $tunnel;
}

sub create_route
{
    my ($from, $to, @other) = @_;
    my $result = LWP::UserAgent->new()->request(
                                                POST $from,
                                                [ do_method => 'route',
                                                  kn_to => $to,
                                                  @other ]
                                               );
    if ($result->code() !~ /^[123]/)
    {
        die "Failed to create route: " . $result->as_string();
    }
}

sub new 
{
    my ($class, $topic) = @_;
    if ($topic !~ /\A[a-zA-Z][-a-zA-Z0-9+.]*:/)  # RFC 2396 section 3.1
    {
        $topic = build_path($url, $topic);
    }
    if (is_journal($topic))
    {
        return PubSub::TopicState::Journal->new($topic);
    }
    else
    {
        return PubSub::TopicState::Ordinary->new($topic);
    }
}

sub topic
{
    return $_[0]->{'topic'};
}

1;

# End of TopicState.pm
