#!/usr/bin/perl -w

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
# $Id: post_status.plx,v 1.1 2002/11/07 07:09:27 troutgirl Exp $

# To call: post_event.plx status [username]

# Put this in /usr/local/bin -- makes use of a .procmailrc that looks like:

# MAILDIR=/home/adam/Mail
# DEFAULT=$MAILDIR/Mailbox
# LOGFILE=$MAILDIR/from
# 
# :0c
# |/usr/local/bin/post_status.plx "$1" 'adam'

use strict;
use LWP::UserAgent;
use HTTP::Request::Common 'POST';

# Parse an incoming mail message and post it to somebody's status

exit unless $ARGV[0] eq 'status';

# Skip header

while (<STDIN>) 
{
	last if /^$/;
}

$_ = <STDIN>;
my ($code, $message) = split /\s+/, $_, 2;
my $user = $ARGV[1];

my $ua = LWP::UserAgent->new;
my $result = 
	$ua->request(
		POST 
		# "http://kncgi:kncgi\@experimental.knownow.com/~kragen/kn/cgi-bin/kn.cgi/who/$user/status", 
		"http://kncgi:kncgi\@www.knownow.net/kn/cgi-bin/kn.cgi/who/$user/status", 
		[userid => "/who/$user", status => $code, 
		 kn_payload => $message]);

# Running from procmail, no way to report failure and bounce mail

# open OUTPUT, ">>/home/kragen/statuslog";
# print OUTPUT $result->as_string;

# End of post_status.plx
