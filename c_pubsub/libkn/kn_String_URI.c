/**
 * PubSub Client Library
 * URI string routines
 *
 * Wilfredo Sanchez
 **/
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
 **/

#include "kn_config.h"

RCSID("$Id: kn_String_URI.c,v 1.3 2004/04/19 05:39:08 bsittler Exp $");

#include <stdlib.h>
#include <limits.h>
#include <string.h>

#include "kn_Object.h"
#include "kn_String_URI.h"

/**
 * Accessors
 **/

kn_Protocol kn_StringGetProtocolFromName (kn_StringRef aName)
{
  kn_StringRef anHTTP    = kn_StringCreateWithCString("http" );
  kn_StringRef anHTTPS   = kn_StringCreateWithCString("https");
  kn_Protocol  aProtocol = kn_PROTOCOL_UNKNOWN;

  if (!(kn_StringCompareCaseInsensitive(aName, anHTTP   ))) aProtocol = kn_PROTOCOL_HTTP    ;
  if (!(kn_StringCompareCaseInsensitive(aName, anHTTPS)))   aProtocol = kn_PROTOCOL_HTTPS;

  kn_Release (anHTTP   );
  kn_Release (anHTTPS);

  return aProtocol;
}

kn_StringRef kn_StringCreateWithProtocolFromURI (kn_StringRef aURI)
{
  const char* aStart = kn_StringGetBytes(aURI);
  const char* anEnd  = aStart + kn_StringGetSize(aURI);

  if (!aStart) return NULL;

  if (!(anEnd = memchr(aStart, ':', anEnd-aStart))) return NULL; /* Ends at ':' */

  return kn_StringCreateWithBytes(aStart, anEnd-aStart);
}

kn_Protocol kn_StringGetProtocolFromURI (kn_StringRef aURI)
{
  kn_StringRef aName     = kn_StringCreateWithProtocolFromURI(aURI);
  kn_Protocol  aProtocol = kn_PROTOCOL_INVALID;

  if (aName) aProtocol = kn_StringGetProtocolFromName(aName);

  kn_Release(aName);

  return aProtocol;
}

kn_StringRef kn_StringCreateWithUserNameFromURI (kn_StringRef aURI)
{
  const char* aStart = kn_StringGetBytes(aURI);
  const char* anEnd1 = aStart + kn_StringGetSize(aURI);
  const char* anEnd2;

  if (!aStart) return NULL;

  if (!(aStart = memchr(aStart, ':', anEnd1-aStart))) return NULL; aStart += 3; /* Scan past '://'               */
  if (!(anEnd1 = memchr(aStart, '/', anEnd1-aStart))) return NULL;              /* Ends at next '/'              */
  if (!(anEnd1 = memchr(aStart, '@', anEnd1-aStart))) return NULL;              /* Ends at previous '@'          */
  if ( (anEnd2 = memchr(aStart, ':', anEnd1-aStart))) anEnd1 = anEnd2;          /* Unless there is ':<password>' */

  return kn_StringCreateWithBytes(aStart, anEnd1-aStart);
}

kn_StringRef kn_StringCreateWithPasswordFromURI (kn_StringRef aURI)
{
  const char* aStart = kn_StringGetBytes(aURI);
  const char* anEnd  = aStart + kn_StringGetSize(aURI);

  if (!aStart) return NULL;

  if (!(aStart = memchr(aStart, ':', anEnd-aStart))) return NULL; aStart += 3; /* Scan past '://'      */
  if (!(anEnd  = memchr(aStart, '/', anEnd-aStart))) return NULL;              /* Ends at next '/'     */
  if (!(anEnd  = memchr(aStart, '@', anEnd-aStart))) return NULL;              /* Ends at previous '@' */
  if (!(aStart = memchr(aStart, ':', anEnd-aStart))) return NULL; aStart += 1; /* Scan past ':'        */

  return kn_StringCreateWithBytes(aStart, anEnd-aStart);
}

kn_StringRef kn_StringCreateWithHostNameFromURI (kn_StringRef aURI)
{
  const char* aStart1 = kn_StringGetBytes(aURI);
  const char* aStart2;
  const char* anEnd1  = aStart1 + kn_StringGetSize(aURI);
  const char* anEnd2;

  if (!aStart1) return NULL;

  if (!(aStart1 = memchr(aStart1, ':', anEnd1-aStart1))) return NULL; aStart1 += 3; /* Scan past '://'           */
  if (!(anEnd1  = memchr(aStart1, '/', anEnd1-aStart1))) return NULL;               /* Ends at next '/'          */
  if ( (aStart2 = memchr(aStart1, '@', anEnd1-aStart1))) aStart1 = aStart2+1;       /* Unless there is '<user>@' */
  if ( (anEnd2  = memchr(aStart1, ':', anEnd1-aStart1))) anEnd1  = anEnd2   ;       /* Unless there is ':<port>' */

  return kn_StringCreateWithBytes(aStart1, anEnd1-aStart1);
}

unsigned short int kn_StringGetPortFromURI (kn_StringRef aURI)
{
  const char* aStart1 = kn_StringGetBytes(aURI);
  const char* aStart2;
  const char* anEnd   = aStart1 + kn_StringGetSize(aURI);

  if (!aStart1) return 0;

  if (!(aStart1 = memchr(aStart1, ':', anEnd-aStart1))) return 0; aStart1 += 3; /* Scan past '://'       */
  if (!(anEnd   = memchr(aStart1, '/', anEnd-aStart1))) return 0;               /* Ends at next '/'      */
  if ( (aStart2 = memchr(aStart1, '@', anEnd-aStart1))) aStart1 = aStart2+1;    /* Scan past '@', if any */
        aStart1 = memchr(aStart1, ':', anEnd-aStart1);                          /* Starts after next ':' */

  if (aStart1)
    return (unsigned short int)strtol(aStart1+1, NULL, 10);
  else
    {
      switch (kn_StringGetProtocolFromURI(aURI))
        {
        case kn_PROTOCOL_INVALID : return   0;
        case kn_PROTOCOL_UNKNOWN : return   0;
        case kn_PROTOCOL_HTTP    : return  80;
        case kn_PROTOCOL_HTTPS   : return 443;
        }
    }

  return 0; /* Shut up bogus CC warning. */
}

kn_StringRef kn_StringCreateWithPathFromURI (kn_StringRef aURI)
{
  const char* aStart = kn_StringGetBytes(aURI);
  const char* anEnd0 = aStart + kn_StringGetSize(aURI);
  const char* anEnd1 = anEnd0;
  const char* anEnd2 = anEnd0;

  if (!aStart) return NULL;

  if (!(aStart = memchr(aStart, ':', anEnd0-aStart))) return NULL; /* Starts up to first ':' */
        aStart += 3;                                               /* Scan past '://'        */
  if (!(aStart = memchr(aStart, '/', anEnd0-aStart))) return NULL; /* Starts at next '/'     */
        anEnd1 = memchr(aStart, '?', anEnd0-aStart);               /* Ends at '?'            */
        anEnd2 = memchr(aStart, '#', anEnd0-aStart);               /* Ends at '#'            */

  if (!anEnd1) anEnd1 = anEnd2;

  anEnd1 = (anEnd1 < anEnd2) ? anEnd1 : anEnd2;

  if (!anEnd1) anEnd1 = anEnd0;

  return kn_StringCreateWithBytes(aStart, anEnd1-aStart);
}
