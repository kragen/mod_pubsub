/**
 * PubSub Client Library
 * Error codes and routines
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

#ifndef _KN_KN_ERROR_H_
#define _KN_KN_ERROR_H_ "$Id: kn_Error.h,v 1.4 2004/04/19 05:39:08 bsittler Exp $"

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * @header kn_Error
 * kn_Error denotes PubSub client error codes.
 */

/*!
 * @enum kn_Error
 * @discussion Error status codes
 * @constant kn_SUCCESS      Success
 * @constant kn_FAIL         Nondescript failure
 * @constant kn_UNSUPPORTED  Operation not supported 
 * @constant kn_OOPS         Unimplemented
 * @constant kn_INTERNAL     Internal error (shouldn't ever happen...!)
 * @constant kn_INVALID      Invalid request
 * @constant kn_PERMISSION   Operation not allowed
 * @constant kn_NOENTRY      Entry does not exist
 * @constant kn_EXISTS       Entry already exists
 * @constant kn_PROTECTED    Access denied to resource
 * @constant kn_TIMEOUT      Operation timed out
 * @constant kn_INTERRUPTED  Request interrupted
 * @constant kn_IOERROR      I/O error
 * @constant kn_NOTCONNECTED Not connected
 * @constant kn_MEMORYFAIL   Out of memory
 */
typedef enum {
  kn_SUCCESS = 0,
  kn_FAIL,

  kn_UNSUPPORTED = 100,
  kn_OOPS,
  kn_INTERNAL,

  kn_INVALID = 200,
  kn_PERMISSION,
  kn_NOENTRY,
  kn_EXISTS,
  kn_PROTECTED,

  kn_TIMEOUT = 300,
  kn_INTERRUPTED,
  kn_IOERROR,
  kn_NOTCONNECTED,

  kn_MEMORYFAIL = 400

} kn_Error;

#include <errno.h>

#if !defined (ELIBBAD)
#  if defined (ESHLIBVERS)
#    define ELIBBAD ESHLIBVERS
#  else
#    warning ELIBBAD undefined and no equivalent available.
#    define ELIBBAD EOPNOTSUPP;
#  endif
#endif /** ELIBBAD */

#ifdef __cplusplus
}
#endif

#endif /* _KN_KN_ERROR_H_ */
