
/**
 * Copyright 2000-2004 KnowNow, Inc.  All rights reserved.
 *
 * @KNOWNOW_LICENSE_START@
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 
 * 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 * 
 * 3. Neither the name of the KnowNow, Inc., nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * @KNOWNOW_LICENSE_END@
 * 
 *
 * $Id: KSerializer.java,v 1.2 2004/04/19 05:39:10 bsittler Exp $
 *
 **/

//
// KSerializer.java -
//
//	Serialize a simple Java object into a string suitable
//	for use as a kn_payload, and back.
//
//	A simple Java object has the following characteristics:
//
//	* It contains no references to other objects.
//	* It has public fields of the following types:
//		String
//		int
//
//	The serialization syntax is as follows:
//
//	object:CLASS_NAME([FIELD=VALUE]*)
//
//	Each field is a field name, and each value is an encoded form
//	of the field value. The literal string "object:" is always
//	found at the start.
//
//	Copyright 2002-2004 KnowNow, Inc.  All rights reserved.
//

import java.lang.*;
import java.lang.reflect.*;

class KSerializer
{
    //
    // Serialize -
    //
    //		Serialize the given object. Raise an Exception
    //		if unusual fields are encountered.
    //

    static
	String Serialize(Object o)
	throws Exception
    {
	StringBuffer payload = new StringBuffer();
	
	// Get basic info about the class
	Class  oc = o.getClass();
	String cn = oc.getName();

	// Start payload with "object:" and class name
	payload.append("object:");
	payload.append(cn);
	payload.append("(");

	// Get the public fields
	Field[] of = oc.getFields();

	// Process each field
	for (int i = 0; i < of.length; i++)
	{
	    Field  f = of[i];

	    // Get class name, field type, and name of field type
	    String fn = f.getName();
	    Class  fc = f.getType();
	    String tn = fc.getName();

	    if (tn.equals("java.lang.String"))
	    {
		try
		{
		    String sv = (String) f.get(o);
		    payload.append(fn + "=" + Encode(sv));
		}
		catch (Exception e)
		{
		    throw e;
		}
	    }
	    else
	    if (tn.equals("int"))
	    {
		try
		{
		    Integer iv = (Integer) f.get(o);
		    String sv  = iv.toString();
		    payload.append(fn + "=" + Encode(sv));
		}
		catch (Exception e)
		{
		    throw e;
		}
	    }
	    else
	    {
		throw new Exception("Unimplemented object field type");
	    }

	    if (i != (of.length - 1))
	    {
		payload.append(",");
	    }
	}
	
	payload.append(")");

	return payload.toString();
    }

    //
    //	Instantiate -
    //
    //		Instantiate a Java object from the given
    //		serialized string.
    //

    static
	Object Instantiate(String payload)
	throws Exception
    {
	// Make sure that this looks like a serialized object
	if (!payload.startsWith("object:"))
	{
	    throw new Exception("Not a serialized object");
	}

	// Isolate the class name
	int op = payload.indexOf('(');
	int cp = payload.indexOf(')');
	if ((op == -1) || (cp == -1))
	{
	    throw new Exception("No class name found in payload");
	}
	String cn = payload.substring(7, op);

	// Create a class object, and use it to create the object
	Class  co = Class.forName(cn);
	Object o  = co.newInstance();

	// Now process and set the field values
	String   a  = payload.substring(op + 1, cp);
	String[] al = a.split(",");

	for (int i = 0; i < al.length; i++)
	{
	    String[] vl = al[i].split("=");

	    if (vl.length == 2)
	    {
		// Get field object and string value for it
		Field  f = co.getField(vl[0]);
		String v = Decode(vl[1]);

		// Get class name, field type, and name of field type
		String fn = f.getName();
		Class  fc = f.getType();
		String tn = fc.getName();

		if (tn.equals("java.lang.String"))
		{
		    f.set(o, v);
		}
		else
		if (tn.equals("int"))
		{
		    f.setInt(o, Integer.parseInt(v));
		}
		else
		{
		    throw new Exception("Unimplemented object field type");
		}
	    }
	    else
	    {
		throw new Exception("Malformed serialized data");
	    }
	}

	return o;
    }

    //
    // Encode -
    //
    //	Return a hex-encoded form of the given string. Raises
    //  an exception if a character of the string does not
    //	fit into 8 bits.
    //

    static private
	String Encode(String s)
	throws Exception
    {
	StringBuffer e = new StringBuffer();

	// Put initial marker
	e.append("[");

	for (int i = 0; i < s.length(); i++)
	{
	    char   ch   = s.charAt(i);
	    int    cv   = ch;
	    
	    if (cv > 255)
	    {
		throw new Exception("Character won't fit in 1 byte");
	    }

	    String cvx = Integer.toHexString(cv);

	    if (cvx.length() < 2)
	    {
		e.append("0" + cvx);
	    }
	    else
	    {
		e.append(cvx);
	    }
	}

	// Put final marker
	e.append("]");

	// Return the string
	return e.toString();
    }

    //
    //	Decode -
    //
    //		Decode the hex-encode string back in to its
    //		original form.
    //

    static private
	String Decode(String s)
	throws Exception
    {
	StringBuffer d = new StringBuffer();

	// Sanity check
	if ((s.length() % 2) != 0)
	{
	    throw new Exception("String must be multiple of 2 chars long");
	}

	if (!s.startsWith("[") || !s.endsWith("]"))
	{
	    throw new Exception("Encoding markers not found");
	}

	// Process the string 2 characters at a time
	for (int i = 1; i < s.length() - 1; i += 2)
	{
	    String x  = s.substring(i, i + 2);
	    int    cv = Integer.parseInt(x, 16);
	    d.append(new Character((char) cv));
	}

	return d.toString();
    }
}

