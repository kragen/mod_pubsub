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
# $Id: make_filters.plx,v 1.2 2004/04/19 05:39:14 bsittler Exp $

# This program creates routes, with kn_content_filters on them, from a
# single clearinghouse topic into a variety of per-subject topics.

# Example of use:
#   make_filters.plx rssall_filters http://dogfood.example.com/cgi-bin/pubsug.cgi/what/rss/all

# To run the filter program:
#   make_filters.plx config-file pubsub-server

# Where config-file contains topic specification lines, each of which
# consists of a topic name (which can be relative to pubsub.cgi or
# absolute) and zero or more patterns specifying which events to route
# to that topic.  The various fields of the line are separated by
# whitespace.

# Here is an example config-file:

# /what/tech/linux (?i)linux (?i)red.?hat (?i)slashdot (?i)andover\.net (?i)torvalds (?i)alan.cox RHAT LNUX SUSE S.U.S.E. (?i)debian GNU
# /what/nasdaq/akam (?i)akamai AKAM (?i)tom.*leighton (?i)dan.*lewin (?i)conrades

# This specifies that events whose payloads contain case-insensitive
# 'linux' or case-insensitive 'slashdot', among other things, get
# routed to the topic /what/tech/linux , and events whose payloads
# contain case-sensitive 'AKAM' or case-insensitive 'akamai' -- or
# 'tom', followed by any amount of text, followed by 'leighton' on the
# same line, all case-insensitive -- should be routed to the topic
# /what/nasdaq/akam

# Each keyword gets turned into a separate route request to the
# server, which accordingly gets turned into a separate route in the
# event pool.

# Whitelines and lines that begin with # are ignored.

use strict;
use LWP::UserAgent;
use HTTP::Request::Common;

my ($filename, $url) = @ARGV;
my $ua = new LWP::UserAgent;

open INFILE, "< $filename" or die "Can't open $filename: $!";

while (<INFILE>) 
{
    next if /^\s*$/ or /^\#/;
    my ($topic, @keywords) = split;
    for (@keywords)
    {
        my $result = $ua->request(POST $url, [do_method => 'route', 
                                              kn_to => $topic, 
                                              kn_content_filter => $_]);
        warn "got " . $result->as_string unless $result->code =~ /^2/;
    }
}

# End of make_filters.plx
