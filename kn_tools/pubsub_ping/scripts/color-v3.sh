#!/bin/bash --

# color-v3.sh - color -verbose=3 stderr output from pubsub_ping
# $Id: color-v3.sh,v 1.1 2003/03/22 06:59:18 ifindkarma Exp $

# When pubsub_ping is run with the -verbose=3 option, it sends output to
# stderr which can be analyzed to track individual pubsub_ping
# events. However, this output is somewhat verbose, so this filter
# script has been provided to extract the most useful information and
# color-code it. It's still not a GUI, but it is interesting to watch.

# An example invocation:
# [pubsub_ping]$ ./bin/pubsub_ping 

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

# $Id: color-v3.sh,v 1.1 2003/03/22 06:59:18 ifindkarma Exp $

default="$(tput setaf 9)"

cat -- "$@" |
    tr '\0' ' ' |
    sed '

        s/[013578]/'"$(tput rev)&$(tput sgr0)"'/g;
        s/\(onPing\)[^A-Za-z].*kn_id. = .\(.*\). }/'"$(tput setaf 1;tput setab 7)\1$(tput setaf 9;tput setab 9)"' \2/g;
        s/\(onPingSuccess\)[^A-Za-z].*/'"$(tput setaf 4;tput setab 7)\1$(tput setaf 9;tput setab 9)"'/g;
        s/\(doPing\)[^A-Za-z].*kn_id. = .\(.*\). }/'"$(tput setaf 2;tput setab 7)\1$(tput setaf 9;tput setab 9)"' \2/g;

        '

# $Log: color-v3.sh,v $
# Revision 1.1  2003/03/22 06:59:18  ifindkarma
# Added command line ping utility that uses mod_pubsub/c_pubsub/libkn/ .
#

