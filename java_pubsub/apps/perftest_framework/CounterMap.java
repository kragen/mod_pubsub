
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
 * $Id: CounterMap.java,v 1.2 2004/04/19 05:39:10 bsittler Exp $
 *
 **/

//
// CounterMap.java -
//
//	Implements a map from a topic name (a String) to a counter
//	name (another String) and finally into a Counter.
//
//	Copyright 2002-2004 KnowNow, Inc.  All rights reserved.
//

import java.lang.*;
import java.util.*;
import java.io.*;

public class CounterMap
{
    // Class variables
    private HashMap     topicMap;
    private HashMap	counterMap;

    //
    // CounterMap::CounterMap -
    //
    //		Constructor.
    //

    CounterMap()
    {
	topicMap   = new HashMap();
	counterMap = new HashMap();
    }

    //
    // CounterMap::GetCounterKeySet -
    //
    //		Return a Set for iteration over the counters in
    //		the CounterMap.

    public Set GetCounterKeySet()
    {
	return counterMap.keySet();
    }

    //
    // CounterMap::GetTopicKeySet -
    //
    //		Return a Set for iteration over the topics in
    //		the CounterMap.
    //

    public Set GetTopicKeySet()
    {
	return topicMap.keySet();
    }

    //
    //	CounterMap::GetCounter -
    //
    //		Establish a mapping from the given topic (which
    //		must not exist) to the given counter (which may
    //		or may not exist). Return the Counter object.
    //

    synchronized Counter GetCounter(String topic, String counterName)
    {
	System.out.println("GetCounter(" + topic + "," + counterName + ")");

	// First, get the Counter
	Counter counter = GetCounter(counterName);

	// Now map the topic to the counter name
	topicMap.put(topic, counterName);

	return counter;
    }

    //
    //	CounterMap::GetCounterForRoute -
    //
    //		Return the counter for the given "from" route (which ends
    //		in a topic name) by performing and "endsWith" mapping on
    //		the route.
    //

    synchronized Counter GetCounterForRoute(String route)
    {
	Set      topicSet         = topicMap.keySet();
	Iterator topicSetIterator = topicSet.iterator();

	while (topicSetIterator.hasNext())
	{
	    String  topic = (String) topicSetIterator.next();

	    System.out.println("GetCounterForRoute: " + topic + " ? " + route + ")");

	    if (route.endsWith(topic))
	    {
		String name = (String) topicMap.get(topic);
		return GetCounter(name);
	    }
	}

	return null;
    }

    //
    //	CounterMap::GetCounter -
    //
    //		Return the Counter for the given counter name,
    //		creating it first if necessary.
    //

    synchronized Counter GetCounter(String name)
    {
	try
	{
	    if (counterMap.containsKey(name))
	    {
		return (Counter) counterMap.get(name);
	    }
	    else
	    {
		Counter counter = new Counter(name);
		counterMap.put(name, counter);

		return counter;
	    }
	}
	catch (Exception Ex)
	{
	    Common.DumpException("CounterMap", Ex);
	    return null;
	}
    }

    //
    //	CounterMap::DeleteCounters -
    //
    //		Delete all of the topics and counters in the map.
    //

    void DeleteCounters()
    {
	topicMap.clear();
	counterMap.clear();
    }

    //
    //	CounterMap::ResetCounters -
    //
    //		Reset all counters to 0.
    //

    void ResetCounters()
    {
	Set      counterSet         = counterMap.keySet();
	Iterator counterSetIterator = counterSet.iterator();

	while (counterSetIterator.hasNext())
	{
	    String  name    = (String)  counterSetIterator.next();
	    Counter counter = (Counter) counterMap.get(name);
	    counter.Reset();
	}
    }

    //
    //	CounterMap::Dump -
    //
    //		Dump out all the counters in text format to the given
    //		PrintStream.
    //

    void Dump(PrintStream outStream)
    {
	Set      counterSet         = counterMap.keySet();
	Iterator counterSetIterator = counterSet.iterator();

	outStream.println("    Count              Name");
	outStream.println(" ---------- ------------------------------");

	while (counterSetIterator.hasNext())
	{
	    String  name     = (String) counterSetIterator.next();
	    Counter counter  = (Counter) counterMap.get(name);
	    String  strCount = Integer.toString(counter.GetCount());
	    
	    outStream.println(Common.RightJustify(strCount, 10) + " " +
			      counter.GetName());
	}

	outStream.println();
    }

}
