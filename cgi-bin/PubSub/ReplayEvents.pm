package PubSub::ReplayEvents;

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
# $Id: ReplayEvents.pm,v 1.3 2004/04/19 05:39:09 bsittler Exp $

use strict;
use Exporter;
use base 'Exporter';

@PubSub::ReplayEvents::EXPORT_OK = qw(begin_replaying_events);

use POSIX '_exit';
use PubSub::Error;

sub begin_replaying_events 
{ 
    my ($topic, $warpfactor, $usr, $pw, $mod_pubsub_dir) = @_;
    # spawn external script
    my $pid = fork;
    if (not defined $pid)
    {
        htdiepage "Couldn't fork: $!";
    }
    elsif (not $pid)        # child
    {
        # FIXME: this means we're throwing away error logs
        open STDOUT, ">/dev/null" or htdiepage "couldn't open stdout: $!";
        open STDIN, "</dev/null" or htdiepage "couldn't open stdin: $!";
        open STDERR, ">/dev/null" or htdiepage "couldn't open stderr: $!";
        exec("perl", "-I.",
             $mod_pubsub_dir . "../kn_sense/replay.plx",
             $topic, $warpfactor, $usr, $pw)
            or htdiepage "couldn't exec replay: $!";
        POSIX::_exit(1);
    }
    else # parent
    {
        return;
    }
}

1;

# End of ReplayEvents.pm
