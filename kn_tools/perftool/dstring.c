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
 * $Id: dstring.c,v 1.2 2003/05/06 04:42:16 bsittler Exp $
 **/

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include "util.h"
#include "dstring.h"

static unused char rcsid[] = "@(#) $Id: dstring.c,v 1.2 2003/05/06 04:42:16 bsittler Exp $";

dstring *new_dstring()
{
    dstring *s = (dstring *) malloc(sizeof(dstring));
    if (s) {
        s->start = NULL;
        s->len = 0;
        s->buflen = 0;
    }
    return s;
}

void dstring_free(dstring *s)
{
    if (s) {
        free(s->start);
        free(s);
    }
}

char *dstring_end(dstring *s)
{
    return s->start + s->len;
}

int dstring_expand(dstring *s, int extra_len)
{
    if (s->buflen < s->len + extra_len) {
        int newlen = (s->len + extra_len) * 2;
        char *newbuf = (char *) realloc((void *) s->start, newlen);
        if (!newbuf) return 0;
        s->start = newbuf;
        s->buflen = newlen;
    }
    return 1;
}

/* expand a vector to 'newsize' bytes, zeroing newly-added bytes. */
int dstring_vector_ready (dstring *ds, size_t newsize) {
    if (newsize > ds->len) {
        int oldsize = ds->len;
        if (!dstring_expand(ds, newsize - oldsize)) return 0;
        memset(ds->start + oldsize, '\0', newsize - oldsize);
        ds->len = newsize;
    }
    return 1;
}

int dstring_append_cstr(dstring *s, char *b, int len)
{
    if (!dstring_expand(s, len)) return 0;
    memmove(dstring_end(s), b, len);
    s->len += len;
    return 1;
}

int dstring_append_s(dstring *ds, char *s)
{
    return dstring_append_cstr(ds, s, strlen(s));
}

int dstring_append(dstring *dest, dstring *src)
{
    return dstring_append_cstr(dest, src->start, src->len);
}

int dstring_eq_cstr(dstring *ds, char *b, int n)
{
    return (n == ds->len) && !memcmp(ds->start, b, n);
}

int dstring_eq_s(dstring *ds, char *s)
{
    return dstring_eq_cstr(ds, s, strlen(s));
}

int dstring_copy_cstr(dstring *ds, char *b, int n)
{
    ds->len = 0;
    return dstring_append_cstr(ds, b, n);
}

int dstring_copy_s(dstring *ds, char *s)
{
    return dstring_copy_cstr(ds, s, strlen(s));
}

int dstring_copy(dstring *dest, dstring *src)
{
    return dstring_copy_cstr(dest, src->start, src->len);
}

dstring *dstring_dup(dstring *src)
{
    dstring *dest = new_dstring();
    if (!dest) return NULL;
    if (!dstring_copy(dest, src)) {
        dstring_free(dest);
        return NULL;
    }
    return dest;
}

        
dstring *dstring_dup_s(char *src)
{
    dstring *dest = new_dstring();
    if (!dest) return NULL;
    if (!dstring_copy_s(dest, src)) {
        dstring_free(dest);
        return NULL;
    }
    return dest;
}

int dstring_read(int fd, dstring *dest, int maxsize)
{
    int rv;
    if (!dstring_expand(dest, maxsize)) return -1;
    /* depends on read() being correct not to overflow */
    rv = read(fd, dest->start + dest->len, maxsize);
    if (rv > 0) dest->len += rv;
    assert(dest->len <= dest->buflen);
    return rv;
}

int dstring_write(int fd, dstring *src)
{
    int rv = write(fd, src->start, src->len);
    return rv;
}


int dstring_shift(dstring *ds, int n)
{
    if (ds->len < n || n < 0) return 0;
    memmove(ds->start, ds->start + n, ds->len - n);
    ds->len -= n;
    return 1;
}

/* simple dumb string search algorithm; no KMP or BM here */
/* in perftool, whose bottleneck was this function, we're mostly
 * searching through forests of NULs and x's for strings that contain
 * neither; in_needle skips past them Boyer-Moorishly without
 * computing the Boyer-Moore state tables by checking to see if the
 * character at the end of the current position is a character not
 * found in the needle.  This function is still a major bottleneck in
 * perftool, especially for small numbers of connections, but in_needle
 * improved its speed by a factor of 7.5. */
int dstring_find(dstring *ds, int start, char *needle, int needlelen)
{
    int i, j, last_place_to_look = ds->len - needlelen + 1;
    char in_needle[256];
    memset(in_needle, 0, 256);
    for (i = 0; i < needlelen; i++) in_needle[(int)needle[i]] = 1;
    for (i = start; i < last_place_to_look; i++) {
        if (!in_needle[(int)ds->start[i + needlelen - 1]]) {
            i += needlelen - 1;
        } else {
            for (j = 0; j < needlelen; j++) {
                if (needle[j] != ds->start[i + j]) goto next_pos;
            }
            return i;
        }
      next_pos: /* continue to the next iteration of the loop */ ;
    }
    return -1;
}

int dstring_find_s(dstring *ds, int start, char *needle)
{
    return dstring_find(ds, start, needle, strlen(needle));
}
