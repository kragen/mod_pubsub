#!/usr/bin/perl -w

# Copyright 2000-2003 KnowNow, Inc.  All Rights Reserved.

# @KNOWNOW_LICENSE_START@

# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:

# 1. Redistributions of source code must retain the above copyright
# notice, this list of conditions and the following disclaimer.

# 2. Redistributions in binary form must reproduce the above copyright
# notice, this list of conditions and the following disclaimer in
# the documentation and/or other materials provided with the
# distribution.

# 3. The name "KnowNow" is a trademark of KnowNow, Inc. and may not
# be used to endorse or promote any product without prior written
# permission from KnowNow, Inc.

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

# @KNOWNOW_LICENSE_END@

# $Id: list_events.plx,v 1.1 2003/02/22 03:12:43 ifindkarma Exp $

# list_events.plx is a command line utility.

# Input: a router uri and a topic name.
# Output: the events at that topic.

# NOTE: you must include a trailing '/' at the end of the router uri.
# See this example usage:

#   $ perl list_events.plx http://127.0.0.1:8000/kn/ /what/chat
#   391f7a28-2ff7-11d7-8dd2-6b61626f6f6d [Fri Jan 24 15:54:49 2003]
#   392100dc-2ff7-11d7-8dd2-6b61626f6f6d [Fri Jan 24 15:54:49 2003]
#   3a52ca12-2ff7-11d7-8dd2-6b61626f6f6d [Fri Jan 24 15:54:51 2003]

use strict;
use lib '../cgi-bin';
use PubSub::Client;

my $serv = new PubSub::Client(shift @ARGV);
my $me = (getpwuid($<))[0];
$serv->displayname($me);
my $topic = (shift @ARGV || '/');
my $requestid = $serv->{tunneluri} . '/kn_status/' . PubSub::UUID::uuid();
$serv->{dispatch}->{$requestid} = sub {
    delete $serv->{dispatch}->{$requestid};
    exit 0;
};
$serv->handle_events(); # jump-start event processing so we don't miss any...
$serv->subscribe($topic, sub {
                     my ($event) = @_;
                     my $kn_id = $event->{kn_id};
                     my $time = localtime($event->{kn_time_t});
                     print "$kn_id [$time]\n";
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

# End of list_events.plx
