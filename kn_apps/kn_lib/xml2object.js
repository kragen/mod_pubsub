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

// $Id: xml2object.js,v 1.2 2004/04/19 05:39:12 bsittler Exp $

// A library of XML-related utility functions.


/** This finds the attribute with the specified value.
 *  @param element the XML element
 *  @param value the attribute value to search for
 *  @return the name of the attribute, or null if it isn't found.
 */

function findAttributeName(element,value)
{
    for (var i in element.attributes)
    {
        if (typeof element.attributes[i] != 'function')
        {
            if (element.attributes[i] == value)
            {
                return name;
            }
        }
    }
    return null;
}


/** This extracts the namespace prefix from a piece of text. It gets the
 *  text before the ':' character.
 * @param name the text
 */

function getNamespace(name)
{
    var pos;
    var namespace="";
    if (name == null)
        return null;
        
    pos = name.indexOf(":");
    if (pos >=0)
    {
        namespace = name.substring(0,pos);
    }
    
    return namespace;
}


/** Gets first child element that has the tag name of 'name'
 *  @param element the XML element
 *  @param name the tagname to look for
 */

function getChildByTagName(element,name)
{
    var node=null;

    for (var i =0; i < element.childNodes.length; i++)
    {
        node = element.childNodes[i];
        if ((node.nodeType == "element") && 
            (node.tagName == name || name == "*"))
        {
            break;
        }
    }
    return node;
}


/** Gets list of immediate child elements filtered by tag name.
 *  @param element the XML element
 *  @param name the tagname to look for
 */

function getChildrenByTagName(element,name)
{
    var nodes=new Array;
    var node;

    for (var i =0; i < element.childNodes.length; i++)
    {
        node = element.childNodes[i];
        if ((node.nodeType == "element") && 
            (node.tagName == name))
        {
            nodes[nodes.length] = node;
        }
    }
    return nodes;
}


/** Recursive function to add named properties to a javascript object.
 *  @param obj JavaScript object that will hold the properties
 *  @param node XML Element that holds the names, data and child nodes
 *  @param bKeepNamespace determines if namespace prefix is trimmed from tag names.
 */

function addProps(obj,node,bKeepNamespace)
{
    if (node == null)
        return;

    if (node.nodeType == "element")
    {
        var name;
        var value;
        
        name = getNodeName(node,bKeepNamespace);
        
        // Look for pure-text elements.
        if (isDateElement(node))
        {
            value = getElementDate(node);
            obj[name] = value;
        }
        else
        if (isTextElement(node))
        {
            // Add as property.
            value = getElementText(node);
            obj[name] = value;
        }
        else
        {
            // Create a placeholder property & add children.
            value = new Object;
            
            // If there is already this type, add at end of array.
            obj[name] = value;    
            
            for (var i = 0; i < node.childNodes.length; i++)
            {
                addProps(value,node.childNodes[i],bKeepNamespace);
            }
        }
    }   
}


/**
 * xmlns handling (XML namespaces)
 */


/* Find the namespace for an XMLNode */

function findNamespace(node, tagName)
{
    var prefix = getNamespace(tagName);
    var xmlns = "xmlns";
    if (prefix != "")
        xmlns = xmlns + ":" + prefix;
    return findNamespaceAttribute(node, xmlns);
}


/* Recursively find an xmlns[:prefix] attribute for an XMLNode and
   resolve it to a URI. */

function findNamespaceAttribute(node, xmlns)
{
    var rv = node.getAttribute(xmlns);
    if (rv) return rv;
    var parent = node.getParent();
    if (parent) return findNamespaceAttribute(parent, xmlns);
    return defaultNamespace(getName(xmlns));
}


/* (Private) table of known namespace URIs indexed by common prefix. */

_defaultNamespace = new Object();
_defaultNamespace["SOAP"] = "http://schemas.xmlsoap.org/soap/envelope/";
_defaultNamespace["SOAP-ENV"] = _defaultNamespace["SOAP"];
_defaultNamespace["SOAP-ENC"] = "http://schemas.xmlsoap.org/soap/encoding/";
_defaultNamespace["xsi"] = "http://www.w3.org/1999/XMLSchema-instance";
_defaultNamespace["xsd"] = "http://www.w3.org/1999/XMLSchema";


