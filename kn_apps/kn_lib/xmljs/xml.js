// =========================================================================
//
// XML for SCRIPT - an XML parser in JavaScript.
//
// =========================================================================
//
// Copyright (C) 2000  Michael Houghton (mike@idle.org)
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.

// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.

// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

// =========================================================================
// VERSION 0.22
// =========================================================================
// THANKS
// =========================================================================
//
//  TO: David Joham <djoham@criadvantage.com> 
// FOR: a patch to this function to correctly set the XMLNode.content 
//      property for CDATA and COMMENT nodes
//
//  TO: Stu Coates <Stuart.Coates@sherwoodinternational.com>
// FOR: the same fix as David Joham :)  
//      
//  TO: Mike Yukish <may106@psu.edu>
// FOR: a better trim() that works properly on all-whitespace strings 
//
// =========================================================================


// =========================================================================
// define the characters which constitute whitespace, and quotes
// =========================================================================

var whitespace = "\n\r\t ";
var quotes = "\"'";

// =========================================================================
// isEmpty() - convenience function to identify an empty string
// =========================================================================

function isEmpty(str) { return (str==null) || (str.length==0); }

// =========================================================================
// trim() - helper function to trim a string s of leading (l=true) and 
//          trailing (r=true) whitespace.
// =========================================================================

function trim(s, l,r)
{
 if(isEmpty(s)) return "";
 
 // the general focus here is on minimal method calls - hence only one 
 // substring is done to complete the trim.

 var left=0; var right=0;
 var i=0; var k=0;

// modified to properly handle strings that are all whitespace
 if(l) { while((i<s.length) && 
                        (whitespace.indexOf(s.charAt(i++))!=-1)) left++;}
 if(r) { k=s.length-1; while((k>=left) && 
                        (whitespace.indexOf(s.charAt(k--))!=-1)) right++; }
 return s.substring(left, s.length - right);
}

// =========================================================================
// firstWhiteChar() - return the position of the first whitespace character
//                    in str after position pos.
// =========================================================================

function firstWhiteChar(str,pos)
{
 if(isEmpty(str)) return -1; 
 
 while(pos < str.length)
 {
  if(whitespace.indexOf(str.charAt(pos))!=-1)
   return pos;
  else pos++; 
 }

 return str.length;
}

// =========================================================================
// XMLNode() is a constructor for a node of XML (text, comment, cdata, 
//           tag, etc.)
//     src:  contains the text for the tag or text entity
//   isTag:  identifies whether the text has been found within < >
//           characters.
//     doc:  contains a reference to the XMLDoc object describing the 
//           document.
// =========================================================================

function XMLNode(nodeType,doc, str)
{
 this.nodeType = nodeType;          // the type of the node 

  // the content of text (also CDATA and COMMENT) nodes
 if (nodeType=='TEXT' || nodeType=='CDATA' || nodeType=='COMMENT' ) {
    this.content = str;
 }
 else {
    this.content = null;
 }

 this.doc = doc;         // a reference to the document
 this.parent;
  
 this.tagName;           // the name of the tag (if a tag node)
 this.attributes = null; // an array of attributes (used as a hash table)
 this.children = null;   // an array (list) of the children of this node

 // configure the methods

 this.toString = _XMLNode_toString;
 this.getText = _XMLNode_getText;
 
 this.getAttribute = _XMLNode_getAttribute;
 this.getAttributeNames = _XMLNode_getAttributeNames;
 this.addAttribute = _XMLNode_addAttribute;

 this.getElements = _XMLNode_getElements;
 this.addElement = _XMLNode_addElement;
 
 this.getParent = _XMLNode_getParent;  

}

// =========================================================================
// XMLNode.getText() - a method to get the text of a given node 
//                     (recursively, if it's an element.)
// =========================================================================

function _XMLNode_getText() 
{ 
 if(this.nodeType=='ELEMENT')
 {
  if(this.children==null) return null;
 
  var str = "";
  
  for(var i=0; i < this.children.length; i++)
  {
   var t = this.children[i].getText();
   str +=  (t == null ? "" : t);
  }
  return str;
 }
 
 else if((this.nodeType=='TEXT') || (this.nodeType=='CDATA'))
  return this.content;
 
 else return null;  // comment nodes get caught here
}

// =========================================================================
// XMLNode.getAttribute() - get the value of a named attribute from an 
//                          element node
// =========================================================================

