#!/usr/bin/perl -w

# PubSubService.cgi -- Example of a mod_pubsub SOAP gateway.

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

# $Id: PubSubService.cgi,v 1.4 2004/04/19 05:39:09 bsittler Exp $

package main;

use CGI;
use SOAP::Lite;
use SOAP::Transport::HTTP;
use LWP::UserAgent;
use HTTP::Request::Common qw();
use URI;
use strict;

use vars qw($SERVER_LOOPBACK $SERVER $AUTHORIZATION $COOKIE $ua $init_done);

package PubSub;

# Only these methods are exposed to the world:
sub pubsubrequest { return (main::pubsubrequest(@_)); }
sub publish { return (main::publish(@_)); }
sub subscribe { return (main::subscribe(@_)); }
sub unsubscribe { return (main::unsubscribe(@_)); }

package main;

sub init {
    return unless not $init_done;
    $init_done = 1;

    # $SERVER is the SOAP interface URI (i.e. something which runs
    # this script, minus query string).  This is written into the
    # autogenerated WSDL as the HTTP transport endpoint URI.

    if (not defined $SERVER)
    {
        # single world:
        #$SERVER = "http://pubsub.bigcorp.com:8000/kn";
        # single world w/SSL accelerator:
        #$SERVER = "https://ssl-accel.bigcorp.com:8443/kn";

        # dual world: (CGI)
        $SERVER = "http://127.0.0.1/mod_pubsub/cgi-bin/soap_gateway/PubSubService.cgi";
    }

    # $SERVER_LOOPBACK is the server URI as seen by this script; this
    # will be seen by clients from time to time, but only embedded in
    # "opaque" strings, e.g. route URIs.

    if (not defined $SERVER_LOOPBACK)
    {
        # single world: (slow, but fairly portable)
        #$SERVER_LOOPBACK = $SERVER;

        # single world: (true loopback, but non-portable; needed for
        # some SSL accelerators, reverse-proxies, etc.)
        #$SERVER_LOOPBACK = "http://127.0.0.1:8000/kn";

        $SERVER_LOOPBACK = "http://127.0.0.1/kn";

        # try to deduce loopback URI based on CGI parameters
        if (defined $ENV{SERVER_HOST} &&
            defined $ENV{SERVER_PORT})
        {
            my $protocol = "http";
            if ($ENV{SERVER_PORT} eq '443' or
                defined $ENV{SSL} and $ENV{SSL} eq 'ON')
            {
                $protocol = "https";
            }
            elsif (defined $ENV{SERVER_PROTOCOL})
            {
                $protocol = lc((split('/', $ENV{SERVER_PROTOCOL}))[0]);
            }
            $SERVER_LOOPBACK =
                "${protocol}://$ENV{SERVER_HOST}:$ENV{SERVER_PORT}/kn";
        }

        # dual world examples:
        #$SERVER_LOOPBACK = "http://pubsub.bigcorp.com:8000/kn";
        #$SERVER_LOOPBACK = "http://127.0.0.1/mod_pubsub/cgi-bin/pubsub.cgi";
    }

    # $AUTHORIZATION is the value to place in the HTTP Authorization
    # header sent to the server.

    # $COOKIE is the value to place in the HTTP Cookie header sent to
    # the server.

    $ua = LWP::UserAgent->new();
}

sub topic2uri {
    init();
    my ($topic_str) = @_;
    my $base = URI->new($SERVER_LOOPBACK);
    my $base_path = $base->path();
    if (! $base_path =~ m|/$|)
    {
        $base->path($base_path . "/");
    }
    if ($topic_str =~ m|^/|)
    {
        $topic_str = "." . $topic_str;
    }
    return URI->new_abs($topic_str, $base)->canonical->as_string;
}

