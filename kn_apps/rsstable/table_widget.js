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

// $Id: table_widget.js,v 1.2 2004/04/19 05:39:13 bsittler Exp $

// This is a sortable, filterable table widget that is document.write
// created, not DOM method calls created.

// Each column is defined in an array.  Here are what the elements mean: 
//   1. Column name that appears in the table header and Filter Window 
//   2. Data type code, a number from 0 to 4:
//           0 = Logical (yes/no data) 
//           1 = Number (integer or float) 
//           2 = Calendar date of the form MM/DD/JJJJ
//           3 = Options (a limited number of text alternatives) 
//           4 = Text (plain or HTML)
//   3. Minimum width of the column in the table (used in TH tags)
//   4. Option tags and texts, only needed if the data type code is 3.

// Modified from the Bungers code:
//     http://www.siteexperts.com/paradise/getResource.asp?r_id=1604
//     http://www.siteexperts.com/tips/databinding/ts06/paper/dhtml_table.htm

// From Dieter Bungers:
//     This script is based on ideas of the author.
// You may copy, modify and use it for any purpose. The only condition 
// is that if you publish web pages that use this script you point to 
// its author at a suitable place and don't remove this Statement from it.
// It's your responsibility to handle possible bugs even if you didn't 
// modify anything. I cannot promise any support.

// Known bugs:
//   1. Doesn't work in NS. (What did we do? Bungers' code worked in NS!)
//   2. Closing filter dialog causes next-page button to produce JS errors.
//   3. Displays past the end of the actual data.

// FIXME: This code is unstable for anything more than demos.
// It is in need of a significant rewrite.

function TableWidget( theNameInWindow, aDrawContext, someColumns, someRows )
{
  this.nameInWindow = theNameInWindow;
  this.drawContext = aDrawContext;
  this.columns = someColumns
  this.rows = someRows;

  this.id = Math.random().toString().substring( 2, 8 ); 
  this.filterDialog = null;
  this.cssUrl = "";
  this.cellSpacing = 0;
  this.cellPadding = 0;
  this.cellAlignment = "center";

  this.getPageHeader = TableWidget_getPageHeader;
  this.getTableHead = TableWidget_getTableHead;
  this.getTableColumnsHeader = TableWidget_getTableColumnsHeader;
  this.getTableBody = TableWidget_getTableBody;
  this.getControls = TableWidget_getControls;
  this.getPageFooter = TableWidget_getPageFooter;

  this.init = TableWidget_init;
  this.syncTableData = TableWidget_syncTableData;
  this.showFilterDialog = TableWidget_showFilterDialog; 
  this.refresh = TableWidget_refresh; 
  this.getID = TableWidget_getID;
  this.setFilterDialog = TableWidget_setFilterDialog;
  this.setPaginationPage = TableWidget_setPaginationPage;
  this.applyFilterToRows = TableWidget_applyFilterToRows;
  this.sortByColumn = TableWidget_sortByColumn;
  this.getColumnDefs = TableWidget_getColumnDefs;

  this.setCssUrl = TableWidget_setCssUrl;
  this.setCellPadding = TableWidget_setCellPadding;
  this.setCellSpacing = TableWidget_setCellSpacing;

  // Table rows to be displayed at once.
  this.paginationChunkSize = 10;

  // Index of the first and last table rows to be displayed.
  this.iFirst = 0;                  
  this.iLast = this.paginationChunkSize;        

  this.lastColumnSortedBy = 0; // First column
  this.isSortAscending = true;
  this.theColumn = new Array(); 
  this.syncTableData();
  this.init();
}

function TableWidget_init() 
{
  var filterDialog = new FilterDialog( this );
  this.setFilterDialog( filterDialog );
}

function TableWidget_syncTableData()
{
  for( var i=0; i < this.rows.length; i++ ) 
    { this.theColumn[ i ] = new Array( 0, i ); }
  this.tableWidth = 0;          
  for( j=0; j < this.columns.length; j++ ) 
    { this.tableWidth += this.columns[ j ][ 2 ]; }
  this.tableWidth += 2 * ( this.columns.length - 1 )
}

function TableWidget_getID()
{
  return this.id;
}

function TableWidget_setCssUrl( anUrl )
{
  this.cssUrl = anUrl;
  this.filterDialog.setCssUrl( anUrl );
}

function TableWidget_setFilterDialog( aFilterDialog )
{
  this.filterDialog = aFilterDialog;
}

function TableWidget_showFilterDialog() 
{   
  this.filterDialog.setUiFocusToWindow();
}

