/**
 * Copyright 2001-2004 KnowNow, Inc.  All rights reserved.
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
 * $Id: dstr_interp.c,v 1.2 2004/04/19 05:39:14 bsittler Exp $
 **/

#include <stdio.h>
#include <assert.h>
#include <stdarg.h>
#include <string.h>
#include "util.h"
#include "dstring.h"

static unused char rcsid[] = "@(#) $Id: dstr_interp.c,v 1.2 2004/04/19 05:39:14 bsittler Exp $";

/* a sort of subset of vsprintf.  %s is a NUL-terminated C string; %k is a 
 * dstring; %d is an integer; %g is a double. */
char interpbuf[15];
int dstring_append_vinterp_cstr(dstring *dest, char *fmtbuf, int fmtlen, 
                                va_list args)
{
    int si = 0; /* source index */
    char *next;
    int len;
    while (si < fmtlen) {
        next = memchr(fmtbuf + si, '%', fmtlen - si);
        if (next == NULL) {
            return dstring_append_cstr(dest, fmtbuf + si, fmtlen - si);
        } else {
            if (!dstring_append_cstr(dest, fmtbuf + si, next - (fmtbuf + si))) 
                return 0;
            if (next == fmtbuf + fmtlen - 1) { /* the last char */
                return dstring_append_s(dest, "(unterminated final % sign)");
            } else {
                double tmp;
                switch(next[1]) {
                case 's':
                    if (!dstring_append_s(dest, va_arg(args, char*))) 
                        return 0;
                    break;
                case 'k':
                    if (!dstring_append(dest, va_arg(args, dstring *)))
                        return 0;
                    break;
                case 'd':
                    len = snprintf(interpbuf, sizeof(interpbuf), "%d", 
                                   va_arg(args, int));
                    if (len != strlen(interpbuf))  /* shouldn't happen */
                        return 0;
                    if (!dstring_append_s(dest, interpbuf)) return 0;
                    break;
                case 'g':
                    len = snprintf(interpbuf, sizeof(interpbuf), "%g",
                                   va_arg(args, double));
                    if (len != strlen(interpbuf)) /* again, shouldn't happen */
                        return 0;
                    if (!dstring_append_s(dest, interpbuf)) return 0;
                    break;
                case 'f':
                    tmp = va_arg(args, double);
                    /* find out how much space is needed */
                    len = snprintf(interpbuf, sizeof(interpbuf), "%f",
                                   tmp);
                    /* ensure we have enough */
                    if (!dstring_expand(dest, len + 1)) return 0;
                    len = snprintf(dest->start + dest->len, 
                                   dest->buflen - dest->len, 
                                   "%f", tmp);
                    /* now that we have enough, we should *always* succeed */
                    if (len != strlen(dest->start + dest->len)) abort();
                    dest->len += len;
                    break;
                case '%':
                    if (!dstring_append_cstr(dest, "%", 1)) return 0;
                    break;
                default:
                    interpbuf[0] = next[1];
                    interpbuf[1] = '\0';
                    if (!dstring_append_s(dest, "(unrecognized escape %"))
                        return 0;
                    if (!dstring_append_s(dest, interpbuf)) return 0;
                    if (!dstring_append_s(dest, ")")) return 0;
                    break;
                }
                si = next - fmtbuf + 2;  /* skip % and next char */
            }
        }
    }
    /* we get here if a %char pair was the last thing in the string */
    return 1;
}

dstring *dstring_vinterp_cstr(char *fmt, int fmtlen, va_list args)
{
    int retval;
    dstring *dest = new_dstring();
    if (!dest) return dest;
    retval = dstring_append_vinterp_cstr(dest, fmt, fmtlen, args);
    if (!retval) {
        dstring_free(dest);
        return NULL;
    }
    return dest;
}

dstring *dstring_interp_s(char *fmt, ...)
{
    dstring *retval;
    va_list args;
    va_start(args, fmt);
    retval = dstring_vinterp_cstr(fmt, strlen(fmt), args);
    va_end(args);
    return retval;
}

dstring *dstring_interp_cstr(char *fmt, int fmtlen, ...)
{
    dstring *retval;
    va_list args;
    va_start(args, fmtlen);
    retval = dstring_vinterp_cstr(fmt, fmtlen, args);
    va_end(args);
    return retval;
}