// we prefix with _ because of the weird 'length' meta-property

function _XMLNode_getAttribute(name) 
{ 
 if(this.attributes == null) return null;
 return this.attributes['_' + name]; 
}

// =========================================================================
// XMLNode.getAttributeNames() - get a list of attribute names for the node 
// =========================================================================

// we prefix with _ because of the weird 'length' meta-property

function _XMLNode_getAttributeNames() 
{ 
 if(this.attributes == null) return null;
 
 var attlist = new Array();

 for(var a in this.attributes)
 {
  attlist[attlist.length] = a.substring(1);
 }
 return attlist; 
}

// =========================================================================
// XMLNode.getParent() - get the parent of this node
// =========================================================================

function _XMLNode_getParent() { return this.parent; }

// =========================================================================
// XMLNode.getElements(byName) - get an array of element children of a node, 
//                               with an optional filter by name
// =========================================================================

function _XMLNode_getElements(byName) 
{ 
 if(this.children==null)
  return null;
 var elements = new Array();
 for(var i=0; i<this.children.length; i++)
 {
  if((this.children[i].nodeType=='ELEMENT') &&
      ((byName==null) || (this.children[i].tagName == byName)))
   
   elements[elements.length] = this.children[i];
 }
 return elements;
}

// =========================================================================
// XMLNode.toString() - produce a diagnostic string description of a node
// =========================================================================

function _XMLNode_toString() 
{  
 return "" + this.nodeType + ":" + 
  (this.nodeType=='TEXT' || this.nodeType=='CDATA' || 
    this.nodeType=='COMMENT' ? this.content : this.tagName); 
}

// =========================================================================
// XMLNode.addAttribute() - add an attribute to a node
// =========================================================================

function _XMLNode_addAttribute(n,v) 
{  
 if(this.getAttribute(n)!=null)
   return this.doc.error("cannot repeat attribute names in elements");  

 this.attributes['_' + n] = v;
 return true;
}

// =========================================================================
// XMLNode.addElement() - add an element child to a node
// =========================================================================

function _XMLNode_addElement(node) 
{  
 node.parent = this;
 this.children[this.children.length] = node;

 return true;
}


// =========================================================================
// XMLDoc  - a constructor for an XML document
//   source: the string containing the document
//    logFn: the (optional) function used to log the stream for debugging
//    errFn: the (optional) function used to log errors
// =========================================================================

function XMLDoc(source, errFn)
{
 this.source = source;        // the string source of the document
 this.docNode;                // the document node
 this.hasErrors = false;      // were errors found during the parse?
  
 // stack for document construction
 
 this.topNode=null;

 // set up the methods for this object
 
 this.errFn = errFn;          // user defined error functions

 this.handleNode = _XMLDoc_handleNode;

 this.error = _XMLDoc_error; 
 this.parse = _XMLDoc_parse;
 this.parseTag = _XMLDoc_parseTag;
 this.parseAttribute = _XMLDoc_parseAttribute;
 this.parsePI = _XMLDoc_parsePI;
 this.parseDTD = _XMLDoc_parseDTD;

 // parse the document
  
 if(this.parse()) 
 {
  // we've run out of markup - check the stack is now empty
  if(this.topNode!=null) 
   return this.error("expected close " + this.topNode.tagName);
  else return true;
 }
}

// =========================================================================
// XMLDoc.parseTag() - parse out a non-text element (incl. CDATA, comments)
//                   - handles the parsing of attributes 
// =========================================================================