function TableWidget_getColumnDefs() 
{   
  return this.columns;
}

function TableWidget_setCellSpacing( anInt )
{
  this.cellSpacing = anInt;
}

function TableWidget_setCellPadding( anInt )
{
  this.cellPadding = anInt;
}

function TableWidget_getPageHeader() 
{
   var str = "<html>\n<head>\n" +
       "<meta http-equiv=\"content-type\" content=\"text/html; charset=utf-8\" />\n" +
       "<title>" + this.title + "</title>\n";
   if (this.cssUrl && (navigator.appName != "Netscape")) {
    str += "<link rel='stylesheet' type='text/css' href='"+this.cssUrl+"'>\n";
   }

   str +=
      "  <script language='JavaScript' > \n" +
      "    function TableProxy() \n" +
      "      { \n" +
      "      this.handleButtonClick = TableProxy_handleButtonClick; \n" +
      "      this.showFilterDialog = TableProxy_showFilterDialog; \n" +
      "      this.sortByColumn = TableProxy_sortByColumn; \n" + 
      "      } \n" +
      "    function TableProxy_handleButtonClick( buttonID ) \n" +
      "      { \n" +
      "      parent." + this.nameInWindow + ".setPaginationPage( buttonID ); \n" +
      "      } \n" +
      "    function TableProxy_showFilterDialog() \n" +
      "      { \n" +
      "      parent." + this.nameInWindow + ".showFilterDialog(); \n" +
      "      } \n" +
      "    function TableProxy_sortByColumn( aCol ) \n" +
      "      { \n" +
      "      parent." + this.nameInWindow + ".sortByColumn( aCol ); \n" +
      "      parent." + this.nameInWindow + ".refresh(); \n" +
      "      } \n" +
      "    var tableProxy = new TableProxy(); \n" +

      // Add in tool tip functionality
      "    function showToolTip(current,e,text) \n" +
      "      { \n" + 
      "      if (document.all) \n" +
      "        { \n" + 
      "        thetitle=text.split('<br>') \n" + 
      "        if (thetitle.length>1) \n" +
      "          { \n" + 
      "          thetitles='' \n" + 
      "          for (i=0;i<thetitle.length;i++) \n" + 
      "            thetitles+=thetitle[i] \n" + 
      "          current.title=thetitles \n" + 
      "          } \n" +
      "        else \n" +
      "          current.title=text \n" + 
      "        } \n" +
      "      else if (document.layers) \n" +
      "        { \n" + 
      "        document.tooltip.document.write('<layer bgColor=\"white\" ' + \n" +
      "            'style=\"border:1px solid black;font-size:12px;\">'+text+'</layer>') \n" + 
      "        document.tooltip.document.close() \n" + 
      "        document.tooltip.left=e.pageX+5 \n" + 
      "        document.tooltip.top=e.pageY+5 \n" + 
      "        document.tooltip.visibility='show' \n" +
      "        } \n" +
      "      } \n" +
      " \n" +
      "    function hideToolTip() \n " +
      "      { \n" +
      "      if (document.layers) \n" + 
      "        document.tooltip.visibility='hidden' \n " +
      "      } \n" + 
      "    function escapeHTMLAttribute(str) \n" +
      "      { \n" +
      "      var sNew = str.replace(/\\'/g,\"`\"); \n " +
      "      sNew = sNew.replace(/\\\"/g,\"`\"); \n " + 
      "      sNew = sNew.replace(/\\n/g,\" \"); \n " +
      "      sNew = sNew.replace(/\\r/g,\" \"); \n " + 
      "      return sNew; \n " +
      "      } \n " + 
      "    </script> \n" +
      "  </head> \n" +
      "<body> \n";

  return str;
}

function TableWidget_getTableHead()
{
  return "  <table border='0' cellspacing='" + this.cellSpacing + 
         "' cellpadding='" + this.cellPadding + "' width='100%'> \n";
}

// I give up, this code cannot be saved.  You're on your own, kid.  :)

