/**
 * PubSub Client Library
 * libkn test suite
 *
 * WARNING: Don't use this as a source of good examples of how to
 * program the library; its job is only to test it.  In particular,
 * one should note that we very often return from a function without
 * having released all of the objects we have hanging about.  This is
 * rather poor practice, as it leaks memory, but we're going to exit
 * the program when that happens, and this is all messy enough as is.
 * -Wilfredo Sanchez
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

RCSID("$Id: kn_test.c,v 1.3 2004/04/19 05:39:08 bsittler Exp $");

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <time.h>

#include <kn/kn.h>

/*
 * You might wonder why these are C strings instead of KNSTR() constant kn_Strings...
 * That's because KNSTR()'s underlying implementation uses kn_Dictionary, and
 * we want to test strings before dictionaries without having the string test
 * break if dictionary is broken.  So we avoid KNSTR() until after the dictionary test.
 */
#define HELLO_WORLD_PART1            "Hello, World!\n"
#define HELLO_WORLD_PART2            "1!2@3#4$5%6^7&8*9(0)\n"
#define HELLO_WORLD_PART3            "Wilfredo S\xc3\xa1nchez\n"
#define HELLO_WORLD                  HELLO_WORLD_PART1 HELLO_WORLD_PART2 HELLO_WORLD_PART3
#define HELLO_WORLD_QUOTED_PRINTABLE "Hello=2C=20World!=0A1!2=403=234=245=256=5E7=268*9(0)=0AWilfredo=20S=C3=A1nchez=0A"
#define HELLO_WORLD_URLHEX           "Hello%2C%20World!%0A1!2%403%234%245%256%5E7%268*9(0)%0AWilfredo%20S%C3%A1nchez%0A"
#define HELLO_WORLD_BASE64           "SGVsbG8sIFdvcmxkIQoxITJAMyM0JDUlNl43JjgqOSgwKQpXaWxmcmVkbyBTw6FuY2hlego="

#define PUBSUB_URI       "http://127.0.0.1/mod_pubsub/cgi-bin/pubsub.cgi"
#define PUBSUB_USER      NULL
#define PUBSUB_PASSWORD  NULL
#define PUBSUB_SUB_TOPIC KNSTR("/what/chat")
#define PUBSUB_PUB_TOPIC KNSTR("/test/kn_test.c/routed_topic/")

#define PROXY_HOST	NULL /* "127.0.0.1" */
#define PROXY_USER	NULL /* "kncgi" */
#define PROXY_PASSWD	NULL /* "kncgi" */
#define	PROXY_PORT	3128

#define BOBO_HEADER_NAME KNSTR("bobo")

static char* gPubSubURI      = PUBSUB_URI;
static char* gPubSubUser     = PUBSUB_USER;
static char* gPubSubPassword = PUBSUB_PASSWORD;

static char* gProxyHost		  = PROXY_HOST;
static char* gProxyUser		  = PROXY_USER;
static char* gProxyPasswd	  = PROXY_PASSWD;

int reference_count ()
{
  kn_StringRef aString = kn_StringCreateWithCString(HELLO_WORLD);

  int aRetainCount;

  printf("Testing reference counting:\n");

  printf("ref/init..."); fflush(stdout);
  {
    /* We should start at 1 */

    if ((aRetainCount = kn_GetRetainCount(aString)) != 1)
      {
	printf("\n\tFAIL: New object has retain count %d (!= 1)\n", aRetainCount);
	return 1;
      }
  }
  printf("OK\n");

  printf("ref/count");
  {
    /* Do a bunch of retains and releases; should end with a dealloc */

    int aCount = random() % 100;
    int anIndex;

    printf("(x%d)...", aCount); fflush(stdout);

    for (anIndex = 0; anIndex < aCount; anIndex++) kn_Retain (aString);
    for (anIndex = 0; anIndex < aCount; anIndex++) kn_Release(aString);

    if ((aRetainCount = kn_GetRetainCount(aString)) != 1)
      {
	printf("\n\tFAIL: Retain/release cycle (%d) ended with retain count %d (!= 1)\n",
	       aCount, aRetainCount);
	return 1;
      }
  }
  printf("OK\n");

  kn_Release(aString);

  printf("\n"); fflush(stdout);
  return 0;
}

