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

// $Id: object2xml.js,v 1.1 2002/11/07 07:08:09 troutgirl Exp $

// Recursive function that output XML text from all the properties of a
// JavaScript object. 

// This does not detect cycles (sub-objects that have already been
// output), so it could get caught in an infinite loop.

// The format of the output will be built in a way that Xml2Object will 
// re-create the same property structure.

// This format currently does not use attributes.

function Object2Xml(obj)
{
    var xml="";
    
    var tagname=null;
    xml += Object2Xml_dump(obj,null);
    
    return xml;
}

function Object2Xml_isArray(obj)
{
    if (!obj || (typeof obj.length == (void 0)))
        return false;

    if (obj.constructor == Array)
        return true;
    
    if (obj.constructor == String)
        return false;

    var bFound = false;
    for (var i in obj)
    {
        if (typeof obj[i] == 'function')
            continue;

        if (isNaN(parseInt(i)))
        {
            return false;
        }
        bFound = true;
    }
    return bFound;
}

function Object2Xml_hasProps(obj)
{
    for (var i in obj)
    {
        if ((typeof obj[i] != 'function') && 
            ((typeof i == 'string') && 
             (i.charAt(0) != '$')))
        {
            return true;
        }
    }
    return false;
}

function Object2Xml_dump(obj,parentTag)
{
    var xml = "";
    
    // Do children  

    for (var name in obj)
    {
        var value;
        var tagname=name;
        var type=null;
        var isArray=false;
        
        // Skip system properties

        if ((typeof name == 'string') && 
            (name.charAt(0) == '$'))
            continue;
        
        value = obj[name];
        if (parentTag)
            tagname = parentTag;

        if (typeof value != 'function')
        {
            if ((value != null) && !Object2Xml_hasProps(value))
            {
                // look for Date()
                type = Object2Xml_getType(value);
            }
            
            isArray=Object2Xml_isArray(value);
            if (isArray)
            {
                tagname = name;
            }
            else
            {
                if (type)
                    xml += "<"+tagname+" xsi:type='xsd:"+type+"'>";
                else
                    xml += "<"+tagname+">";
            }
            
            if (Object2Xml_isNodal(value) || isArray)
            {
                xml += "\n";
                if (isArray)
                    xml += Object2Xml_dump(value,tagname);
                else
                    xml += Object2Xml_dump(value,null);
            }
            else
            {
                if (value != null)
                {
                    if (type == "timeInstant")
                        xml += Object2Xml_getTimeValue(value);
                    else
                        xml += value.toString();
                }
            }
            
            if (!isArray)
            {
                xml += "</"+tagname+">\n";
            }
        }
    }
    return xml;
}

function Object2Xml_isNodal(value)
{
    if (value == null)
        return false;
        
    if ((typeof value == 'string') ||
        (typeof value == 'number') ||
        (typeof value == 'boolean'))
    {
        return false;
    }
    
    if ((value.constructor == String) ||
        (value.constructor == Number) ||
        (value.constructor == Boolean) ||
        (value.constructor == Date))
    {
        return false;
    }

    if (Object2Xml_isArray(value))
    {
        return true;
    }
    
    if (!Object2Xml_hasProps(value))
    {
        return false;
    }
    
    return true;
}

function Object2Xml_getType(value)
{
    var type;
    
    if (value == null) 
        return null;
    
    if (value.constructor == Date)
    {
        type = "timeInstant";
    }
    else
    if ((typeof value == "number") ||
        (value.constructor == Number))
    {
        if (value.toString().indexOf('.') >= 0)
        {
            type = "decimal";
        }
        else
        {
            type = "int";
        }
    }
    else
    if ((typeof value == "boolean") ||
        (value.constructor == Boolean))
    {
        type = "boolean";
    }
    else
    if (value.$ && value.$.attributes)
    {
        type = value.$.attributes['xsi:type'];
        if (type)
        {
            type = getName(type);
        }
        else
        if (value.$.namespace == "SOAP-ENC")
        {
            type = getName(value.$.tagName);
        }
    }
    return type;
}

function Object2Xml_getTimeValue(value)
{
    var text="";
    
    text += value.getFullYear();
    text += "-";
    text += value.getMonth();
    text += "-";
    text += value.getDate();
    text += "T";
    text += value.getHours();
    text += ":";
    text += value.getMinutes();
    text += ":";
    text += value.getSeconds();
    text += ".";
    text += value.getMilliseconds();

    // no time zone yet.

    return text;
}

// End of object2xml.js