function TableWidget_getControls() 
{   
  var strBuffer  = 
    "<form name=tableControls onSubmit='return false'>\n"+  
    "   <input type='button' value='&nbsp;|<<&nbsp;' onclick='tableProxy.handleButtonClick(-2)' >&nbsp;&nbsp; \n" +
    "   <input type='button' value='&nbsp;<&nbsp;' onclick='tableProxy.handleButtonClick( -1 )'>&nbsp;&nbsp; \n" ;

  var filteredRowsCount = this.theColumn.length;
  if( filteredRowsCount > 0 )
    {
      strBuffer += ( "Showing " + ( this.iFirst + 1 ) + " to " + Math.min( this.iLast, filteredRowsCount ) + " of " + filteredRowsCount + "\n" );
    }
  else
    { 
      strBuffer += "No rows match the current filter \n ";
    } 

  strBuffer +=
    "&nbsp;&nbsp;" +  
    "   <input type='button' value='&nbsp;>&nbsp;' onclick='tableProxy.handleButtonClick( 1 )' >&nbsp;&nbsp; \n" +
    "   <input type=button value='&nbsp;>>|&nbsp;' onclick='tableProxy.handleButtonClick( 2 )'> \n" +
    "   <input type=button value=' Filters... ' onClick='tableProxy.showFilterDialog()' >\n"+
    "</form> \n";

  return strBuffer;
}

function TableWidget_getTableColumnsHeader()
{
  var str = "   <tr> \n" ;

  // Build up the th's of column names.
  for( var k = 0; k < this.columns.length; k++ ) 
    {
      str += "<th "+ ((this.columns[k][2] == 0) ? "NOWRAP" : 
    ("width='" + Math.round( 100 * this.columns[ k ][ 2 ] / this.tableWidth) + "%'")) +
    " align='left' bgcolor='#000099'>" +
    "<a class='head' href='' onClick='tableProxy.sortByColumn(" + k + ");return false;'>"+
    "<font color=#ffffff face=sans-serif >" + this.columns[ k ][ 0 ];

    // If this is the column being sorted then throw in the switch-sort-order control.
    if( this.lastColumnSortedBy == k && this.columns.length > 1 )
    {
      if( this.isSortAscending )
        { str += "<img src='up.gif' border='0' />" ; }
      else
        { str += "<img src='down.gif' border= 0' />" ; }
    } else { str += "&nbsp;" ; }

    str += "</font></a></th> \n";
  }
  str += "</tr> \n";
  return str;
}

function TableWidget_getTableBody() 
{   
  var currentRows = "";
  var stopIndex = Math.min( this.iLast, this.theColumn.length );

  for( var k=this.iFirst; k < stopIndex; k++ ) 
  {
    theRec = this.rows[ this.theColumn[ k ][ 1 ] ];

    // Give the alternating rows background color.
    currentRows += ( "\n<tr class=" + ( ( Math.round(k/2) % 2 == 0 ) ? "light" : "dark" ) +
   " bgcolor=" + ( ( Math.round((k+1) / 2) % 2 == 0 ) ? "#ddddff" : "#ffdddd" ) +" >");

    for( l = 0; l < this.columns.length; l++ ) 
      {
      var currentValue = ( theRec[ l ] == "" ) ? "&nbsp;" : theRec[ l ];
      var theCell = "<td";

      // Render data based on the datatype.
      if( this.columns[ l ] [ 1 ] == 0 )
        {
        // Binary so is it checked or not.
        theCell += ( " align="+this.cellAlignment+">" + ( ( theRec[ l ] ) ? "<img src='check.gif'>" : "&nbsp;" ) );
        }
      else if( this.columns[ l ][ 1 ] <= 2 ) 
        {
        // Plain numerics I'm guessing.
        theCell += ( " align="+this.cellAlignment+">" + currentValue );
        } 
      else if ( this.columns[ l ][ 1 ] == 3 ) 
        {
        // This case is for multiple choices expressed in a select.
        // Twisted: the split results in an array and it is indexed into on currentValue.
        theCell += (" align="+this.cellAlignment+">"+this.columns[l][3].split("<option>")[currentValue]);
        } 
      else {
    if (this.columns[l][2] == 0) 
        theCell += " NOWRAP";
        theCell += ( " align="+this.cellAlignment+">" + currentValue ) ;
      }
      currentRows += ( theCell + "</td>" );
      }
    currentRows += "</tr>";
    }
  return currentRows;
  }


function TableWidget_getPageFooter()
{
  return
"\n <address align=right> <small>&copy;2000 KnowNow, Inc.<small></address>"+
"\n</body> \n </html>\n";
}

var ctr=0;
function TableWidget_refresh() 
{
  if( kn_isReady(this.drawContext) )
  {
    this.syncTableData();

    this.applyFilterToRows();

    this.isSortAscending = !this.isSortAscending; // Pre-flip the bit, since sortBy re-flips.
    this.sortByColumn(this.lastColumnSortedBy);

    this.drawContext.document.close();
    this.drawContext.document.open("text/html");

   s= this.getPageHeader() + 
    this.getTableHead() +
        this.getTableColumnsHeader() + 
    this.getTableBody()  + 
    "</table>\n" +
      this.getControls();
      this.getPageFooter(); 
    this.drawContext.document.write(s);
    this.drawContext.document.close();
   }
}


