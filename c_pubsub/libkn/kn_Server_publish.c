/**
 * PubSub Client Library
 * Event routines (publish)
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

#define _POST_ID_ "$Id: kn_Server_publish.c,v 1.4 2004/04/19 05:39:08 bsittler Exp $"

/**
 * Actions
 **/

static kn_StringRef kn_EventCreateStringByEncodingAsPostFields (kn_EventRef anEvent, kn_StringRef aTopic)
{
  kn_MutableStringRef aPostFields  = kn_StringCreateMutableWithSize(0);
  size_t              aHeaderCount = kn_EventGetCount(anEvent);
  kn_StringRef*       aNames       = (kn_StringRef*)malloc(aHeaderCount*sizeof(kn_StringRef));
  kn_StringRef*       aValues      = (kn_StringRef*)malloc(aHeaderCount*sizeof(kn_StringRef));

  if (aNames && aValues)
    {
      kn_StringRef kFormatName = KNSTR("kn_response_format");
      kn_StringRef kTopicName  = KNSTR("kn_to"             );

      kn_EventGetNamesAndValues(anEvent, aNames, aValues);

      if (kn_StringAppendPostField(aPostFields, kFormatName, KNSTR("simple"))) goto abort;
      if (kn_StringAppendPostField(aPostFields, kTopicName , aTopic         )) goto abort;

      while (aHeaderCount--)
        {
          kn_StringRef aName = aNames[aHeaderCount];

          /* Don't pass on response format or topic name in the event, if any; we just provided our own. */
          if ( kn_StringCompare(aName, kFormatName) &&
               kn_StringCompare(aName, kTopicName ) )
            if (kn_StringAppendPostField(aPostFields, aNames[aHeaderCount], aValues[aHeaderCount])) goto abort;
        }
    }
  else
    {
abort:
      kn_Release(aPostFields);
      aPostFields = NULL;
    }

  free(aNames );
  free(aValues);

  return aPostFields;
}

kn_Error kn_ServerPublishEventToTopic (kn_ServerRef aServerRef, kn_EventRef anEvent, kn_StringRef aTopic,
                                       kn_EventHandler aStatusHandler, void* aUserData, kn_BOOL aWaitFlag)
{
  kn_Error   anError = kn_SUCCESS;
  kn_Server* aServer = (kn_Server*)aServerRef;

  if (kn_ServerConnect(aServer, &aServer->post)) return kn_IOERROR;

  if (aServer->post.state == kStateSendHTTPRequest)
    {
      kn_MutableStringRef aRequest    = kn_StringCreateMutable();
      kn_StringRef	  aPath       = kn_StringCreateWithPathFromURI(aServer->uri);
      kn_StringRef        aHostName   = kn_StringCreateWithHostNameFromURI(aServer->uri);
      kn_StringRef        aPostFields = kn_EventCreateStringByEncodingAsPostFields(anEvent, aTopic);
      char                aContentLength[1024]; /* FIXME: we can compute the correct buffer size */

      sprintf(aContentLength, "%ld", (long int)kn_StringGetSize(aPostFields));

      kn_StringAppendCString(aRequest, "POST "                           );
      if (aServer->proxy_host
#ifdef HAVE_OPENSSL
	  && !aServer->post.ssl_connection
#endif /* HAVE_OPENSSL */
	  )
        kn_StringAppendString (aRequest, aServer->uri                    );
      else
        kn_StringAppendString (aRequest, aPath				 ); 
      kn_StringAppendCString(aRequest, " HTTP/1.0\r\n"                   );
      kn_StringAppendString (aRequest, aServer->auth_header              );
      kn_StringAppendCString(aRequest, aServer->auth_header ? "\r\n" : "");
      kn_StringAppendString (aRequest, aServer->proxy_auth               );
      kn_StringAppendCString(aRequest, aServer->proxy_auth ?  "\r\n" : "");
      kn_StringAppendCString(aRequest, "Host: "                          );
      kn_StringAppendString (aRequest, aHostName                         );
      kn_StringAppendCString(aRequest, "\r\n"                            );
      kn_StringAppendCString(aRequest, "Content-type: text/plain\r\n"    );
      kn_StringAppendCString(aRequest, "Content-length: "                );
      kn_StringAppendCString(aRequest, aContentLength                    );
      kn_StringAppendCString(aRequest, "\r\n\r\n"                        );
      kn_StringAppendString (aRequest, aPostFields                       );

      kn_Release(aPostFields);
      kn_Release(aHostName  );

#ifdef DEBUG_SERVER_IO
      printf("Sending publish request:\n");
      kn_StringWriteToStream(aRequest, stdout);
      printf("\n");
#endif

#ifdef HAVE_OPENSSL
      if (aServer->post.ssl_connection)
	{
	  ssize_t aWrote  = 0;
	  ssize_t aResult = SSL_write(aServer->post.ssl_connection,
				      kn_StringGetBytes(aRequest)+aWrote,
				      kn_StringGetSize (aRequest)-aWrote);

	  switch (SSL_get_error(aServer->post.ssl_connection, aResult))
	    {
	    case SSL_ERROR_NONE:
	      aWrote += aResult;
	      break;
	    case SSL_ERROR_WANT_WRITE:
	    default:
	      anError = kn_IOERROR;
	    }
	}
      else
#endif
	{
	  if (write(aServer->post.socket, kn_StringGetBytes(aRequest), kn_StringGetSize(aRequest))
	      != kn_StringGetSize(aRequest))
	    anError = kn_IOERROR;
	}

      kn_Release(aRequest);

      aServer->post.state = kStateReadHTTPProtocol;
    }

#ifdef DEBUG
      if (anError) printf("Read response failed with error %d.\n", anError);
#endif

  anError = kn_ServerReadResponse(aServer, &aServer->post, aStatusHandler, aUserData);

  kn_ServerDisconnect(aServer, &aServer->post);

  return anError;
}
