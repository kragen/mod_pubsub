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
# $Id: soap_filter.cgi,v 1.2 2003/03/19 03:05:28 ifindkarma Exp $

# soap_filter.cgi : Third-party SOAP Service Proxy for mod_pubsub.
# Used with kn_next_hop to go mod_pubsub -> SOAP -> mod_pubsub.

# This foolishly assumes that a valid SOAP request has been published 
# to it. :)

# For another (better) example of a SOAP Service Proxy for mod_pubsub
# please see mod_pubsub/cgi-bin/soap_gateway/ .

use CGI ':standard';
use IO::Socket;
use PubSub::Event;
use IO::Handle;

my $q = new CGI();
my ($name, $host_uri, $post_uri, $method, $uri, $kn_payload,
    $payload_length, $hostname, $socket, $port, @response, $topic,
    $length, $tmp); 

print $q->header('text/html; charset=utf-8');

my @params = $q->param();


# Extract the important parameters for building the SOAP request.

foreach $name (@params) {
	$_ = $q->param($name);

	if ($name eq "soap_endpoint") {
	    $_ =~ m/^http:\/\/(\S+)\/(\S+)/i;
	    $host_uri = $1;
	    ($hostname, $port) = split(':', $1);
	    if(! $port ) {
		$port = 80;
	    }
	    if(! $hostname ) {
		$hostname = $1;
		$hostname =~ s|^(.*)\/|$1|;
	    }
	    $hostname =~ m|^([^/]+)(\/)|;
	    $hostname = $1;
	    $post_uri = "/" . $2;
	} elsif ($name eq "soap_method") {
	    $method = $_;
	} elsif ($name eq "soap_uri") {
	    $uri = $_;
	} elsif ($name eq "kn_payload") {
	    $kn_payload = $_;
	}
    }

$payload_length = length($kn_payload);

$socket = IO::Socket::INET->new(PeerAddr => $hostname,
				PeerPort => $port,
				Proto => "tcp",
				Type => SOCK_STREAM) 
    or die "Couldn't open connection to $hostname:$port : $@\n";

print $socket "POST $post_uri HTTP/1.1\n";
print $socket "Host: $host_uri\n";
print $socket "Accept: text/*\n";
print $socket "Content-Type: text/xml\n";
print $socket "Content-Length: $payload_length\n";
print $socket "SOAPAction: $uri#$method\n";
print $socket "\r\n";
print $socket $kn_payload;

while( $_ = <$socket> ) {
    if($_ =~ m|content-length|i) {
	($tmp, $len) = split(":", $_);
	($len) = split(" ", $len);
    }
    if ( $_ =~ m/^\r?$/ ) {
	last;
    }
}

sysread($socket, $response, $len);
close($socket);

my $now = time();

my $kn_url = url();

# kn_next_hop is where we send the return answer.
# Once again we don't care what it is, just as long as it exists.

$topic = $q->param('kn_next_hop');

$kn_url =~ s|[^/]*$|pubsub.cgi|;

my $kn_userid = remote_user();
my $kn_displayname = user_name();

# If there wasn't an ID, we are probably running on a
# non-authenticated http machine.  If authenticated, then we will use
# the kncgi:kncgi response.  There has been talk of using a more
# secure solution... until then this will have to do.

if (!$kn_userid)
{
    $kn_userid = 'anonymous';
    $kn_displayname = 'Anonymous User';
} else {
    $kn_url =~ s|//|//kncgi:kncgi@|;
}

PubSub::Event::kn_url_is($kn_url);

my $event = new PubSub::Event(kn_payload => $response,
			  displayname => $kn_displayname,
                          'content-type' => "text/xml",
                          soapaction => "true"
			  );

$event->post($topic);

# End of soap_filter.cgi
