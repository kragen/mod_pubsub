#!/usr/bin/perl

# logwatcher.pl -- example usage of PubSubService.cgi
# Does a SOAP Publish.

# Copyright 2000-2004 KnowNow, Inc.  All rights reserved.

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

# $Id: publish.pl,v 1.2 2004/04/19 05:39:09 bsittler Exp $

# static int usage ()
#     {
#           fprintf(stderr,
#                             "usage: kn_publish [-PS] [-D name] [-H name value] [-p proxyHost proxyPort] uri topic file
# \n"
#                             "       -P: input is payload data (default)\n"
#                             "       -S: input is a pre-formatted event in \"simple\" format\n"
#                             "       -D: delete header from the published event (useful with -S)\n"
#                             "       -H: add header name and value the published event\n"
#                         "       -p: proxyHost and proxyPort \n"
#                             );
#             exit(EX_USAGE);
#           }

use vars qw($SERVER $SERVICE $topic);

$SERVER = "http://127.0.0.1:8000/kn";

if (defined $ENV{KN_SERVER})
{
    $SERVER = $ENV{KN_SERVER};
}

$SERVICE = "$SERVER?do_method=wsdl";

if (defined $ENV{KN_SERVICE})
{
    $SERVICE = $ENV{KN_SERVICE};
}

if ($#ARGV >= 0)
{
    $SERVICE = shift @ARGV;
}

$topic = "/what/chat";

if ($#ARGV >= 0)
{
    $topic = shift @ARGV;
}

$userid = getlogin || getpwuid($<) || "soapuser";

my ($name,$passwd,$uid,$gid,
    $quota,$comment,$gcos,$dir,$shell,$expire) = getpwnam($userid);

my $event = {
    "userid" => $userid,
    "displayname" => $gcos || "SOAP User",
    "kn_payload" => ""
};

if ($#ARGV < 0)
{
    unshift @ARGV, "-";
}

while ($#ARGV >= 0)
{
    my $filename = shift @ARGV;
    if ($filename ne "-")
    {
        open DATAFILE, "<", $filename || die "$0: $filename: $!";
        binmode DATAFILE;
        my $oldfh = select(DATAFILE); undef $/; select($oldfh);
        my $slurp = <DATAFILE>;
        if (not defined $slurp)
        {
            die "$0: $filename: $!";
        }
        close DATAFILE;
        $event->{"kn_payload"} .= $slurp;
    }
    else
    {
        binmode STDIN;
        undef $/;
        my $slurp = <>;
        if (not defined $slurp)
        {
            die "$0: $filename: $!";
        }
        $event->{"kn_payload"} .= $slurp;
    }
}

use SOAP::Lite;
@list = SOAP::Lite
    -> service($SERVICE)
    -> on_fault(sub {
        my $soap = shift;
        my $res = shift;
        ref $res ? die(join " ", "SOAP FAULT:", $res->faultstring, "\n") 
            : die(join " ", "TRANSPORT ERROR:", $soap->transport->status, "\n");
        return new SOAP::SOM;
    })
    -> publish(SOAP::Data->type('string', $topic),
               SOAP::Data->type('map', $event));
($id, $statusEvent) = @list;
print "return: $id\n";
if (ref $statusEvent)
{
    print "statusEvent: { ".(
                             join(
                                  ", ",
                                  map(
                                      ("\"".$_."\": \"".$statusEvent->{$_}."\""),
                                      keys(%$statusEvent)))
                             )." }\n";
}
else
{
    print "statusEvent: $statusEvent\n";
}

# End of publish.pl