int string ()
{
  /* Create some strings and check for sanity */

  kn_StringRef        aString;
  kn_StringRef        aNutherString;
  kn_MutableStringRef aMutableString;
  int                 aSize;

  printf("Testing string object:\n");

  printf("string/init.size..."); fflush(stdout);
  aMutableString = kn_StringCreateMutableWithSize(100);
  if (!aMutableString)
    {
      printf("\n\tFAIL: kn_StringCreateMutableWithSize returned NULL: %s\n", strerror(errno));
      return 1;
    }
  kn_Release(aMutableString);
  printf("OK\n");

  printf("string/init.cString..."); fflush(stdout);
  aString = kn_StringCreateWithCString(HELLO_WORLD);
  if (!aString)
    {
      printf("\n\tFAIL: kn_StringCreateWithCString returned NULL: %s\n", strerror(errno));
      return 1;
    }
  if ((aSize = kn_StringGetSize(aString)) != sizeof(HELLO_WORLD)-1)
    {
      printf("\n\tFAIL: String from \"%s\" has size %d (!= %d)\n",
	     HELLO_WORLD, aSize, (int)sizeof(HELLO_WORLD)-1);
      return 1;
    }
  printf("OK\n");

  printf("string/compare..."); fflush(stdout);
  aNutherString = kn_StringCreateWithCString(HELLO_WORLD);
  if (kn_StringCompare(aString, aNutherString))
    {
      printf("\n\tFAIL: String compare (\"");
      kn_StringWriteToStream(aString, stdout);
      printf("\" != \"");
      kn_StringWriteToStream(aNutherString, stdout);
      printf("\")\n");
      return 1;
    }
  kn_Release(aNutherString);
  aNutherString = kn_StringCreateWithCString("AAAAA");
  if (kn_StringCompare(aString, aNutherString) <= 0)
    {
      printf("\n\tFAIL: String compare (\"");
      kn_StringWriteToStream(aString, stdout);
      printf("\" > \"");
      kn_StringWriteToStream(aNutherString, stdout);
      printf("\")\n");
      return 1;
    }
  kn_Release(aNutherString);
  aNutherString = kn_StringCreateWithCString("zzzzz");
  if (kn_StringCompare(aString, aNutherString) >= 0)
    {
      printf("\n\tFAIL: String compare (\"");
      kn_StringWriteToStream(aString, stdout);
      printf("\" < \"");
      kn_StringWriteToStream(aNutherString, stdout);
      printf("\")\n");
      return 1;
    }
  kn_Release(aNutherString);
  printf("OK\n");

  printf("string/copy..."); fflush(stdout);
  aMutableString = kn_StringCreateMutableCopy(aString);
  if (kn_StringCompare(aString, aMutableString))
    {
      printf("\n\tFAIL: Copied string is not the same as original (\"");
      kn_StringWriteToStream(aMutableString, stdout);
      printf("\" != \"");
      kn_StringWriteToStream(aString, stdout);
      printf("\")\n");
      return 1;
    }
  kn_Release(aMutableString);
  printf("OK\n");

  printf("string/append..."); fflush(stdout);
  aMutableString = kn_StringCreateMutableWithCString(HELLO_WORLD_PART1);
  aNutherString  = kn_StringCreateWithCString(HELLO_WORLD_PART2);
  kn_StringAppendString(aMutableString, aNutherString);
  kn_Release(aNutherString);
  aNutherString  = kn_StringCreateWithCString(HELLO_WORLD_PART3);
  kn_StringAppendString(aMutableString, aNutherString);
  kn_Release(aNutherString);
  if (kn_StringCompare(aMutableString, aString))
    {
      printf("\n\tFAIL: Appended string doesn't match expected result (\"");
      kn_StringWriteToStream(aMutableString, stdout);
      printf("\" != \"");
      kn_StringWriteToStream(aString, stdout);
      printf("\")\n");
      return 1;
    }
  kn_Release(aMutableString);
  printf("OK\n");

  kn_Release(aString);

  printf("\n"); fflush(stdout);
  return 0;
}

#define PROTOCOL "http://"
#define USER     "spammer"
#define PASS     "supersneakysecret"
#define HOST     "marketblitzer"
#define PORT     ":8000"
#define PATH     "/win/free/sex"

