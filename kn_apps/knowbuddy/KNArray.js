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

// $Id: KNArray.js,v 1.1 2002/12/07 10:08:19 ifindkarma Exp $

// Extended methods for Array objects.
KNArray_initClass();

function KNArray_initClass()
{
    Array.prototype.add      = KNArray_add;
    Array.prototype.remove   = KNArray_remove;
    Array.prototype.contains = KNArray_contains;
    Array.prototype.findByProp = KNArray_findByProp;
    Array.prototype.clear = KNArray_clear;
    Array.prototype.count = KNArray_count;
}

function KNArray_count()
{
    var count=0;
    for (var i = 0; i < this.length; i++)
    {
        if (this[i])
            count++;
    }
    return count;
}

function KNArray_clear()
{
    for (var i=this.length-1; i >= 0; --i)
    {
        delete this[i];
    }
    this.length = 0;
}

function KNArray_findByProp(name,value)
{
    for (var i=0; i < this.length; i++)
    {
        if (!this[i])
            continue;
            
        if (this[i][name] == value)
            return this[i];
    }
    return null;
}

function KNArray_add(obj)
{
    this[this.length] = obj;
}

function KNArray_remove(obj)
{
    for (var i=0; i < this.length; i++)
    {
        if (this[i] == obj)
            delete this[i];
    }
}

function KNArray_contains(obj)
{
    for (var i=0; i < this.length; i++)
    {
        if (this[i] == obj)
            return true;
    }
    return false;
}

// End of KNArray.js
