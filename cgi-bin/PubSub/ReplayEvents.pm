package PubSub::ReplayEvents;

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
# $Id: ReplayEvents.pm,v 1.1 2002/11/07 07:08:03 troutgirl Exp $

use strict;
use Exporter;
use base 'Exporter';

@PubSub::ReplayEvents::EXPORT_OK = qw(begin_replaying_events);

use POSIX '_exit';
use PubSub::Error;

sub begin_replaying_events 
{ 
    my ($topic, $warpfactor, $usr, $pw) = @_;
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
        exec "perl", "-I.", "../kn_sense/replay.plx", $topic, $warpfactor, $usr, $pw 
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