int string_uri ()
{
  kn_StringRef aURI  = kn_StringCreateWithCString(PROTOCOL USER ":" PASS "@" HOST PORT PATH);
  kn_StringRef aURI2 = kn_StringCreateWithCString(PROTOCOL                   HOST      PATH);

  printf("Testing string URI handling...\n");

  printf("string/uri.proto..."); fflush(stdout);
  {
    kn_Protocol aProtocol = kn_StringGetProtocolFromURI(aURI);
    if (aProtocol != kn_PROTOCOL_HTTP)
      {
	printf("\n\tFAIL: Protocol mismatch (%d != %d).\n", aProtocol, kn_PROTOCOL_HTTP);
	return 1;
      }
  }
  printf("OK\n");

  printf("string/uri.user..."); fflush(stdout);
  {
    kn_StringRef aUsername = kn_StringCreateWithUserNameFromURI(aURI);
    if (strcmp(kn_StringGetBytes(aUsername), USER))
      {
	printf("\n\tFAIL: Username mismatch (%s != "USER").\n", kn_StringGetBytes(aUsername));
	return 1;
      }
    kn_Release(aUsername);
  }
  printf("OK\n");

  printf("string/uri.password..."); fflush(stdout);
  {
    kn_StringRef aPassword = kn_StringCreateWithPasswordFromURI(aURI);
    if (strcmp(kn_StringGetBytes(aPassword), PASS))
      {
	printf("\n\tFAIL: Password mismatch (%s != "PASS").\n", kn_StringGetBytes(aPassword));
	return 1;
      }
    kn_Release(aPassword);
  }
  printf("OK\n");

  printf("string/uri.host..."); fflush(stdout);
  {
    kn_StringRef aHostname = kn_StringCreateWithHostNameFromURI(aURI);
    if (strcmp(kn_StringGetBytes(aHostname), HOST))
      {
	printf("\n\tFAIL: Hostname mismatch (%s != "HOST").\n", kn_StringGetBytes(aHostname));
	return 1;
      }
    kn_Release(aHostname);
    aHostname = kn_StringCreateWithHostNameFromURI(aURI2);
    if (strcmp(kn_StringGetBytes(aHostname), HOST))
      {
	printf("\n\tFAIL: Hostname mismatch (%s != "HOST").\n", kn_StringGetBytes(aHostname));
	return 1;
      }
    kn_Release(aHostname);
  }
  printf("OK\n");

  printf("string/uri.port..."); fflush(stdout);
  {
    unsigned short int aPort = kn_StringGetPortFromURI(aURI);
    if (aPort != 8000)
      {
	printf("\n\tFAIL: Port mismatch (%d != 8000) for URI %s.\n", aPort, kn_StringGetBytes(aURI));
	return 1;
      }
    aPort = kn_StringGetPortFromURI(aURI2);
    if (aPort != 80)
      {
	printf("\n\tFAIL: Port mismatch (%d != 80) for URI %s.\n", aPort, kn_StringGetBytes(aURI));
	return 1;
      }
  }
  printf("OK\n");

  printf("string/uri.path..."); fflush(stdout);
  {
    kn_StringRef aPath = kn_StringCreateWithPathFromURI(aURI);
    if (strcmp(kn_StringGetBytes(aPath), PATH))
      {
	printf("\n\tFAIL: Path mismatch (%s != "PATH").\n", kn_StringGetBytes(aPath));
	return 1;
      }
    kn_Release(aPath);
    aPath = kn_StringCreateWithPathFromURI(aURI2);
    if (strcmp(kn_StringGetBytes(aPath), PATH))
      {
	printf("\n\tFAIL: Path mismatch (%s != "PATH").\n", kn_StringGetBytes(aPath));
	return 1;
      }
    kn_Release(aPath);
  }
  printf("OK\n");

  kn_Release(aURI );
  kn_Release(aURI2);

  printf("\n"); fflush(stdout);
  return 0;
}

int string_webencode ()
{
  kn_StringRef aString = kn_StringCreateWithCString(HELLO_WORLD);
  kn_StringRef anEncodedString;
  kn_StringRef anExpectedString;

  printf("Testing string web encoding...\n");

  printf("string/encode.qp..."); fflush(stdout);
  anExpectedString = kn_StringCreateWithCString(HELLO_WORLD_QUOTED_PRINTABLE);
  anEncodedString  = kn_StringCreateByEncodingQuotedPrintable(aString);
  if (kn_StringCompare(anExpectedString, anEncodedString))
    {
      printf("\n\tFAIL: Expected Quoted Printable:\n");
      kn_StringWriteToStream(anExpectedString, stdout);
      printf("\n");
      printf("\n\tBut instead we got:\n");
      kn_StringWriteToStream(anEncodedString, stdout);
      printf("\n");
      return 1;
    }
  kn_Release(anExpectedString);
  printf("OK\n");

  printf("string/decode.qp..."); fflush(stdout);
  anExpectedString = kn_StringCreateByDecodingQuotedPrintable(anEncodedString);
  if (kn_StringCompare(aString, anExpectedString))
    {
      printf("\n\tFAIL: Expected De-Quoted Printable:\n");
      kn_StringWriteToStream(aString, stdout);
      printf("\n");
      printf("\n\tBut instead we got:\n");
      kn_StringWriteToStream(anExpectedString, stdout);
      printf("\n");
      return 1;
    }
  kn_Release(anExpectedString);
  kn_Release(anEncodedString);
  printf("OK\n");

  printf("string/encode.url..."); fflush(stdout);
  anExpectedString = kn_StringCreateWithCString(HELLO_WORLD_URLHEX);
  anEncodedString  = kn_StringCreateByEncodingURLHex(aString);
  if (kn_StringCompare(anExpectedString, anEncodedString))
    {
      printf("\n\tFAIL: Expected Quoted URL Hex:\n");
      kn_StringWriteToStream(anExpectedString, stdout);
      printf("\n");
      printf("\n\tBut instead we got:\n");
      kn_StringWriteToStream(anEncodedString, stdout);
      printf("\n");
      return 1;
    }
  kn_Release(anExpectedString);
  printf("OK\n");

  printf("string/decode.url..."); fflush(stdout);
  anExpectedString = kn_StringCreateByDecodingURLHex(anEncodedString);
  if (kn_StringCompare(aString, anExpectedString))
    {
      printf("\n\tFAIL: Expected De-Quoted URL Hex:\n");
      kn_StringWriteToStream(aString, stdout);
      printf("\n");
      printf("\n\tBut instead we got:\n");
      kn_StringWriteToStream(anExpectedString, stdout);
      printf("\n");
      return 1;
    }
  kn_Release(anExpectedString);
  kn_Release(anEncodedString);
  printf("OK\n");

  printf("string/encode.base64..."); fflush(stdout);
  anExpectedString = kn_StringCreateWithCString(HELLO_WORLD_BASE64);
  anEncodedString  = kn_StringCreateByEncodingBase64(aString);
  if (kn_StringCompare(anExpectedString, anEncodedString))
    {
      printf("\n\tFAIL: Expected Base64:\n");
      kn_StringWriteToStream(anExpectedString, stdout);
      printf("\n");
      printf("\n\tBut instead we got:\n");
      kn_StringWriteToStream(anEncodedString, stdout);
      printf("\n");
      return 1;
    }
  kn_Release(anExpectedString);
  kn_Release(anEncodedString);
  printf("OK\n");

  printf("\n"); fflush(stdout);
  return 0;
}

