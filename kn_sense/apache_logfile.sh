#!/bin/sh -e 

## 
#
# Apache log file -> PubSub event generator 
# By Wilfredo Sanchez
# 
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
# $Id: apache_logfile.sh,v 1.3 2004/04/19 05:39:14 bsittler Exp $
#
# This sensor monitors an Apache log file (common or combined) and
# generates events as log entries are written to it. 
# 
# A simple way to monitor the events: 
#  apache_logfile http://myrouter/kn 
#  kn_subscribe -f http://myrouter/kn/what/apps/weblog/`hostname`/access 
#
# This script doesn't yet handle error logs.
## 

##
# Defaults
##

PATH=${HOME}/bin:/bin:/usr/bin:/usr/local/bin:/usr/local/apache/bin;

# Find httpd
httpd=`apxs -q TARGET`;

# Get httpd settings
eval `${httpd} -V | grep '="' | sed -e 's/ -D //'`;

if echo "${DEFAULT_XFERLOG}" | grep '^/'; then
    access_log="${DEFAULT_XFERLOG}";
else
    access_log="${HTTPD_ROOT}/${DEFAULT_XFERLOG}"
fi;

if echo "${DEFAULT_ERRORLOG}" | grep '^/'; then
    error_log="${DEFAULT_ERRORLOG}";
else
    error_log="${HTTPD_ROOT}/${DEFAULT_ERRORLOG}";
fi;

##
# Command line
##

  kn_uri="";
kn_topic="/what/apps/weblog/`hostname`/access";
   debug="";

usage ()
{
    echo "Usage: `basename ${0}` uri [topic]";
    echo "      uri:   PubSub Server URI";
    echo "      topic: Topic on server to publish to [${kn_topic}]";
    exit 1;
}

if args=`getopt d $*`; then :; else usage; fi;
set -- ${args};
for option in "$@"; do
    case "${option}" in
      -d)
        debug="-d";
        shift; break;
        ;;
      --)
        shift; break;
        ;;
    esac;
done;

if [ "${1}" = "--" ]; then shift; fi;

if [ $# -gt 0 ]; then   kn_uri="${1}"; shift; else usage; fi;
if [ $# -gt 0 ]; then kn_topic="${1}"; shift; fi;

##
# Routines
##

access_logger ()
{
    tail -1f "${access_log}" | (
        while true; do
            # Parse input

            read line;
            IFS=' '; set - ${line}l
    
             client_host="${1}"           ; if [ $# -gt 0 ]; then shift 1; fi
                     huh="${1}"           ; if [ $# -gt 0 ]; then shift 1; fi
             client_user="${1}"           ; if [ $# -gt 0 ]; then shift 1; fi
                    date="${1} ${2}"      ; if [ $# -gt 0 ]; then shift 2; fi
                 request="${1} ${2} ${3}" ; if [ $# -gt 0 ]; then shift 3; fi
                  status="${1}"           ; if [ $# -gt 0 ]; then shift 1; fi
                   bytes="${1}"           ; if [ $# -gt 0 ]; then shift 1; fi
                 referer="${1}"           ; if [ $# -gt 0 ]; then shift 1; fi
            client_agent="${1}"           ; if [ $# -gt 0 ]; then shift 1; fi

            # Get rid of quotes

            IFS='"';
            set - ${request} ; request="${1}";
            set - ${referer} ; referer="${1}";

            # Parse request

            IFS=' ';
            set - ${request};

            if [ "${request}" = "-" ]; then
                  method="";
                     uri="";
                protocol="";
            else
                  method="${1}" ; if [ $# -gt 0 ]; then shift 1; fi;
                     uri="${1}" ; if [ $# -gt 0 ]; then shift 1; fi;
                protocol="${1}" ; if [ $# -gt 0 ]; then shift 1; fi;
            fi;

            echo "${request}" | kn_publish -P ${debug}  \
                -H "host"         "${client_host}"      \
                -H "huh"          "${huh}"              \
                -H "user"         "${client_user}"      \
                -H "date"         "${date}"             \
                -H "request"      "${request}"          \
                -H "method"       "${method}"           \
                -H "uri"          "${uri}"              \
                -H "protocol"     "${protocol}"         \
                -H "status"       "${status}"           \
                -H "bytes"        "${bytes}"            \
                -H "referer"      "${referer}"          \
                -H "agent"        "${client_agent}"     \
                -- "${kn_uri}" "${kn_topic}" -;
        done;
    )
}

##
# Do The Right Thing
##

access_logger;
