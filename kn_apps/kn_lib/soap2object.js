// Copyright 2000-2002 KnowNow, Inc.  All Rights Reserved.

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
// notice, this list of conditions and the following disclaimer in
// the documentation and/or other materials provided with the
// distribution.
// 
// 3. The name "KnowNow" is a trademark of KnowNow, Inc. and may not
// be used to endorse or promote any product without prior written
// permission from KnowNow, Inc.
// 
// THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESSED OR IMPLIED
// WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
// IN NO EVENT SHALL KNOWNOW, INC. OR ITS CONTRIBUTORS BE LIABLE FOR ANY
// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
// GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
// IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
// OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
// ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// 
// @KNOWNOW_LICENSE_END@

// $Id: soap2object.js,v 1.1 2002/11/07 07:08:12 troutgirl Exp $

// soap2object.js depends on xml2object.js


/* Here is a sample SOAP message:

<SOAP-ENV:Envelope
  xmlns:SOAP-ENV="http://schemas.xmlsoap.org/soap/envelope/"
  SOAP-ENV:encodingStyle="http://schemas.xmlsoap.org/soap/encoding/" >

    <SOAP-ENV:Header>
        <t:Transaction xmlns:t="some-URI" SOAP-ENV:mustUnderstand="1">
            5
        </t:Transaction>
    </SOAP-ENV:Header>

    <SOAP-ENV:Body>
        <m:GetLastTradePrice xmlns:m="Some-URI">
            <symbol>DEF</symbol>
            <Company>DEF Corp</Company>
            <Price>34.1</Price>
        </m:GetLastTradePrice>
    </SOAP-ENV:Body>

</SOAP-ENV:Envelope>  

*/


/** This creates a JavaScript object to access the data specified by a
 *  SOAP message.  See http://www.w3.org/TR/SOAP for more information.
 *  @param soap text string of a SOAP message.
 *  @return an object
 */

function Soap2Object(soap, bKeepNamespace)
{
    // Parse it.
    var doc = new XMLDoc(soap, function (x) { alert(x) } );
    var root = doc.docNode;
    
    // Get the body.
    var body;
    var elements = root.getElements();

    for (var i = 0; i < elements.length; i ++)
    {
        if (isSameName(elements[i],
                       elements[i].tagName,
                       "SOAP-ENV:Body"))
        {
            break;
        }
    }
    if (i < elements.length)
    {
        body = elements[i];
    }
    else
    {
        alert("No SOAP-ENV:Body or equivalent element found!");
        return null;
    }
    
    // Add properties to the body.
    var obj = new Object;
    var children = body.getElements();
    for (var i = 0; i < children.length; i++)
    {
        Soap_addProps(obj, null, children[i], bKeepNamespace);
    }
    return obj; 
}  


/** Recursive function to add named properties to a javascript object.
 *  @param obj JavaScript object that will hold the properties
 *  @param node XMLNode that holds the names, data and child nodes
 *  @param bKeepNamespace determines if namespace prefix is trimmed from tag names.
 */

function Soap_addProps(obj, propName, node, bKeepNamespace)
{
    if (node == null)
        return;
        
    if (node.nodeType == "ELEMENT")
    {
        var name = propName;
        var isArray = false;
        var value;
        
        if (name == null)
            name = getNodeName(node, bKeepNamespace);
        
        // Look for pure-text elements.

        if (isTextElement(node))
        {
            // Add as property.
            value = node.getText();
            var soapType = getSameAttribute(node, "xsi:type");
            if (isSameName(node, soapType, "xsd:int"))
            {
                value = new Number(parseInt(value));
            }
            else if (isSameName(node, soapType, "xsd:decimal"))
            {
                value = new Number(parseFloat(value));
            }
            
            obj[name] = value;
        }
        else if (isDateElement(node))
        {
            obj[name] = new Date(node.getText());
        }
        else
        {
            isArray = getSameAttribute(node, "SOAP-ENC:arrayType");
            
            // Create a placeholder property & add children.
            if (isArray)
                value = new Array;
            else
                value = new Object;
            
            // If there is already this type, add at end of array
            obj[name] = value;
            
            if (isArray)
            {
                var children = node.getElements();
                for (var i = 0; i < children.length; i ++)
                {
                    Soap_addProps(value, value.length, children[i], bKeepNamespace);
                }
            }
            else
            {
                // STRUCT
                var children = node.getElements();
                for (var i = 0; i < children.length; i ++)
                {
                    Soap_addProps(value, null, children[i], bKeepNamespace);
                }
            }
        }
    }   
}

// End of soap2object.js
