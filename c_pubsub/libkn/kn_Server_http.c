/**
 * PubSub Client Library
 * PubSub Client Library (HTTP+KN protocol)
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

#define _PROTO_ID_ "$Id: kn_Server_http.c,v 1.2 2003/03/07 06:16:08 wsanchez Exp $"

#include <sysexits.h>

#ifdef HAVE_OPENSSL
#include <openssl/err.h>
#include "entropy.c"

/* Eit.  OpenSSL is an init routine.  That's bogus, but we get stuck with it. */
static int gOpenSSLInitialized = 0;
#endif /* HAVE_OPENSSL */

static kn_EventRef kn_ServerCreateNextEvent (kn_Server* aServer, kn_Server_Connection* aConnection);
static void        kn_ServerDisconnect      (kn_Server* aServer, kn_Server_Connection* aConnection);

static void kn_ServerProcessEventsForDescriptor (kn_ServerRef aServer, int aFileDescriptor)
{
  if (aFileDescriptor == aServer->event.socket) kn_ServerProcessEvents(aServer);
}

static kn_Error kn_ServerSetSSLTunnel(kn_Server* aServer, kn_Server_Connection* aConnection);

static kn_Error kn_ServerConnect (kn_Server* aServer, kn_Server_Connection* aConnection)
{
  if (aConnection->socket == -1)
    {
      kn_StringRef 		 aHostName = NULL;
      struct hostent*    aHostEntry;
      struct sockaddr_in aSocketAddress;

      if (aServer->proxy_port != 0)
        {
          aHostName = kn_StringCreateCopy(aServer->proxy_host);
        }
      else
        {
	  aHostName = kn_StringCreateWithHostNameFromURI(aServer->uri);
        }

      if (aConnection->state != kStateBegin) kn_ServerDisconnect(aServer, aConnection);

      if (!aHostName)
	{
#ifdef DEBUG
	  fprintf(stderr, "Server %p has invalid URI (%s).\n", aServer, kn_StringGetBytes(aServer->uri));
#endif
	  return kn_INVALID;
	}

#ifdef DEBUG_CONNECT
      fprintf(stderr, "Connecting to server %s (%p)...", kn_StringGetBytes(aHostName), aServer);
#endif

      if (!(aHostEntry = gethostbyname(kn_StringGetBytes(aHostName)))) /* FIXME: This might block. */
        {
#ifdef DEBUG_CONNECT
          fprintf(stderr, "failed: %s\n", strerror(errno));
#endif
          return kn_NOENTRY;
        }

      kn_Release(aHostName);

#ifdef BSD
      aSocketAddress.sin_len    = sizeof(aSocketAddress);
#endif
      aSocketAddress.sin_family = AF_INET;
      if (aServer->proxy_port != 0)
        {
          aSocketAddress.sin_port = htons(kn_ServerGetProxyPort(aServer));
        }
      else
        {
          aSocketAddress.sin_port   = htons(kn_StringGetPortFromURI(aServer->uri));
        }
      memcpy(&aSocketAddress.sin_addr, aHostEntry->h_addr_list[0], sizeof(aSocketAddress.sin_addr));
      memset(&aSocketAddress.sin_zero, '\0', sizeof(aSocketAddress.sin_zero));

      if ((aConnection->socket = socket(PF_INET, SOCK_STREAM, 0)) == -1)
	return kn_IOERROR;

      if (connect(aConnection->socket, (struct sockaddr*)&aSocketAddress, sizeof(aSocketAddress)))
	{
#ifdef DEBUG_CONNECT
	  fprintf(stderr, "failed: %s\n", strerror(errno));
#endif
	  return kn_NOTCONNECTED;
	}

#ifdef DEBUG_CONNECT
      fprintf(stderr, "connected.\n");
#endif

#ifdef HAVE_OPENSSL
      if (kn_StringGetProtocolFromURI(aServer->uri) == kn_PROTOCOL_HTTPS)
	{
	  kn_Error anError = kn_SUCCESS;

#ifdef DEBUG_CONNECT
	  fprintf(stderr, "Setting up SSL...");
#endif

	  /* Initialize OpenSSL */
	  if (! gOpenSSLInitialized)
	    {
	      SSL_library_init();

	      if (SSLeay() != OPENSSL_VERSION_NUMBER)
		{
		  fprintf(stderr, "OpenSSL version mismatch."
			          "Built against %lx; running %lx",
			  OPENSSL_VERSION_NUMBER, SSLeay());
		  errno = ELIBBAD;
		  anError = kn_INTERNAL;
		  goto fail;
		}

	      if (!init_prng()) { if (!errno) errno = EACCES; anError = kn_FAIL; goto fail; }
	    }

	  /* Initialize SSL context */
	  if (! aServer->ssl_context) aServer->ssl_context = SSL_CTX_new(SSLv23_client_method());
	  if (! aServer->ssl_context) { anError = kn_MEMORYFAIL; errno = ENOMEM; goto fail; }

          /** setup SSL tunneling when using Proxy with SSL */ 
          if (aServer->proxy_port != 0 && (anError = kn_ServerSetSSLTunnel(aServer, aConnection)) != kn_SUCCESS)
            goto fail;

	  /* Initialize the SSL connection */
	  if (! (aConnection->ssl_connection = SSL_new(aServer->ssl_context)))
	    { anError = kn_MEMORYFAIL; errno = ENOMEM; goto fail; }

	  SSL_set_connect_state(aConnection->ssl_connection);

	  if (! SSL_set_fd(aConnection->ssl_connection, aConnection->socket))
	    { anError = kn_NOTCONNECTED; errno = ENOTCONN; goto fail; }

	  if (!seed_prng()) { anError = kn_FAIL; errno = EACCES; goto fail; }

	  /* SSL handshake */
	  if (SSL_connect(aConnection->ssl_connection) != 1)
	    { anError = kn_NOTCONNECTED; errno = ENOTCONN; goto fail; }

	fail:
	  if (anError)
	    {
#ifdef DEBUG_CONNECT
	      fprintf(stderr, "failed (%d): %s\n", anError, strerror(errno));
#endif
#if defined(DEBUG) || defined(DEBUG_CONNECT)
	      SSL_load_error_strings();
	      ERR_print_errors_fp(stderr);
#endif
	      kn_ServerDisconnect(aServer, aConnection);
	      return anError;
	    }
	  else
	    {
#ifdef DEBUG_CONNECT
	      fprintf(stderr, "established.\n");
#endif
	    }
	}
#endif /* HAVE_OPENSSL */

      if (aServer->connect_callback)
        {
          if (aConnection == &aServer->post)
              aServer->connect_callback(aServer, aConnection->socket,
                                        NULL, kn_ServerProcessEventsForDescriptor, NULL,
                                        aServer->connect_userdata);
          else if (aConnection == &aServer->event)
              aServer->connect_callback(aServer, aConnection->socket,
                                        kn_ServerProcessEventsForDescriptor, NULL, NULL,
                                        aServer->connect_userdata);
        }

      aConnection->state = kStateSendHTTPRequest;
    }

  return kn_SUCCESS;
}

