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

# $Id: chat.plx,v 1.2 2003/02/22 03:12:43 ifindkarma Exp $

# chat.plx tests the functionality of PubSub::Client.
# Functionality: It's a command-line chat client.

use strict;
use lib '../cgi-bin';
use PubSub::Client;

my $sgr0 = `tput sgr0`;
my $bold = `tput bold`;
my $red = `tput setaf 1`;
my $cyan = `tput setaf 6`;
my $serv = new PubSub::Client(shift @ARGV);
my $me = (getpwuid($<))[0];
$serv->displayname($me);
$serv->subscribe("/what/chat", sub {
                     my ($event) = @_;
                     my $displayname = $event->{displayname} || 'unknown';
                     my $payload = $event->{kn_payload};
                     my $time = localtime($event->{kn_time_t});
                     if ($displayname eq $me)
                     {
                         print "$red";
                     }
                     print "$bold$displayname$sgr0: $payload $cyan"."[$time]$sgr0\n";
                 }, { do_max_age => '600' });


my $rin = '';

for (;;) {
    $rin = '';
    vec($rin, fileno STDIN, 1) = 1;
    my @servnos = $serv->filenos();
    for my $fileno (@servnos) 
    {
        vec($rin, $fileno, 1) = 1;
    }
    select($rin, undef, undef, undef);
    if (vec($rin, fileno STDIN, 1)) 
    {
        my $msg = <STDIN>;  # assume a line or EOF is there; hang if not
        exit if not defined $msg;
        chomp $msg;
        $serv->publish("/what/chat", {kn_payload => $msg});
    }
    $serv->handle_events(); # always handle events because looking at $rin is too hard
}

# End of chat.plx
