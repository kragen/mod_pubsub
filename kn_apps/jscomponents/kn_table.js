/*
 * Copyright (c) 2000-2002 KnowNow, Inc. All rights reserved.
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
 *
 * $Id: kn_table.js,v 1.1 2002/11/07 07:08:31 troutgirl Exp $
 */

//////////////////////////////
// this crazy table widget was written
// by meg hourihan (meg@megnut.com) with 
// mucho assistance from bryan boyer
// for the kind folks at knownow
// 
// 2/13/01 bryan has added a bunch of
// templating stuff to it so that you
// can make your table look however
// you would like it to look.
//
// 4/11/01 meg removed the dependency
// on KNDocument. now it uses DOM reference
// to write table. Me gusta!

var kn_tables = new Array();

// KN_TABLE INFO
// data = array of data to display
// columns = object in order of display with properties and values to display
// display = object, allows you to customize how you'd like things to display (e.g. <table>, <td>, etc.)
// headers = boolean, set to true to display table headers
// pagination = object, can set start_point and max_per_page
// sorting = object, can set order, if nothing, set to ascending
// container = the reference for a KNDocument which the table will be written to

function KN_Table(container,data,columns,display,headers,pagination,sorting) {

	this.name = kn_tables.length;
	kn_tables[kn_tables.length] = this;
	
	this.container = container;

	if (!pagination) pagination = { start_point : 0, max_per_page : 10 };
	if (!sorting) sorting = { order : "ascending", on : "" , last_sort_order : "" };
	if (!display) display = {};	

	// set pagination-related props
	this.start_point = (pagination.start_point) ? pagination.start_point : 0;	// where to start in the array
	this.max_per_page = (pagination.max_per_page) ? pagination.max_per_page : 3;	// how many rows to display on a page

	// set sorting-related props
	this.sort_order = sorting.order;		// ascending or descending
	this.sort_on = sorting.on;			// which column to sort on
	this.last_sort_order = sorting.order;		// used for status checking later on
	this.last_sort_on = sorting.on;			// used for status checking later on

	// get the data now
	this.data = data;				// array of events
	this.columns = columns;				// column properties
	
	// show headers
	this.header = (headers) ? headers : false;
	// setting some display-related properties
	this.table_header = (display.header) ? display.header : "<table>";
	this.table_footer = (display.footer) ? display.footer : "</table>";
	this.table_row_open = (display.row_open) ? display.row_open : "<tr>";
	this.table_row_close = (display.row_close) ? display.row_close : "</tr>";
	this.table_cell_open = (display.cell_open) ? display.cell_open : "<td>";
	this.table_cell_close = (display.cell_close) ? display.cell_close : "</td>";
	this.table_cell_templates = (display.table_cell_templates) ? display.table_cell_templates : false;
	this.table_column_templates = (display.table_column_templates) ? display.table_column_templates : false;
	this.buttons = (display.buttons) ? display.buttons : "*begin* *back* (*current_min* - *current_max*) *forward* *end*";
	this.forward_button = (display.forward_button) ? display.forward_button : '<input type="button" value=">" onclick="*code*">';
	this.back_button = (display.back_button) ? display.back_button : '<input type="button" value="<" onclick="*code*">';
	this.begin_button = (display.begin_button) ? display.begin_button : '<input type="button" value="|<" onclick="*code*">';
	this.end_button = (display.end_button) ? display.end_button : '<input type="button" value=">|" onclick="*code*">';

	this.table_header_cell_open = (display.header_cell_open) ? display.header_cell_open : "<td>";
	this.table_header_cell_close = (display.header_cell_close) ? display.header_cell_close : "</td>";
	this.table_header_cell_template_off = (display.table_header_cell_template_off) ? display.table_header_cell_template_off : '<a href="javascript: *code*"><b>*name*</b></a>';
	this.table_header_cell_template_ascending = (display.table_header_cell_template_ascending) ? display.table_header_cell_template_ascending : '<a href="javascript: *code*"><b>*name*</b>&nbsp;<img src="' + kn_resolvePath(location.pathname,'./') + 'images/down.gif" border="0"></a>';
	this.table_header_cell_template_descending = (display.table_header_cell_template_descending) ? display.table_header_cell_template_descending : '<a href="javascript: *code*"><b>*name*</b>&nbsp;<img src="' + kn_resolvePath(location.pathname,'./') + 'images/up.gif" border="0"></a>';

	if (this.forward_button != "") {
		this.forward_button = this.forward_button.replace(/\*code\*/,"javascript: page_me(" + this.name + ",1)");
	}

	if (this.back_button != "") {
		this.back_button = this.back_button.replace(/\*code\*/,"javascript: page_me(" + this.name + ",-1)");
	}

	if (this.begin_button != "") {
		this.begin_button = this.begin_button.replace(/\*code\*/,"javascript: page_me(" + this.name + ",-2)");
	}

	if (this.end_button != ""){
		this.end_button = this.end_button.replace(/\*code\*/,"javascript: page_me(" + this.name + ",2)");
	}
	// functions!
	this.draw = kn_table_draw;
	this.sort = kn_table_sort;

}

