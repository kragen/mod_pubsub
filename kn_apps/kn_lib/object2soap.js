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

// $Id: object2soap.js,v 1.2 2004/04/19 05:39:12 bsittler Exp $

// object2soap.js depends on object2xml.js


// Recursive function that output XML text from all the properties of a
// JavaScript object.  This does not detect cycles (sub-objects that
// have already been output), so it could get caught in an infinite loop.

// The format of the output will be built in a way that soap2object will 
// re-create the same property structure.

// This format currently does not intelligently use attributes.

function Object2Soap(obj)
{
    var xml="";
    
    var tagname=null;
    
    xml += "<SOAP-ENV:Envelope " + 
      "xmlns:SOAP-ENV=\"http://schemas.xmlsoap.org/soap/envelope/\" " +
      "xmlns:xsi=\"http://www.w3.org/1999/XMLSchema-instance\" " + 
           "xmlns:xsd=\"http://www.w3.org/1999/XMLSchema\">\n" +
            "<SOAP-ENV:Body>\n";

    xml += Object2Soap_dump(obj,null);

    xml +=  "</SOAP-ENV:Body>\n"+
            "</SOAP-ENV:Envelope>\n";
    
    return xml;
}


function Object2Soap_dump(obj,parentTag)
{
    var xml = "";

    if (obj.soap_uri && obj.soap_method) {
      xml += "<m:" + obj.soap_method + " xmlns:m=\"" + obj.soap_uri + 
	"\" SOAP-ENV:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\">\n";
    }

    // Do children  

    for (var name in obj)
    {
        var value;
        var tagname=null;
        var type=null;
        var isArray=false;
        
        // Skip system properties

        if ((typeof name == 'string') && 
            (name.charAt(0) == '$'))
            continue;

	// Skip the SOAP parts of the object

	if ( (name == 'soap_uri') || (name == 'soap_method') || (name == 'soap_endpoint') ) {
	  continue;
	}
        
        value = obj[name];
        
        // Compute the name to use when writing the tag.

        tagname=name;
        if (parentTag)
            tagname = parentTag;

        if (typeof value != 'function')
        {
            // Get type from a variety of indicators.
            if (value && !Object2Xml_hasProps(value))
            {
                type = Object2Xml_getType(value);
            }

            // If the value remembers what tag it came from, then use it.
            if (value && value.$ && value.$.tagName)
            {
                // Qualify the name with a namespace, if available.
                tagname = getQName(value,tagname);
                
                // Don't use computed type if the tagname is a 'type' identifier.
                if (value.$.namespace == "SOAP-ENC")
                {
                    type = null;
                }
            }
            
            isArray=Object2Xml_isArray(value);
            if (isArray)
            {
                // TODO: Get the arrayType from the value
                xml += "<"+tagname+" SOAP-ENC:arrayType='xsd:string["+value.length+"]'>";
            }
            else
            {
                if (type)
                    xml += "<"+tagname+" xsi:type='xsd:"+type+"'>";
                else
                    xml += "<"+tagname+" xsi:type='xsd:string'>";
            }
            
            if (Object2Xml_isNodal(value))
            {
                xml += "\n";
                if (isArray)
                    xml += Object2Soap_dump(value,tagname);
                else
                    xml += Object2Soap_dump(value,null);
            }
            else
            {
                if (value)
                    xml += value;
            }
            
            xml += "</"+tagname+">\n";
        }
    }
    if (obj.soap_uri && obj.soap_method) {
      xml += "</m:" + obj.soap_method + ">\n";
    }

    return xml;
}

function getQName(obj,name)
{
    if (obj.$ && obj.$.tagName)
    {
        if (obj.$.namespace)
            return obj.$.namespace + ":" + obj.$.tagName;
        else
            return obj.$.tagName;
    }
    return name;
}

// End of object2soap.js