int dictionary ()
{
  kn_DictionaryRef        aDictionary;
  kn_MutableDictionaryRef aMutableDictionary;

  kn_StringRef aName1  = kn_StringCreateWithCString("Name #1");
  kn_StringRef aName2  = kn_StringCreateWithCString("Name the 2nd");
  kn_StringRef aName3  = kn_StringCreateWithCString("3rd name");
  kn_StringRef aName4  = kn_StringCreateWithCString("Name 4");
  kn_StringRef aValue1 = kn_StringCreateWithCString("This is value #1");
  kn_StringRef aValue2 = kn_StringCreateWithCString("This is value the 2nd");
  kn_StringRef aValue3 = kn_StringCreateWithCString("This is the 3rd value");
  kn_StringRef aValue4 = kn_StringCreateWithCString("This is value 4");

  kn_StringRef aValue;

  printf("Testing dictionary object:\n");

  printf("dictionary/init.cap..."); fflush(stdout);
  aMutableDictionary = kn_DictionaryCreateMutableWithCapacity(100);
  if (!aMutableDictionary)
    {
      printf("\n\tFAIL: kn_DictionaryCreateMutableWithCapacity returned NULL: %s\n", strerror(errno));
      return 1;
    }
  kn_Release(aMutableDictionary);
  printf("OK\n");

  printf("dictionary/init.varargs..."); fflush(stdout);
  aDictionary = kn_DictionaryCreateMutableWithNamesAndValues(aName1, aValue1,
							     aName2, aValue2,
							     aName3, aValue3,
							     aName4, aValue4,
							     NULL  , NULL   );
  if (!aDictionary)
    {
      printf("\n\tFAIL: kn_DictionaryCreateMutableWithNamesAndValues returned NULL: %s\n", strerror(errno));
      return 1;
    }
  if (kn_DictionaryGetCount(aDictionary) != 4)
    {
      printf("\n\tFAIL: dictionary count is %ld (!= 4)", (long)kn_DictionaryGetCount(aDictionary));
      return 1;
    }
  if (!(aValue = kn_DictionaryGetValue(aDictionary, aName1)))
    {
      printf("\n\tFAIL: Value #1 lost\n");
      return 1;
    }
  if (kn_StringCompare(aValue, aValue1))
    {
      printf("\n\tFAIL: Value #1 mismatch (\"");
      kn_StringWriteToStream(aValue, stdout);
      printf("\" != \"");
      kn_StringWriteToStream(aValue1, stdout);
      printf("\")\n");
      return 1;
    }
  if (!(aValue = kn_DictionaryGetValue(aDictionary, aName3)))
    {
      printf("\n\tFAIL: Value #3 lost\n");
      return 1;
    }
  if (kn_StringCompare(aValue, aValue3))
    {
      printf("\n\tFAIL: Value #3 mismatch (\"");
      kn_StringWriteToStream(aValue, stdout);
      printf("\" != \"");
      kn_StringWriteToStream(aValue3, stdout);
      printf("\")\n");
      return 1;
    }
  printf("OK\n");

  printf("dictionary/init.copy..."); fflush(stdout);
  aMutableDictionary = kn_DictionaryCreateMutableCopy(aDictionary);
  kn_Release(aDictionary);
  if (!aMutableDictionary)
    {
      printf("\n\tFAIL: kn_DictionaryCreateMutableCopy returned NULL: %s\n", strerror(errno));
      return 1;
    }
  if (!(aValue = kn_DictionaryGetValue(aMutableDictionary, aName2)))
    {
      printf("\n\tFAIL: Value #2 lost\n");
      return 1;
    }
  if (kn_StringCompare(aValue, aValue2))
    {
      printf("\n\tFAIL: Value #2 mismatch (\"");
      kn_StringWriteToStream(aValue, stdout);
      printf("\" != \"");
      kn_StringWriteToStream(aValue2, stdout);
      printf("\")\n");
      return 1;
    }
  if (!(aValue = kn_DictionaryGetValue(aMutableDictionary, aName4)))
    {
      printf("\n\tFAIL: Value #4 lost\n");
      return 1;
    }
  if (kn_StringCompare(aValue, aValue4))
    {
      printf("\n\tFAIL: Value #4 mismatch (\"");
      kn_StringWriteToStream(aValue, stdout);
      printf("\" != \"");
      kn_StringWriteToStream(aValue4, stdout);
      printf("\")\n");
      return 1;
    }
  kn_Release(aMutableDictionary);
  printf("OK\n");

  printf("dictionary/append..."); fflush(stdout);
  aMutableDictionary = kn_DictionaryCreateMutableWithCapacity(1);
  if (!aMutableDictionary)
    {
      printf("\n\tFAIL: kn_DictionaryCreateMutableWithCapacity returned NULL: %s\n", strerror(errno));
      return 1;
    }
  kn_DictionarySetValue(aMutableDictionary, aName1, aValue1);
  kn_DictionarySetValue(aMutableDictionary, aName2, aValue2);
  if (kn_DictionaryGetCount(aMutableDictionary) != 2)
    {
      printf("\n\tFAIL: dictionary count is %ld (!= 2)", (long)kn_DictionaryGetCount(aMutableDictionary));
      return 1;
    }
  if (!(aValue = kn_DictionaryGetValue(aMutableDictionary, aName1)))
    {
      printf("\n\tFAIL: Value #1 lost\n");
      return 1;
    }
  if (kn_StringCompare(aValue, aValue1))
    {
      printf("\n\tFAIL: Value #1 mismatch (\"");
      kn_StringWriteToStream(aValue, stdout);
      printf("\" != \"");
      kn_StringWriteToStream(aValue1, stdout);
      printf("\")\n");
      return 1;
    }
  if (!(aValue = kn_DictionaryGetValue(aMutableDictionary, aName2)))
    {
      printf("\n\tFAIL: Value #2 lost\n");
      return 1;
    }
  if (kn_StringCompare(aValue, aValue2))
    {
      printf("\n\tFAIL: Value #2 mismatch (\"");
      kn_StringWriteToStream(aValue, stdout);
      printf("\" != \"");
      kn_StringWriteToStream(aValue2, stdout);
      printf("\")\n");
      return 1;
    }
  kn_Release(aMutableDictionary);
  printf("OK\n");

  kn_Release(aValue4);
  kn_Release(aValue3);
  kn_Release(aValue2);
  kn_Release(aValue1);
  kn_Release(aName4 );
  kn_Release(aName3 );
  kn_Release(aName2 );
  kn_Release(aName1 );

  printf("\n"); fflush(stdout);
  return 0;
}