// this draws the table!
function kn_table_draw(direction) {

 	// get properties as row heading if order is not explicitly set
	var my_columns = this.columns ? this.columns : get_props(this.data);  

	// figure out if data needs to be sorted, avoids unnecessary sorting if 
	// the order's already been applied
	if (this.last_sort_order != this.sort_order || this.sort_on != this.last_sort_on) {
		this.sort(this.sort_order, this.data, this.sort_on);
		this.last_sort_order = this.sort_order;
		this.last_sort_on = this.sort_on;
	}
	// setting the starting point based on which button was clicked
	// direction -1 = back
	// direction 1 = forward
	// direction -2 = goto beginning
	// direction 2 = goto end
	
	if (direction == -1) {
		this.start_point -= this.max_per_page;
		if (this.start_point < 0) { this.start_point = 0 } ;
	} else if (direction == 1)  {
		this.start_point += this.max_per_page;
	} else if (direction == 2)  {
		this.start_point = this.data.length - this.max_per_page;
	} else if (direction == -2)  {
		this.start_point = 0;
	}
	// figure out how much is displaying on one page
	var diff = this.data.length - this.start_point;
	if (this.max_per_page == "infinity") {
		// do nothing because it's already set to max.
	} else if (diff > this.max_per_page ) {
		diff = this.max_per_page + this.start_point;
	} else {
		diff = diff + this.start_point;
	}
	
	// make pagination buttons disappear
	var start = 1;
	var end = 1;
	if (this.start_point <= 0 ) start = -1;
	if (diff == this.data.length) end = -1;

	// get buttons
	var theButtons = pagination_buttons(this.name,start,end,this.start_point,this.max_per_page,this.data.length);
	// display
	var theTable = this.table_header; 	// start table
	
	// this is the table header
	if (this.header && this.header != false && this.header != "false") {
		var header_cell = new String();
		theTable += this.table_row_open;
		mm=0;
		for (var m in my_columns) {
			// nevermind this little hack. it sets the sort_on to the first column if one is not explicitly set
			if (mm==0 && (this.sort_on == "undefined" || this.sort_on == "")){
				this.sort_on = m;
				this.last_sort_on = m; // meg added this line 04/11/01 because it wasn't displaying the whole table for some reason
			}
			mm++;
			theTable += this.table_header_cell_open; 
			
			// if we're writing out the column that was sorted on, make it so that the arrow switches sort order.
			if (this.sort_on == m){
				if (this.sort_order=="ascending"){
					header_cell = this.table_header_cell_template_ascending;
					var code = "kn_tables[" + this.name + "].sort_on='" + m + "'; kn_tables[" + this.name + "].sort_order='descending'; kn_tables[" + this.name + "].draw();";
				} else {
					header_cell = this.table_header_cell_template_descending;
					var code = "kn_tables[" + this.name + "].sort_on='" + m + "'; kn_tables[" + this.name + "].sort_order='ascending'; kn_tables[" + this.name + "].draw();";
				}
			} else {
				header_cell = this.table_header_cell_template_off;
				var code = "kn_tables[" + this.name + "].sort_on='" + m + "'; kn_tables[" + this.name + "].sort_order='ascending'; kn_tables[" + this.name + "].draw();";
			}

		        header_cell = header_cell.replace(/\*code\*/ig,code);
		        header_cell = header_cell.replace(/\*name\*/ig,my_columns[m]);
			
			theTable += header_cell + this.table_header_cell_close;
		}
		theTable += this.table_row_close;
	}
	var j;
	
	// loop through array of objects, writing rows of contents
	for(i=this.start_point; i < diff; i++) { 	
		this_row = this.table_row_open;
			jj=0; // numerical count of prop numbers i.e. columns
			for (j in my_columns) {
				// If there's a template, set the cell to that, otherwise set it to the value for this column
				if (this.table_column_templates[jj]) {
					this_row += this.table_column_templates[jj];
				} else if (this.table_cell_templates[jj]){
					this_row += this.table_cell_open + this.table_cell_templates[jj] + this.table_cell_close; 
				} else {
					this_row += this.table_cell_open + this.data[i][j] + this.table_cell_close;
				}
			jj++;
			}
			
			// replace template keys with real values for this row
			for (b in this.data[i]){
				re = new RegExp("\\*" + b + "\\*","ig");
				str = new String(this_row);

				this_row = str.replace(re,this.data[i][b]);
			}

		this_row += this.table_row_close;
		theTable += this_row;
	}
	
	// finish up table
	theTable += this.table_footer;
	
	// use DOM reference passed to table to write buttons and table
	this.container.innerHTML = theButtons + theTable;
}

