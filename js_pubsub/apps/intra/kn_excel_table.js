// Copyright 2000-2003 KnowNow, Inc.

// @KNOWNOW_LICENSE_START@ 

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

// @KNOWNOW_LICENSE_END@

// $Id: kn_excel_table.js,v 1.1 2003/07/29 00:10:27 ifindkarma Exp $

function KNExcelTableInit()
// initializes specified tables for Excel subscriptions
{
    // get all table elements
    var tables = document.getElementsByTagName("table");
    // loop thru
    for (var i=0;i<tables.length;i++)
    {
        // only elements of the kn_excel class get initialized
        if (tables[i].className.indexOf("kn_excel") != -1)
        {
            tables[i].onMessage = parseExcelTable;
            // get row/column offset, if defined
	    var rco_str = tables[i].getAttribute("rc_offset");
            if (rco_str)
            {
                var rco = rco_str.split(",");
                tables[i].setAttribute("r_offset", rco[0]);
                tables[i].setAttribute("c_offset", rco[1]);
            }

	    {
	      var _src = tables[i].getAttribute("kn_from");
	      var _dst = tables[i];
	      var _opt = _kn_elm2event(tables[i]);
	      kn.subscribe(_src, _dst, _opt);
	    }
        }
    }      
}

function _kn_elm2event(elm)
// retrieves the specified KN request headers from the HTML element
{
    var a = _kn_elm2event.lookup;
    var o = {};
    for (var i=0;i<a.length;i++)
    {
        var value = elm.getAttribute(a[i]);
        if (value)
        {
            o[a[i]] = value; 
        }
    }
    return o;
}
_kn_elm2event.lookup = ["do_max_n","do_max_age","kn_expires","kn_deletions"];


function parseExcelTable(e)
{
    // we only deal with table format right now, so convert anything
    // else to look like a 1x1 table
    if (e.kn_event_format != "table")
    {
        e.kn_table_rc_0_0 = e.kn_payload;
	    e.kn_table_first = "0_0";
    	e.kn_table_last = "0_0";
    }

    // get the proper table reference (needed for Moz 1.0)
    var table = (document.all) ? this.firstChild : this;

    // retrieve row and column offsets
    var display_offset =
      [
       parseInt(table.getAttribute("r_offset") || "0"),
       parseInt(table.getAttribute("c_offset") || "0")
      ];
    
    
    var excluded_rows = [];
    var exr = this.getAttribute("exclude_rows");
    if (exr)
    {
        exr = exr.split(" ").join("");
        excluded_rows = exr.split(",");
    }
    
    var excluded_cols = [];
    var exr = this.getAttribute("exclude_cols");
    if (exr)
    {
        exr = exr.split(" ").join("");
        excluded_cols = exr.split(",");
    }
    
    var max_rows = parseInt(this.getAttribute("max_rows"));
    var max_cols = parseInt(this.getAttribute("max_cols"));

    // allow an extra space after each comma
    var firsts = e.kn_table_first.split(", ").join(",").split(",");
    var lasts = e.kn_table_last.split(", ").join(",").split(",");

    // clip the first array if the last one is shorter
    if (lasts.length < firsts.length) firsts.length = lasts.length;

    // allow strange mapping offsets
    var offset = [ 0, 0 ];
    var ranges = [];
    for (var i = 0; i < firsts.length; i ++)
    {
    	var first = firsts[i].split("_");
    	var last = lasts[i].split("_");
    	var delta = [ 1, 1 ];
    	if (first.length < 2 || last.length < 2) continue;
    	for (var coord = 0; coord < 2; coord ++)
    	{
    	    first[coord] = parseInt(first[coord]);
    	    last[coord] = parseInt(last[coord]);
    	    if (first[coord] < offset[coord]) offset[coord] = first[coord];
    	    if (last[coord] < offset[coord]) offset[coord] = last[coord];
    	    if (first[coord] > last[coord]) delta[coord] = 0 - delta[coord];
    	}
    	ranges[ranges.length] = [ first, last, delta ];
    }

    // allow events without range declarations by creating a 1x1
    // "range" for each cell we find
    if (! ranges.length)
    {
	// loop thru event headers
    	for (var header in e)
    	{
    	    // we're only interested in row/column info
    	    if (header.indexOf("kn_table_rc_") == -1)
    	    {
    		    // these aren't the droids you're looking for
    		    continue;
    	    } else {
        		var item = header.split("_");
        		if (item.length < 2) continue;
        		item[0] = parseInt(item[0]);
        		item[1] = parseInt(item[1]);
        		ranges[ranges.length] = [ item, item, [ 1, 1 ] ];
    	    }
    	}
    }

    // loop through ranges and populate the table, enlarging it as
    // needed to accomodate the data
    
    var rowcount = 0;
    var colcount = 0;
    
    for (var rindex = 0; rindex < ranges.length; rindex ++)
    {
	    var range = ranges[rindex];

	// loop through rows
    
	    for (var row = range[0][0]; row - range[2][0] != range[1][0]; row += range[2][0])
    	{
            if (max_rows && rowcount >= max_rows)
            {
                return;
            }
    	    // calculate population row based on source row and offset
            
            if (!valueIsIn(row,excluded_rows))
            {
                rowcount++;
        	    var vrow = row + display_offset[0];
    
                // need more rows?  add them
                while (vrow >= table.childNodes.length)
                {
                    var tr = document.createElement("tr");
                    //tr.setAttribute("valign","top");
                    table.appendChild(document.createElement("tr"));
                }
    
    	        // find this row in the DOM
        	    var rowElement = table.childNodes[vrow];
        
        	    // loop through columns
        	    for (var column = range[0][1]; column - range[2][1] != range[1][1]; column += range[2][1])
        	    {
                
                    if (!valueIsIn(column,excluded_cols))
                    {
                    
            		    // calculate population column based on source column and offset
                		var vcolumn = column + display_offset[1];
                
                		// more columns?
                        
                		while(vcolumn >= rowElement.childNodes.length)
                		{
                            var td = document.createElement("td");
                            //td.setAttribute("valign","top");
            	    	    rowElement.appendChild(td);
            		    }
            
                		// find this column in the DOM
                		var columnElement = rowElement.childNodes[vcolumn];
                        
                        if (this.minwidth) columnElement.style.width = this.minwidth + "px";
            
        	        	// find our value
                		var header =
                		  "kn_table_rc_" +
                		  (row + offset[0]) +
                		  "_" +
                		  (column + offset[1]);
                
                		// populate column; display a non-breaking space in
                		// empty/missing columns to force them to render
                		var value = e[header] ? kn_htmlEscape(e[header]) : "&nbsp;";
                        if (this.getAttribute("char_limit")) value =                                     
                                truncate(value,this.getAttribute("char_limit"));
                		columnElement.innerHTML = value;
                    }
           	    }
    	    }
        }
    }
}