int server ()
{
  kn_ServerRef aServer;

  printf("Testing server object:\n");

  printf("server/init..."); fflush(stdout);
  {
    kn_StringRef aServerURI = kn_StringCreateWithCString(gPubSubURI);
    kn_StringRef aUserName  = (gPubSubUser    ) ? kn_StringCreateWithCString(gPubSubUser    ) : NULL;
    kn_StringRef aPassword  = (gPubSubPassword) ? kn_StringCreateWithCString(gPubSubPassword) : NULL;
    if (!(aServer = kn_ServerCreateWithURIUserPassword(aServerURI, aUserName, aPassword)))
      {
        printf("\n\tFAIL: kn_ServerCreateWithURIUserPassword returned NULL: %s\n", strerror(errno));
        return 1;
      }
    kn_Release(aServerURI);
    kn_Release(aUserName );
    kn_Release(aPassword );
  }
  printf("OK\n");

  kn_Release(aServer  );

  printf("\n"); fflush(stdout);
  return 0;
}

int event ()
{
  kn_EventRef        anEvent;
  kn_MutableEventRef aMutableEvent;
  kn_DictionaryRef   aDictionary;

  kn_StringRef kName1  = KNSTR("Name #1");
  kn_StringRef kName2  = KNSTR("Name the 2nd");
  kn_StringRef kName3  = KNSTR("3rd name");
  kn_StringRef kName4  = KNSTR("Name 4");
  kn_StringRef kValue1 = KNSTR("This is value #1");
  kn_StringRef kValue2 = KNSTR("This is value the 2nd");
  kn_StringRef kValue3 = KNSTR("This is the 3rd value");
  kn_StringRef kValue4 = KNSTR("This is value 4");

  kn_StringRef aValue;

  printf("Testing event object:\n");

  printf("event/init.cap..."); fflush(stdout);
  aMutableEvent = kn_EventCreateMutableWithCapacity(100);
  if (!aMutableEvent)
    {
      printf("\n\tFAIL: kn_EventCreateMutableWithCapacity returned NULL: %s\n", strerror(errno));
      return 1;
    }
  kn_Release(aMutableEvent);
  printf("OK\n");

  printf("event/init.dict..."); fflush(stdout);
  aDictionary = kn_DictionaryCreateMutableWithNamesAndValues(kName1, kValue1,
							     kName2, kValue2,
							     kName3, kValue3,
							     kName4, kValue4,
							     NULL  , NULL   );
  anEvent = kn_EventCreateWithDictionary(aDictionary);
  kn_Release(aDictionary);
  if (!anEvent)
    {
      printf("\n\tFAIL: kn_EventCreateWithDictionary returned NULL: %s\n", strerror(errno));
      return 1;
    }
  if (!(aValue = kn_EventGetValue(anEvent, kName1)))
    {
      printf("\n\tFAIL: Value #1 lost\n");
      return 1;
    }
  if (kn_StringCompare(aValue, kValue1))
    {
      printf("\n\tFAIL: Value #1 mismatch (\"");
      kn_StringWriteToStream(aValue, stdout);
      printf("\" != \"");
      kn_StringWriteToStream(kValue1, stdout);
      printf("\")\n");
      return 1;
    }
  if (!(aValue = kn_EventGetValue(anEvent, kName3)))
    {
      printf("\n\tFAIL: Value #3 lost\n");
      return 1;
    }
  if (kn_StringCompare(aValue, kValue3))
    {
      printf("\n\tFAIL: Value #3 mismatch (\"");
      kn_StringWriteToStream(aValue, stdout);
      printf("\" != \"");
      kn_StringWriteToStream(kValue3, stdout);
      printf("\")\n");
      return 1;
    }
  printf("OK\n");

  printf("event/init.copy..."); fflush(stdout);
  aMutableEvent = kn_EventCreateMutableCopy(anEvent);
  if (!aMutableEvent)
    {
      printf("\n\tFAIL: kn_EventCreateMutableCopy returned NULL\n");
      return 1;
    }
  if (!(aValue = kn_EventGetValue(aMutableEvent, kName2)))
    {
      printf("\n\tFAIL: Value #2 lost\n");
      return 1;
    }
  if (kn_StringCompare(aValue, kValue2))
    {
      printf("\n\tFAIL: Value #2 mismatch (\"");
      kn_StringWriteToStream(aValue, stdout);
      printf("\" != \"");
      kn_StringWriteToStream(kValue2, stdout);
      printf("\")\n");
      return 1;
    }
  if (!(aValue = kn_EventGetValue(aMutableEvent, kName4)))
    {
      printf("\n\tFAIL: Value #4 lost\n");
      return 1;
    }
  if (kn_StringCompare(aValue, kValue4))
    {
      printf("\n\tFAIL: Value #4 mismatch (\"");
      kn_StringWriteToStream(aValue, stdout);
      printf("\" != \"");
      kn_StringWriteToStream(kValue4, stdout);
      printf("\")\n");
      return 1;
    }
  kn_Release(aMutableEvent);
  printf("OK\n");

  kn_Release(anEvent);

  printf("event/append..."); fflush(stdout);
  aMutableEvent = kn_EventCreateMutableWithCapacity(1);
  if (!aMutableEvent)
    {
      printf("\n\tFAIL: kn_EventCreateMutableWithCapacity returned NULL: %s\n", strerror(errno));
      return 1;
    }
  kn_EventSetValue(aMutableEvent, kName1, kValue1);
  kn_EventSetValue(aMutableEvent, kName2, kValue2);
  if (!(aValue = kn_EventGetValue(aMutableEvent, kName1)))
    {
      printf("\n\tFAIL: Value #1 lost\n");
      return 1;
    }
  if (kn_StringCompare(aValue, kValue1))
    {
      printf("\n\tFAIL: Value #1 mismatch (\"");
      kn_StringWriteToStream(aValue, stdout);
      printf("\" != \"");
      kn_StringWriteToStream(kValue1, stdout);
      printf("\")\n");
      return 1;
    }
  if (!(aValue = kn_EventGetValue(aMutableEvent, kName2)))
    {
      printf("\n\tFAIL: Value #2 lost\n");
      return 1;
    }
  if (kn_StringCompare(aValue, kValue2))
    {
      printf("\n\tFAIL: Value #2 mismatch (\"");
      kn_StringWriteToStream(aValue, stdout);
      printf("\" != \"");
      kn_StringWriteToStream(kValue2, stdout);
      printf("\")\n");
      return 1;
    }
  kn_Release(aMutableEvent);
  printf("OK\n");

  printf("\n"); fflush(stdout);
  return 0;
}

