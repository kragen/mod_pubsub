/**
 * PubSub Client Library
 * Base object type and routines
 *
 * Wilfredo Sanchez
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

#include "kn_config.h"

RCSID("$Id: kn_Object.c,v 1.2 2003/03/19 05:36:47 ifindkarma Exp $");

#include <stdio.h>

#include "kn_Object_private.h"
#undef kn_Release

unsigned long long int _kn_Object_next_object_id = 0;

/**
 * Accessors
 **/

unsigned int kn_GetRetainCount (kn_ObjectRef anObject)
{
  return (anObject) ? ((kn_Object*)anObject)->retain_count : 0;
}

kn_StringRef kn_CopyDescription (kn_ObjectRef anObject)
{
  if (!anObject) return kn_StringCreateWithCString("*NULL*");

  if (((kn_Object*)anObject)->describe)
    return (((kn_Object*)anObject)->describe(anObject));
  else
    {
      char aDescription[1024] = "";

      sprintf(aDescription, "<kn_Object %p>", anObject);

      return kn_StringCreateWithCString(aDescription);
    }
}

/**
 * Actions
 **/

kn_ObjectRef kn_Retain (kn_ObjectRef anObject)
{
  if (!anObject) return NULL;

#ifdef KN_DEBUG_REFS
  printf("Retaining object %p, count = %d\n", anObject, ((kn_Object*)anObject)->retain_count);
#endif

  ((kn_Object*)anObject)->retain_count++;

  return anObject;
}

void _kn_Release(kn_ObjectRef* anObject)
{
  size_t aCount = kn_GetRetainCount(*anObject);

  kn_Release(*anObject);

  if (aCount == 1) *anObject = NULL;
}

void kn_Release (kn_ObjectRef anObject)
{
  if (anObject)
    {
#ifdef KN_DEBUG_REFS
      printf("Releasing object %p, count = %d\n", anObject, ((kn_Object*)anObject)->retain_count);
#endif

      switch (((kn_Object*)anObject)->retain_count)
	{
	case 1:
#ifdef KN_DEBUG_REFS
	  printf("Dealloc object %p\n", anObject);
#endif
	  ((kn_Object*)anObject)->dealloc((kn_Object*)anObject);
	  break;

	case 0:
	  fprintf(stderr, "kn_Release: YIKES: Object %p released with zero retain count.\n", anObject);
	  break;

	default:
	  ((kn_Object*)anObject)->retain_count--;
	  break;
	}
    }
}
