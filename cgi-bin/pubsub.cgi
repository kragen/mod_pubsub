#!/usr/bin/perl -w

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
# $Id: pubsub.cgi,v 1.3 2003/03/22 11:05:12 ifindkarma Exp $

# If you see this text in a web browser,
# pubsub.cgi is not working properly and
# no pubsub apps will work. The most
# common reason for this is that the web
# server is misconfigured --- commonly,
# either it doesn't have mod_perl, or it
# doesn't AllowOverride the appropriate
# stuff to use the .htaccess file in the
# cgi-bin dir and also doesn't set up
# .cgi files to be run as CGI scripts,
# and so it sends pubsub.cgi (this file)
# instead of the blank page (or whatever)
# pubsub.cgi is supposed to generate.

use strict;

# Where are the KN libraries installed? (Don't include /KN)
# use lib '.../kn/cgi-bin'

use CGI ();
use PubSub::Server qw(set_cgi_url dispatch_request);
use vars '$q';

# Main program logic starts here

$q = new CGI;
set_cgi_url($q->url());

dispatch_request($q);

# End of pubsub.cgi
