#!/usr/bin/perl -w

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
# $Id: replay.plx,v 1.2 2004/04/19 05:39:14 bsittler Exp $

# XXX: note that this script needs to get a little bit smarter about
# reposting routes and subtopics; it needs to not try to repost
# subtopics and use do_method=route to post routes.

use strict;
use PubSub::EventFormat;
use PubSub::UUID;
use LWP::UserAgent;
use HTTP::Request::Common;

sub post_event
{
    my ($event, $url) = @_;
    LWP::UserAgent->new->request(POST $url, [ %$event ]);
}

my $replay_id = uuid;
my %idmap;

sub rename_event_id
{
    my ($old_id) = @_;
    return "${replay_id}_$old_id";
}

my $topic = $ARGV[0];
my $warp = $ARGV[1] || 1;
my $usr = $ARGV[2];
my $pw = $ARGV[3];

die "No topic given on command line" if not $topic;

my @events = PubSub::EventFormat::event_names($topic);

exit if not @events;

my @event_objects = sort { $a->{kn_time_t} <=> $b->{kn_time_t} } 
    map { PubSub::EventFormat::read_event($topic, $_) } @events;

my $old_epoch = $event_objects[0]->{kn_time_t};
my $current_epoch = time;

while (@event_objects)
{
    # The number of virtual seconds that have passed
    my $present_moment = (time - $current_epoch) * $warp;

    my $event = $event_objects[0];

    if (($event->{kn_time_t} - $old_epoch) <= $present_moment)
    {
        $event->{kn_id} = rename_event_id($event->{kn_id});

        my $where = $event->{kn_route_location};
        if ($where =~ s|kn_routes/[^/]+$||)
        {
            if ($usr && $pw) 
            { 
                $where =~ s/(.*\/\/)(.*)/$1$usr:$pw\@$2/ ; 
            }
            print "posting event $event->{kn_id}: $event->{kn_payload} to $where at $present_moment\n";
            post_event($event, $where);
        }
        else
        {
            warn "don't understand this kn_route_location: $where";
        }
        shift @event_objects;
    }
    else
    {
        # Expected time minus current time; compressed by $warp
        sleep ((($event->{kn_time_t} - $old_epoch) - $present_moment)/$warp);
    }
}

# End of replay.plx