sub scalar2hashref {
    my ($scalar) = @_;

    if (not defined $scalar)
    {
        $scalar = "";
    }

    my $hash = {
        "kn_payload" => $scalar
    };

    my $reftype = ref $scalar;

    if ($reftype)
    {
        if ($reftype eq "ARRAY" or
            UNIVERSAL::isa($scalar, "ARRAY"))
        {
            $hash = {};
            my @values = (@$scalar);
            while ($#values >= 0)
            {
                my $key = "kn_payload";
                my $value = shift @values;
                my $vreftype = ref $value;
                if ($vreftype)
                {
                    if ($vreftype eq "HASH" or
                        UNIVERSAL::isa($value, "HASH"))
                    {
                        $key = $value->{'key'};
                        $value = $value->{'value'};
                    }
                    else
                    {
                        die "unsupported item type: $vreftype";
                    }
                }
                elsif ($#values >= 0)
                {
                    $key = $value;
                    $value = shift @values;
                }
                if (defined $key)
                {
                    if (not defined $hash->{$key})
                    {
                        $hash->{$key} = $value;
                    }
                    else
                    {
                        if (ref $hash->{$key} ne "ARRAY" and
                            not UNIVERSAL::isa($hash->{$key}, "ARRAY"))
                        {
                            $hash->{$key} = [ $hash->{$key} ];
                        }
                        my $listref = $hash->{$key};
                        push @$listref, $value;
                    }
                }
            }
        }
        else
        {
            $hash = $scalar;
        }
    }

    return $hash;
}

sub hashref2formref {
    my ($hash) = @_;
    my $form = [];
    my $key;

    foreach $key (keys %$hash)
    {
        my $varray = $hash->{$key};
        my $reftype = ref $varray;
        $varray = [ $varray ] unless ($reftype and
                                      ($reftype eq "ARRAY" or
                                       UNIVERSAL::isa($varray, "ARRAY")));
        while ($#$varray >= 0)
        {
            my $value = shift @$varray;

            if (ref $value)
            {
                $value =
                    HTTP::Request::Common::POST('/',
                                                hashref2formref(scalar2hashref($value)))
                    ->content;
            }
            push @$form, "" . $key => "" . $value;
#            print STDERR "[$form] $key: $value\n";
        }
    }

    return $form;
}

sub pubsubrequest {
    init();
    my ($this, $request) = @_;
    $request = scalar2hashref($request);
    if (! defined($request->{'kn_response_format'}))
    {
        $request->{'kn_response_format'} = 'simple';
    }
    my @headers = ();
    if (defined $AUTHORIZATION)
    {
        push @headers, "Authorization" => $AUTHORIZATION;
    }
    if (defined $COOKIE)
    {
        push @headers, "Cookie" => $COOKIE;
    }
    my $result = $ua->post($SERVER_LOOPBACK, hashref2formref($request), @headers);
    my $statusEvent = {
        "status" => $result->status_line,
        "content-type" => "text/plain",
        "kn_payload" => $result->content
    };
    $result->
        headers->
        scan(sub
             {
                 my ($name, $value) = @_;
                 $name = lc $name;
                 return if ($name =~ m/^(client|connection)($|-)/);
                 if (defined $statusEvent->{"$name"})
                 {
                     $value =
                         $statusEvent->{"$name"} .
                         ", " .
                         $value;
                 }
                 $statusEvent->{"$name"} = $value;
             });
    if ($result->is_error and not length $result->content)
    {
        $statusEvent->{'kn_payload'} = $result->error_as_HTML;
        $statusEvent->{'content-type'} = "text/html";
        delete $statusEvent->{'content-language'};
    }
    my $content = $result->content;
    if ($content =~ /^ *[0-9]+ *\n/s)
    {
        my $length = $content;
        $length =~ s/\n.*//s;
        $length = int($length);
        $content =~ s/^[^\n]*\n//s;
        if ($length <= length $content)
        {
            $statusEvent->{'content-type'} = "text/plain";
            delete $statusEvent->{'content-language'};
            $content = substr $content, 0, $length;
            my ($headers, undef, $payload) =
                $content =~ /^(([^:\n]*: *[^\n]*\n)*)\n(.*)/s;
            my @headerlines = split('\n', $headers);
            my $index;
            foreach $index (0 .. $#headerlines)
            {
                my ($name, $value) =
                    $headerlines[$index] =~ /([^:]*): *(.*)/s;
                $name =~
                    s/=([[:xdigit:]][[:xdigit:]])/chr(hex($1))/esg;
                $value =~
                    s/=([[:xdigit:]][[:xdigit:]])/chr(hex($1))/esg;
                $statusEvent->{$name} = $value;
            }
            $statusEvent->{'kn_payload'} = $payload;
        }
        else
        {
            die "partial event packet in " .
                $result->content_type .
                " HTTP response from $SERVER_LOOPBACK";
        }
    }
    elsif (length($content))
    {
        die "unrecognized event format in " .
            $result->content_type .
            " HTTP response from $SERVER_LOOPBACK";
    }

    delete $statusEvent->{'content-length'};
    delete $statusEvent->{'content-encoding'};

    my $mapArray = [ ];
    my $mapKey;
    my $map2 = [ ];
    foreach $mapKey (keys %$statusEvent)
    {
        my $mapItem = SOAP::Data
            ->type('ordered_hash',
                   [ 'key' => SOAP::Data->type('string', $mapKey),
                     'value' => SOAP::Data->type('string', $statusEvent->{$mapKey})])
            ->name('item');
        $mapItem->attr({'xsi:type' => 'apachens:MapItem'});
        push @$mapArray, 'item', $mapItem;
        push @$map2, $mapItem;
    }
    my $soapData = SOAP::Data
        ->type('ordered_hash', $mapArray)
        ->name('statusEvent');
#    if ($SOAPFORMAT ne "apache")
#    {
#        $soapData = SOAP::Data
#            ->type('array', $map2)
#            ->name('statusEvent');
#    }
    $soapData->attr({'xmlns:apachens' => 'http://xml.apache.org/xml-soap',
                     'xsi:type' => 'apachens:Map'});
    return ($soapData);
}

sub publish {
    init();
    my ($this, $kn_to, $event) = @_;
    if (not defined $kn_to)
    {
        $kn_to = "/";
    }
    $event = scalar2hashref($event);
    my $request = {
        'do_method' => 'notify',
        'kn_to' => $kn_to,
        'kn_id' => "" . int(100000000 * rand)
        };
    my $key;
    foreach $key (keys %$event)
    {
        $request->{$key} = $event->{$key};
    }
    my ($statusEvent) = pubsubrequest($this, $request);
    return (SOAP::Data
            ->type('string',
                   $request->{'kn_id'})
            ->name('return'),
            $statusEvent);
}

sub subscribe {
    init();
    my ($this, $kn_from, $kn_to, $options) = @_;
    if (not defined $options)
    {
        $options = {};
    }
    $options = scalar2hashref($options);
    my $request = {
        'do_method' => 'route',
        'kn_id' => "" . int(100000000 * rand),
        'kn_from' => $kn_from,
        'kn_to' => $kn_to,
        'kn_response_format' => 'simple'
        };
    my $key;
    foreach $key (keys %$options)
    {
        $request->{$key} = $options->{$key};
    }
    $request->{'kn_from'} = topic2uri($request->{'kn_from'});
    if (not defined $request->{'kn_uri'})
    {
        my $rid = $request->{'kn_from'} .
            "/kn_routes/" . URI::Escape::uri_escape($request->{'kn_id'});
        $request->{'kn_uri'} = $rid;
    }
    my ($statusEvent) = pubsubrequest($this, $request);
    return (SOAP::Data->type('string',
                             $request->{'kn_uri'})
            ->name('return'),
            $statusEvent);
}

sub unsubscribe {
    init();
    my ($this, $rid) = @_;
    if (! $rid =~ /^(.*)\/kn_routes\/([^\/]+)$/s)
    {
        my $statusEvent = {
            'status' => "400 Bad Request",
            'kn_payload' =>
                "Client will not delete a route without the magic '/kn_routes/' substring."
            };
        return ($statusEvent);
    }
    my $kn_from = $1;
    my $kn_id = URI::Escape::uri_unescape($2);
    my $request = {
        'do_method' => 'route',
        'kn_id' => $kn_id,
        'kn_uri' => $rid,
        'kn_from' => $kn_from,
        'kn_to' => ''
    };
    my ($statusEvent) = pubsubrequest($this, $request);
    return ($statusEvent);
}

sub handle_string {
    init();
    my ($headers, $content, $method, $url) = @_;

    my $httpTransport = new SOAP::Transport::HTTP::Server;

    if (not defined $content)
    {
        $content = '';
    }
    my %params = ( "kn_payload" => $content,
                   "do_method" => "",
                   map { lc($_) => $headers->{$_} } keys %$headers );

    my $command = $params{"do_method"};

    # allow content to be overridden by explicit kn_payload header
    $content = $params{"kn_payload"};
    my $length = length $content;

    # calculate options to embed in generated URLs
    my $commandPrefix = "";
    if (defined $params{"kn_request_format"})
    {
        $commandPrefix .= "kn_request_format=" .
            URI::Escape::uri_escape($params{"kn_request_format"}) .
            ";";
    }
    if (defined $params{"kn_response_format"})
    {
        $commandPrefix .= "kn_response_format=" .
            URI::Escape::uri_escape($params{"kn_response_format"}) .
            ";";
    }
    $commandPrefix .= "do_method=";

    # allow kn_request_format={soap,wsdl} shortcuts for pure HTTP
    # transport in the WSDL
    $commandPrefix =~
        s|kn_request_format=http;kn_response_format=http;do_method=|kn_request_format=|g;

    $commandPrefix = new CGI("")->escapeHTML("$SERVER?$commandPrefix");

    #pretty_xml("REQUEST", $content);

    if ($command eq "soap")
    {
        if (!$length)
        {
            $httpTransport->response(HTTP::Response->new(411)); # LENGTH REQUIRED
        }
        elsif (defined $SOAP::Constants::MAX_CONTENT_SIZE &&
               $length > $SOAP::Constants::MAX_CONTENT_SIZE)
        {
            $httpTransport->response(HTTP::Response->new(413)); # REQUEST ENTITY TOO LARGE
        }
        else
        {
            if (defined $params{'authorization'})
            {
                $AUTHORIZATION = $params{'authorization'};
            }
            else
            {
                undef $AUTHORIZATION;
            }
            if (defined $params{'cookie'})
            {
                $COOKIE = $params{'cookie'};
            }
            else
            {
                undef $COOKIE;
            }
            $httpTransport->request(HTTP::Request->new( 
                                                        $method => $url,
                                                        HTTP::Headers->new($headers),
                                                        $content,
                                                        ));
            $httpTransport
                -> dispatch_to('PubSub')
                -> handle;
        }
    }
    elsif ($command eq "wsdl")
    {
        my $wsdlHeaders = HTTP::Headers->new(Content_Type => 'text/xml; charset=utf-8');
        my $wsdlContent = << ".";
<?xml version="1.0" encoding="UTF-8"?>
<definitions
   xmlns:tns="${commandPrefix}wsdl"
   xmlns:xsd="http://www.w3.org/2001/XMLSchema"
   xmlns:soap="http://schemas.xmlsoap.org/wsdl/soap/"
   xmlns:soapenc="http://schemas.xmlsoap.org/soap/encoding/"
   xmlns:wsdl="http://schemas.xmlsoap.org/wsdl/"
   xmlns:apachens="http://xml.apache.org/xml-soap"
   xmlns="http://schemas.xmlsoap.org/wsdl/"
   name="PubSub"
   targetNamespace="${commandPrefix}wsdl">
  <types>
    <documentation>
      This interface uses a Map serialization
      mostly compatible with the Apache Map. This
      Map is specified as an array of
      MapItem structures (also known as header
      name-value pairs, or simply headers), where each
      MapItem is a struct with key and
      value properties. The key property
      (also known as the header name) is a string, and the
      value property (also known as the header
      value) is either a string or a nested
      Map [only string header values will be returned by the SOAP
      interface; nested Map header values are used with the
      kn_batch header name when invoking the
      do_method=batch PubSub Server command, which
      allows multiple nested commands to be given in a single
      PubSub Server request.] Note that this interface allows
      other types to be substituted for this Map
      serialization in input values, with the following behaviors:
      
        
          A string, integer or other &quot;scalar&quot; input value is
          considered to be the header value corresponding to the
          kn_payload header name.
        
        
          A null value is considered to be an empty string value
          corresponding to the kn_payload header name.
        
        
          An array is considered to be a list of interspersed header
          names and header values; the 1st element is the first header
          name, the 2nd element is the first header value, the 3rd
          element is the second header name, the 4th element in the
          2nd header value, etc. If the array has an odd number of
          elements, the last element is considered to be the header
          value corresponding to the kn_payload header
          name.
        
        
          Any other type is considered to be a struct. Each property
          name in the struct is used as a header name, with the
          property value used as the corresponding header value.
        
      
    </documentation>
    <xsd:schema
       targetNamespace="http://xml.apache.org/xml-soap">
      <xsd:complexType name="MapItem">
        <xsd:sequence>
           <xsd:element name="key" type="xsd:string"/>
           <xsd:element name="value" type="xsd:anyType"/>
        </xsd:sequence>
      </xsd:complexType>
      <xsd:complexType name="Map">
        <xsd:complexContent mixed="false">
          <xsd:restriction base="soapenc:Array">
            <xsd:sequence>
              <xsd:element
                name="item" type="xsd:anyType" minOccurs="0" maxOccurs="unbounded"/>
            </xsd:sequence>
            <xsd:attribute wsdl:arrayType="xsd:anyType[]" ref="soapenc:arrayType"/>
          </xsd:restriction>
        </xsd:complexContent>
      </xsd:complexType>
    </xsd:schema>
  </types>
  <message name="publishRequest">
    <part name="kn_to" type="xsd:string">
      <documentation>
        Destination Topic URI: The topic to publish the
        event to, specified as a string containing either a relative
        topic URI (e.g. /what/chat) or a fully-qualified
        topic URI
        (e.g. http://pubsub.bigcorp.com:8000/kn/what/chat).
      </documentation>
    </part>
    <part name="event" type="xsd:anyType">
      <documentation>
        Message: The event to be published, specified as
        a Map of name-value pairs (called event
        headers). This could include a message body (in the
        kn_payload event header), and could also
        optionally include other parameters supported by the
        do_method=notify PubSub Server command.
      </documentation>
    </part>
  </message>
  <message name="publishResponse">
    <part name="return" type="xsd:string">
      <documentation>
        Event ID: If the event is lacking an event
        ID (the
        kn_id event header), the publish
        method will generate one. Regardless of whether the event ID
        is specified by the publishing application or generated by
        this method, it will be returned as a string in the
        return value.
      </documentation>
    </part>
    <part name="statusEvent" type="apachens:Map">
      <documentation>
        PubSub Server Response: Contains a status
        event with the response from the PubSub Server. This status
        event is a Map with an HTTP-style status line (in the
        status event header) and possibly a description
        (in the kn_payload event header,) as well as any
        other headers returned by the PubSub Server.
      </documentation>
    </part>
  </message>
  <message name="subscribeRequest">
    <part name="kn_from" type="xsd:string">
      <documentation>
        Source Topic URI: The topic to subscribe to,
        specified as a string containing either a relative topic URI
        (e.g. /what/chat) or a fully-qualified topic URI
        (e.g. http://pubsub.bigcorp.com:8000/kn/what/chat)
      </documentation>
    </part>
    <part name="kn_to" type="xsd:string">
      <documentation>
        Destination URI: The subscriber URI, specified as
        a string containing either a relative topic URI
        (e.g. /what/all or
        /who/anonymous/connector/12376454/kn_journal) or
        a fully-quailified URI
        (e.g. http://pubsub2.bigcorp.com:8000/kn/what/chat
        or
        https://secure.bigcorp.com/ChatListener.asp)
      </documentation>
    </part>
    <part name="options" type="xsd:anyType">
      <documentation>
        Additional Subscription Parameters: Additional
        parameters for the subscription request, specified as a
        Map of name-value pairs. This may include any
        parameter supported by the do_method=route PubSub Server
        command.
      </documentation>
    </part>
  </message>
  <message name="subscribeResponse">
    <part name="return" type="xsd:string">
      <documentation>
        Subscription URI: If the subscription request is
        lacking an ID (the kn_id subscription parameter),
        the subscribe method will generate one; if the
        subscription request is lacking a route URI (the
        kn_uri subscription parameter), this method will
        generate one by concatenating a fully-qualified form of the
        source topic URI, the &quot;magic&quot; string
        /kn_routes/, and the subscription ID. Regardless
        of whether the route URI is specified by the subscribing
        application or generated by this method, it will be returned
        as a string in the
        return value.
      </documentation>
    </part>
    <part name="statusEvent" type="apachens:Map">
      <documentation>
        PubSub Server Response: Contains a status
        event with the response from the PubSub Server. This status
        event is a Map with an HTTP-style status line (in the
        status event header) and possibly a description
        (in the kn_payload event header,) as well as any
        other headers returned by the PubSub Server.
      </documentation>
    </part>
  </message>
  <message name="unsubscribeRequest">
    <part name="rid" type="xsd:string">
      <documentation>
        Subscription URI: A route URI containing the
        &quot;magic&quot; /kn_routes/ substring (as
        returned by the subscribe method),
        specified as a string.
      </documentation>
    </part>
  </message>
  <message name="unsubscribeResponse">
    <part name="statusEvent" type="apachens:Map">
      <documentation>
        PubSub Server Response: Contains a status
        event with the response from the PubSub Server. This status
        event is a Map with an HTTP-style status line (in the
        status event header) and possibly a description
        (in the kn_payload event header,) as well as any
        other headers returned by the PubSub Server.
      </documentation>
    </part>
  </message>
  <message name="pubsubrequestRequest">
    <part name="request" type="xsd:anyType">
      <documentation>
        PubSub Server Request: Contains the request
        to be processed, specified as a Map of name-value
        pairs (called request headers). This could include
        a PubSub Server command (in the do_method
        request header), and could also optionally include other
        parameters supported by the HTTP interface to the
        PubSub Server.
      </documentation>
    </part>
  </message>
  <message name="pubsubrequestResponse">
    <part name="statusEvent" type="apachens:Map">
      <documentation>
        PubSub Server Response: Contains a status
        event with the response from the PubSub Server. This status
        event is a Map with an HTTP-style status line (in the
        status event header) and possibly a description
        (in the kn_payload event header,) as well as any
        other headers returned by the PubSub Server.
      </documentation>
    </part>
  </message>
  <portType name="PubSubPortType">
    <operation name="publish" parameterOrder="kn_to event statusEvent">
      <documentation>
        Publish an Event: Delivers an
        event (notification message) to a topic on the
        PubSub Server.
      </documentation>
      <input message="tns:publishRequest"/>
      <output message="tns:publishResponse"/>
    </operation>
    <operation name="subscribe" parameterOrder="kn_from kn_to options statusEvent">
      <documentation>
        Start a Subscription: Creates a
        route (subscription record) connecting a source
        topic on the PubSub Server to a destination URI.
      </documentation>
      <input message="tns:subscribeRequest"/>
      <output message="tns:subscribeResponse"/>
    </operation>
    <operation name="unsubscribe" parameterOrder="rid statusEvent">
      <documentation>
        End a Subscription: Given a route URI
        containing the &quot;magic&quot; /kn_routes/
        substring (as returned by the subscribe method),
        the unsubscribe method removes the
        corresponding route from the PubSub Server.
      </documentation>
      <input message="tns:unsubscribeRequest"/>
      <output message="tns:unsubscribeResponse"/>
    </operation>
    <operation name="pubsubrequest" parameterOrder="request statusEvent">
      <documentation>
        Invoke a PubSub Server Command:
        Exposes the HTTP interface to the PubSub Server to SOAP clients.
      </documentation>
      <input message="tns:pubsubrequestRequest"/>
      <output message="tns:pubsubrequestResponse"/>
    </operation>
  </portType>
  <binding name="PubSubBinding" type="tns:PubSubPortType">
    <soap:binding style="rpc" transport="http://schemas.xmlsoap.org/soap/http"/>
    <operation name="publish">
      <soap:operation soapAction="http://tempuri.org/PubSub#publish"/>
      <input>
        <soap:body use="encoded" namespace="http://tempuri.org/PubSub"
           encodingStyle="http://schemas.xmlsoap.org/soap/encoding/"/>
      </input>
      <output>
        <soap:body use="encoded" namespace="http://tempuri.org/PubSub"
           encodingStyle="http://schemas.xmlsoap.org/soap/encoding/"/>
      </output>
    </operation>
    <operation name="subscribe">
      <soap:operation soapAction="http://tempuri.org/PubSub#subscribe"/>
      <input>
        <soap:body use="encoded" namespace="http://tempuri.org/PubSub"
           encodingStyle="http://schemas.xmlsoap.org/soap/encoding/"/>
      </input>
      <output>
        <soap:body use="encoded" namespace="http://tempuri.org/PubSub"
           encodingStyle="http://schemas.xmlsoap.org/soap/encoding/"/>
      </output>
    </operation>
    <operation name="unsubscribe">
      <soap:operation soapAction="http://tempuri.org/PubSub#unsubscribe"/>
      <input>
        <soap:body use="encoded" namespace="http://tempuri.org/PubSub"
           encodingStyle="http://schemas.xmlsoap.org/soap/encoding/"/>
      </input>
      <output>
        <soap:body use="encoded" namespace="http://tempuri.org/PubSub"
           encodingStyle="http://schemas.xmlsoap.org/soap/encoding/"/>
      </output>
    </operation>
    <operation name="pubsubrequest">
      <soap:operation soapAction="http://tempuri.org/PubSub#pubsubrequest"/>
      <input>
        <soap:body use="encoded" namespace="http://tempuri.org/PubSub"
           encodingStyle="http://schemas.xmlsoap.org/soap/encoding/"/>
      </input>
      <output>
        <soap:body use="encoded" namespace="http://tempuri.org/PubSub"
           encodingStyle="http://schemas.xmlsoap.org/soap/encoding/"/>
      </output>
    </operation>
  </binding>
  <service name="PubSubService">
    <documentation>PubSub Server</documentation>
    <port name="PubSubPort" binding="tns:PubSubBinding">
      <soap:address
         location="${commandPrefix}soap"/>
    </port>
  </service>
</definitions>
.
        $httpTransport->response(HTTP::Response->new(200, # OK
                                                     "Ok, whatever",
                                                     $wsdlHeaders,
                                                     $wsdlContent));
    }
    else
    {
        $httpTransport->response(HTTP::Response->new(500, # INTERNAL SERVER ERROR
                                                     "Internal Server Error (Unknown do_method)",
                                                     HTTP::Headers->new(Content_Type => 'text/plain; charset=utf-8'),
                                                     "Unknown do_method ($command).\n" .
                                                     "\n" .
                                                     "Please specify either do_method=soap or do_method=wsdl,\n" .
                                                     "or use the kn_request_format=soap or kn_request_format=wsdl shortcuts.\n"));
    }

    my $code = $httpTransport->response->code;
    my $status = $code . " " . HTTP::Status::status_message($code);
    my $responseHeaders = { };
    $httpTransport->
        response->
        headers->
        scan(sub
             {
                 my ($name, $value) = @_;
                 $responseHeaders->{$name} = $value;
             });
    my $responseContent = $httpTransport->response->content;

    if ($httpTransport->response->is_error and not length $responseContent)
    {
        $responseContent = $httpTransport->response->error_as_HTML;
        $responseHeaders->{'Content-Type'} = "text/html";
    }
    $responseHeaders->{'Content-Length'} = length $responseContent;

    #pretty_xml("RESPONSE", $responseContent);

    return ($status, $responseHeaders, $responseContent);
}

sub pretty_xml {
    my ($name, $xml) = @_;

    print STDERR "$name [[[\n";
    open PRETTY, "|/usr/local/bin/xmllint --format - >&2";
    print PRETTY $xml;
    close PRETTY;
    print STDERR "]]]\n";
}

sub handle_cgi {
    my $content;
    $SERVER = new CGI('')->url;
    my $length = $ENV{'CONTENT_LENGTH'} || 0;
    if ($length > 0)
    {
        read(STDIN,$content,$length) || die "short read: $!";
    }
    my $method = $ENV{'REQUEST_METHOD'} || '';
    my $url = $ENV{'SCRIPT_NAME'};
    my $headers = {};
    map { if (/^HTTP_(.+)/i) { $headers->{lc $1} = $ENV{$_}; }; } keys %ENV;
    my $query = $ENV{'QUERY_STRING'};
    if (defined $query)
    {
        $query =~ s/[+]/ /g;
        my @querySplit = split(/[&]/, $query);
        @querySplit = split(/[;]/, $query) if ($#querySplit == 0);
        my $queryPair;
        foreach $queryPair (@querySplit)
        {
            my @pairSplit = split(/[=]/, $queryPair);
            if ($#pairSplit == 1)
            {
                @pairSplit = map { URI::Escape::uri_unescape($_) } @pairSplit;
                $headers->{$pairSplit[0]} = $pairSplit[1];
            }
        }
    }

    # allow kn_request_format={soap,wsdl} shortcuts to be re-applied
    # just in case we're being invoked outside the usual context;
    # normally the PubSub Server must do this part for us
    if (defined $headers->{"kn_request_format"} and
        ($headers->{"kn_request_format"} eq "wsdl" or
         $headers->{"kn_request_format"} eq "soap"))
    {
        if (not defined $headers->{"do_method"})
        {
            $headers->{"do_method"} = $headers->{"kn_request_format"};
        }
        $headers->{"kn_request_format"} = "http";
        if (not defined $headers->{"kn_response_format"})
        {
            $headers->{"kn_response_format"} = "http";
        }
    }

    # in a CGI context we default to do_method=soap
    if (not defined $headers->{"do_method"})
    {
        $headers->{"do_method"} = "soap";
    }

    my ($status, $responseHeaders, $responseContent) =
        handle_string($headers, $content, $method, $url);

    my $response =
        "Status: $status\r\n" .
        join('',
             map(
                 ("$_: ".$responseHeaders->{$_}."\r\n"),
                 keys(%$responseHeaders))) . "\r\n" .
        $responseContent;

    print STDOUT $response;
}

binmode STDIN;
binmode STDOUT;

if (defined $ENV{REQUEST_METHOD})
{
    if ($ENV{REQUEST_METHOD} eq "GET" and
        (not defined $ENV{QUERY_STRING} or
         $ENV{QUERY_STRING} eq ""))
    {
        print "Content-Type: text/plain; charset=utf-8\r\n\r\n";
        open SELF, "<", $ENV{SCRIPT_FILENAME} || die "$!";
        undef $/;
        print <SELF>;
        close SELF;
    }
    else
    {
        handle_cgi();
    }
}
else
{
    undef $/;
    my $content = <>;

    my ($status, $responseHeaders, $responseContent) =
        handle_string({}, $content, "POST", "/");

    my $response =
        "Status: $status\r\n" .
        join('',
             map(
                 ("$_: ".$responseHeaders->{$_}."\r\n"),
                 keys(%$responseHeaders))) . "\r\n" .
        $responseContent;

    print STDOUT $response . "\r\n";
}

# End of PubSubService.cgi