// During this function, the filterDialog is scanned for what the user wanted.
// Those UI settings are translated into a string called theFilter which will
// be used to see if a row passes the filtration test for viewing.


// Dieter Bungers wrote:
// Building the filters for selecting the records to be displayed in the table. 
// This is a string of JavaScript code, named "theFilter". It's only "true" if 
// the user didn't define any filter. Otherwise it consists of some comparision 
// expressions corresponding to the user's input in the Filter Window. The data 
// type of a column decides the way the user's input has to be transfomed into 
// a comparision expression. The comparision expressions are concatenated by the 
// AND-Operation "&&".

function TableWidget_applyFilterToRows()
{
  // FIXME: this should be modified so that it checks if a param was sent in. 
  // If so then use it as the filter to choose by instead of building it from UI.

  theFilter = "true ";
  for( j = 0; j < this.columns.length; j++ ) 
  {
    var dataType = this.columns[ j ][ 1 ];
    var thisVal = "thisRow[" + j + "]";

    var stupidWindow = this.filterDialog.dialogWindow;
    if (stupidWindow && typeof(stupidWindow.closed) != "undefined" && !stupidWindow.closed) {
      with( stupidWindow.document.inputs ) 
    {
      if( dataType == 0 && eval( "filterI_" + j + ".checked" )) 
        {
          theFilter += "&&" + thisVal;
        } 
      else if( dataType == 1 && ( eval( "filterS_" + j + ".selectedIndex" ) != 0 ) ) 
        {
          var theOp = eval( "filterS_" + j + ".options[ filterS_" + j + ".selectedIndex ].text" );
          theFilter += "&&(" + thisVal + theOp + eval( "filterI_" + j +".value" ) + ")";
        }
      else if (dataType == 2 && ( eval( "filterS_"+j+".selectedIndex" ) != 0 ) ) 
        {
          var theOp = eval( "filterS_"+j+".options[filterS_"+j+".selectedIndex].text" );
          var rightTerm = dateToNum( eval( "filterI_"+j+".value" ) );
          var leftTerm = "dateToNum(" + thisVal + ")";
          theFilter += ("&&(" + leftTerm + theOp + rightTerm + ")");
        }
      else if (dataType == 3 && (eval("filterS_"+j+".selectedIndex") != 0)) 
        {
          var theValue = eval("filterS_"+j+".selectedIndex");
          theFilter += ("&&(" + thisVal + "==" + theValue + ")");
        }
      else if (dataType == 4 && (eval("filterS_"+j+".selectedIndex") != 0)) 
        {
          var opIndex = eval( "filterS_" + j +".selectedIndex" );
          var theValue = eval( "filterI_" + j + ".value.toUpperCase()" );
          if( opIndex < 3 ) 
        theFilter += "&&(innerText(" + thisVal + ").toUpperCase().indexOf(\"" + theValue + "\"" 
          + ((opIndex==1) ? ")==0)" : ")!=-1)" );
          else  
        theFilter += ( "&&(innerText(" + thisVal + ").toUpperCase()==\"" + theValue + "\")" );
        }
    }
    }
  }

  // Now using the filter to obtain the indexes of the table rows
  // (that is, of the array theRow) that have to be displayed. This
  // results in an update of the container "theColumn", the second
  // subelement of the elements of which are the selected indexes.  

  myColumn = new Array();
  j = 0;
  for( var i = 0; i < this.rows.length; i++ ) 
    {
      var thisRow = this.rows[ i ];
      if( eval( theFilter ) ) 
    {
      myColumn[ j ] = new Array( 0, i );
      j++;
    }
    }
  this.theColumn = myColumn;

  // Sorting and redisplaying the table by means of the mySort and
  // scrollTable functions.

  //this.isSortAscending = !this.isSortAscending;
  //this.sortByColumn( this.lastColumnSortedBy  );  
  //this.setPaginationPage( -2 );
}



// This determines the segment (pagination page) of the array "theColumn" 
// the elements of which point to the table rows that currently have to be 
// displayed. Then the function redisplays the table by calling 
// "refreshTable()". - Meaning of the parameter x:
//     -2 : show first rows 
//     -1 : show previous rows
//      0 : recalculate currently visible rows
//      1 : show next rows
//      2 : show last rows
//      3 : show last N rows // Ro-chat hack that made it into our architecture!
// The function is called from the onclick events of the scroll buttons
// in the bottom line of the table and from the function mySort. 

