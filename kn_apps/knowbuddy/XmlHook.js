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

// $Id: XmlHook.js,v 1.1 2002/12/07 10:08:19 ifindkarma Exp $

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
