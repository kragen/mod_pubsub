/**
 * PubSub Client Library
 * PubSub Client Library
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

#include <sys/param.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <fcntl.h>
#ifdef HAVE_OPENSSL
#include <openssl/ssl.h>
#endif

#include "kn_Object_private.h"
#include "kn_String_URI.h"
#include "kn_String_WebEncoding.h"
#include "kn_Dictionary.h"
#include "kn_Route.h"
#include "kn_Server_private.h"

typedef enum {
  /* Not connected */
  kStateBegin = 0,

  /* HTTP request/response */
  kStateSendHTTPRequest,
  kStateReadHTTPProtocol,
  kStateReadHTTPVersion,
  kStateReadHTTPStatus,
  kStateReadHTTPCR1,
  kStateReadHTTPNL1,
  kStateReadHTTPCR2,
  kStateReadHTTPNL2,

  /* Status event */
  kStateReadStatusEvent,

  /* Event stream */
  kStateReadEventSpacers,
  kStateReadEventSize,
  kStateReadEventData,

} State;

typedef struct __kn_Server_Connection {
  int      socket;
#ifdef HAVE_OPENSSL
  SSL*     ssl_connection;
#endif
  char*    buf;
  size_t   buf_used;
  size_t   buf_size;
  State    state;
} kn_Server_Connection;

typedef struct __kn_Server {
  __KN_OBJECT_COMMON_FIELDS

  /* server attributes */
  kn_StringRef uri;
  kn_StringRef auth_header;

  /* connection management */
  kn_ServerConnectionCallback connect_callback;
  void*                       connect_userdata;

  /* connections */
#ifdef HAVE_OPENSSL
  SSL_CTX*             ssl_context;
#endif
  kn_Server_Connection post;
  kn_Server_Connection event;

  /* event callbacks */
  kn_StringRef            callback_uri;
  kn_MutableDictionaryRef callbacks;

  /* proxy settings */
   kn_StringRef   	proxy_host; 
   unsigned short int proxy_port;
   kn_StringRef		proxy_auth;

} kn_Server;

/* Private (static) API prototypes. */
static kn_Error kn_ServerStartTunnel (kn_Server* aServer);

#include "kn_Server_http.c"
#include "kn_Server_callback.c"
#include "kn_Server_subscribe.c"
#include "kn_Server_publish.c"

#ifndef _RAND_ID_
#define _RAND_ID_ ""
#endif

RCSID("$Id: kn_Server.c,v 1.2 2003/03/07 06:16:08 wsanchez Exp $"_PROTO_ID_""_RAND_ID_""_POST_ID_""_GET_ID_""_Callback_ID_);

/**
 * Allocators
 **/

static void kn_ServerDealloc (kn_ObjectRef anObject)
{
  kn_Server* aServer = (kn_Server*)anObject;

#ifdef KN_DEBUG_REFS
  printf("Dealloc server %p\n", aServer);
#endif

  kn_Release(aServer->uri        );
  kn_Release(aServer->auth_header);
  kn_Release(aServer->proxy_host );
  kn_Release(aServer->proxy_auth );
  kn_Release(aServer->callbacks  );

  kn_ServerDisconnect(aServer, &aServer->event);
  kn_ServerDisconnect(aServer, &aServer->post );

#ifdef HAVE_OPENSSL
  if (aServer->ssl_context) SSL_CTX_free(aServer->ssl_context);
#endif

  free(aServer);
}

static kn_StringRef kn_ServerDescribe (kn_ObjectRef anObject)
{
  kn_Server*          aServer      = (kn_Server*)anObject;
  kn_MutableStringRef aDescription = kn_StringCreateMutable();
  char                aStart[1024] = "";

  sprintf(aStart, "<kn_Server %p: ", aServer);

  kn_StringAppendCString(aDescription, aStart      );
  kn_StringAppendString (aDescription, aServer->uri);
  kn_StringAppendCString(aDescription, ">"         );

  return aDescription;
}

kn_ServerRef kn_ServerCreateWithURI (kn_StringRef aURI)
{
  return kn_ServerCreateWithURIUserPassword(aURI, NULL, NULL);
}