// function to sort data
function kn_table_sort(theOrder,theArray,theProp) {
	if (theOrder == "ascending"){
		return theArray.sort(function(a,b) {
			if (a[theProp] > b[theProp]) {
				return 1;
			} 
			if (a[theProp] < b[theProp]){
				return -1;
			} else {
				return 0;
			}
		});  
	} else 	if (theOrder == "descending") {
		return theArray.sort(function(a,b) {
			if (a[theProp] > b[theProp]) {
				return -1;
			} 
			if (a[theProp] < b[theProp]) {
				return 1;
			} else {
				return 0;
			}
		});
	}
}


// if no columns are passed to the object, this function is called to 
// generate a list of column headers with names of properties

function get_props(myArray,single) { 

	// this is a total hack to make get_props work with an arry or objects, or just a single object
	if (single){
		var props = new Object();
		for(prop in myArray) {
			if (prop != null && prop != "undefined" && prop != "") {
				i++;
				props[prop] = prop;
			}
		}
	} else {
		var x = myArray.length;
		var props = new Object();
		var i=0;
		if (x>1) x=1; // this only gets the props from the first loop of the array...possibly bad bad bad
		for(prop in myArray[x]) {
			if (prop != null && prop != "undefined" && prop != "") {
				i++
				props[prop] = prop;
			}
		}
	}
	return props;
}


// build buttons for pagination
// table_ref = table object
// left = boolean  whether to display or not
// right = boolean whether to display or not
// start = start point for this display
// per_page number of rows per page
// max total number of rows

function pagination_buttons(table_ref,left,right,start,per_page,max) {
	var back = "";
	var forward = "";
	var begin = "";
	var end = "";
	if (left == 1) { 
		begin = kn_tables[parseInt(table_ref)].begin_button;
		back = kn_tables[parseInt(table_ref)].back_button;
	} 
	if (right == 1) { 
		forward = kn_tables[parseInt(table_ref)].forward_button;
		end = kn_tables[parseInt(table_ref)].end_button;
	}

	// run it through a list of regexps that put the formatted links
	// into the buttons template.
	var buttons = new String(kn_tables[parseInt(table_ref)].buttons);

	buttons = buttons.replace(/\*current_max\*/ig,start+per_page);
	buttons = buttons.replace(/\*current_min\*/ig,start);
	buttons = buttons.replace(/\*forward\*/ig,forward);
	buttons = buttons.replace(/\*back\*/ig,back);
	buttons = buttons.replace(/\*end\*/ig,end);
	buttons = buttons.replace(/\*begin\*/ig,begin);

	return buttons
}


// when button is clicked this function is called to redraw table
// table_ref = object reference
// direction = integer indicating direction of movement

function page_me(table_ref,direction) {
	kn_tables[parseInt(table_ref)].draw(direction);
}

// this sets a kn_num property on the event object. You should call this in the event handler
// that's part of your application. use this to add a running number to your table
// e = the event which was sent to the handler
// the_array = the array which is holding all of the events

function set_number(e,the_array){
	e.kn_num = the_array.length;
}

// End of kn_table.js