static int route_to_topic_status    = 0;
static int route_to_function_status = 0;
static int route_to_function_event  = 0;
static int send_status              = 0;

static int error = 0;

static void status_handler(kn_EventRef anEvent, void* aJunk)
{
  kn_StringRef anEventDescription = kn_CopyDescription(anEvent);

  const char* aSender = (const char*)aJunk;

       if (!strcmp("-rtt-" , aSender)) ++route_to_topic_status;
  else if (!strcmp("-send-", aSender)) ++send_status;
  else                                 ++route_to_function_status;

#if 0
  printf("\n\nReceived status event (%s): ", aSender);
  kn_StringWriteToStream(anEventDescription, stdout);
  printf("\n\n");
#endif

  kn_Release(anEventDescription);
}

static void event_handler(kn_EventRef anEvent, void* aJunk)
{
  kn_StringRef aHello   = (kn_StringRef)aJunk;
  kn_StringRef aPayload = kn_EventGetValue(anEvent, kn_EVENT_PAYLOAD_HEADER_NAME);
  kn_StringRef aBobo    = kn_EventGetValue(anEvent, BOBO_HEADER_NAME);

  route_to_function_event++;

  if (aPayload && aBobo)
    {
      if (kn_StringCompare(aPayload, aHello))
        {
          kn_StringRef anEventDescription = kn_CopyDescription(anEvent);
          printf("\n\tFAIL: Event payload does not match expected value (\"");
          kn_StringWriteToStream(aPayload, stdout);
          printf("\" != \"");
          kn_StringWriteToStream(aHello, stdout);
          printf("\") Event is:\n");
          kn_StringWriteToStream(anEventDescription, stdout);
          printf("\n");
          kn_Release(anEventDescription);
          error = 1;
        }
      if (kn_StringCompare(aBobo, aHello))
        {
          kn_StringRef anEventDescription = kn_CopyDescription(anEvent);
          printf("\n\tFAIL: Event bobo does not match expected value (\"");
          kn_StringWriteToStream(aBobo, stdout);
          printf("\" != \"");
          kn_StringWriteToStream(aHello, stdout);
          printf("\") Event is:\n");
          kn_StringWriteToStream(anEventDescription, stdout);
          printf("\n");
          kn_Release(anEventDescription);
          error = 1;
        }
    }
  else
    {
      kn_StringRef anEventDescription = kn_CopyDescription(anEvent);
      printf("\n\tFAIL: Missing payload or bobo in event:");
      kn_StringWriteToStream(anEventDescription, stdout);
      printf("\n");
      kn_Release(anEventDescription);
      error = 1;
    }
}