kn_ServerRef kn_ServerCreateWithURIUserPassword (kn_StringRef aURI, kn_StringRef aUser, kn_StringRef aPassword)
{
  kn_Server* aServer;

  {
    kn_StringRef aNutherUser     = kn_StringCreateWithUserNameFromURI(aURI);
    kn_StringRef aNutherPassword = kn_StringCreateWithPasswordFromURI(aURI);

    /*
     * If the user name and/or password are specified in both the URL
     * and in aUser/aPassword, make sure that the agree with each other.
     * Otherwise, return NULL with errno = EINVAL.
     */
    if ( (aUser     && aNutherUser     && kn_StringCompare(aUser    , aNutherUser    )) ||
	 (aPassword && aNutherPassword && kn_StringCompare(aPassword, aNutherPassword)) )
      {
	kn_Release(aNutherUser    );
	kn_Release(aNutherPassword);
	errno = EINVAL; return NULL;
      }

    /* Set user name/password from URI if not set. */
    if (!aUser    ) aUser     = aNutherUser    ;
    if (!aPassword) aPassword = aNutherPassword;

    /* If only one of user name or password is specified, set the other to "". */
    if (aUser     && !kn_Retain(aPassword)) aPassword = kn_StringCreateWithCString("");
    if (aPassword && !kn_Retain(aUser    )) aUser     = kn_StringCreateWithCString("");

    kn_Release(aNutherUser    );
    kn_Release(aNutherPassword);
  }

  aServer = (kn_Server*)malloc(sizeof(kn_Server));

  if (aServer)
    {
      kn_object_init(aServer, kn_ServerDealloc, kn_ServerDescribe);

	  if (kn_StringGetProtocolFromURI(aURI) != kn_PROTOCOL_HTTP && 
		  kn_StringGetProtocolFromURI(aURI) != kn_PROTOCOL_HTTPS)
	  {
		errno = EINVAL; 
		return NULL;
	  }
      aServer->uri                  = kn_Retain(aURI);
      aServer->auth_header          = NULL;
      aServer->connect_callback     = NULL;
      aServer->connect_userdata     = NULL;
#ifdef HAVE_OPENSSL
      aServer->ssl_context          = NULL;
#endif
      aServer->post.socket          = -1;
#ifdef HAVE_OPENSSL
      aServer->post.ssl_connection  = NULL;
#endif
      aServer->post.buf             = NULL;
      aServer->post.buf_used        =  0;
      aServer->post.buf_size        =  0;
      aServer->post.state           = kStateBegin;
      aServer->event.socket         = -1;
#ifdef HAVE_OPENSSL
      aServer->event.ssl_connection = NULL;
#endif
      aServer->event.buf            = NULL;
      aServer->event.buf_used       =  0;
      aServer->event.buf_size       =  0;
      aServer->event.state          = kStateBegin;
      aServer->callback_uri         = NULL;
      aServer->callbacks            = NULL;
      aServer->proxy_host	        = NULL;
	  aServer->proxy_port		    =  0;
	  aServer->proxy_auth			= NULL;

      if (aUser && aPassword)
	{
	  /* Basic auth header is "Authorization: Basic <aUser>:<aPassword>" */
	  kn_MutableStringRef anAuthHeader = kn_StringCreateMutableWithCString("Authorization: Basic ");
	  kn_MutableStringRef anAuthInfo   = kn_StringCreateMutableWithSize(kn_StringGetSize(aUser     ) +
									    sizeof          (":"       ) +
									    kn_StringGetSize(aPassword ) );
	  kn_StringRef anAuthData;
  
	  kn_StringAppendString (anAuthInfo, aUser    );
	  kn_StringAppendCString(anAuthInfo, ":"      );
	  kn_StringAppendString (anAuthInfo, aPassword);
  
	  anAuthData = kn_StringCreateByEncodingBase64(anAuthInfo);
  
	  kn_StringAppendString(anAuthHeader, anAuthData);
  
	  kn_Release(anAuthData);
	  kn_Release(anAuthInfo);
  
	  aServer->auth_header = anAuthHeader;
	}
    }
  else
    errno = ENOMEM;

  kn_Release(aUser    );
  kn_Release(aPassword);

  return aServer;
}

/**
 * Accessors
 **/

kn_StringRef       kn_ServerGetURI       (kn_ServerRef aServer) { return aServer->uri       ; }
kn_StringRef       kn_ServerGetProxyHost (kn_ServerRef aServer) { return aServer->proxy_host; }
unsigned short int kn_ServerGetProxyPort (kn_ServerRef aServer) { return aServer->proxy_port; }

void kn_ServerSetConnectionCallback (kn_ServerRef aServerRef, kn_ServerConnectionCallback aCallback, void* aUserData)
{
  kn_Server* aServer = (kn_Server*)aServerRef;

  aServer->connect_callback = aCallback;
  aServer->connect_userdata = aUserData;
}

void kn_ServerSetProxy(kn_ServerRef       aServerRef, 
                       kn_StringRef       aProxyHost, 
                       unsigned short int aProxyPort,
                       kn_StringRef       aProxyUser,
                       kn_StringRef       aProxyPasswd) 
{
  kn_Server *aServer = (kn_Server *)aServerRef;

  aServer->proxy_host = kn_Retain(aProxyHost);	
  aServer->proxy_port = aProxyPort;

  /* If only one of user or password is specified, set the other to "" */
  if (aProxyUser   && !kn_Retain(aProxyPasswd)) aProxyPasswd = kn_StringCreateWithCString("");
  if (aProxyPasswd && !kn_Retain(aProxyUser  )) aProxyUser   = kn_StringCreateWithCString("");

  if (aProxyUser && aProxyPasswd)
    {	
      /* Basic auth header is "Proxy-Authorization: Basic <aProxyUser>:<aProxyPasswd>" */
      kn_MutableStringRef anAuthHeader = kn_StringCreateMutableWithCString("Proxy-Authorization: Basic ");
      kn_MutableStringRef anAuthInfo   = kn_StringCreateMutableWithSize(kn_StringGetSize(aProxyUser) +
                                                                        sizeof(":")                  +
                                                                        kn_StringGetSize(aProxyPasswd));
      kn_StringRef anAuthData;
  
      kn_StringAppendString (anAuthInfo, aProxyUser);
      kn_StringAppendCString(anAuthInfo, ":");
      kn_StringAppendString (anAuthInfo, aProxyPasswd);
  
      anAuthData = kn_StringCreateByEncodingBase64(anAuthInfo);
  
      kn_StringAppendString(anAuthHeader, anAuthData);

      kn_Release(anAuthData);
      kn_Release(anAuthInfo);

      aServer->proxy_auth = anAuthHeader;
    }
}
