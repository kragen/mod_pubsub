// Copyright 2000-2003 KnowNow, Inc.  All Rights Reserved.

// @KNOWNOW_LICENSE_START@

// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:

// 1. Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.

// 2. Redistributions in binary form must reproduce the above copyright
// notice, this list of conditions and the following disclaimer in
// the documentation and/or other materials provided with the
// distribution.

// 3. The name "KnowNow" is a trademark of KnowNow, Inc. and may not
// be used to endorse or promote any product without prior written
// permission from KnowNow, Inc.

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

// @KNOWNOW_LICENSE_END@

// $Id: perftest_throughput.java,v 1.1 2003/03/05 23:38:55 ifindkarma Exp $

// perftest_throughput.java is a command-line utility.
// It does not require any PubSub client library to run.

// Example of usage:
// $ java perftest_throughput
// usage: perftest_throughput host events threads
// $ java perftest_throughput http://berzerk:8000/kn 100 10
// Thread-0: publishing 10
// Thread-1: publishing 10
// Thread-2: publishing 10
// Thread-3: publishing 10
// Thread-4: publishing 10
// Thread-5: publishing 10
// Thread-6: publishing 10
// Thread-7: publishing 10
// Thread-8: publishing 10
// Thread-9: publishing 10
// Elapsed ms: 2087
// $

import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.net.HttpURLConnection;
import java.net.URL;
import java.util.ArrayList;

class perftest_throughput implements Runnable
{
    //
    // constructor
    //
    private perftest_throughput(String host, int events)
    {
        m_host = host;
        m_events = events;
    }

    //
    // url to send events
    //
    private String m_host;

    //
    // number of events to send
    //
    private int m_events;

    //
    // Runnable implementation
    //
    public void run()
    {
        String name = Thread.currentThread().getName();

        //
        // Provide feedback
        //
        System.out.println(name + ": publishing " + m_events);

        try
        {
            //
            // Create url
            //
            String s = m_host + "?"
                              + "do_method=notify&"
                              + "kn_to=/what/perf&"
                              + "kn_payload=test&"
                              + "kn_id=" + name;

            //
            // Send events
            //
            for (int i = 0; i < m_events; ++i)
            {
                //
                // Create unique URL
                //
                URL url = new URL(s + "_" + i);

                //
                // Create a connection
                //
                HttpURLConnection conn = (HttpURLConnection)url.openConnection();

                //
                // Add keep-alive header
                //
                conn.setRequestProperty("Connection", "Keep-Alive");

                //
                // Connect to the event router
                //
                conn.connect();

                //
                // Bail on error
                //
                if (conn.getResponseCode() != 200)
                {
                    System.err.println(name + ": " + conn.getResponseCode());
                    break;
                }

                //
                // Drain input
                //
                BufferedReader reader = new BufferedReader(
                                            new InputStreamReader(
                                                conn.getInputStream()));

                while (reader.readLine() != null)
                {
                    continue;
                }

                reader.close();
            }
        }
        catch (Exception e)
        {
            System.err.println(e);
        }
    }

    public static void main(String[] args)
        throws Exception
    {
        //
        // Validate command line arguments
        //
        if (args.length < 3)
        {
            System.err.println("usage: perftest_throughput host events threads");
            System.exit(1);
        }

        //
        // Parse command line arguments
        //
        String host = args[0];
        int events = Integer.parseInt(args[1]);
        int threads = Integer.parseInt(args[2]);

        //
        // Create the threads
        //
        ArrayList list = new ArrayList();

        for (int i = (events / threads); threads-- > 0; events -= i)
        {
            //
            // Create a new perf object
            //
            perftest_throughput perf = new perftest_throughput(host, threads > 0 ? i : events);

            //
            // Bind it to a thread
            //
            list.add(new Thread(perf));
        }

        long begin = System.currentTimeMillis();

        //
        // Run the threads
        //
        for (int i = 0; i < list.size(); ++i)
        {
            ((Thread)list.get(i)).start();
        }

        //
        // Wait for threads
        //
        for (int i = 0; i < list.size(); ++i)
        {
            ((Thread)list.get(i)).join();
        }

        long end = System.currentTimeMillis();

        //
        // Print timing information
        //
        System.out.println("Elapsed ms: " + (end - begin));
    }
}

// End of perftest_throughput.java
