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
 * $Id: doubletime.c,v 1.3 2004/04/19 05:39:14 bsittler Exp $
 **/

#include <time.h>
#include <math.h>
#include <sys/time.h>
#include <unistd.h>
#include <errno.h>
#include "util.h"
#include "doubletime.h"

static unused char rcsid[] = "@(#) $Id: doubletime.c,v 1.3 2004/04/19 05:39:14 bsittler Exp $";

/* IEEE-754 8-byte double has 53 mantissa bits --- enough to not
   lose any precision with 32 bits of tv_sec and 20 bits of tv_usec */
double gettimeofday_double() 
{
    struct timeval now_tv;
    gettimeofday(&now_tv, NULL);
    return now_tv.tv_sec + (now_tv.tv_usec / 1e6);
}

double sleep_double(double how_long)
{
  struct timespec ts[2];
  int rv;
  ts[0].tv_sec = floor(how_long);
  ts[0].tv_nsec = (how_long - floor(how_long)) * 1e9;
  while (ts[0].tv_nsec < 0L)
    {
      ts[0].tv_sec --;
      ts[0].tv_nsec += 1000000000L;
    }
  while (ts[0].tv_nsec >= 1000000000L)
    {
      ts[0].tv_sec ++;
      ts[0].tv_nsec -= 1000000000L;
    }
  ts[1].tv_sec = 0;
  ts[1].tv_nsec = 0;
  how_long = 0.0;
  do
    {
      how_long += ts[1].tv_sec + (ts[1].tv_nsec / 1e9);
      rv = nanosleep(ts, ts + 1);
    }
  while (rv == -1 && errno == EINTR);
  return how_long;
}
