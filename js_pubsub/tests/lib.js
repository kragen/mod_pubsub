// Copyright 2000-2003 KnowNow, Inc.  All Rights Reserved.

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

// $Id: lib.js,v 1.1 2003/07/20 07:09:16 ifindkarma Exp $

////////////////////////////////////////////////////////////////////////

function invoke_parent(name, arg)
{
    if (top.kn_test_frame)
        top.kn_test_frame[name](arg);
    else
    {
        var a = arg;
        if (a.name && a.msg)
        {
            a = "[" + a.name + "]\n" + a.msg;
        }
        alert("Test result:\n" + name + "\n" + a);
    }
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

function subtestTable(subtests)
{
    var str = '<table border="0" cellspacing="0" align="center">';
    for (var i = 0; i < subtests.length; i ++)
    {
        var subtest = subtests[i];
        if (document.layers)
        {
            str += '<form name="' + i + '">';
        }
        if (i & 2)
            str += '<tr>';
        else
            str += '<tr bgcolor="#ddddff">';
        str +=
            '<th align="left">' +
            (self.kn_htmlEscape ?
             self.kn_htmlEscape(subtest.name) :
             subtest.name.
                split("&").join("&" + "amp;").
                split("<").join("&" + "lt;")) +
            '<' + '/th>' +
            '<td>';
        if (! document.layers)
        {
            str += '<form style="display: inline" name="' + i + '">';
        }
        str +=
            '<input type="radio" name="result" value="succeed"' +
            ' disabled="disabled" />OK' +
            '<input type="radio" name="result" value="fail"' +
            ' disabled="disabled" />Failed';
        if (! document.layers)
        {
            str +=
                '<' + '/form>';
        }
        str +=
            '<' + '/td>' +
            '<' + '/tr>';
        if (document.layers)
        {
            str +=
                '<' + '/form>';
        }
    }
    str += '<' + '/table>';
    return str;
}

if (! this.kn_argv)
{
    kn_argv = new Object();
    if (location.hash && location.hash.length > 1)
    {
        kn_argv.kn_topic = unescape(location.hash.substring(1));
    }
}

if (kn_argv.kn_topic == null)
{
    kn_argv.kn_topic = "/what/manual-test/" + 
        Math.random().toString().substr(2);
}

// End of lib.js
