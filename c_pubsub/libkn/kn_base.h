/**
 * PubSub Client Library
 * Basic C types and routines
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

#ifndef _KN_BASE_H_
#define _KN_BASE_H_ "$Id: kn_base.h,v 1.3 2003/03/19 05:36:47 ifindkarma Exp $"

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * @header kn_base
 * kn_base provides basic fundamental contructs
 */

/*!
  @enum kn_BOOL
  @discussion Boolean value
  @constant kn_FALSE False value
  @constant kn_TRUE  True value
 */
typedef enum {
  kn_FALSE = 0,
  kn_TRUE  = 1
} kn_BOOL;

#if defined (__APPLE_CC__)
#define kn_private_extern __private_extern__
#else
#define kn_private_extern
#endif

/*!
 * @function   kn_LibraryGetVersion
 * @discussion kn_LibraryGetVersion provides the version of the library as a C string.
 * @result Returns a C string constant.
 */
const char* kn_LibraryGetVersion();

#ifdef __cplusplus
}
#endif

#endif /* _KN_BASE_H_ */