function _XMLDoc_parseTag(src)
{

 // if it's a comment, strip off the packaging, mark it a comment node 
 // and return it
 
 if(src.indexOf('!--')==0)
  return new XMLNode('COMMENT',null,src.substring(3,src.length-2));

 // if it's CDATA, do similar

 if(src.indexOf('![CDATA[')==0) {
  return new XMLNode('CDATA',null,src.substring(8,src.length-2));
  }
 var n = new XMLNode();
 
 // if it's a closing tag - mark it as a CLOSE node for use in pass 3, and 
 // snip off the first character

 if(src.charAt(0)=='/') { n.nodeType = 'CLOSE'; src = src.substring(1); }
 
 // otherwise it's an open tag (possibly an empty element)

 else n.nodeType = 'OPEN';

 // if the last character is a /, check it's not a CLOSE tag
 
 if(src.charAt(src.length-1)=='/')
 {
  if(n.nodeType=='CLOSE') 
   return this.error("singleton close tag");
  else n.nodeType = 'SINGLE';
  
  // strip off the last character
  
  src = src.substring(0,src.length-1);
 }

 // set up the properties as appropriate
    
 if(n.nodeType!='CLOSE') n.attributes = new Array();
 if(n.nodeType=='OPEN') n.children = new Array();
       
 // trim the whitespace off the remaining content  
       
 src = trim(src,true,true);
 
 // chuck out an error if there's nothing left
 
 if(src.length==0) return this.error("empty tag");
   
 // scan forward until a space...
 
 var endOfName = firstWhiteChar(src,0);
 
 // if there is no space, this is just a name (e.g. (<tag>, <tag/> or </tag>
 
 if(endOfName==-1) { n.tagName = src; return n; } 

 // otherwise, we should expect attributes - but store the tag name first

 n.tagName = src.substring(0,endOfName);
 
 // start from after the tag name
 
 var pos = endOfName; 
 
 // now we loop:

 while(pos< src.length) 
 {
 
  pos = this.parseAttribute(src, pos, n); 
  if(pos==0)
   return null;

  // and loop
          
 }
 return n;
}

// =========================================================================
// XMLDoc.parseAttribute() - parse an attribute out of a tag string 
// =========================================================================

function _XMLDoc_parseAttribute(src,pos,node)
{
 
 // chew up the whitespace, if any
  
 while((pos<src.length) && (whitespace.indexOf(src.charAt(pos))!=-1)) pos++; 
 
 // if there's nothing else, we have no (more) attributes - just break out

 if(pos >= src.length) return pos; 
    
 var p1 = pos;
   
 while((pos < src.length) && (src.charAt(pos)!='=')) pos++; 
   
 var msg = "attributes must have values";
  
 // parameters without values aren't allowed.
   
 if(pos >= src.length) return this.error(msg);
     
 // extract the parameter name
   
 var paramname = trim(src.substring(p1,pos++),false,true);
  
 // chew up whitespace
          
 while((pos < src.length) && (whitespace.indexOf(src.charAt(pos))!=-1))
  pos++;  
  
 // throw an error if we've run out of string

 if(pos >= src.length) return this.error(msg); 
  
 msg = "attribute values must be in quotes";
   
 // check for a quote mark to identify the beginning of the attribute value 
   
 var quote = src.charAt(pos++);
  
 // throw an error if we didn't find one
  
 if(quotes.indexOf(quote)==-1)  return this.error(msg);
  p1 = pos;
   
 while((pos < src.length) && (src.charAt(pos)!=quote)) pos++; 
  
 // throw an error if we found no closing quote
  
 if(pos >= src.length) return this.error(msg);
    
 // store the parameter
  
 if(!node.addAttribute(paramname,trim(src.substring(p1,pos++),false,true))) 
  return 0;
  
 return pos;
}


// =========================================================================
// XMLDoc.error() - used to log an error in parsing or validating
// =========================================================================

function _XMLDoc_error(str)
{
 this.hasErrors=true;
 if(this.errFn) this.errFn("ERROR: " + str);
 return 0;
}

// =========================================================================
// XMLDoc.parse() - scans through the source for opening and closing of tags
//                - checks that the tags open and close in a sensible order
// =========================================================================

