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
 * $Id: post_event.h,v 1.1 2003/03/21 05:23:56 ifindkarma Exp $
 **/

#ifndef __POST_EVENT_H
#define __POST_EVENT_H

#include "dstring.h"

/* constructs an HTTP message containing a request to post one or more
 * events.  dest_topic is the destination topic (i.e. the local-part
 * of the URL minus the leading "/kn/"), serial is the kn_id of the
 * first event to post, payloadsize is the number of 'x' bytes to
 * include in the payload, and batch specifies the size of the batch.
 * If it's 0, it means to just post a single event without putting it
 * in a batch.
 * */
dstring *post_event(char *dest_topic, int serial, int payloadsize, int batch);

#endif
