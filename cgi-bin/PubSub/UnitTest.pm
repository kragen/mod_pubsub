package PubSub::UnitTest;

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
# $Id: UnitTest.pm,v 1.1 2002/11/07 07:08:03 troutgirl Exp $

use strict;
use Exporter;
use base 'Exporter';

@PubSub::UnitTest::EXPORT_OK = qw(test_list test);

sub asciify 
{
    my ($string) = @_;
    $string =~ s/([\x00-\x1f\x80-\xff`'])/sprintf("\\%3.3o", ord($1))/ge;
    return "`$string'";
}

sub asciify_list
{
    return join '', '(', (map { asciify $_ } @_), ')';
}

# Test a routine in list context and die if its results don't match.

sub test_list (&$@)
{
    my ($block, $name, @results) = @_;
    my @real_results = $block->();
    my $resultstring = asciify_list(@results);
    my $real_resultstring = asciify_list(@real_results);
    die "TEST FAILURE: $name: wrong number of results: got " .
    "\n$real_resultstring\n expected \n$resultstring\"n" 
    if @real_results != @results;
    for my $i (0..$#results)
    {
        if ($results[$i] ne $real_results[$i])
        {
            my $expected = asciify $results[$i];
            my $got = asciify $real_results[$i];
            die "TEST FAILURE: $name: result $i differs: expected " .
                "\n$expected\n got \n$got\n; full results " .
                "expected \n$resultstring\n got \n$real_resultstring\n";
        }
    }
}

test_list { ('A', 'B', 'C'); } "test_list test", (qw(A B C));

# Test a routine to see if it is boolean true and die if not.

sub test (&$)
{
    my ($block, $name) = @_;
    die "$name: returned false" if not $block->();
}

1;

# End of UnitTest.pm
