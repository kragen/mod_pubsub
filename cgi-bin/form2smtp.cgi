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
# $Id: form2smtp.cgi,v 1.1 2002/11/07 07:07:57 troutgirl Exp $

# Input: HTTP POST request (PATH_INFO, kn_payload).
# Output: Sends an email via sendmail.

use strict;
use CGI ':standard';

# Subroutine to send mail

sub sendmail {

  # Input: Mail message and people to send it to.
  # Output: Nothing.
  # Behavior: Sends a mail message.

  my ($message, $person) = @_;  # Make another routine for @people ?

  # Pipe off sendmail doesn't work on non-UNIX systems.

  open MAILPIPE, "|/usr/lib/sendmail -oem -t" or die "Couldn't fork; $!";

  # Generate headers.

  unless (print MAILPIPE "To: $person\n") {

    print STDERR ("Error producing mail message.\n");

  }

  print MAILPIPE "\n";

  # Send mail and close.

  print MAILPIPE $message;
  close MAILPIPE;
}


# Main routine.

if (request_method eq "POST") {

  my $mypath = path_info;
  my $payload = param("kn_payload");

  $mypath =~ s|^/||;
  sendmail ($payload, $mypath);

  print header, start_html("delivered message to $mypath"), end_html;

} else {

  print header, start_html("sms only works with a POST request"), end_html;

}

# End of sendmail.cgi
