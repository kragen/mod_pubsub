/**
 * PubSub Client Library
 * String data type and routines
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

RCSID("$Id: kn_String.c,v 1.2 2003/03/19 05:36:47 ifindkarma Exp $");

#include <string.h>
#include <stdlib.h>
#include <sysexits.h>
#include <errno.h>

#include "kn_Object_private.h"
#include "kn_String.h"
#include "kn_Dictionary.h"

typedef struct __kn_String {
  __KN_OBJECT_COMMON_FIELDS
  char*   bytes;
  size_t  size;
  size_t  asize;
  kn_BOOL is_mutable;
  kn_BOOL should_free;
} kn_String;

/**
 * Allocators
 **/

static void kn_StringDealloc (kn_ObjectRef anObject)
{
  kn_String* aString = (kn_String*)anObject;

#ifdef KN_DEBUG_REFS
  printf("Dealloc string %p: ", aString); kn_StringWriteToStream(aString,stdout); printf("\n");
#endif

  if (aString->should_free) free(aString->bytes);
                            free(aString       );
}

static kn_StringRef kn_StringDescribe (kn_ObjectRef anObject)
{
  return kn_StringCreateCopy((kn_StringRef)anObject);
}

kn_MutableStringRef kn_StringCreateMutable ()
{
  return kn_StringCreateMutableWithSize(0);
}

kn_MutableStringRef kn_StringCreateMutableWithSize (size_t aSize)
{
  kn_String* aString = (kn_String*)malloc(sizeof(kn_String));

  if (!aString) { errno = ENOMEM; return NULL; }

  if (!(aString->bytes = (char*)malloc(aSize+1)))
    {
      free(aString);
      errno = ENOMEM;
      return NULL;
    }

  kn_object_init(aString, kn_StringDealloc, kn_StringDescribe);

  aString->size        = 0;
  aString->asize       = aSize;
  aString->is_mutable  = kn_TRUE;
  aString->should_free = kn_TRUE;

  *(aString->bytes+aString->size) = '\0';

  return aString;
}

kn_StringRef kn_StringCreateWithCString (const char* aCString)
{
  return kn_StringCreateWithBytes(aCString, strlen(aCString));
}

kn_MutableStringRef kn_StringCreateMutableWithCString (const char* aCString)
{
  return kn_StringCreateMutableWithBytes(aCString, strlen(aCString));
}

static kn_MutableDictionaryRef gConstantStrings = NULL;

kn_StringRef _kn_StringMakeStringConstantWithCStringConstant (const char* aCString)
{
  kn_StringRef aString    = NULL;
  kn_StringRef aNewString = kn_StringCreateWithBytesNoCopy(aCString, strlen(aCString), kn_FALSE);

  if (!gConstantStrings) gConstantStrings = kn_DictionaryCreateMutable();

  if (gConstantStrings)
    {
      kn_DictionaryAddValue(gConstantStrings, aNewString, aNewString);
      aString = kn_DictionaryGetValue(gConstantStrings, aNewString);
    }

  kn_Release(aNewString);

  return aString;
}

kn_StringRef kn_StringCreateWithBytes (const char* aBytes, size_t aSize)
{
  kn_String* aString = kn_StringCreateMutableWithBytes(aBytes, aSize);

  if (aString) aString->is_mutable = kn_FALSE;

  return aString;
}

kn_StringRef kn_StringCreateWithBytesNoCopy (const char* aBytes, size_t aSize, kn_BOOL aFreeOnDeallocFlag)
{
  kn_String* aString = (kn_String*)malloc(sizeof(kn_String));

  if (!aString) { errno = ENOMEM; return NULL; }

  kn_object_init(aString, kn_StringDealloc, kn_StringDescribe);

  (const char*)aString->bytes       = aBytes;
               aString->size        = aSize;
	       aString->asize       = 0;
	       aString->is_mutable  = kn_FALSE;
	       aString->should_free = aFreeOnDeallocFlag;

  return aString;
}

kn_MutableStringRef kn_StringCreateMutableWithBytes (const char* aBytes, size_t aSize)
{
  kn_MutableStringRef aString = kn_StringCreateMutableWithSize(aSize);

  if (aString)
    {
      memmove(aString->bytes, aBytes, aSize);

      aString->size = aSize;

      *(aString->bytes+aString->size) = '\0';
    }

  return aString;
}

kn_StringRef kn_StringCreateCopy (kn_StringRef aString)
{
  if (aString->is_mutable)
    return kn_StringCreateWithBytes(aString->bytes, aString->size);
  else
    return kn_Retain(aString);
}

kn_MutableStringRef kn_StringCreateMutableCopy (kn_StringRef aString)
{
  return kn_StringCreateMutableWithBytes(aString->bytes, aString->size);
}

/**
 * Accessors
 **/

const char* kn_StringGetBytes  (kn_StringRef aString) { return (aString->bytes); }
size_t      kn_StringGetSize   (kn_StringRef aString) { return (aString->size ); }

/**
 * Actions
 **/

int kn_StringCompare (kn_StringRef aString1, kn_StringRef aString2)
{
  int aComparison =
    memcmp(aString1->bytes, aString2->bytes,
	   (aString1->size < aString2->size) ? aString1->size : aString2->size);

  if (!aComparison && (aString1->size != aString2->size))
    return ((aString1->size < aString2->size) ? 1 : -1);

  return aComparison;
}

int kn_StringCompareCaseInsensitive (kn_StringRef aString1, kn_StringRef aString2)
{
  int aComparison =
    strncasecmp(aString1->bytes, aString2->bytes,
                (aString1->size < aString2->size) ? aString1->size : aString2->size);

  if (!aComparison && (aString1->size != aString2->size))
    return ((aString1->size < aString2->size) ? 1 : -1);

  return aComparison;
}

#define STRING_REALLOC_STEP_SIZE BUFSIZ

kn_Error kn_StringAppendString (kn_MutableStringRef aString, kn_StringRef anAppend)
{
  if (!aString) return kn_INVALID;

  if (anAppend)
    {
      size_t aNewASize = aString->size +
	((anAppend->size > STRING_REALLOC_STEP_SIZE) ? anAppend->size : STRING_REALLOC_STEP_SIZE);

      if (aString->asize < aNewASize)
	{
	  /* We need to grow the allocated size of aString. */
	  aString->bytes = (char*)realloc(aString->bytes, (aString->asize = aNewASize)+1);

	  if (!aString->bytes)
	    {
	      /* Lame attempt to clean up; but we've already lost data... */
	      aString->size  = 0;
	      aString->asize = 0;

	      errno = ENOMEM;
	      return kn_MEMORYFAIL;
	    }
	}

      memmove(aString->bytes+aString->size, anAppend->bytes, anAppend->size);

      aString->size += anAppend->size;

      *(aString->bytes+aString->size) = '\0';
    }

  return kn_SUCCESS;
}

kn_Error kn_StringAppendCString (kn_MutableStringRef aString, const char* anAppend)
{
  kn_StringRef anAppendString = kn_StringCreateWithCString(anAppend);

  kn_Error anError = kn_StringAppendString(aString, anAppendString);

  kn_Release(anAppendString);

  return anError;
}

size_t kn_StringWriteToStream (kn_StringRef aString, FILE* aStream)
{
  return fwrite(aString->bytes, 1, aString->size, aStream);
}