function valueIsIn(val,arry)
{
    for (var i=0;i<arry.length;i++)
    {
        if (arry[i] == val)
        {
            return true;
        }
    }
    return false;
}


// untimely ripp'd from Ben's visibleEscape. Needs cleaning.

function truncate(string,limit)
{
    if (!limit) return;
    var codes = kn_charCodesFromString(string);
    var visible = [];
    // pre must start with '<' for SP (space) to be displayed properly.
    var pre = '<em' +
        ' style="background-color:#ffffbb;color:black;font-style:normal;text-decoration:none"' +
        '> <small>';
    var post = '<' + '/small> <' + '/em>';
    // hidepre prefixes a string with a sometimes-hidden section
    var hidepre = '';
    // hidemid and hidepost surround a sometimes-hidden section of the string
    var hidemid = '';
    var hidepost = '';
    // DOM compatibility check, thanks to Scott LePera
    if (document.getElementById)
    {
        hidepre =
            '<span style="display: inline">';
        hidemid =
            '<span style="display: none">';
        hidepost =
            '<' + '/span>' +
            '<' + '/span>' +
            '<span style="display: none">' +
            ' <a href="#" ' +
            ' onclick="with(parentNode){style.display=previousSibling.lastChild.style.display=\'none\';nextSibling.style.display=\'inline\';}; return false;"' +
            '>' +
            '<small><nobr>&' + 'laquo;' + '<' + '/nobr><' + '/small>' +
            '<' + '/a>' +
            '<' + '/span>' +
            '<span style="display: inline">' +
            '... <a href="#" ' +
            ' onclick="with(parentNode){style.display=\'none\';previousSibling.style.display=previousSibling.previousSibling.lastChild.style.display=\'inline\';}; return false;"' +
            '>' +
            '<small><nobr>&' + 'raquo;' + '<' + '/nobr><' + '/small>' +
            '<' + '/a>' +
            '<' + '/span>';
    }
    for (var i = 0; i < codes.length; i ++)
    {
        var code = codes[i];
        var picture = '';
        if (picture)
        {
            picture = pre + picture + post;
        }
        else
        {
            picture = kn_stringFromCharCode(code);
        }
        visible[i] = picture;
    }
    
    if (visible.length >= limit)
    {
        visible[0] = hidepre + visible[0];
        visible[limit] += hidemid;
        visible[visible.length - 1] += hidepost;
    }
    return visible.join('');
}

// End of kn_excel_table.js
