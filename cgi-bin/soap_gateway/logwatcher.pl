#!/usr/bin/perl -w

# logwatcher.pl -- example usage of PubSubService.cgi
# Acts like "tail -f" but publishes each line instead of printing it.

# Copyright 2000-2003 KnowNow, Inc.  All Rights Reserved.

# @KNOWNOW_LICENSE_START@

# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:

# 1. Redistributions of source code must retain the above copyright
# notice, this list of conditions and the following disclaimer.

# 2. Redistributions in binary form must reproduce the above copyright
# notice, this list of conditions and the following disclaimer in
# the documentation and/or other materials provided with the
# distribution.

# 3. The name "KnowNow" is a trademark of KnowNow, Inc. and may not
# be used to endorse or promote any product without prior written
# permission from KnowNow, Inc.

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

# @KNOWNOW_LICENSE_END@

# $Id: logwatcher.pl,v 1.1 2003/03/18 05:52:08 ifindkarma Exp $

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