function TableWidget_setPaginationPage( x ) 
{
  var oldIFirst=this.iFirst;
  var oldILast=this.iLast;

  this.paginationChunkSize = this.filterDialog.getPaginationChunkSize();

  if( x == -2 ) {
    // Show first row:
    this.iFirst = 0;
  } else if( x == 3 ) {
    // Show last #chunk rows:
    var a = (this.theColumn.length - this.paginationChunkSize) - 1 ;
    this.iFirst = Math.max( 0 , a );
  } else {
    // Show last (length%chunk) rows:
    var firstRowOfLastPage = (this.theColumn.length) - 
      (this.theColumn.length % this.paginationChunkSize);
    firstRowOfLastPage = Math.max( 0 , Math.min( firstRowOfLastPage, 
             this.theColumn.length-this.paginationChunkSize));
    if (x == 2) {
      this.iFirst = firstRowOfLastPage;
    } else {
      // Move forward or back by +/-1 * #chunk
      var firstRowOfNextIntendedPage = 
    Math.max( 0, ( this.iFirst += ( x * this.paginationChunkSize ) ) );
      this.iFirst = Math.min( firstRowOfNextIntendedPage, firstRowOfLastPage );
    }

   } 

  var unclippedLast = this.iFirst + this.paginationChunkSize;
  this.iLast = Math.min( unclippedLast, this.theColumn.length );
  if ((this.iFirst != oldIFirst) ||
      (this.iLast != oldILast))
  this.refresh();
}


// This sorts the indexes of the filtered table rows (stored in
// theColumn[j][1], j=1,2,....) by applying JavaScript's sort method to
// the row values (stored in theColumn[j][0], j=1,2,....). 

function TableWidget_sortByColumn( columnToSortBy ) 
{   
  //alert("sorting up: " + this.isSortAscending);
  if( this.lastColumnSortedBy == columnToSortBy ) 
    { 
      // Same column to be sorted so the sort direction toggles.
      this.isSortAscending = !this.isSortAscending;
    }
  else 
    {
      this.lastColumnSortedBy = columnToSortBy;  
      this.isSortAscending = true;
    }

  // Determining the sort value depending on the sort column's data type 
  // and storing it to theColumn[j][0], j=1,2,...: 

  var dataType = this.columns[ columnToSortBy ][ 1 ]; 
  for( var j=0; j < this.theColumn.length; j++ ) 
    { 
      var x = this.rows[ this.theColumn[ j ][ 1 ] ][ columnToSortBy ];
      if( dataType == 0 ) 
    {
      this.theColumn[ j ][ 0 ] = (x) ? 1 : 0;
    }
      else if( dataType == 1 || dataType == 3 ) 
    {
      this.theColumn[ j ][ 0 ] = eval( x );
    }
      else if( dataType == 2 ) 
    {
      if( x != "" ) 
        this.theColumn[ j ][ 0 ] = dateToNum( x );
      else 
        this.theColumn[ j ][ 0 ] = 0;
    }
      else 
    {
      this.theColumn[ j ][ 0 ] = innerText( x ).toUpperCase();
    }
    }
  if( this.isSortAscending )
    {
      this.theColumn.sort( compareAscendingly );
    }
  else
    {
      this.theColumn.sort( compareDescendingly ); // This performs the sort
    }
  //this.setPaginationPage( -2 ); 
}


// Gets passed to built-in sort() as the compare function to sort by.

function compareDescendingly( a, b )
  {
  if( a[ 0 ] < b[ 0 ] )
    return 1;
  if( a[ 0 ] > b[ 0 ] )
    return -1;
  return 0;
  }


// Gets passed to built-in sort() as the compare function to sort by.

function compareAscendingly( a, b )
  {
  if( a[ 0 ] < b[ 0 ] )
    return -1;
  if( a[ 0 ] > b[ 0 ] )
    return 1;
  return 0;
  }

// - - - - -  utils - - - - - - - - - - - - -

function dateToNum( dateString ) 
  {
  var dateArray = dateString.split("/");
  return dateArray[ 2 ] * 10000 + dateArray[ 0 ] * 100 + eval( dateArray[ 1 ] );
 }


// Eliminating HTML-tags from text to be filtered or sorted.

function innerText( str ) 
  { 
  while (str.indexOf("<") != -1) 
    {
    a = str.indexOf("<");
    b = str.indexOf(">");
    str = str.substring(0,a) + str.substring(b+1,str.length);
    }
  return( str );
  }

// End of table_widget.js
