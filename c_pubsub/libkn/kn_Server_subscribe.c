/**
 * PubSub Client Library
 * PubSub Client Library (subscribe)
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

#define _GET_ID_ "$Id: kn_Server_subscribe.c,v 1.2 2003/03/13 00:42:27 ifindkarma Exp $"

/**
 * Actions
 **/

kn_Error kn_ServerWaitForEventData (kn_ServerRef aServerRef)
{
  kn_Server* aServer = (kn_Server*)aServerRef;
  kn_Error   anError = kn_SUCCESS;
  fd_set     aReadFDs;

  if ((anError = kn_ServerConnect(aServer, &aServer->event))) return anError;

  FD_ZERO(&aReadFDs); FD_SET(aServer->event.socket, &aReadFDs);

  if (select(aServer->event.socket+1, &aReadFDs, NULL, NULL, NULL) == -1)
    return kn_IOERROR;

  return anError;
}

static void kn_ServerProcessEvent (kn_Server* aServer, kn_EventRef anEvent)
{
  if (aServer->callbacks)
    {
      kn_StringRef aRouteLocation = kn_EventGetValue(anEvent, kn_EVENT_ROUTE_LOCATION_HEADER_NAME);

      if (aRouteLocation)
	{
	  kn_EventCallbackRef aCallback = kn_DictionaryGetValue(aServer->callbacks, aRouteLocation);

	  if (aCallback) aCallback->destination(anEvent, aCallback->user_data);
	}
    }
}

kn_Error kn_ServerProcessNextEvent (kn_ServerRef aServerRef)
{
  kn_Server*  aServer = (kn_Server*)aServerRef;
  kn_EventRef anEvent;

  while (!(anEvent = kn_ServerCreateNextEvent(aServer, &aServer->event)))
    {
      if (errno == EAGAIN)
        {
	  kn_Error anError = kn_ServerWaitForEventData(aServer);
	  if (anError) return anError;
	}
      else
        break;
    }

  if (!anEvent) return kn_FAIL;

  kn_ServerProcessEvent(aServer, anEvent);

  return kn_SUCCESS;
}

size_t kn_ServerProcessEvents (kn_ServerRef aServerRef)
{
  kn_Server*  aServer = (kn_Server*)aServerRef;
  size_t      aCount  = 0;
  kn_EventRef anEvent;

  while ((anEvent = kn_ServerCreateNextEvent(aServer, &aServer->event)))
    {
      kn_ServerProcessEvent(aServer, anEvent);

      kn_Release(anEvent);

      if (++aCount > 4) break;
    }

  return aCount;
}

/**
 * Internal hooey
 **/

static kn_Error kn_ServerGenerateCallbackURI (kn_Server* aServer)
{
  if (aServer->event.state == kStateBegin)
    {
      kn_Release(aServer->callback_uri);
      aServer->callback_uri = NULL;
    }

  if (aServer->callback_uri == NULL)
    {
      kn_MutableStringRef aCallbackURI = kn_StringCreateMutable();
  
      char aBarf[1024] = ""; /* FIXME: Figure out the real size some time. */
  
      /* Huk, huk, huk, blorp! */
      snprintf(aBarf, sizeof(aBarf), "libkn.server_%ld.%ld.%ld",
                (long int)aServer->id,
                (long int)aServer,
                (time(NULL) ^ (getpid() << 16)));

      kn_StringAppendCString(aCallbackURI, "/what/apps/libkn/"    );
      kn_StringAppendCString(aCallbackURI, aBarf                  );
      kn_StringAppendString (aCallbackURI, kn_TOPIC_JOURNAL_SUFFIX);
  
      aServer->callback_uri = aCallbackURI;
    }

  return kn_SUCCESS;
}

static kn_Error kn_ServerSendSubscribeRequest (kn_Server* aServer)
{
  if (kn_ServerConnect(aServer, &aServer->event)) return kn_IOERROR;

  if (aServer->event.state == kStateSendHTTPRequest)
    {
      kn_Error            anError   = kn_SUCCESS;
      kn_MutableStringRef aRequest  = kn_StringCreateMutable();
      kn_StringRef        aPath     = kn_StringCreateWithPathFromURI(aServer->uri);
      kn_StringRef        aHostName = kn_StringCreateWithHostNameFromURI(aServer->uri);

      kn_StringAppendCString(aRequest, "GET "                               );
	  if (aServer->proxy_host
#ifdef HAVE_OPENSSL
	      && !aServer->event.ssl_connection
#endif /* HAVE_OPENSSL */
	      )
      	kn_StringAppendString (aRequest, aServer->uri                       );
      else
        kn_StringAppendString (aRequest, aPath                              );
      kn_StringAppendCString(aRequest, "?kn_response_format=simple;kn_from=");
      kn_StringAppendString (aRequest, aServer->callback_uri                );
      kn_StringAppendCString(aRequest, " HTTP/1.0\r\n"                      );
      kn_StringAppendString (aRequest, aServer->auth_header                 );
      kn_StringAppendCString(aRequest, aServer->auth_header ? "\r\n" : ""   );
      kn_StringAppendString (aRequest, aServer->proxy_auth                  );
      kn_StringAppendCString(aRequest, aServer->proxy_auth ?  "\r\n" : ""   );
      kn_StringAppendCString(aRequest, "Host: "                             );
      kn_StringAppendString (aRequest, aHostName                            );
      kn_StringAppendCString(aRequest, "\r\n\r\n"                           );

#ifdef DEBUG_SERVER_IO
      printf("Sending subscribe request:\n");
      kn_StringWriteToStream(aRequest, stdout);
      printf("\n");
#endif

#ifdef HAVE_OPENSSL
      if (aServer->event.ssl_connection)
	{
	  ssize_t aWrote  = 0;
	  ssize_t aResult = SSL_write(aServer->event.ssl_connection,
				      kn_StringGetBytes(aRequest)+aWrote,
				      kn_StringGetSize (aRequest)-aWrote);

	  switch (SSL_get_error(aServer->event.ssl_connection, aResult))
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
	  if (write(aServer->event.socket, kn_StringGetBytes(aRequest), kn_StringGetSize(aRequest))
	      != kn_StringGetSize(aRequest))
	    anError = kn_IOERROR;
	}

      kn_Release(aRequest );
      kn_Release(aHostName);

      if (!anError) aServer->event.state = kStateReadHTTPProtocol;

#ifdef DEBUG
      if (anError) printf("Subscribe request failed with error %d.\n", anError);
#endif

      return anError;
    }

  return kn_INVALID;
}

static kn_Error kn_ServerStartTunnel (kn_Server* aServer)
{
  kn_Error anError;

  if ( (anError = kn_ServerGenerateCallbackURI (aServer                             )) ||
       (anError = kn_ServerSendSubscribeRequest(aServer                             )) ||
       (anError = kn_ServerReadResponse        (aServer, &aServer->event, NULL, NULL)) )
    {
      fprintf(stderr, "Error (%d) starting tunnel connection; aborting.\n", anError);
      kn_ServerDisconnect(aServer, &aServer->event);
    }

  return anError;
}