int message ()
{
  kn_Error            anError;
  kn_ServerRef        aServer;
  kn_RouteRef         aRouteToTopic;
  kn_RouteRef         aRouteToFunction;
  kn_MutableStringRef aPubTopic = kn_StringCreateMutableCopy(PUBSUB_PUB_TOPIC);
  kn_StringRef        aHello    = kn_StringCreateWithCString(HELLO_WORLD);
  kn_StringRef 		  aProxyHost = gProxyHost ? kn_StringCreateWithCString(gProxyHost) : NULL;
  unsigned short int  aProxyPort = PROXY_PORT; 

  {
    kn_EventRef  anEvent = kn_EventCreateMutable();
    kn_StringRef aKey    = kn_EVENT_ID_HEADER_NAME;
    kn_StringAppendString(aPubTopic, kn_EventGetValue(anEvent, aKey));
    kn_Release(aKey);
    kn_Release(anEvent);
  }

  printf("Testing publish and subscribe:\n");

  {
    kn_StringRef aServerURI = kn_StringCreateWithCString(gPubSubURI);
    kn_StringRef aUserName  = (gPubSubUser    ) ? kn_StringCreateWithCString(gPubSubUser    ) : NULL;
    kn_StringRef aPassword  = (gPubSubPassword) ? kn_StringCreateWithCString(gPubSubPassword) : NULL;
	kn_StringRef	aProxyUser = (gProxyUser) ? kn_StringCreateWithCString(gProxyUser) : NULL;
	kn_StringRef	aProxyPasswd = (gProxyPasswd) ? kn_StringCreateWithCString(gProxyPasswd) : NULL;
    aServer = kn_ServerCreateWithURIUserPassword(aServerURI, aUserName, aPassword);

    if (aProxyHost) kn_ServerSetProxy(aServer, aProxyHost, aProxyPort, aProxyUser, aProxyPasswd); 
    kn_Release(aServerURI);
    kn_Release(aProxyHost);
    kn_Release(aUserName );
    kn_Release(aPassword );
	kn_Release(aProxyUser);
	kn_Release(aProxyPasswd);
  }

  printf("route/create.totopic..."); fflush(stdout);
  {
    if (!(aRouteToTopic = kn_RouteCreateFromTopicToTopicViaServer (aPubTopic, PUBSUB_SUB_TOPIC,
                                                                   aServer, NULL, status_handler, "-rtt-",
                                                                   kn_TRUE)))
      {
        printf("\n\tFAIL: kn_RouteCreateFromTopicToTopicViaServer returned NULL: %s\n", strerror(errno));
        printf("WARNING: Skipping pub/sub tests.\n");
        return 2;
      }
    if (route_to_topic_status != 1)
      {
        printf("\n\tFAIL: %s (%d) status event for route->topic.\n",
	       (route_to_topic_status == 0) ? "No" : "More than one",
	       route_to_topic_status);
        return 1;
      }
  }
  printf("OK\n");

  printf("route/create.tofunction..."); fflush(stdout);
  {
    if (!(aRouteToFunction = kn_RouteCreateFromTopicToFunctionViaServer (PUBSUB_SUB_TOPIC, event_handler,
                                                                         aServer, NULL, status_handler, (void*)aHello,
                                                                         kn_TRUE)))
      {
        printf("\n\tFAIL: kn_RouteCreateFromTopicToFunctionViaServer returned NULL: %s\n", strerror(errno));
        return 1;
      }
    if (route_to_function_status != 1)
      {
        printf("\n\tFAIL: %s (%d) status event for route->function.\n",
	       (route_to_function_status == 0) ? "No" : "More than one",
	       route_to_function_status);
        return 1;
      }
  }
  printf("OK\n");

  printf("event/send..."); fflush(stdout);
  {
    kn_DictionaryRef aDictionary =
      kn_DictionaryCreateMutableWithNamesAndValues(KNSTR("displayname")        , KNSTR("kn_test.c"),
                                                   kn_EVENT_PAYLOAD_HEADER_NAME, aHello            ,
                                                   BOBO_HEADER_NAME            , aHello            ,
                                                   NULL                        , NULL              );
    kn_EventRef anEvent = kn_EventCreateWithDictionary(aDictionary);
    anError = kn_ServerPublishEventToTopic(aServer, anEvent, aPubTopic, status_handler, "-send-", kn_TRUE);
    if (anError)
      {
        printf("\n\tFAIL: kn_ServerPublishEventToTopic returned error %d.\n", anError);
        return 1;
      }
    kn_Release(anEvent    );
    kn_Release(aDictionary);
    if (send_status != 1)
      {
        printf("\n\tFAIL: No status event for send.\n");
        return 1;
      }
  }
  printf("OK\n");

  printf("event/get..."); fflush(stdout);
  if ((anError = kn_ServerProcessNextEvent(aServer)))
    {
      printf("\n\tFAIL: kn_ServerProcessNextEvent returned error (%d).\n", anError);
      return 1;
    }
  if (route_to_function_event != 1)
    {
      printf("\n\tFAIL: No event for send via route->function (%d).\n", route_to_function_event);
      return 1;
    }
  if (error) return error;
  printf("OK\n");

  printf("route/delete..."); fflush(stdout);
  if ((anError = kn_RouteDelete(aRouteToTopic, NULL, NULL, kn_TRUE)))
    {
      printf("\n\tFAIL: kn_RouteDelete returned error %d.\n", anError);
      return 1;
    }
  {
    kn_DictionaryRef aDictionary =
      kn_DictionaryCreateMutableWithNamesAndValues(KNSTR("displayname")        , KNSTR("kn_test"                    ),
                                                   kn_EVENT_PAYLOAD_HEADER_NAME, KNSTR("This should not get routed."),
                                                   NULL                        , NULL                               );
    kn_EventRef      anEvent = kn_EventCreateWithDictionary(aDictionary);
    anError = kn_ServerPublishEventToTopic(aServer, anEvent, aPubTopic, NULL, NULL, kn_TRUE);
    if (anError)
      {
        printf("\n\tFAIL: kn_ServerPublishEventToTopic returned error %d.\n", anError);
        return 1;
      }
    kn_Release(anEvent    );
    kn_Release(aDictionary);
  }
  if (kn_ServerProcessEvents(aServer) != 0)
    printf("\n\tWARNING: kn_ServerProcessEvents found unexpected events.\n");
  sleep(5);
  if (kn_ServerProcessEvents(aServer) != 0)
    printf("\n\tWARNING: kn_ServerProcessEvents found unexpected events.\n");
  if (route_to_function_event != 1)
    {
      printf("\n\tFAIL: Too many events for send (%d != 1).\n", route_to_function_event);
      return 1;
    }
  if ((anError = kn_RouteDelete(aRouteToFunction, NULL, NULL, kn_TRUE)))
    {
      printf("\n\tFAIL: kn_RouteDelete returned error %d.\n", anError);
      return 1;
    }
  printf("OK\n");

  kn_Release(aHello          );
  kn_Release(aServer         );
  kn_Release(aRouteToTopic   );
  kn_Release(aRouteToFunction);
  kn_Release(aPubTopic       );

  printf("\n"); fflush(stdout);
  return 0;
}

void fail ()
{
  printf("Test suite failed.\n");
  exit(1);
}

int main (int anArgCount, char *anArgs[])
{
  if (anArgCount >= 2) gPubSubURI      = anArgs[1];
  if (anArgCount >= 3) gPubSubUser     = anArgs[2];
  if (anArgCount >= 4) gPubSubPassword = anArgs[3];

  srandom (time(NULL) ^ (getpid() << 16));

  printf("Running test suite against %s:\n\n", gPubSubURI);

  if (reference_count  () == 1) fail();
  if (string           () == 1) fail();
  if (string_uri       () == 1) fail();
  if (string_webencode () == 1) fail();
  if (dictionary       () == 1) fail();
  if (server           () == 1) fail();
  if (event            () == 1) fail();
  if (message          () == 1) fail();

  printf("All tests passed.\n");

  exit(0);
}
