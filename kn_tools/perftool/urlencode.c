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
 * $Id: urlencode.c,v 1.1 2003/03/21 05:23:56 ifindkarma Exp $
 **/

#include <string.h>
#include "urlencode.h"
#include "util.h"

static unused char rcsid[] = "@(#) $Id: urlencode.c,v 1.1 2003/03/21 05:23:56 ifindkarma Exp $";

int dstring_append_urlencoded(dstring *dest, dstring *src)
{
    return dstring_append_urlencoded_cstr(dest, src->start, src->len);
}

int dstring_append_urlencoded_s(dstring *dest, char *src)
{
    return dstring_append_urlencoded_cstr(dest, src, strlen(src));
}

static int safe_is_initialized = 0;
static char safe[256] = { 0 };
static char buf[3] = "%";
static char *hexdigits = "0123456789ABCDEF";

int dstring_append_urlencoded_cstr(dstring *dest, char *s, int len)
{
    if (!safe_is_initialized)
    {
        char *d = ("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_"
                   "0123456789-~");
        while (*d)
        {
            safe[(int)*d] = 1;
            d++;
        }
        safe_is_initialized = 1;
    }
    
    for (; len != 0; s++, len--)
    {
        if (safe[(int)*s]) 
        {
            if (!dstring_append_cstr(dest, s, 1)) return 0;
        }
        else
        {
            buf[1] = hexdigits[(int)(unsigned char)*s >> 4];
            buf[2] = hexdigits[(int)*s & 0x0f];
            if (!dstring_append_cstr(dest, buf, 3)) return 0;
        }
    }
    return 1;
}
