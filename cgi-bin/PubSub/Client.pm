package PubSub::Client;

# Examples of usage of the Perl PubSub Client Library are
#     mod_pubsub/kn_tools/chat.plx
#     mod_pubsub/kn_tools/list_events.plx
#     mod_pubsub/kn_tools/list_routes.plx
#     mod_pubsub/kn_tools/list_subtopics.plx
#     mod_pubsub/kn_sense/forex.plx
#     mod_pubsub/kn_sense/nasdaq.plx
#     mod_pubsub/kn_sense/nymex.plx
#     mod_pubsub/kn_sense/quake.plx

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
# $Id: Client.pm,v 1.4 2004/04/19 05:39:09 bsittler Exp $

# This is a simple Perl client for use in event-driven programs.
# It can publish and subscribe in (more or less) the same way
# the PubSub JavaScript Library does.

# It presently lacks these PubSub JavaScript Library features:
# - Nonblocking sending of requests
# - Status callbacks on success and failure
# - Batched requests

# FIXME: Finish out reconnect to server logic on disconnect.

use PubSub::Tunnel;
use PubSub::UUID;
use URI;
use strict;
use LWP::UserAgent;
use HTTP::Request::Common 'POST';
use vars '$VERSION';

$VERSION = '0.10';

sub new
{
    my ($class, $uri) = @_;
    my $self = bless {}, $class;

    my $serveruri = new URI($uri);
    my $path = $serveruri->path;
    if (not $path =~ m|/$|)
    {
        $serveruri->path($serveruri->path . "/");
    }
    $self->{serveruri} = $serveruri;
    my $user = $serveruri->userinfo();
    if (defined $user)
    {
        $user =~ s/:.*//;  # Remove optional password to get username.
    }
    else
    {
        $user = 'anonymous';
    }
    $self->{userid} = $user;

    my $number = rand;
    $number =~ s/^0\.//;
    my $tunneluri = new_abs URI("who/$user/s/$number/kn_journal", $serveruri);

    $self->{tunneluri} = $tunneluri;
    $self->{tunnel} = new PubSub::Tunnel($tunneluri);
    $self->{dispatch} = {};
    $self->{ua} = new LWP::UserAgent;
    $self->{displayname} = 'Anonymous User';
  
    return $self;
}

sub displayname
{
    my ($self, $name) = @_;
    $self->{displayname} = $name;
}

sub publish
{
    my ($self, $topic, $event) = @_;
    $self->_enqueue_request({%$event, 
                             'kn_to' => $topic, 
                             do_method => 'notify'});
}

sub subscribe
{
    my ($self, $topic, $function, $qualifiers) = @_;
    $qualifiers = {} if not defined $qualifiers;
    my $uuid = PubSub::UUID::uuid();
    if (defined $qualifiers->{'kn_id'})
    {
        $uuid = $qualifiers->{'kn_id'};
    }
    $topic =~ s|^/+||;
    $topic = new_abs URI($topic, $self->{serveruri})->as_string;
    if (defined $qualifiers->{'kn_from'})
    {
        $topic = $qualifiers->{'kn_from'};
    }
    my $subid = "$topic/kn_routes/$uuid";
    $subid =~ s|^/+||;
    $subid = new_abs URI($subid, $self->{serveruri})->as_string;
    if (defined $qualifiers->{'kn_uri'})
    {
        $subid = $qualifiers->{'kn_uri'};
    }
    my $dest = $self->{tunneluri}->as_string;
    if (defined $qualifiers->{'kn_to'})
    {
        $dest = $qualifiers->{'kn_to'};
    }
    my $method = "route";
    if (defined $qualifiers->{'do_method'})
    {
        $method = $qualifiers->{'do_method'};
    }
    $self->{dispatch}->{$subid} = $function;
    $self->_enqueue_request({%$qualifiers, 
                             kn_from => $topic, 
                             kn_to => $dest,
                             kn_uri => $subid, 
                             kn_id => $uuid,
                             do_method => $method});
    return $subid;
}

sub unsubscribe
{
    my ($self, $subscription_id) = @_;
    if ($subscription_id !~ m|(.*)/kn_routes/(.*)|) {
        return;
    }
    $self->_enqueue_request({kn_from => $1,
                             kn_to => '',
                             kn_id => $2,
                             do_method => 'route',
                             kn_expires => '+5'});
    return;
}

sub _enqueue_request
{
    my ($self, $request) = @_;
    if (! $self->{tunnel}->{tunnel_established}) {
        $self->{tunnel}->establish_tunnel();
    }
    if ($self->{tunnel}->{tunnel_established}) {
		my $rv = $self->{ua}->request(POST $self->{serveruri}, 
									  [ userid => $self->{userid},
										displayname => $self->{displayname},
										%$request ]);
    }
}

sub filenos
{
    my ($self) = @_;
    return $self->{tunnel}->fileno();
}

sub handle_events
{
    my ($self) = @_;
    my @events = $self->{tunnel}->get_events();
    for my $event (@events)
    {
        if (exists $event->{kn_route_location} and 
            exists $self->{dispatch}->{$event->{kn_route_location}})
        {
            $self->{dispatch}->{$event->{kn_route_location}}->($event);
        }
    }
    
}

1;

# End of Client.pm
