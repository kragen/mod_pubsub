package PubSub::UUID;

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
# $Id: UUID.pm,v 1.1 2002/11/07 07:08:00 troutgirl Exp $

# NAME
# PubSub::UUID - Module for creating Universally Unique IDs.

# SYNOPSIS
# use PubSub::UUID;
# my $id = uuid();

# DESCRIPTION
# This module generates unique IDs.

# FIXME: The IDs aren't very U, let alone UU.

use strict;
use Exporter;
use base 'Exporter';

@PubSub::UUID::EXPORT = 'uuid';

my $seqno = 0;

# Ideally, this returns a universally-unique identifier; practically,
# fast PID recycling or bad luck with multiple machines can destroy
# uniqueness.

sub uuid
{
    $seqno++;
    return join '_', time, $$, $seqno;
}

1;

# End of UUID.pm
