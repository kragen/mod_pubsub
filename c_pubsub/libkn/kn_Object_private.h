/**
 * PubSub Client Library
 * Super secret internals
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

#ifndef _KN_KN_OBJECT_PRIVATE_H_
#define _KN_KN_OBJECT_PRIVATE_H_ "$Id: kn_Object_private.h,v 1.2 2003/03/19 05:36:47 ifindkarma Exp $"

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/types.h>
#include "kn_Object.h"

#define KN_USERAGENT "PubSub client library version "VERSION

extern unsigned long long int _kn_Object_next_object_id;

#define __KN_OBJECT_COMMON_FIELDS		\
  unsigned long long id;			\
  unsigned int       retain_count;		\
  void        (*dealloc )(kn_ObjectRef);	\
  kn_StringRef(*describe)(kn_ObjectRef);

#define kn_object_init(anObject,aDealloc,aDescribe)	\
{							\
  anObject->id           = _kn_Object_next_object_id++;	\
  anObject->retain_count = 1;				\
  anObject->dealloc      = aDealloc;			\
  anObject->describe     = aDescribe;			\
}

#define kn_object_make_immutable(anObject) { anObject->is_mutable = kn_FALSE; }

typedef struct __kn_Object {
  __KN_OBJECT_COMMON_FIELDS
} kn_Object;

#ifdef __cplusplus
}
#endif

#endif /* _KN_KN_OBJECT_PRIVATE_H_ */
