/**
 * PubSub Client Library
 * String encodings for web
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

RCSID("$Id: kn_String_WebEncoding.c,v 1.1 2002/12/21 03:38:44 bsittler Exp $");

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "kn_Object.h"
#include "kn_String_WebEncoding.h"

/* Pull in Apache base64 functionality. */
#define APU_DECLARE(aType) static aType
#include "apr_base64.c"

/**
 * Allocators
 **/

#define is_safe(aCharacter)       \
	( isalnum(aCharacter)  || \
	  (aCharacter == '-' ) || \
	  (aCharacter == '_' ) || \
	  (aCharacter == '.' ) || \
	  (aCharacter == '!' ) || \
	  (aCharacter == '~' ) || \
	  (aCharacter == '*' ) || \
	  (aCharacter == '\'') || \
	  (aCharacter == '(' ) || \
	  (aCharacter == ')' ) )

static kn_StringRef kn_StringEncodeHex (kn_StringRef aString, char anEscapeCharacter)
{
  kn_StringRef aResult;
  const char*  anInputBytes        = kn_StringGetBytes(aString);
  size_t       anInputBytesLength  = kn_StringGetSize (aString);
  char *       anOutputBytes       = NULL;
  size_t       anOutputBytesLength = 0;
  size_t       anIterator;

  /* Calculate length of encoded string. */
  for (anIterator = 0; anIterator < anInputBytesLength; anIterator++)
    {
      unsigned char aCharacter = (unsigned char)anInputBytes[anIterator];

      /* Quoted characters take 3 bytes; others take 1. */
      if (is_safe(aCharacter))
	anOutputBytesLength += 1;
      else
	anOutputBytesLength += 3;
    }

  anOutputBytes       = (char*)malloc(anOutputBytesLength+1);
  anOutputBytesLength = 0;

  if (!anOutputBytes) { errno = ENOMEM; return NULL; }

  for (anIterator = 0; anIterator < anInputBytesLength; anIterator++)
    {
      unsigned char aCharacter = (unsigned char)anInputBytes[anIterator];

      if (is_safe(aCharacter))
	anOutputBytes[anOutputBytesLength++] = aCharacter;
      else
	{
	  char aHex[4];

	  snprintf(aHex, sizeof(aHex), "%c%02X", anEscapeCharacter, aCharacter);

	  memmove(anOutputBytes+anOutputBytesLength, aHex, 3);

	  anOutputBytesLength += 3;
	}
    }

  anOutputBytes[anOutputBytesLength] = '\0';

  aResult = kn_StringCreateWithBytesNoCopy(anOutputBytes, anOutputBytesLength, kn_TRUE);

  return aResult;
}

static kn_StringRef kn_StringDecodeHex (kn_StringRef aString, char anEscapeCharacter)
{
  kn_StringRef aResult;
  const char*  anInputBytes        = kn_StringGetBytes(aString);
  size_t       anInputBytesLength  = kn_StringGetSize (aString);
  char *       anOutputBytes       = NULL;
  size_t       anOutputBytesLength = anInputBytesLength;
  size_t       anIterator;

  /* Calculate length of decoded string. */
  for (anIterator = 0; anIterator < anInputBytesLength; anIterator++)
    {
      if (anInputBytes[anIterator] == anEscapeCharacter)
	{
	  anOutputBytesLength -= 2; /* Un-quoted characters lose 2 bytes (3->1). */
	  anIterator          += 2; /* Skip the rest of the quoted character.    */
	}
    }

  anOutputBytes       = (char*)malloc(anOutputBytesLength);
  anOutputBytesLength = 0;

  if (!anOutputBytes) { errno = ENOMEM; return NULL; }

  for (anIterator = 0; anIterator < anInputBytesLength; anIterator++)
    {
      if (anInputBytes[anIterator] == anEscapeCharacter)
	{
	  char aHex[3];

	  aHex[0] = anInputBytes[++anIterator];
	  aHex[1] = anInputBytes[++anIterator];
	  aHex[2] = '\0';

	  anOutputBytes[anOutputBytesLength++] = strtol(aHex, NULL, 16);
	}
      else
	anOutputBytes[anOutputBytesLength++] = anInputBytes[anIterator];
    }

  aResult = kn_StringCreateWithBytesNoCopy(anOutputBytes, anOutputBytesLength, kn_TRUE);

  return aResult;
}

kn_StringRef kn_StringCreateByEncodingQuotedPrintable (kn_StringRef aString)
{
  return kn_StringEncodeHex(aString, '=');
}

kn_StringRef kn_StringCreateByDecodingQuotedPrintable (kn_StringRef aString)
{
  return kn_StringDecodeHex(aString, '=');
}

kn_StringRef kn_StringCreateByEncodingURLHex (kn_StringRef aString)
{
  return kn_StringEncodeHex(aString, '%');
}

kn_StringRef kn_StringCreateByDecodingURLHex (kn_StringRef aString)
{
  return kn_StringDecodeHex(aString, '%');
}

kn_StringRef kn_StringCreateByEncodingBase64 (kn_StringRef aString)
{
  const char* anInputBytes        = kn_StringGetBytes(aString);
  size_t      anInputBytesLength  = kn_StringGetSize (aString);
  size_t      anOutputBytesLength = apr_base64_encode_len(anInputBytesLength);
  char*       anOutputBytes       = (char*)malloc((anOutputBytesLength+1)*sizeof(char));

  apr_base64_encode(anOutputBytes, anInputBytes, anInputBytesLength);

  return kn_StringCreateWithCString(anOutputBytes);
}

/**
 * Actions
 **/

kn_Error kn_StringAppendPostField (kn_MutableStringRef aPostString, kn_StringRef aName, kn_StringRef aValue)
{
  kn_Error     anError        = kn_SUCCESS;
  kn_StringRef anEncodedName  = kn_StringCreateByEncodingURLHex(aName );
  kn_StringRef anEncodedValue = kn_StringCreateByEncodingURLHex(aValue);

  if (anEncodedName && anEncodedValue)
    {
      if (kn_StringGetSize(aPostString)) kn_StringAppendCString(aPostString, ";"           );
                                         kn_StringAppendString (aPostString, anEncodedName );
                                         kn_StringAppendCString(aPostString, "="           );
                                         kn_StringAppendString (aPostString, anEncodedValue);
    }
  else
    anError = kn_MEMORYFAIL;

  kn_Release(anEncodedName );
  kn_Release(anEncodedValue);

  return anError;
}