static void kn_ServerDisconnect (kn_Server* aServer, kn_Server_Connection* aConnection)
{
#ifdef DEBUG_CONNECT
  fprintf(stderr, "Disconnecting from server %p.\n", aServer);
#endif

#ifdef HAVE_OPENSSL
  if (aConnection->ssl_connection) SSL_shutdown(aConnection->ssl_connection);
#endif

  if (aConnection->socket != -1)
    {
      if (aServer->connect_callback)
        aServer->connect_callback(aServer, aConnection->socket, NULL, NULL, NULL, aServer->connect_userdata);

      close (aConnection->socket);
    }

  free(aConnection->buf);

  aConnection->socket         = -1;
#ifdef HAVE_OPENSSL
  aConnection->ssl_connection = NULL;
#endif
  aConnection->buf            = NULL;
  aConnection->buf_used       =  0;
  aConnection->buf_size       =  0;
  aConnection->state          = kStateBegin;
}

static kn_Error kn_ServerReadResponse (kn_Server* aServer, kn_Server_Connection* aConnection,
                                       kn_EventHandler aStatusHandler, void* aUserData)
{
  unsigned int aStatus = 0;

  if ( aConnection->state < kStateReadHTTPProtocol ||
       aConnection->state > kStateReadStatusEvent  )
    return kn_INVALID;

  if (aConnection->state < kStateReadStatusEvent)
    {
#define EVENT_BUFFER_SIZE BUFSIZ

      char*  aBuffer = (char*)malloc(EVENT_BUFFER_SIZE);
      size_t aUsage  = 0;
      State  aState  = aConnection->state;

      /* Save aBuffer pointer in case we abort. */
      aConnection->buf = aBuffer;

#define rawsizeof(aString) (sizeof(aString)-1)

      while (1)
        {
          ssize_t     aRead = 0;
	  const char* anIndex;

#ifdef HAVE_OPENSSL
	  if (aConnection->ssl_connection)
	    {
	      ssize_t aResult;

	      do {
		aResult = SSL_read(aConnection->ssl_connection, aBuffer+aUsage+aRead, EVENT_BUFFER_SIZE-aUsage-aRead);

		switch (SSL_get_error(aConnection->ssl_connection, aResult))
		  {
		  case SSL_ERROR_NONE:
		    aRead += aResult;
		    break;
		  case SSL_ERROR_WANT_READ:
		  default:
		    errno = ECONNABORTED;
		    return kn_IOERROR;
		  }
	      } while (SSL_pending(aConnection->ssl_connection));
	    }
	  else
#endif /* HAVE_OPENSSL */
	    {
	      aRead = read(aConnection->socket, aBuffer+aUsage, EVENT_BUFFER_SIZE-aUsage);

	      if (aRead == 0 && aUsage < EVENT_BUFFER_SIZE)
		{
#ifdef DEBUG
		  fprintf(stderr, "End-of-file while reading server response.\n");
#endif
		  errno = ECONNABORTED;
		  return kn_IOERROR;
		}

	      if (aRead == -1)
		{
#ifdef DEBUG
		  fprintf(stderr, "Error while reading server response: %s\n", strerror(errno));
#endif
		  return kn_IOERROR;
		}
	    }

#ifdef DEBUG_SERVER_IO
          printf("H(%d): Read %ld bytes: ", aState, (long int)aRead);
          printf("[-");
          fwrite(aBuffer, 1, aUsage, stdout);
          printf("-|-");
          fwrite(aBuffer+aUsage, 1, aRead, stdout);
          printf("-]\n");
#endif

          aUsage += aRead;

	again:
          switch (aState)
            {
            case kStateReadHTTPProtocol: /* Response starts with HTTP version. */
#ifdef DEBUG_PARSER_STATE
	      printf("state = kStateReadHTTPProtocol\n");
#endif
              if (aUsage < rawsizeof("HTTP/")) break;

              if (strncmp(aBuffer, "HTTP/", rawsizeof("HTTP/")))
                {
#ifdef DEBUG
                  fprintf(stderr, "Unexpected response from server.\n");
#endif
                  errno = ECONNABORTED;
                  return kn_IOERROR;
                }

              aState = kStateReadHTTPVersion;

            case kStateReadHTTPVersion: /* Scan past version number and single space after the protocol version. */
#ifdef DEBUG_PARSER_STATE
	      printf("state = kStateReadHTTPVersion\n");
#endif
              if ((anIndex = memchr(aBuffer, ' ', aUsage)))
                {
                  memmove(aBuffer, anIndex+1, (aBuffer+aUsage)-(anIndex+1));
                  aUsage -= (anIndex+1)-aBuffer;
                  aState = kStateReadHTTPStatus;
                }
              else
		{
		  aUsage = 0;
		  break;
		}

            case kStateReadHTTPStatus: /* Get the HTTP status code. */
#ifdef DEBUG_PARSER_STATE
	      printf("state = kStateReadHTTPStatus\n");
#endif
#define STATUS_BYTE_SIZE 3
#define STATUS_CODE(aStatus) (((aStatus[0]-'0')*10+aStatus[1]-'0')*10+aStatus[2]-'0')
              if (aUsage < STATUS_BYTE_SIZE) break;

              aStatus = STATUS_CODE(aBuffer);

              memmove(aBuffer, aBuffer+STATUS_BYTE_SIZE, aUsage-STATUS_BYTE_SIZE);
              aUsage -= STATUS_BYTE_SIZE;
              aState  = kStateReadHTTPCR1;

            case kStateReadHTTPCR1: /* Scan past remaining header data. */
            case kStateReadHTTPNL1: /* (Headers end with "\r\n\r\n".)   */
            case kStateReadHTTPCR2:
            case kStateReadHTTPNL2:
#ifdef DEBUG_PARSER_STATE
	      printf("state = kStateReadHTTP{CR,NL}{1,2} (%d)\n", aState-kStateReadHTTPCR1);
#endif
              {
                char aNextChar = '\0';

                switch (aState)
                  {
                  case kStateReadHTTPCR1: case kStateReadHTTPCR2: aNextChar = '\r'; break;
                  case kStateReadHTTPNL1: case kStateReadHTTPNL2: aNextChar = '\n'; break;
                  default: break; /* Not possible. */
                  }

                if ((anIndex = memchr(aBuffer, aNextChar, aUsage)))
                  {
                    /* Optimization: cuts out 3 unnecessary memmove()s. */
                    if ( aState == kStateReadHTTPCR1     &&
                         aBuffer+aUsage > anIndex+3      &&
                         !memcmp(anIndex+1, "\n\r\n", 3) )
                      {
                        memmove(aBuffer, anIndex+1+3, (aBuffer+aUsage)-(anIndex+1+3));
                        aUsage -= (anIndex+1+3)-aBuffer;
                        goto done;
                      }

                    memmove(aBuffer, anIndex+1, (aBuffer+aUsage)-(anIndex+1));
                    aUsage -= (anIndex+1)-aBuffer;

                    switch (aState)
                      {
                      case kStateReadHTTPCR1: aState = kStateReadHTTPNL1; goto again;
                      case kStateReadHTTPNL1: if (anIndex == aBuffer) aState = kStateReadHTTPCR2; else aState = kStateReadHTTPCR1; goto again;
                      case kStateReadHTTPCR2: if (anIndex == aBuffer) aState = kStateReadHTTPNL2; else aState = kStateReadHTTPNL1; goto again;
                      case kStateReadHTTPNL2: if (anIndex == aBuffer) goto done                 ; else aState = kStateReadHTTPCR1; goto again;
                      default: break; /* Not possible. */
                      }
                  }

		aUsage = 0;
              }
              break;

            default: /* We should never get here. */
#ifdef DEBUG
              fprintf(stderr, "YIKES! Internal parser error (state=%d) while setting up tunnel (in headers).\n", aState);
#endif
              errno = ECONNABORTED;
              return kn_INTERNAL;
            }
        } /* while (1) */

    done:
      /* Finished parsing; remember leftover data in the event buffer. */
      aConnection->buf_used = aUsage;
      aConnection->buf_size = EVENT_BUFFER_SIZE;
      aConnection->state    = kStateReadStatusEvent;

#ifdef DEBUG_SERVER_IO
      printf("Remaining event data: \n");
      fwrite(aBuffer, 1, aUsage, stdout);
#endif
    }

  if (aConnection->state == kStateReadStatusEvent)
    {
      kn_EventRef aStatusEvent;

      aConnection->state = kStateReadEventSpacers;

      aStatusEvent = kn_ServerCreateNextEvent(aServer, aConnection);

      /* Do non-blocking I/O on the socket going forward.                         */
      /* FIXME: We should have done most of this setup in a non-blocking fashion. */
      /*        That is, this call belongs with the connect code.                 */
      {
        int aFlags = fcntl(aConnection->socket, F_GETFL, 0);
        if (aFlags != -1) fcntl(aConnection->socket, F_SETFL, aFlags|O_NONBLOCK);
      }

      if (aStatusEvent)
        {
          /* Be pendantic: verify that the status codes match. */
          {
            kn_StringRef aStatusName = kn_StringCreateWithCString("status");

            int anEventStatus =
              (unsigned int)strtol(kn_StringGetBytes(kn_EventGetValue(aStatusEvent, aStatusName)), NULL, 10);

            if (anEventStatus != aStatus)
              {
#ifdef DEBUG
                fprintf(stderr, "Status code mismatch between HTTP response and status event: (%d != %d)\n",
                        anEventStatus, aStatus);
#endif
                aStatus = 0;
              }

            kn_Release(aStatusName);
          }

          /* FIXME: We need to be able to handle some other-than 200 status codes. */
          switch (aStatus)
            {
            case 200: /* OK */
#ifdef DEBUG_SERVER_IO
              fprintf(stderr, "Dispatching status event %p.1\n", aStatusEvent);
#endif
              if (aStatusHandler) aStatusHandler(aStatusEvent, aUserData);
              break;

            default:
#ifdef DEBUG
              fprintf(stderr, "Received unsupported HTTP response status (%d).\n", aStatus);
#endif
              kn_Release(aStatusEvent);
              errno = ECONNABORTED;
              return kn_UNSUPPORTED;
            }

          kn_Release(aStatusEvent);
        }
      else
        {
#ifdef DEBUG
          fprintf(stderr, "Server response failed to yield status event.\n");
#endif
          errno = ECONNABORTED;
          return kn_IOERROR;
        }
    }

  if (aConnection->state < kStateReadEventSpacers) return kn_INVALID;

  return kn_SUCCESS;
}