/* Look up a namespace URI for a common prefix. */

function defaultNamespace(prefix)
{
    return _defaultNamespace[prefix];
}


/* Given an XMLNode, a (possibly qualified) tag or attribute name
   relative to that node, and another tag or attribute name qualified
   with a known common prefix, isSameName returns true only when the
   two names are equivalent and in the same namespace. */

function isSameName(node, name, otherName)
{
    if (! name)
        return false;
    if (getName(name) != getName(otherName))
        return false;
    var otherUri = defaultNamespace(getNamespace(otherName));
    var uri = findNamespace(node, name);
    return otherUri == uri;
}


/* Look for an attribute with a given name according to isSameName. */

function getSameAttribute(node, name)
{
    var names = node.getAttributeNames();
    for (var i = 0; i < names.length; i++)
    {
        if (isSameName(node, names[i], name))
            return node.getAttribute(names[i]);
    }
    return void null;
}


/* Look for sub-nodes that are nothing but chardata (text). */

function isTextElement(element)
{
    var children = element.getElements();
    for (var i = 0; i < children.length; i++)
    {
        if ((children[i].nodeType != "TEXT") &&
            (children[i].nodeType != "CDATA"))
            return false;
    }
    return true;
}


function isDateElement(element)
{
    return isSameName(element,
                      getSameAttribute(element, "xsi:type"),
                      "xsd:timeInstant");
}


/* Concatenate all the values of an elements child nodes. Only does
 'chardata'. */

function getElementText(element)
{
    var value="";
    var node;
    
    for (var i = 0; i < element.childNodes.length; i++)
    {
        node = element.childNodes[i];
        if (node.nodeType == "chardata")
        {
            if (value == null) value = "";
            // don't add whitespace, unless you are a pathetic moron.
            value += node.nodeValue;
        }
    }
    if (value)
        value = new String(value);

    return value;
}


/* Parse date text into a JavaScript Date. */

function getElementDate(element)
{
    var text;
    var value;

    var info;
    var date_info;
    var time_info;

    var year=0;
    var month=0;
    var date=0; 
    var hours=0; 
    var minutes=0;
    var seconds=0;
    var ms=0;
    
    iso8601 = getElementText(element);

    // TODO: Parse based on ISO8601 format.
    // XML Schema: timeDuration: PnYnMnDTnHnMnS : P1Y2M3DT10H30M
    // XML Schema: recurringDuration: CCYY-MM-DDThh:mm:ss.sss[Zhh:mm] : 1999-06-04T12:00:00
    // 
    // new Date(year, month, date[, hours[, minutes[, seconds[,ms]]]]) 

    info = iso8601.split('T');
    date_info = info[0].split('-');
    
    if (info.length > 1)
    {
        // extract stuff before TimeZone
        info = info[1].split('Z');
        time_info = info[0].split(':');
    }

    year = parseInt(date_info[0]);
    month = parseInt(date_info[1])-1;
    date = parseInt(date_info[2]);

    if (time_info)
    {
        hours = parseInt(time_info[0]);
        minutes = parseInt(time_info[1]);
        
        // extract milliseconds
        info = time_info[2].split('.');
        seconds = parseInt(info[0]);
        ms = parseInt(info[1]);

        if (isNaN(ms))
            ms = 0;
    }

    value = new Date(year,month,date,hours,minutes,seconds,ms);
    
    return value;
}


/* Trims the leading namespace prefix. */

function getNodeName(node,bKeepNamespace)
{
    if (node.nodeType == "ELEMENT")
    {
        if (bKeepNamespace)
        {
            return node.tagName;
        }
        else
        {
            return getName(node.tagName);
        }
    }
    return null;
}


function getName(text)
{
    var pos;
    pos = text.indexOf(":");
    return text.substr(pos+1);
}


function getNamespace(text)
{
    var pos;
    pos = text.indexOf(":");
    if (pos > 0)
        return text.substr(0,pos);
    return null;
}


function getNodeNamespace(node)
{
    if (node.nodeType == "element")
    {
        return getNamespace(node.tagName);
    }
    return null;
}


// End of xml2object.js
