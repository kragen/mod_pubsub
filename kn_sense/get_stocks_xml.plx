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
# $Id: get_stocks_xml.plx,v 1.2 2004/04/19 05:39:14 bsittler Exp $

# ----------------------------------------------------------------------

# This script pulls stock quotes in Excel format from Yahoo Financial
# Services, and converts them to XML.

# Example of using this script:
#   get_stocks_xml.plx MSFT INTC CSCO ORCL DELL SUNW WCOM QCOM JDSU YHOO

# Sample URL for getting a quote:
#   http://finance.yahoo.com/d/quotes.csv?s=AOL+TWX&f=sl1d1t1c1ohgv&e=.csv

# FIXME: This program used to work in 2000, now seems a little shaky.
# You'll see by the warnings you get when you run it.

# ----------------------------------------------------------------------

use strict;
# use LWP::Simple;
use LWP::UserAgent;
use HTTP::Request::Common;
use IO::Handle;

my $yahooURL = "http://finance.yahoo.com/d/quotes.csv?f=sl1d1t1c1ohgv&e=.csv";
my $symbols  = join("+", @ARGV);
my $ua       = new LWP::UserAgent;
my $result   = $ua->request(GET "$yahooURL&s=$symbols");
my $stocks   = $result->as_string;
print $stocks;
my $line;

# Convert a stock quote from comma separated values to XML.

sub csvtoxml {
    my ($csv) = @_;
    my @values = split(/,/ , $csv);
    my @fields = qw (symbol ask qdate qtime change open dayhigh daylow volume);
    my $count = 0;
    my $field;
    my %val;

    foreach $field (@fields)
    {
        $values[$count] =~ s/"//g;
        $val{$field} = $values[$count];
        $count += 1;
    }

    print("<stock_quote>\n");
    print(" <symbol>$val{'symbol'}</symbol>\n");
    print(" <when>\n");
    print("   <date>$val{'qdate'}</date>\n");
    print("   <time>$val{'qtime'}</time>\n");
    print(" </when>\n");
    print(" <price type=\"ask\" value=\"$val{'ask'}\"/>\n");
    print(" <price type=\"open\"    value=\"$val{'open'}\"/>\n");
    print(" <price type=\"dayhigh\" value=\"$val{'dayhigh'}\"/>\n");
    print(" <price type=\"daylow\"  value=\"$val{'daylow'}\"/>\n");
    print(" <change>$val{'change'}</change>\n");
    print(" <volume>$val{'volume'}</volume>\n");
    print("</stock_quote>\n");
}

# XMLify.

print("<stock_quotes>\n\n");

foreach $line (split("\n", $stocks)) {
    next if ($line eq "") or ($line eq "HTTP/1.0 200 OK");
    csvtoxml $line;
    print "\n";
}

print("\n</stock_quotes>\n");

# End of get_stocks_xml.plx
