// Copyright 2000-2004 KnowNow, Inc.  All rights reserved.

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
// notice, this list of conditions and the following disclaimer in the
// documentation and/or other materials provided with the distribution.
// 
// 3. Neither the name of the KnowNow, Inc., nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// 
// @KNOWNOW_LICENSE_END@
// 

// $Id: XmlHook.js,v 1.2 2004/04/19 05:39:12 bsittler Exp $

function hook_props(self,obj)
{
    // copy properties
    for (var i in obj)
    {
        if (typeof obj[i] != 'function')
        {
            self[i] = obj[i];
        }
    }
}

function hook_array(self,name)
{
    if (!self[name] || self[name].constructor != Array)
    {
        var a = new Array();
        var old = self[name];
        if (old != null)
        {
            a[0] = old;
            /**
            if (!old[0])
                a[0] = old;
            else
            {
                for (var i in old)
                {
                    // Hmm. dangerous
                    a[i] = old[i];
                }
            }
            **/
        }
        self[name] = a;
    }
}

function hook_array_elements(self,name,ctor)
{
    var temp;
    for (var i=0; i < self[name].length; i++)
    {
        temp = new ctor();
        temp.attach(self[name][i]);
        self[name][i] = temp;
    }
}

// End of XmlHook.js
