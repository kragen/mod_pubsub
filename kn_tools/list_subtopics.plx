#!/usr/bin/perl -w

# Copyright 2000-2004 KnowNow, Inc.  All rights reserved.

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

# $Id: list_subtopics.plx,v 1.3 2004/04/19 05:39:14 bsittler Exp $

# list_subtopics.plx is a command line utility.

# Input: a router uri and a topic name.
# Output: the subtopics of that topic.

#   $ perl list_subtopics.plx http://127.0.0.1:8000/kn /what
#   chat [Fri Jan 24 15:53:30 2003]

use strict;
use lib '../cgi-bin';
use PubSub::Client;

my $serv = new PubSub::Client(shift @ARGV);
my $me = (getpwuid($<))[0];
$serv->displayname($me);
my $topic = (shift @ARGV || '').'/kn_subtopics';
my $requestid = $serv->{tunneluri} . '/kn_status/' . PubSub::UUID::uuid();
$serv->{dispatch}->{$requestid} = sub {
    delete $serv->{dispatch}->{$requestid};
    exit 0;
};
$serv->handle_events(); # jump-start event processing so we don't miss any...
$serv->subscribe($topic, sub {
                     my ($event) = @_;
                     my $kn_payload = $event->{kn_payload};
                     my $time = localtime($event->{kn_time_t});
                     print "$kn_payload [$time]\n";
                 }, {
                     do_max_age => 'infinity',
                     kn_status_from => $requestid,
                     kn_status_to => "$serv->{tunneluri}"
                  });

my $rin = '';

for (;;) {
    $rin = '';
    my @servnos = $serv->filenos();
    for my $fileno (@servnos) 
    {
        vec($rin, $fileno, 1) = 1;
    }
    select($rin, undef, undef, undef);
    $serv->handle_events(); # always handle events because looking at $rin is too hard
}

# End of list_subtopics.plx
