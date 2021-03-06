#!/bin/bash --

# Copyright 2003-2004 KnowNow, Inc.  All rights reserved.

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

# $Id: test_session.bash,v 1.2 2004/04/19 05:39:15 bsittler Exp $

export date_fmt="${date_fmt:-%Y-%m-%d_%H.%M.%S}"

export env="$1" \
       cfg="$2" \
       session="$(date -u +"$date_fmt")_$RANDOM" \
       pubsub_ping="${pubsub_ping:-../pubsub_ping/bin/pubsub_ping}"

function log_msg
{
    echo "$(date -u +"$date_fmt"): $*" >&2
}

function error_cat
{
    cat "$@" >&2
    exit 2
}

function error_msg
{
    echo ":$*" | sed 's/://' | error_cat
}

if [ $# != 2 ]
then
    error_cat <<EOF
Summary: $0 ENVFILE INIFILE

example ENVFILE is testenv.bash;
example INIFILE is config.ini
EOF
    exit 2
fi

if ! [ -f "$env" -a -r "$env" ]
then
    error_msg "$env: not a readable file"
    exit 1
fi

if ! [ -f "$cfg" -a -r "$cfg" ]
then
    error_msg "$cfg: not a readable file"
    exit 1
fi

if ! [ -f "$pubsub_ping" -a -x "$pubsub_ping" ]
then
    error_msg "$pubsub_ping: not a readable file"
    exit 1
fi

if ! "$pubsub_ping" --help > /dev/null
then
    error_msg "$pubsub_ping: does not work (tried --help parameter)"
    exit 1
fi

export pubsub_server="$(
 (
    source "$env"
    echo "$pubsub_server"
 ) 2>/dev/null |
 tail -1
)"

if [ :"$pubsub_server" == :"" ]
then
    error_msg "$env: cannot determine pubsub server URI; $""pubsub_server was not set"
    exit 1
fi

log_msg "session $session started"

trap '' INT

cp "$env" $session-env || exit $?

"$pubsub_ping" "$pubsub_server" > latency-$session 2>&1 & ping=$!

(
    trap INT
    log_msg "starting first test run"
    cp "$cfg" $session-first.ini || exit $?
    source $session-env
    PATH=.:"$PATH" bash runall.bash $session-first.ini
    log_msg "finished first test run"
)

(
    trap INT
    log_msg "waiting 10 minutes to allow event expiration"
    sleep 600
)

(
    trap INT
    log_msg "starting second test run"
    cp "$cfg" $session-second.ini || exit $?
    source $session-env
    PATH=.:"$PATH" bash runall.bash $session-second.ini
    log_msg "finished second test run"
)

kill -INT $ping
mkdir .session-$session
mv *$session* .session-$session
mv .session-$session session-$session
log_msg "session $session completed, results in session-$session"
exit 0