static kn_EventRef kn_ServerCreateNextEvent (kn_Server* aServer, kn_Server_Connection* aConnection)
{
  kn_EventRef anEvent     = NULL;
  int         aSocket     = aConnection->socket;
#ifdef HAVE_OPENSSL
  SSL*        anSSL       = aConnection->ssl_connection;
#endif
  char*       aBuffer     = aConnection->buf;
  size_t      aUsage      = aConnection->buf_used;
  size_t      aSize       = aConnection->buf_size;
  State       aState      = aConnection->state;
  size_t      anEventSize = 0;
  ssize_t     aRead       = 0;

       if (aState <  kStateReadEventSpacers) { errno = EINVAL; return NULL; }
  else if (aState == kStateReadEventData   ) aState = kStateReadEventSize;

  while (1)
    {
      switch (aState)
        {
        case kStateReadEventSpacers:
#ifdef DEBUG_PARSER_STATE
	  printf("state = kStateReadEventSpacers\n");
#endif
          {
            /* Read past leading spaces */
            const char* anIndex;

            for (anIndex = aBuffer; anIndex < aBuffer+aUsage; anIndex++)
              {
                if (*anIndex != ' ')
                  {
                    aUsage -= anIndex-aBuffer;
                    memmove(aBuffer, anIndex, aUsage);
                    aState = kStateReadEventSize;
		    break;
                  }
              }
          }
	  if (aState != kStateReadEventSize)
	    {
	      aUsage = 0;
	      break;
	    }
	  /* Fall through. */

        case kStateReadEventSize:
#ifdef DEBUG_PARSER_STATE
	  printf("state = kStateReadEventSize\n");
#endif
          {
            /* Get event size */
            const char* anIndex  = memchr(aBuffer, '\n', aUsage);
	          char* anIndex2;

            /* If the size string didn't fit into the buffer, we're in big trouble. */
            if (!anIndex)
              {
#ifdef DEBUG
                fprintf(stderr, "ACK! Incoming event size is way too big.\n");
#endif
                errno = E2BIG;
                goto abort;
              }

            /* Also add the meta-data to the size: the size text itself plus newline, plus trailing newline. */
            anEventSize = (anIndex-aBuffer) + 1 + strtol(aBuffer, &anIndex2, 10) + 1;

            /* If strtol didn't stop at anIndex, we didn't get numbers, and we are confused. */
            if (anIndex != anIndex2)
              {
#ifdef DEBUG
                fprintf(stderr, "Received unexpected event data from server while reading event size.\n");
#endif
                errno = ECONNABORTED;
                goto abort;
              }

            /* If the event doesn't fit into aBuffer, we need to reallocate it. */
            if (anEventSize > aSize)
              {
                aBuffer = (char*)realloc(aBuffer, anEventSize);

                /* Save aBuffer's pointer right away, in case we abort later. */
                if (aBuffer)
                  aConnection->buf = aBuffer;
                else
                  {
                    errno = ENOMEM;
                    goto abort;
                  }

                aSize = anEventSize;
              }

            aState = kStateReadEventData;
          }
          /* Fall through. */

        case kStateReadEventData:
#ifdef DEBUG_PARSER_STATE
	  printf("state = kStateReadEventData\n");
#endif
          /* Get the rest of the event into aBuffer. */
          if (aUsage >= anEventSize)
            {
              if (aBuffer[anEventSize-1] != '\n')
                {
#ifdef DEBUG
                  fprintf(stderr,
                          "Received unexpected event data ('%c') from server while reading event terminator.\n",
                          aBuffer[anEventSize-1]);
#endif
                  errno = ECONNABORTED;
                  goto abort;
                }

	      {
		kn_StringRef anEventString = kn_StringCreateWithBytesNoCopy(aBuffer, anEventSize, kn_FALSE);
		anEvent = kn_EventCreateWithSimpleEventFormat(anEventString);
		kn_Release(anEventString);
	      }

	      if (!anEvent)
		{
#ifdef DEBUG
		  fprintf(stderr, "Failed to parse event from server.\n");
#endif
		  errno = ECONNABORTED;
		  goto abort;
		}

              /* Recover data from the next event. */
              memmove(aBuffer, aBuffer+anEventSize, aUsage-anEventSize);

              aUsage -= anEventSize;
              aState  = kStateReadEventSpacers;
              goto done;
            }
          break;

        default:
          /* We should never get here! */
#ifdef DEBUG
          fprintf(stderr, "YIKES! Internal parser error (state=%d) reading event data from server.\n", aState);
#endif
          errno = ECONNABORTED;
          goto abort;
        }

#ifdef HAVE_OPENSSL
      if (anSSL)
	{
	  ssize_t aResult;

	  aRead = 0;
	  do {
	    aResult = SSL_read(anSSL, aBuffer+aUsage+aRead, aSize-aUsage-aRead);

	    switch (SSL_get_error(anSSL, aResult))
	      {
	      case SSL_ERROR_NONE:
		aRead += aResult;
		break;
	      case SSL_ERROR_WANT_READ:
		goto done;
	      default:
		errno = ENOTCONN;
		goto disconnect;;
	      }
	  } while (SSL_pending(anSSL));
	}
      else
#endif /* HAVE_OPENSSL */
	{
	  aRead = read(aSocket, aBuffer+aUsage, aSize-aUsage);

	  if (aRead == 0 && aUsage < aSize)
	    {
#ifdef DEBUG
	      fprintf(stderr, "Server connection has closed.\n");
#endif
	      errno = ENOTCONN;
	      goto disconnect;
	    }

	  if (aRead == -1)
	    {
	      if (errno == EAGAIN) goto done;

#ifdef DEBUG
	      fprintf(stderr, "Error while reading event from server: %s\n", strerror(errno));
#endif
	      break;
	    }
	}

#ifdef DEBUG_SERVER_IO
      printf("E(%d): Read %ld bytes: ", aState, (long int)aRead);
      printf("[-");
      fwrite(aBuffer, 1, aUsage, stdout);
      printf("-|-");
      fwrite(aBuffer+aUsage, 1, aRead, stdout);
      printf("-]\n");
#endif

      aUsage += aRead;
    }

done:
  /* Remember any possibly unsaved server state changes. */
  aConnection->buf_used = aUsage;
  aConnection->buf_size = aSize ;
  aConnection->state    = aState;

  return anEvent;

abort:
  /* We're confused, so let's whack the connection and start over. */
#ifdef DEBUG
  fprintf(stderr, "Aborting connection.\n");
#endif

disconnect:
  kn_ServerDisconnect(aServer, aConnection);

  kn_Release(anEvent);

  return NULL;
}

