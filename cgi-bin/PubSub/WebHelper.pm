package PubSub::WebHelper;

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
# $Id: WebHelper.pm,v 1.1 2002/11/07 07:08:03 troutgirl Exp $

# NAME
# PubSub::WebHelper - Utility functions to help programs speak HTTP.

# SYNOPSIS
# use PubSub::WebHelper qw(encode_form http_request);
# $socket->send(http_request('POST', '/some/path', encode_form(foo => 'bar')));

# DESCRIPTION
# LWP contains a lot of useful functionality, but some of it is tied
# together in ways that make it inconvenient to use.  This provides a
# more convenient way to construct simple HTTP requests and
# application/x-www-form-urlencoded data.

# FIXME: This probably should be folded into LWP as a bit of refactoring.

# FIXME: POST assumes your content-type is application/x-www-form-urlencoded.

use URI;
use Exporter;
use base 'Exporter';
use strict;
use vars '@EXPORT_OK';
use MIME::Base64 ();

@EXPORT_OK = qw(encode_form http_request);

sub encode_form
{
    # This code is from HTTP::Request::Common::POST .
    # We use a temporary URI object to format
    # the application/x-www-form-urlencoded content.
    my $url = URI->new('http:');
    $url->query_form(@_);
    return $url->query;
}

# Given an HTTP method (like 'GET' or 'POST'), a URI object, and
# possibly an application/x-www-form-urlencoded body, return an HTTP
# request as a string.

sub http_request
{
    my ($method, $uri, $body) = @_;
    my @rv;

    push @rv, $method, " ", $uri->path_query(), " HTTP/1.0\r\n";

    # Authorization header.
    if (defined $uri->userinfo())
    {
        # RFC 2616, section 3.7.1, says
        # This flexibility regarding line breaks applies only to text
        # media in the entity-body; a bare CR or LF MUST NOT be
        # substituted for CRLF within any of the HTTP control
        # structures (such as header fields and multipart boundaries).

        # This causes us pain, because MIME::Base64::encode
        # inconsiderately appends a \n.

        my $str = "Authorization: Basic " . MIME::Base64::encode($uri->userinfo());
        chop $str; # Yes, really. Not chomp.
        push @rv, $str, "\r\n";
    }

    push @rv, "Host: ", $uri->host_port(), "\r\n";

    if (defined $body)
    {
        push @rv, ("Content-Length: ", length $body, "\r\n",
                   "Content-Type: application/x-www-form-urlencoded\r\n");
    }

    push @rv, "\r\n"; # End of headers.

    if (defined $body)
    {
        push @rv, $body;
    }

    return join '', @rv;
}

1;

# End of WebHelper.pm
