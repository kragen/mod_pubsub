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
 * $Id: dstring.h,v 1.2 2004/04/19 05:39:15 bsittler Exp $
 **/

#ifndef _DSTRING_H
#define _DSTRING_H

#include <stdlib.h>
#include <stddef.h>

typedef struct {
    char *start;
    int len;
    int buflen;
} dstring;

/* set up a dstring */
dstring *new_dstring();
/* tear it down */
void dstring_free(dstring *s);

/* ensure that a dstring has enough space to hold N more chars beyond
 * what it currently contains; be very careful in functions that use
 * this */
int dstring_expand(dstring *s, int added_len);

/* When using a dstring as resizable storage for a number of structs
 * or a bitmap, it is often desirable to zero newly-added space.  This
 * function makes sure that the dstring is big enough for the
 * specified number of bytes, zeroing any newly-added length. */
int dstring_vector_ready(dstring *s, size_t newsize);

/* return a pointer one past the last char of a dstring */
char *dstring_end(dstring *s);

/* append a dstring to another dstring */
int dstring_append(dstring *dest, dstring *src);
/* append a null-terminated string to a dstring */
int dstring_append_s(dstring *ds, char *s);
/* append a counted string to a dstring */
int dstring_append_cstr(dstring *s, char *b, int n);

/* is this dstring equal to a null-terminated string? */
int dstring_eq_s(dstring *ds, char *s);
/* is this dstring equal to a counted string? */
int dstring_eq_cstr(dstring *s, char *b, int n);

/* overwrite a dstring with the contents of another dstring */
int dstring_copy(dstring *dest, dstring *src);
/* overwrite a dstring with a null-terminated string */
int dstring_copy_s(dstring *ds, char *s);
/* overwrite a dstring with a counted string */
int dstring_copy_cstr(dstring *ds, char *b, int n);

/* duplicate a dstring */
dstring *dstring_dup(dstring *src);
/* duplicate a C string as a dstring */
dstring *dstring_dup_s(char *src);

/** input and output **/
/* read up to 'maxsize' bytes from file descriptor 'fd' and append them to
 * 'dest'.  Return -1 on error or the number of bytes read. */
int dstring_read(int fd, dstring *dest, int maxsize);
/* write the contents of a dstring to file descriptor 'fd'.  Return -1
 * on error or the number of bytes written. */
int dstring_write(int fd, dstring *src);

/* chop off the first n characters of the dstring.  Use with care;
 * takes time proportional to the length of the remaining part of the
 * dstring. */
int dstring_shift(dstring *ds, int n);

/* find a string in a dstring, starting at 'start'; returns the offset
 * into the dstring where its beginning is found, or -1 if it's not
 * found. */
int dstring_find(dstring *ds, int start, char *needle, int needlelen);
int dstring_find_s(dstring *ds, int start, char *needle);

#endif

