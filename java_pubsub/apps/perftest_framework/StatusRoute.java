
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
 * $Id: StatusRoute.java,v 1.1 2003/08/15 23:41:07 ifindkarma Exp $
 *
 **/

//
// StatusRoute.java -
//
//	Report on the status of creating routes (true for success or
//	false for failure).
//
//	Note: All fields in this object must have types that can
//	      be serialized by KSerializer.
//
//	Copyright (c) 2002 - KnowNow, Inc.

import java.lang.*;

public class StatusRoute
{
    public String	agentID;
    public String	routerURI;
    public String	sourceTopic;
    public String	journalTopic;
    public int		status;

    //
    // StatusRoute::StatusRoute -
    //
    //		Constructors.
    //

    StatusRoute()
    {
	agentID = new String();
	status	= 0;
    }

    StatusRoute(String  agentID,
		String  routerURI,
		String  sourceTopic,
		String  journalTopic,
		boolean status)
    {
	this.agentID      = agentID;
	this.routerURI    = routerURI;
	this.sourceTopic  = sourceTopic;
	this.journalTopic = journalTopic;
	this.status       = status ? 1 : 0;
    }    
}
