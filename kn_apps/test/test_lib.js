// Copyright 2000-2002 KnowNow, Inc.  All Rights Reserved.

// @KNOWNOW_LICENSE_START@
// 
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
// 
// 1. Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
// 
// 2. Redistributions in binary form must reproduce the above copyright
// notice, this list of conditions and the following disclaimer in
// the documentation and/or other materials provided with the
// distribution.
// 
// 3. The name "KnowNow" is a trademark of KnowNow, Inc. and may not
// be used to endorse or promote any product without prior written
// permission from KnowNow, Inc.
// 
// THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESSED OR IMPLIED
// WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
// IN NO EVENT SHALL KNOWNOW, INC. OR ITS CONTRIBUTORS BE LIABLE FOR ANY
// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
// GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
// IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
// OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
// ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// 
// @KNOWNOW_LICENSE_END@

// $Id: test_lib.js,v 1.1 2002/11/07 07:08:59 troutgirl Exp $

function invoke_parent(name, arg)
{
    if (top.kn_test_frame)
        top.kn_test_frame[name](arg);
    else
        alert("Test result:\n" + name + "\n" + arg);
}

function succeed()
{
    invoke_parent('succeed', '');
}

function fail(msg)
{
    invoke_parent('fail', msg);
}

// Some tests have subtests.

function expect_subtests(n)
{
    invoke_parent('expect_subtests', n);
}

// Subtests can succeed or fail.

function subsucceed(name)
{
    invoke_parent('subsucceed', name);
}

function subfail(name, msg)
{
    invoke_parent('subfail', {name: name, msg: msg});
}


if (kn_argv['kn_topic'] == null)
{
    kn_argv['kn_topic'] = "/what/manual-test/" + 
        Math.random().toString().substr(2);
}

// End of test_lib.js
