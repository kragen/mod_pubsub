/**
 * PubSub Client Library
 * Dictionary data type and routines
 *
 * Wilfredo Sanchez
 **/
/**
 * Copyright (c) 2001-2002 KnowNow, Inc.  All rights reserved.
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

RCSID("$Id: kn_Dictionary.c,v 1.1 2002/12/21 03:38:44 bsittler Exp $");

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>

#include "kn_Object_private.h"
#include "kn_Dictionary.h"

typedef struct __kn_Entry {
  kn_StringRef name;
  kn_ObjectRef value;
} kn_Entry;

typedef struct __kn_Dictionary {
  __KN_OBJECT_COMMON_FIELDS
  kn_Entry* entries;
  size_t    entry_count;
  size_t    entry_acount;
  kn_BOOL   is_mutable;
} kn_Dictionary;

/**
 * Allocators
 **/

static void kn_DictionaryDealloc (kn_ObjectRef anObject)
{
  kn_Dictionary* aDictionary = (kn_Dictionary*)anObject;

#ifdef KN_DEBUG_REFS
  printf("Dealloc dictionary %p\n", aDictionary);
#endif

  if (aDictionary->entries)
    {
      while (aDictionary->entry_count--)
	{
	  kn_Release(aDictionary->entries[aDictionary->entry_count].name );
	  kn_Release(aDictionary->entries[aDictionary->entry_count].value);
	}

      free(aDictionary->entries);
    }

  free(aDictionary);
}

static kn_StringRef kn_DictionaryDescribe (kn_ObjectRef anObject)
{
  kn_Dictionary*      aDictionary  = (kn_Dictionary*)anObject;
  kn_MutableStringRef aDescription = kn_StringCreateMutableWithCString("{ ");
  size_t              aCount       = aDictionary->entry_count;

  while (aCount--)
    {
      kn_StringRef aValue = kn_CopyDescription(aDictionary->entries[aCount].value);

      kn_StringAppendCString(aDescription, "\"");
      kn_StringAppendString (aDescription, aDictionary->entries[aCount].name);
      kn_StringAppendCString(aDescription, "\" = \"");
      kn_StringAppendString (aDescription, aValue);
      kn_StringAppendCString(aDescription, (aCount) ? "\", " : "\" }");

      kn_Release(aValue);
    }

  return aDescription;
}

kn_MutableDictionaryRef kn_DictionaryCreateMutable ()
{
  return kn_DictionaryCreateMutableWithCapacity(0);
}

kn_MutableDictionaryRef kn_DictionaryCreateMutableWithCapacity (size_t aCapacity)
{
  kn_Dictionary* aDictionary = (kn_Dictionary*)malloc(sizeof(kn_Dictionary));

  if (aDictionary)
    {
      kn_object_init(aDictionary, kn_DictionaryDealloc, kn_DictionaryDescribe);

      aDictionary->entries      = (kn_Entry*)calloc(aCapacity, sizeof(kn_Entry));
      aDictionary->entry_count  = 0;
      aDictionary->entry_acount = aCapacity;
      aDictionary->is_mutable   = kn_TRUE;

      if (!aDictionary->entries)
	{
	  kn_Release(aDictionary);
          errno = ENOMEM;
	  return NULL;
	}
    }
  else
    errno = ENOMEM;

  return aDictionary;
}

kn_DictionaryRef kn_DictionaryCreateCopy (kn_DictionaryRef aDictionary)
{
  if (aDictionary->is_mutable)
    {
      kn_Dictionary* aCopy = kn_DictionaryCreateMutableCopy(aDictionary);

      aCopy->is_mutable = kn_FALSE;

      return aCopy;
    }
  else
    return kn_Retain(aDictionary);
}

kn_MutableDictionaryRef kn_DictionaryCreateMutableCopy (kn_DictionaryRef aDictionary)
{
  kn_Dictionary* aCopy = NULL;

  if ((aCopy = kn_DictionaryCreateMutableWithCapacity(aDictionary->entry_count)))
    {
      aCopy->entry_count = aDictionary->entry_count;

      memmove(aCopy->entries, aDictionary->entries, aDictionary->entry_count*sizeof(kn_Entry));

      /* Now go through each entry and retain the strings contained in there. */
      {
	size_t     anIndex;
	kn_Entry* anEntry;

	for (anIndex = 0; anIndex < aCopy->entry_count; anIndex++)
	  {
	    anEntry = &aCopy->entries[anIndex];

	    kn_Retain(anEntry->name );
	    kn_Retain(anEntry->value);
	  }
      }
    }

  return aCopy;
}

#define ENTRY_REALLOC_STEP_SIZE 8

static kn_Error kn_DictionaryAppendEntry (kn_MutableDictionaryRef aDictionary, kn_StringRef aName, kn_ObjectRef aValue)
{
  size_t anAllocationCount = aDictionary->entry_acount;

  if (anAllocationCount <= aDictionary->entry_count)
    {
      /* We don't have anywhere to stick the new entry, so let's grow our pool */
      size_t aNewAllocationCount = anAllocationCount + ENTRY_REALLOC_STEP_SIZE;

      aDictionary->entries      = (kn_Entry*)realloc(aDictionary->entries, aNewAllocationCount*sizeof(kn_Entry));
      aDictionary->entry_acount = aNewAllocationCount;

      if (!aDictionary->entries)
	{
	  /* Lame attempt to clean up; but we've already lost data... */
	  aDictionary->entry_count  = 0;
	  aDictionary->entry_acount = 0;
          errno = ENOMEM;
	  return kn_MEMORYFAIL;
	}
    }

  aDictionary->entries[aDictionary->entry_count].name  = kn_Retain(aName );
  aDictionary->entries[aDictionary->entry_count].value = kn_Retain(aValue);

  aDictionary->entry_count++;

  return kn_SUCCESS;
}

