// Copyright 2000-2004 KnowNow, Inc.  All rights reserved.

// @KNOWNOW_LICENSE_START@
// 
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
// 
// 1. Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
// 
// 2. Redistributions in binary form must reproduce the above copyright
// notice, this list of conditions and the following disclaimer in the
// documentation and/or other materials provided with the distribution.
// 
// 3. Neither the name of the KnowNow, Inc., nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// 
// @KNOWNOW_LICENSE_END@
// 

// $Id: kn_xml_msg.js,v 1.2 2004/04/19 05:39:12 bsittler Exp $

// kn_xml_msg contains routines for pubbing and subbing SOAP and XML.
// Note this JavaScript file has dependencies on other libraries, so
// to use this library, also include the following first:

// <script src="../kn_lib/xmljs/xml.js"></script>
// <script src="../kn_lib/object2xml.js"></script>
// <script src="../kn_lib/xml2object.js"></script>
// <script src="../kn_lib/soap2object.js"></script>
// <script src="../kn_lib/object2soap.js"></script>

// And then include this library:

// <script src="../kn_lib/kn_xml_msg.js"></script>

function kn_generateSoap(name, obj)
{
    var namedObject = new Object();
    namedObject[name] = obj;
    return Object2Soap(namedObject);
}

// opt can have the following options:
// kn_next_hop:   the route to publish the request to
// soap_uri:      the URI that will be listed in the SOAP headers
//                (for example, "urn:xmethods-delayed-quotes")
// soap_method:   Remote method to be invoked (for example, "getQuote")
// soap_endpoint: "http://services.xmethods.net:80/soap"

function kn_object2event(obj, opt)
{
    var e = new Object();
    var ret;

    e["content-type"] = "text/xml";
    e["soapaction"] = "true";

    if (opt) // Pump my lemma
    {
        for (var i in opt) {
            e[i] = opt[i];
        }
    }

    e.do_method = 'notify';
    e.kn_payload = Object2Soap(obj);
    e.kn_payload.replace(/</g, "&lt;");
    e.kn_payload.replace(/>/g, "&gt;");
    e.kn_payload.replace(/&/g, "&amp;");
    e.kn_payload.replace(/\"/g, "&quot;");

    return e;
}

function kn_event2object(obj) 
{
    var e = new Object();

    obj.kn_payload.replace(/&lt;/g, "<");
    obj.kn_payload.replace(/&gt;/g, ">");
    obj.kn_payload.replace(/&amp;/g, "&");
    obj.kn_payload.replace(/&quot;/g, "\"")
    e = Soap2Object(obj.kn_payload);

    return e;
}

function kn_activateSoap(kn_to, obj, headers) 
{
    var e = new Object();
    var ret;

    e["content-type"] = "text/xml";
    e["soapaction"] = "true";

    if (headers)
    {
        for (var i in headers)
        {
            e[i] = headers[i];
        }
    }

    e.do_method = 'notify';
    e.kn_payload = Object2Soap(obj);

    kn_retryInterval = 100000;
    ret = kn_publish(kn_to, e);
    kn_retryInterval = 4000;

    return ret;
}

function kn_publishAsSoap(kn_to, name, obj, headers)
{
    var e = new Object();
    
    e["content-type"] = "text/xml";
    e["soapaction"] = "true";

    if (headers)
    {
        for (var i in headers)
        {
            e[i] = headers[i];
        }
    }

    e.do_method = 'notify';
    e.kn_payload = kn_generateSoap(name,obj);
    
    return kn_publish(kn_to,e);
}

function kn_publishAsXml(kn_to, name, obj, headers)
{
    var e = new Object();
    var namedObject = new Object();
        namedObject[name] = obj;
    
    e["content-type"] = "text/xml";

    if (headers)
    {
        for (var i in headers)
        {
            e[i] = headers[i];
        }
    }

    e.do_method = 'notify';
    e.kn_payload = Object2Xml(namedObject);

    return kn_publish(kn_to,e);
}
  
function filter_message(e)
{
    if (((e["content-type"] == "text/xml") ||
         (e["content-type"] == "application/xml")) &&
          e["soapaction"])
    {
            e.body = Soap2Object(e.kn_payload);
    }

    return e;
}

// End of kn_xml_msg.js
