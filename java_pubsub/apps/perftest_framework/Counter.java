
/**
 * Copyright (c) 2000-2003 KnowNow, Inc.  All rights reserved.
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
 * notice, this list of conditions and the following disclaimer in
 * the documentation and/or other materials provided with the
 * distribution.
 * 
 * 3. The name "KnowNow" is a trademark of KnowNow, Inc. and may not
 * be used to endorse or promote any product without prior written
 * permission from KnowNow, Inc.
 * 
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESSED OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL KNOWNOW, INC. OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
 * IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * @KNOWNOW_LICENSE_END@
 *
 * $Id: Counter.java,v 1.1 2003/08/15 23:41:06 ifindkarma Exp $
 *
 **/

//
// Counter.java -
//
// 	Implements an object which counts the number of messages 
//	received for a given named counter.
//
//	Copyright (c) 2002 - KnowNow, Inc.
//

import java.lang.*;
import java.util.*;

public class Counter
{
    // Instance variables
    private String	name;
    private int		count;

    //
    // Counter::Counter -
    //
    //		Constructors.
    //

    Counter()
    {
	name = new String();
	count = 0;
    }

    Counter(String name)
    {
	this.name = name;
	count      = 0;
    }

    //
    // Counter::Reset -
    //
    //		Reset the counter.
    //

    public void Reset()
    {
	count = 0;
    }

    // 
    // Counter::Increment -
    //
    //		Bump up the count.
    //

    public void Increment()
    {
	count++;
    }

    //
    //	Counter::GetName -
    //	Counter::GetCount -
    //
    //		Accessors.
    //

    public String GetName()
    {
	return name;
    }

    public int GetCount()
    {
	return count;
    }
}