kn_DictionaryRef kn_DictionaryCreateWithVAList (kn_StringRef aName, kn_ObjectRef aValue, va_list anArgs)
{
  kn_Dictionary* aDictionary = kn_DictionaryCreateMutableWithVAList(aName, aValue, anArgs);

  if (aDictionary) aDictionary->is_mutable = kn_FALSE;

  return aDictionary;
}

kn_MutableDictionaryRef kn_DictionaryCreateMutableWithVAList (kn_StringRef aName, kn_ObjectRef aValue, va_list anArgs)
{
  kn_MutableDictionaryRef aDictionary = kn_DictionaryCreateMutable();

  if (aDictionary && aName && aValue)
    {
      do kn_DictionaryAppendEntry(aDictionary, aName, aValue);
      while ((aName  = va_arg(anArgs, kn_StringRef)) &&
	     (aValue = va_arg(anArgs, kn_ObjectRef)) );
    }

  return aDictionary;
}

kn_DictionaryRef kn_DictionaryCreateWithNamesAndValues (kn_StringRef aName, kn_ObjectRef aValue, ...)
{
  kn_DictionaryRef aDictionary;
  va_list          anArgs;

  va_start(anArgs, aValue);
  aDictionary = kn_DictionaryCreateWithVAList(aName, aValue, anArgs);
  va_end(anArgs);

  return aDictionary;
}

kn_MutableDictionaryRef kn_DictionaryCreateMutableWithNamesAndValues (kn_StringRef aName, kn_ObjectRef aValue, ...)
{
  kn_MutableDictionaryRef aDictionary;
  va_list                 anArgs;

  va_start(anArgs, aValue);
  aDictionary = kn_DictionaryCreateMutableWithVAList(aName, aValue, anArgs);
  va_end(anArgs);

  return aDictionary;
}

/**
 * Accessors
 **/

size_t kn_DictionaryGetCount (kn_DictionaryRef aDictionary)
{
  return aDictionary->entry_count;
}

kn_ObjectRef kn_DictionaryGetValue (kn_DictionaryRef aDictionary, kn_StringRef aName)
{
  size_t    anIndex;
  kn_Entry* anEntry;

  for (anIndex = 0; anIndex < aDictionary->entry_count; anIndex++)
    {
      anEntry = &aDictionary->entries[anIndex];

      if (!kn_StringCompare(anEntry->name, aName)) return anEntry->value;
    }

  return NULL;
}

kn_Error kn_DictionaryGetNamesAndValues (kn_DictionaryRef aDictionary, kn_StringRef* aNames, kn_ObjectRef* aValues)
{
  size_t anIndex;

  for (anIndex = 0; anIndex < aDictionary->entry_count; anIndex++)
    {
      if (aNames ) aNames [anIndex] = aDictionary->entries[anIndex].name;
      if (aValues) aValues[anIndex] = aDictionary->entries[anIndex].value;
    }

  return kn_SUCCESS;
}

kn_Error kn_DictionaryAddValue (kn_MutableDictionaryRef aDictionary, kn_StringRef aName, kn_ObjectRef aValue)
{
  size_t anIndex;

  for (anIndex = 0; anIndex < aDictionary->entry_count; anIndex++)
    if (!kn_StringCompare(aDictionary->entries[anIndex].name, aName))
      return kn_EXISTS;

  return kn_DictionaryAppendEntry(aDictionary, aName, aValue);
}

kn_Error kn_DictionaryReplaceValue (kn_MutableDictionaryRef aDictionary, kn_StringRef aName, kn_ObjectRef aValue)
{
  size_t    anIndex;
  kn_Entry* anEntry;

  for (anIndex = 0; anIndex < aDictionary->entry_count; anIndex++)
    {
      anEntry = &aDictionary->entries[anIndex];

      if (!kn_StringCompare(anEntry->name, aName))
	{
	  kn_ObjectRef anOldValue = anEntry->value;

	  anEntry->value = kn_Retain(aValue);

	  kn_Release(anOldValue);

	  return kn_SUCCESS;
	}
    }

  return kn_NOENTRY;
}

kn_Error kn_DictionarySetValue (kn_MutableDictionaryRef aDictionary, kn_StringRef aName, kn_ObjectRef aValue)
{
  kn_Error aStatus = kn_DictionaryReplaceValue(aDictionary, aName, aValue);

  if (aStatus == kn_NOENTRY)
    return kn_DictionaryAppendEntry(aDictionary, aName, aValue);
  else
    return aStatus;
}

kn_Error kn_DictionaryRemoveValue (kn_MutableDictionaryRef aDictionary, kn_StringRef aName)
{
  size_t    anIndex;
  kn_Entry* anEntry;

  for (anIndex = 0; anIndex < aDictionary->entry_count; anIndex++)
    {
      anEntry = &aDictionary->entries[anIndex];

      if (!kn_StringCompare(anEntry->name, aName))
	{
	  kn_Release(anEntry->value);

	  memmove((aDictionary->entries)+anIndex  ,
		  (aDictionary->entries)+anIndex+1,
		  sizeof(kn_Entry)*(aDictionary->entry_count-anIndex-1));

	  aDictionary->entry_count--;

	  return kn_SUCCESS;
	}
    }

  return kn_NOENTRY;
}
