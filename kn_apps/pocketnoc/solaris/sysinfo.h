/**
 * sysinfo.h - v1.5
 * author: Paul Sharpe
 *
 * Header to export the CreateSysDictionary function
 **/

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
 **/

/* libkn include */
#include <kn/kn.h>

/**
 * Creates a PubSub dictionary that contains the following parameters 
 * for a PubSub event:
 *
 * - "cpuusage"  : Total percentage of cpu in use on the system
 * - "memtotal"  : Total system memory in MB
 * - "memfree"   : Total free memory in MB
 * - "swaptotal" : Total swap memory in MB
 * - "swapfree"  : Total free swap in MB
 * - "loadavg"   : Load average over the past minute
 * - "numusers"  : Number of users logged on
 *
 * @return A reference to a PubSub dictionary


kn_DictionaryRef CreateSysDictionary(void);
*/

void CpuUsage(knevent_mref knEvent);
void MemoryUsage(knevent_mref knEvent);
void SwapUsage(knevent_mref knEvent);
void LoadAvg(knevent_mref knEvent);
void NumUsers(knevent_mref knEvent);
