
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
 * $Id: CommandRoute.java,v 1.2 2004/04/19 05:39:09 bsittler Exp $
 *
 **/

//
// CommandRoute.java -
//
//	Tell a single Agent to create a series of routes. 
//
//	Note: All fields in this object must have types that can
//	      be serialized by KSerializer.
//
//	Copyright 2002-2004 KnowNow, Inc.  All rights reserved.
//

import java.lang.*;

public class CommandRoute
{
    public String	agentID;
    public String 	routerURI;
    public String	sourceTopic;
    public String	journalTopic;
    public int		seqStart;
    public int		seqEnd;
    public int		numPublishers;

    //
    // CommandRoute::CommandRoute -
    //
    //		Constructors.
    //

    CommandRoute()
    {
	agentID       = new String();
	routerURI     = new String();
	sourceTopic   = new String();
	journalTopic  = new String();
	seqStart      = 0;
	seqEnd        = 0;
	numPublishers = 0;
    }

    CommandRoute(String agentID,
		 String routerURI,
		 String	sourceTopic,
		 String	journalTopic,
		 int	seqStart,
		 int	seqEnd,
		 int	numPublishers)
    {
	this.agentID       = agentID;
	this.routerURI     = routerURI;
	this.sourceTopic   = sourceTopic;
	this.journalTopic  = journalTopic;
	this.seqStart      = seqStart;
	this.seqEnd        = seqEnd;
	this.numPublishers = numPublishers;
    }    
}
