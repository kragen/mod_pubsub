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

// $Id: todo_table.js,v 1.1 2002/11/07 07:08:20 troutgirl Exp $

// Table renderer.

// Pass an Array of Objects you want to render as a table; the
// objects' properties form the table column headers.  Properties that
// are functions for all objects in the table will not be displayed.
// Properties that are functions for some objects but not others will
// be displayed for all objects.  This is probably a bad thing, so
// don't do that.

function HTMLTable(rows)
{
    return HTMLTableUsingColumns(rows, tableColumns(rows));
}

function HTMLTableUsingColumns(rows, columns, headings, format, attributes, rowOrdering)
{
    var rv = "<table "+(attributes?attributes:"")+" >\n<tr>";

    for (var i in columns)
    {
        rv += "<th>" 
            + (format ? format[0] : "") 
            + ((headings && headings[i]) ? headings[i] : columns[i]) 
            + (format ? format[1] : "") 
            + "</th>";
    }
    rv += "</tr>\n";

    function internalClosure(j)
    {
        rv += "<tr>";
        for (var i in columns)
        {
            var value = rows[j][columns[i]];
            if (isUndefined(value))
            {
                rv += "<td></td>";
            }
            else
            {
                rv += "<td align=center valign=baseline>" + (format ? format[0] : "") + value + (format ? format[1] : "") + "</td>";
            }
        }
        rv += "</tr>\n";
    }

    if (rowOrdering == null)
	for (var k in rows) internalClosure(k);
    else
        for (var k=0; k< rowOrdering.length; k++) internalClosure(rowOrdering[k]);

    rv += "</table>\n";
    return rv;
}

function tableColumns(rows)
{
    var rvobj = new Object();
    var rvarray = new Array();

    for (var i in rows)
    {
        for (var j in rows[i])
        {
            if (typeof rows[i][j] != 'function')
            {
                rvobj[j] = 1;
            }
        }
    }

    for (var i in rvobj)
    {
        push(rvarray, i);
    }
    rvarray.sort();
    return rvarray;
}

// Utility functions

function push(array, value)
{
    array[array.length] = value;
}

function isUndefined(value)
{
    var undef;
    return value == undef;
}

// End of todo_table.js
