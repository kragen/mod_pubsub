/**
 * Copyright (c) 2001-2003 KnowNow, Inc.  All rights reserved.
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
 * $Id: analyze_conn.h,v 1.1 2003/03/21 05:23:56 ifindkarma Exp $
 **/

#ifndef ANALYZE_CONN_H
#define ANALYZE_CONN_H

void initAnalyzeConn(void);

/* store the number of bytes received on a connection */
void connSetBytes(int conn, long nbytes);
/* get the number of bytes received on a connection */
long connGetBytes(int conn);

/* the smallest and largest connection numbers yet known */
int minConn(void);
int maxConn(void);

/* set the number of times we should expect to have received each event */
void expectToReceiveEachEventNTimes(int ntimes);

/* store the datum that a connection received a particular event */
void connReceivedEvent(int conn, int event, double latency);
/* printf out the events received for a particular connection */
void printReceivedEvents(int conn);

/* the smallest and largest event numbers yet known */
int minEvent(void);
int maxEvent(void);

/* How long did it take for the expected number of connections to
 * receive the event?  NaN if not enough connections have. */
double eventFinalLatency(int event);


#endif
