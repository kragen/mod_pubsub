#!/bin/sh -e 

## 
#
# Apache log file -> PubSub event generator 
# By Wilfredo Sanchez
# 
# Copyright 2000-2003 KnowNow, Inc.  All Rights Reserved.
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
# $Id: apache_logfile.sh,v 1.1 2003/03/05 23:47:35 ifindkarma Exp $
#
#
# 
# This sensor monitors an Apache log file (common or combined) and
# generates events as log entries are written to it. 
# 
# A simple way to monitor the events: 
#  weblog http://myrouter/kn 
#  kn_subscribe -f http://myrouter/kn/what/apps/weblog/`hostname`/access 
#
#
#
# ----- Original Message:
# From: Fred Sanchez
# To: Adam Rifkin
# Sent: 5/11/01 11:56 PM
# Subject: I'm da man
# 
# OK, this is freaking cool.  Log into any machine with
# the libkn library (in c_pubsub) installed and run this: 
#
#     kn_subscribe -f http://pong:8000/kn /what/apps/weblog/pong
#
# Then go to the Apache web server on pong (http://pong/)
# and watch the spew come out from the subscribe command above. 
# 
# I just finished a sensor written in... wait for it... the Bourne
# shell.  It tails the Apache log file, parses it, and sends it via
# the kn_publish command which is installed by libkn. 
# 
# Add -S after -f to the subscribe command above to see the full
# events in simple format, which includes all information about
# the request (user, user agent, referer, status, etc.). 
# 
#	-Fred 
## 

## 
# Defaults 
## 

PATH=${HOME}/bin:/bin:/usr/bin:/usr/local/bin:/usr/local/apache/bin 

# Find httpd 
httpd=`apxs -q TARGET` 

# Get httpd settings 
eval `${httpd} -V | grep '="' | sed -e 's/ -D //'` 

if echo "${DEFAULT_XFERLOG}" | grep '^/'; then 
    access_log="${DEFAULT_XFERLOG}" 
else 
    access_log="${HTTPD_ROOT}/${DEFAULT_XFERLOG}" 
fi 

if echo "${DEFAULT_ERRORLOG}" | grep '^/'; then 
    error_log="${DEFAULT_ERRORLOG}" 
else 
    error_log="${HTTPD_ROOT}/${DEFAULT_ERRORLOG}" 
fi 

## 
# Command line 
## 

  kn_uri="" 
kn_topic="/what/apps/weblog/`hostname`/access" 
   debug="" 

usage () 
{ 
    echo "Usage: `basename ${0}` uri [topic]" 
    echo "      uri:   PubSub Server URI" 
    echo "      topic: Topic on server to publish to [${kn_topic}]" 
    exit 1 
} 

if args=`getopt d $*`; then :; else usage; fi 
set -- ${args} 
for option in "$@"; do 
  case "${option}" in 
    -d) 
        debug="-d" 
        shift; break 
        ;; 
    --) 
        shift; break 
        ;; 
  esac 
done 

if [ "${1}" = "--" ]; then shift; fi 

if [ $# -gt 0 ]; then kn_uri="${1}"; shift; else usage; fi 
if [ $# -gt 0 ]; then kn_topic="${1}"; shift; fi 

## 
# Routines 
## 

access_logger () 
{ 
    tail -1f "${access_log}" | ( 
        while true; do 
            # Parse input 
     
            read line 
            IFS=' '; set - ${line} 
     
             client_host="${1}"           ; if [ $# -gt 0 ]; then shift
1; fi 
                     huh="${1}"           ; if [ $# -gt 0 ]; then shift
1; fi 
             client_user="${1}"           ; if [ $# -gt 0 ]; then shift
1; fi 
                    date="${1} ${2}"      ; if [ $# -gt 0 ]; then shift
2; fi 
                 request="${1} ${2} ${3}" ; if [ $# -gt 0 ]; then shift
3; fi 
                  status="${1}"           ; if [ $# -gt 0 ]; then shift
1; fi 
                   bytes="${1}"           ; if [ $# -gt 0 ]; then shift
1; fi 
                 referer="${1}"           ; if [ $# -gt 0 ]; then shift
1; fi 
            client_agent="${1}"           ; if [ $# -gt 0 ]; then shift
1; fi 
     
            # Get rid of quotes 
     
            IFS='"' 
            set - ${request} ; request="${1}" 
            set - ${referer} ; referer="${1}" 
     
            # Parse request 
     
            IFS=' ' 
            set - ${request} 
     
            if [ "${request}" = "-" ]; then 
                  method="" 
                     uri="" 
                protocol="" 
            else 
                  method="${1}" ; if [ $# -gt 0 ]; then shift 1; fi 
                     uri="${1}" ; if [ $# -gt 0 ]; then shift 1; fi 
                protocol="${1}" ; if [ $# -gt 0 ]; then shift 1; fi 
            fi 

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
                -- "${kn_uri}" "${kn_topic}" - 

        done 
    ) 
} 

## 
# Do The Right Thing 
## 

access_logger 

# End of apache_logfile.sh
