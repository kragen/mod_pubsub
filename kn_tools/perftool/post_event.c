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
 * $Id: post_event.c,v 1.1 2003/03/21 05:23:56 ifindkarma Exp $
 **/

#include "util.h"
#include "dstring.h"
#include "dstr_interp.h"
#include "urlencode.h"

static unused char rcsid[] = "@(#) $Id: post_event.c,v 1.1 2003/03/21 05:23:56 ifindkarma Exp $";

static dstring *event(int serial, int payloadsize)
{
    int i;
    dstring *body = dstring_interp_s("kn_id=%d&kn_payload=", serial);
    if (!body) return 0;
    for (i = 0; i < payloadsize; i++)
    {
        if (!dstring_append_s(body, "x"))
        {
            dstring_free(body);
            return 0;
        }
    }
    return body;
}

/* append an event to a batch. */
static int append_event(dstring *dest, int serial, int payloadsize, 
                        char *topic)
{
    dstring *s = event(serial, payloadsize);
    int rv = 0;
    
    if (!s) return 0;
    rv = (dstring_append_s(s, "&do_method=notify&kn_to=") &&
          dstring_append_urlencoded_s(s, topic) &&
          dstring_append_urlencoded(dest, s));
    dstring_free(s);
    return rv;
}

dstring *post_event(char *dest_topic, int serial, int payloadsize, int batch)
{
    dstring *body, *message = 0;
    if (!batch)
    {
        body = event(serial, payloadsize);
    }
    else
    {
        int i = 0;
        body = new_dstring();
        if (!body) return 0;
        if (!dstring_append_s(body, "do_method=batch")) goto cleanup;
         
        for (i = 0; i != batch; i++)
        {
            if (!dstring_append_s(body, "&kn_batch=")) goto cleanup;
            if (!append_event(body, serial + i, payloadsize, dest_topic)) 
                goto cleanup;
        }
    }
    
    message = dstring_interp_s("POST /kn/%s HTTP/1.0\r\n"
                               "Content-Type: x-www-form-urlencoded\r\n"
                               "Content-Length: %d\r\n"
                               "\r\n"
                               "%k", dest_topic, body->len, body);
  cleanup:
    dstring_free(body);
    return message;
}
