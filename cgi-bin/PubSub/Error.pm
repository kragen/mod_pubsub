package PubSub::Error;

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
# $Id: Error.pm,v 1.1 2002/11/07 07:08:03 troutgirl Exp $

use strict;
use Exporter;
use base 'Exporter';
use vars '@EXPORT';

@EXPORT = qw(htdie htdiepage does_htdie);

my $expecting_htdeath = 0;

# Can only be called once because it exits

sub htdie {
  my ($str) = @_;
  my ($package, $filename, $line) = caller;
  print "<p> error: $str at $filename:$line\n</body></html>\n" 
      unless $expecting_htdeath;
  die "$str at $filename line $line\n";
}

# FIXME: you need to know the global state of the program to know
# whether you should call this or htdie; all these means of output (print,
# htdie, htdiepage, Logger) should be integrated into a single module.

# Generate CGI headers and an HTML prologue, then htdie.
# Expects a message string

sub htdiepage
{
    my ($msg) = @_;
    print <<eom unless $expecting_htdeath;
Status: 500 Error: $msg
Content-type: text/html; charset=utf-8

<!DOCTYPE HTML PUBLIC "-//IETF//DTD HTML//EN">
<html><head>
<title>Error: $msg</title></head>
<body>
eom
    # fool htdie into thinking our caller called htdie
    # so it won't say it died somewhere in htdiepage
    @_ = ($msg);
    goto &htdie;
}

sub does_htdie (&)
{
    my ($block) = @_;

    $expecting_htdeath = 1;
    eval { $block->() };
    $expecting_htdeath = 0;

    # return true if it died, false otherwise.
    return $@;
}

1;

# End of Error.pm
