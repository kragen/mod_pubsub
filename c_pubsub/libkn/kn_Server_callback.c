/**
 * PubSub Client Library
 * PubSub Client Library: event client callback
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

#define _Callback_ID_ "$Id: kn_Server_callback.c,v 1.1 2002/12/21 03:38:44 bsittler Exp $"

typedef const struct __kn_EventCallback * kn_EventCallbackRef;

typedef struct __kn_EventCallback {
  __KN_OBJECT_COMMON_FIELDS
  kn_EventHandler destination;
  kn_EventHandler user_data;
} kn_EventCallback;

/**
 * Allocators
 **/

static void kn_EventCallbackDealloc (kn_ObjectRef anObject)
{
  kn_EventCallback* aCallback = (kn_EventCallback*)anObject;

#ifdef KN_DEBUG_REFS
  printf("Dealloc event callback %p\n", aCallback);
#endif

  free(aCallback);
}

static kn_StringRef kn_EventCallbackDescribe (kn_ObjectRef anObject)
{
  char aDescription[1024] = "";

  sprintf(aDescription, "<kn_EventCallback %p>", anObject);

  return kn_StringCreateWithCString(aDescription);
}

static kn_EventCallbackRef kn_EventCallbackCreateWithDestination (kn_EventHandler aDestination, void* aUserData)
{
  kn_EventCallback* aCallback = (kn_EventCallback*)malloc(sizeof(kn_EventCallback));

  if (aCallback)
    {
      kn_object_init(aCallback, kn_EventCallbackDealloc, kn_EventCallbackDescribe);

      aCallback->destination = aDestination;
      aCallback->user_data   = aUserData;
    }

  return aCallback;
}

/**
 * Accessors
 **/

kn_private_extern kn_StringRef kn_ServerGetCallbackURI (kn_ServerRef aServer)
{
  if (aServer->callback_uri == NULL)
    kn_ServerStartTunnel((kn_Server*)aServer);

  return aServer->callback_uri;
}

/**
 * Actions
 **/

kn_private_extern kn_Error kn_ServerRegisterCallback (kn_ServerRef aServer, kn_StringRef aRouteLocation,
                                                      kn_EventHandler aDestination, void* aUserData)
{
  if (!aServer->callbacks)
    {
      if (!(((kn_Server*)aServer)->callbacks = kn_DictionaryCreateMutable()))
        return kn_MEMORYFAIL;
    }

  {
    kn_EventCallbackRef aCallback = kn_EventCallbackCreateWithDestination(aDestination, aUserData);

    kn_Error anError = kn_DictionarySetValue(aServer->callbacks, aRouteLocation, aCallback);

    kn_Release(aCallback);

    return anError;
  }
}