function _XMLDoc_parse()
{
 
 var pos = 0;

 // set up the arrays used to store positions of < and > characters
 
 err = false;
 
 while(!err)
 {
  var closing_tag_prefix = '';
  var chpos = this.source.indexOf('<',pos);
  var open_length = 1;
  
  var open, close;
    
  if(chpos ==-1)  break; 

  open = chpos;  

  // create a text node
 
  var str = this.source.substring(pos, open);

  if(str.length!=0) err = !this.handleNode(new XMLNode('TEXT',this, str));
  
  // handle PIs - they can't reliably be handled as tags
  
  if(chpos == this.source.indexOf("<?",pos))
  {
   pos = this.parsePI(this.source, pos + 2);
   if(pos==0) err=true;
   continue;
  }
  
  // nobble the document type definition 

  if(chpos == this.source.indexOf("<!DOCTYPE",pos))
  {
   pos = this.parseDTD(this.source, chpos+ 9);
   if(pos==0) err=true;
   continue;
  }
  
  // if we found an open comment, we need to ignore angle brackets
  // until we find a close comment 
  
  if(chpos == this.source.indexOf('<!--',pos))  
  { 
   open_length = 4;
   closing_tag_prefix = '--'; 
  }

  // similarly, if we find an open CDATA, we need to ignore all angle
  // brackets until a close CDATA sequence is found
 
  if(chpos == this.source.indexOf('<![CDATA[',pos)) 
   { open_length = 9; closing_tag_prefix = ']]'; }
 
  // look for the closing sequence

  chpos = this.source.indexOf(closing_tag_prefix + '>',chpos);
  if(chpos ==-1)  
   return this.error("expected closing tag sequence: " + 
                      closing_tag_prefix + '>'); 
  
  close = chpos + closing_tag_prefix.length;
  
  // create a tag node
  
  str = this.source.substring(open+1, close);
 
  var n = this.parseTag(str);

  if(n) err = !this.handleNode(n);
      
  pos = close +1;
  
  // and loop
  
 }
 return !err;
}

// =========================================================================
// XMLDoc.parsePI() - parse a processing instruction
// =========================================================================

function _XMLDoc_parsePI(str,pos) 
{ 
 // we just swallow them up
 
 var closepos = str.indexOf('?>',pos);
 return closepos + 2;
}


// =========================================================================
// XMLDoc.parseDTD() - parse a document type declaration
// =========================================================================

function _XMLDoc_parseDTD(str,pos) 
{
 // we're just going to discard the DTD
 
 var firstClose = str.indexOf('>',pos);

 if(firstClose==-1) return this.error("error in DTD: expected '>'");
 
 var closing_tag_prefix = ''; 

 var firstOpenSquare = str.indexOf('[',pos);

 if((firstOpenSquare!=-1) && (firstOpenSquare < firstClose))
  closing_tag_prefix = ']';
 
 while(true)
 { 
  var closepos = str.indexOf(closing_tag_prefix + '>',pos);
 
  if(closepos ==-1)  
   return this.error("expected closing tag sequence: " + 
                     closing_tag_prefix + '>'); 
    
  pos = closepos + closing_tag_prefix.length +1;

  if(str.substring(closepos-1,closepos+2) != ']]>')
   break;
 }
 return pos;
}



// =========================================================================
// XMLDoc.handleNode() - adds a markup element to the document
//                  str: the string source of the element
//                isTag: identifies a tag element
// =========================================================================


function _XMLDoc_handleNode(current)
{
 
 if((current.nodeType=='COMMENT') && (this.topNode!=null))
  return this.topNode.addElement(current); 
 
 // if the current node is a text node:

 else if((current.nodeType=='TEXT') ||  (current.nodeType=='CDATA'))
 {
  
  // if the stack is empty, and this text node isn't just whitespace, we have
  // a problem (we're not in a document element)
     
  if(this.topNode==null)
  {
   if(trim(current.content,true,false)=="") return true;
   else return this.error("expected document node, found: " + current);
  }
   
  // otherwise, append this as child to the element at the top of the stack
   
  else return this.topNode.addElement(current);
 }
  
 // if we find an element tag (open or empty)
  
 else if((current.nodeType=='OPEN') || (current.nodeType=='SINGLE'))
 {
  var success = false;

  // if the stack is empty, this node becomes the document node

  if(this.topNode==null) 
  { 
   this.docNode = current; current.parent = null;  success = true;
  }
   
  // otherwise, append this as child to the element at the top of the stack
   
  else success = this.topNode.addElement(current);
        
  if(success && (current.nodeType!='SINGLE'))
   this.topNode = current;   
  
  // rename it as an element node 
  
  current.nodeType = "ELEMENT";

  return success;
 }

 // if it's a close tag, check the nesting

 else if(current.nodeType=='CLOSE')
 {
   
  // if the stack is empty, it's certainly an error
  
  if(this.topNode==null)
   return this.error("close tag without open: " +  current.toString());

  // otherwise, check that this node matches the one on the top of the stack

  else
  {
   if(current.tagName!=this.topNode.tagName)
    return this.error("expected closing " + this.topNode.tagName + 
                            ", found closing " + current.tagName);
    
   // if it does, pop the element off the top of the stack
    
   else this.topNode = this.topNode.getParent();    
  }
 }
 return true;
}

