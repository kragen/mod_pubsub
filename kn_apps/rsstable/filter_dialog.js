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

// $Id: filter_dialog.js,v 1.2 2004/04/19 05:39:13 bsittler Exp $

// UI dialog for configuring the filter to apply to a TableWidget's rowset.

// FIXME: This code, like table_widget.js, is unstable for anything more
// than demos.  It could use a significant cleaning and rewrite.

function FilterDialog( aTableWidget )
  {
  this.tableWidget = aTableWidget;
  this.dialogWindow = null;
  this.paginationChunkSize = 10; // by default
  this.cssUrl = "";

  // Establish methods:
  this.setTable = FilterDialog_setTable;
  this.getInitializedFilterControls = FilterDialog_getInitializedFilterControls;
  this.initializeDialogWindow = FilterDialog_initializeDialogWindow;
  this.getPaginationChunkSize = FilterDialog_getPaginationChunkSize;
  this.setUiFocusToWindow = FilterDialog_setUiFocusToWindow;
  this.setCssUrl = FilterDialog_setCssUrl;
  //this.initializeDialogWindow();
  }

function FilterDialog_setCssUrl( anURL )
  {
  this.cssUrl = anURL;
  }

function FilterDialog_initializeDialogWindow()
  {
  var windowName = "FilterDialogForTableWidget" + this.tableWidget.getID();
  this.dialogWindow = window.open( "about:blank", windowName,
            "toolbar=no,status=no,scrollbars=no,resizable=yes,width=400,height=400");
  var writeBuffer = "<html>" ;
  writeBuffer += " <head>" ;
  writeBuffer += "  <meta http-equiv=\"content-type\" content=\"text/html; charset=utf-8\" />";
  writeBuffer += "  <title>Table Filter Controls</title>" ;
            // +" for " + this.tableWidget.getID() + " </title>" ;
  writeBuffer += "   <link rel='stylesheet' type='text/css' href='" + this.cssUrl + "' />" ;
  writeBuffer += "   </head>" ;
  writeBuffer += " " ;
  writeBuffer += " <body onLoad='if (document.all) {f=document.forms[0]; resizeTo(f.offsetWidth, f.offsetHeight+31)}'>" ; //testing for IE implicitly 
  writeBuffer += "  <form name='inputs' >" ;
  writeBuffer += "   <table border='0' cellspacing='2' cellpadding='2' width='400' >" ;
  writeBuffer += "    <tr> <th>Column to test</th> <th>Operator</th> <th>Value</th> </tr>" ;
  writeBuffer += " " ;

  writeBuffer += this.getInitializedFilterControls();

  writeBuffer += "   <tr>" ; 
  writeBuffer += "    <th colspan='3'>" ;
  writeBuffer += "     Rows per page: " ;
  writeBuffer += "       <input name='paginationChunkSize' type='text' size='3'  value='" + 
                           this.paginationChunkSize + "' onChange='opener." + 
                          this.tableWidget.nameInWindow + ".setPaginationPage(0);' />" ;
  writeBuffer += "     &nbsp;&nbsp;&nbsp;&nbsp;" ;
  writeBuffer += "     <input type=button value='Apply Filters' onClick='x=opener." + 
                          this.tableWidget.nameInWindow + ";x.applyFilterToRows();x.setPaginationPage(0);' />" ;
  writeBuffer += "     &nbsp;&nbsp;" ;
  writeBuffer += "     <input type='reset' value='Reset'/>" ;
  writeBuffer += "     </th>" ;
  writeBuffer += "    </tr>" ;
  writeBuffer += "   </table>" ;
  writeBuffer += "  </form>" ;
  writeBuffer += " </body>" ;
  writeBuffer += "</html>" ;

  while (typeof this.dialogWindow.document == "undefined") {}
  this.dialogWindow.document.write( writeBuffer );
  this.dialogWindow.document.close();
  }

// Methods from here on down.

function FilterDialog_setTable( aTableWidget )
  {
  this.tableWidget = aTableWidget;
  }


// This returns the value in the form for how many rows to show on a page.

function FilterDialog_getPaginationChunkSize()
  {
  if( this.dialogWindow )
    {
    var usersInput = parseInt( this.dialogWindow.document.inputs.paginationChunkSize.value, 10 );
    if( isNaN( usersInput ) )
      {
      this.dialogWindow.document.inputs.paginationChunkSize.value = "10";
      this.paginationChunkSize = 10;
      }
    else
      {
      this.paginationChunkSize = usersInput;
      }
    }
  return this.paginationChunkSize;
  }


// This initializes the table rows of the Filter Window.  Each row
// corresponds with a column of the table to be displayed and represents a
// filter corresponding to the data type of this column. The function is
// called from the Filter Window only once when it is loaded.

function FilterDialog_getInitializedFilterControls() 
  {
  var docWriteBuffer = "";
  var columns = this.tableWidget.getColumnDefs();
    
  for( i=0; i < columns.length; i++ ) 
    {
    docWriteBuffer += "<tr class=dark><th>" + columns[ i ][ 0 ] + "</th><td align='right' >" ;
    var dataType = columns[ i ][ 1 ];
    if( dataType == 0 ) 
      docWriteBuffer += "Check:</td><td><input name='filterI_" + i + "' type='checkbox' />" ;
    else if( dataType == 3 ) 
      docWriteBuffer += "Select:</td><td><select name='filterS_" + i + "' >" +
                        "<option>&nbsp;" + columns[ i ][ 3 ] + "</option></select>" ;
    else if( dataType == 4 ) 
      docWriteBuffer += "<select name='filterS_" + i + "' >" +
          "<option>&nbsp;</option><option>Starts with</option><option>Includes</option>" +
          "<option>Equals</option</select></td>" +
          "<td><input name='filterI_" + i + "' type='text' size='10' />" ;
    else 
      docWriteBuffer += "<select name=filterS_" + i + 
          "><option>&nbsp;<option>&lt;<option>&lt;=<option>==<option>&gt;=<option>&gt;</select>" +
          "</td><td><input name=filterI_" + i + " type=text size=10>" ;
    docWriteBuffer += "</tr>" ;
    }
  return docWriteBuffer;
  }


function FilterDialog_setUiFocusToWindow()
  {
  if( this.dialogWindow != null && !this.dialogWindow.closed )
    {
    this.dialogWindow.focus();
    }
  else
    {
    this.initializeDialogWindow();
    }
  }

// End of filter_dialog.js
