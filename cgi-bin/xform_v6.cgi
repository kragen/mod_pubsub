#!/usr/bin/perl -w

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
# $Id: xform_v6.cgi,v 1.2 2004/04/19 05:39:09 bsittler Exp $

use strict;
use CGI ':standard';
use HTTP::Date;

my $payload = param('kn_payload');
print header("text/plain");

# Rohit attempts to do basic implicit markup
# 1. anything ending in an @ is a who
# 2. anything following a : is a where
# 3. anything prefixed by _ is a what
# 4. anything prefixed by an @ is a when

# eg. "Rohit@ is flying to :SanFrancisco @10:05PM to deal with _Fleetwire."
# "<WHO>Rohit</WHO> is flying to <WHERE>SanFrancisco</WHERE> 
#  <WHEN>2000-07-27 22:05 CDT</WHEN> to deal with <WHAT>Fleetwire</WHAT>."

# recognize each rule case, then attempt to parse the contents. 
# stripping newlines and periods

my $topicnamechars = '\w-/';
for ($payload)
{
    s~\b([$topicnamechars]+)@~<WHO>$1</WHO>~g;
    s~(\s|^):([$topicnamechars]+)\b~$1<WHERE>$2</WHERE>~g;
    s~:(\s+)([$topicnamechars]+)\b~$1<WHERE>$2</WHERE>~g;
    s~\b_([$topicnamechars]+)~<WHAT>$1</WHAT>~g;
    s~(\s|^)@([$topicnamechars:]+)~$1<WHEN>$2</WHEN>~g;
}

print "<EVENT>$payload</EVENT>";

# End of xform_v6.cgi
