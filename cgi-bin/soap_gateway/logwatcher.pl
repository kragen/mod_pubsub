#!/usr/bin/perl -w

# logwatcher.pl -- example usage of PubSubService.cgi
# Acts like "tail -f" but publishes each line instead of printing it.

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

# $Id: logwatcher.pl,v 1.2 2004/04/19 05:39:09 bsittler Exp $

my $topic = "/what/chat";

use SOAP::Lite +autodispatch => proxy => "http://127.0.0.1/mod_pubsub/cgi-bin/soap_gateway/PubSubService.cgi" => on_fault => sub {
    my $soap = shift;
    my $res = shift;
    ref $res ? die(join " ", "SOAP FAULT:", $res->faultstring, "\n") 
        : die(join " ", "TRANSPORT ERROR:", $soap->transport->status, "\n");
    return new SOAP::SOM;
};

$for_a_while = 1;
open FILE, "<-";

for ($curpos = tell(FILE); $_ = <FILE>;
     $curpos = tell(FILE)) {
    # ignore existing lines
}

for (;;) {
    for ($curpos = tell(FILE); $_ = <FILE>;
         $curpos = tell(FILE)) {
        # search for some stuff and put it into files
        chomp;
        print "preparing to publish -> :$_:\n";
        @list = publish(
                        $topic,
                        {"userid"=>"logwatcher$$",
                         "displayname"=>"Log Watcher $$",
                         "kn_payload"=>$_});
        ($id, $statusEvent) = @list;
        print "return: $id\n";
        print "statusEvent: { ".(
                                 join(
                                      ", ",
                                      map(
                                          ("\"".$_."\": \"".$statusEvent->{$_}."\""),
                                          keys(%$statusEvent)))
                                 )." }\n";
    }
    sleep($for_a_while);
    seek(FILE, $curpos, 0);
}

# End of logwatcher.pl