kn_Error kn_ServerSetSSLTunnel(kn_Server *aServer, kn_Server_Connection* aConnection)
{
	kn_MutableStringRef aRequest    = kn_StringCreateMutable();
	kn_Error 	    anError     = kn_SUCCESS;
	kn_StringRef        aHostName   = kn_StringCreateWithHostNameFromURI(aServer->uri);
	unsigned short      aPortNumber = kn_StringGetPortFromURI(aServer->uri);
	char 		    aPort[16]   = "";
	char* 	            aBuffer     = (char*)malloc(EVENT_BUFFER_SIZE);
	ssize_t		    aRead       = 0;

	memset(aBuffer, '\0', sizeof(aBuffer));

	sprintf(aPort, "%d", aPortNumber);

	kn_StringAppendCString(aRequest, "CONNECT ");
	kn_StringAppendString (aRequest, aHostName); 
	kn_StringAppendCString(aRequest, ":");
	kn_StringAppendCString(aRequest, aPort);
	kn_StringAppendCString(aRequest, " HTTP/1.0\r\n");
	kn_StringAppendString (aRequest, aServer->proxy_auth);
	kn_StringAppendCString(aRequest, aServer->proxy_auth ? "\r\n" : "");
        kn_StringAppendCString(aRequest, "\r\n");

	kn_Release(aHostName);

	if (write(aConnection->socket, kn_StringGetBytes(aRequest), kn_StringGetSize(aRequest)) != kn_StringGetSize(aRequest))
          {
            anError = kn_IOERROR;
            goto fail;
          }

	aRead = read(aConnection->socket, aBuffer, EVENT_BUFFER_SIZE);

	if (aRead == 0 )
          {
#ifdef DEBUG
            fprintf(stderr, "EOF while reading server response.\n");
#endif
            errno = ECONNABORTED;
            anError = kn_IOERROR;
            goto fail;
          }

	if (aRead == -1)
          {
#ifdef DEBUG
            fprintf(stderr, "Error while reading server response: %s\n", strerror(errno));
#endif
            anError = kn_IOERROR;
            goto fail;
          }

	if (strncmp(aBuffer, "HTTP/1.0 200 Connection established", 35) != 0)
          {
#ifdef DEBUG
            fprintf(stderr, "Failed to setup SSL Tunneling.\n");
#endif
            anError = kn_IOERROR;
          } 
	else
          {
#ifdef DEBUG
            fprintf(stderr, "SSL Tunneling was successful \n");
#endif
          }

 fail:
	kn_Release(aRequest);
	free(aBuffer);		
	return anError;
}
