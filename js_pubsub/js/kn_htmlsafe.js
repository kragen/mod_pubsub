/*! @file kn_htmlsafe.js PubSub HtmlSafe Component
 * <pre>
 * &lt;script src="../../js/kn_config.js" type="text/javascript"&gt;&lt;/script&gt;
 * &lt;script src="../../js/kn_browser.js" type="text/javascript"&gt;&lt;/script&gt;
 * <b>&lt;script type="text/javascript"&gt;
 * kn_include("kn_htmlsafe");
 * &lt;/script&gt;</b>
 * </pre>
 */

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

// $Id: kn_htmlsafe.js,v 1.2 2004/04/19 05:39:10 bsittler Exp $


// asciiString = enitfyUTF16(aString)

kn_include("kn_defaults");

/*!
 * This is a UTF-16 decoder which replaces extended characters with SGML numeric character references.
 *
 * @tparam string aString The string to be decoded.
 * @treturn string The decoded string.
 */
function kn_htmlsafe_entifyUTF16(aString)
{
  var i;
  var result = new Array();
  for (i = 0; i < aString.length; i++)
  {
    var c = aString.charCodeAt(i);
    if ((c < 32) || (c > 126))
    {
      var oc = c;
      if ((i + 1 < aString.length) &&
        ((c & ~KN.utf16mask) == KN.utf16firstLowHalf) &&
        (((oc = aString.charCodeAt(i + 1)) & ~KN.utf16mask) == KN.utf16firstHighHalf))
      {
                                c = c & KN.utf16mask;
                                oc = oc & KN.utf16mask;
        c <<= KN.utf16shift;
        c += KN.utf16offset + oc;
        i++;
      }
      result[result.length] = '&' + '#' + c + ';';
    }
    else
    {
      result[result.length] = String.fromCharCode(c);
    }
  }
  return result.join('');
}

// kn_htmlsafe_urlstrings should match "autohighlighted" URLs.
// [enabled by the "autoLink" flag to kn_htmlsafe_htmlSanitize]

var kn_htmlsafe_urlstrings =
  /^(nntp|news|http|https|mailto|ftp|gopher):[^\000-\037 ""<>]+/i;

// Automatic kn_htmlsafe_smileys and kn_htmlsafe_winkeys.
// [enabled by the "showEmoticons" flag to kn_htmlsafe_htmlSanitize]

var kn_htmlsafe_smileys = /^(:[-^]?\)|\([-^]?:)/;
var kn_htmlsafe_winkeys = /^(;[-^]?\)|\([-^]?;)/;

// html = kn_htmlsafe_smileyTag(name, alt)

// This function generates a smiley image corresponding to alt.

function kn_htmlsafe_smileyTag(name, alt)
{
  var dflt = kn_defaults_get("kn_htmlsafe","ImagePath",kn_browser_includePath + "images/") + name + '.gif';

  var p = kn_resolvePath(location.pathname,dflt);
  
  return ('<img src="' + p + '" alt="' + alt + '" />');
}

function kn_htmlsafe_tokenize(tokenPatterns, aString)
{
  var i;
  var tokens = new Array;
  while (aString != "")
  {
    var old_length = aString.length;
    for (i in tokenPatterns)
    {
      var newString = aString.replace(tokenPatterns[i], "");
      if (newString.length < aString.length)
      {
        tokens[tokens.length] = aString.substr(0, aString.length - newString.length);
        aString = newString;
        break;
      }
    }
    if (aString.length == old_length)
    {
      // Fallback for Opera (which has a broken RegExp implementation).
      tokens[tokens.length] = aString.charAt(0);
      aString = aString.substr(1);
    }
  }
  return tokens;
}

// kn_htmlsafe_htmlemptytags should match HTML tags that do not
// allow content (except for IMG tags, which have their own RegExp).

var kn_htmlsafe_htmlemptytags =
  /^<[bh]r[ \r\n]*\/?>/i;

// kn_htmlsafe_htmlimgtags should match the IMG tag.

var kn_htmlsafe_htmlimgtags =
  /^<img([ \r\n]+src[ \r\n]*=[ \r\n]*('(nntp:|news:|http:|https:|ftp:|gopher:)[^\000-\037 ''<>]*'|"(nntp:|news:|http:|https:|ftp:|gopher:)[^\000-\037 ""<>]*")|[ \r\n]+alt[ \r\n]*=[ \r\n]*("[^""<>]*"|'[^''<>]*'|[^''""=\/<> \r\n]+))+[ \r\n]*\/?>/i;

// kn_htmlsafe_entitites should match SGML character entities.

var kn_htmlsafe_entitites = /^&(#[0-9]+|#x[0-9a-fA-F]+|[A-Za-z]+);/;

var kn_htmlsafe_htmltokens = [
  kn_htmlsafe_smileys,
  kn_htmlsafe_winkeys,
  kn_htmlsafe_htmlemptytags,
  kn_htmlsafe_htmlimgtags,
  kn_htmlsafe_urlstrings,
  kn_htmlsafe_entitites,
  /^<[ \r\n]*\/[ \r\n]*>/i,
  /^<\/?(tt|i|b|big|small|strike|s|u|sub|sup)[ \r\n]*>/i,
  /^<\/?(em|strong|cite|dfn|code|samp|kbd|va[r]|abbr|acronym)[ \r\n]*>/i,
  /^<(q|ins|del)([ \r\n]+cite[ \r\n]*=[ \r\n]*("[^""<>]*"|'[^''<>]*'))?[ \r\n]*>/i,
  /^<\/(q|ins|del)[ \r\n]*>/i,
  /^<a[ \r\n]+href[ \r\n]*=[ \r\n]*('(nntp|news|http|https|mailto|ftp|gopher):[^\000-\037 ''<>]*'|"(nntp|news|http|https|mailto|ftp|gopher):[^\000-\037 ""<>]*")[ \r\n]*>/i,
  /^<\/a[ \r\n]*>/i,
  /^<font([ \r\n]+(color|face|size)[ \r\n]*=[ \r\n]*([^''""=\/<> \r\n]+|'[^''<>]*'|"[^""<>]*"))*[ \r\n]*>/i,
  /^<\/font[ \r\n]*>/i,
  /^(\r\n|\n\r)/,
  /^(.|[\r\n])/
];

function kn_htmlsafe_htmlTagName(aString)
{
  return aString.replace(/^<\/?([a-zA-Z][a-zA-Z0-9]*)[ \/\r\n>](.|[\r\n])*$/, '$1').toLowerCase();
}

function kn_htmlsafe_isHtmlOpenTag(aString)
{
  return aString.match(/^<[a-zA-Z]+([ \r\n]+[a-zA-Z]+[ \r\n]*=[ \r\n]*([^''""=\/<> \r\n]+|"[^""<>]*"|'[^''<>]*'))*[ \r\n]*\/?>$/);
}

function kn_htmlsafe_isHtmlCloseTag(aString)
{
  return aString.match(/^<\/[a-zA-Z]+>$/);
}

function kn_htmlsafe_isHtmlCloseLastTag(aString)
{
  return aString.match(/^<[ \r\n]*\/[ \r\n]*>$/);
}

function kn_htmlsafe_htmlOpenTag(tagname)
{
  return "<" + tagname + ">";
}

function kn_htmlsafe_htmlCloseTag(tagname)
{
  return "<" + "/" + tagname + ">";
}

function kn_htmlsafe_htmlEmptyTag(tagname)
{
  return "<" + tagname + " />";
}

function kn_htmlsafe_closeElement(aStack)
{
  var tag = kn_htmlsafe_htmlCloseTag(aStack[aStack.length-1]);
  aStack.length = aStack.length - 1;
  return tag;
}

function kn_htmlsafe_findTagInStack(tag, aStack)
{
  for (var j in aStack)
  {
    if (aStack[j] == tag)
    {
      return j;
    }
  }
  return null;
}

// safeString = kn_htmlsafe_htmlSanitize(aString, autoLink, showEmoticons)

// Converts random HTML into fairly safe, if annoying, HTML.

// We do this here so that people can get some exciting benefits:
// big and small text, gigantic inline images, and hyperlinks.
// Mostly hyperlinks.

/*!
 * Converts random HTML into fairly safe HTML.
 *
 * @tparam string aString The string to be converted.
 * @tparam boolean autoLink If set to true, hyperlink text is converted to actual links.
 * @tparam boolean showEmoticons If set to true, emoticon characters are converted to inline images.
 * @treturn string The converted string.
 */
function kn_htmlsafe_htmlSanitize(aString, autoLink, showEmoticons)
{
  if (aString == null)
  {
    return aString;
  }
  
  var tokens = kn_htmlsafe_tokenize(kn_htmlsafe_htmltokens, kn_htmlsafe_entifyUTF16(aString));
  var aStack = new Array;
  for (var i in tokens)
  {
    if (tokens[i] == "<")
    {
      tokens[i] = "&" + "lt;";
    } 
    else if (tokens[i] == ">")
    {
      tokens[i] = "&" + "gt;";
    }
    else if (tokens[i] == "&")
    {
      tokens[i] = "&" + "amp;";
    }
    else if (tokens[i].match(kn_htmlsafe_entitites))
    {
      // Pass it through.
    }
    else if (autoLink && tokens[i].match(kn_htmlsafe_urlstrings))
    {
      if (typeof(kn_htmlsafe_findTagInStack('a', aStack)) == typeof(null))
      {
        tokens[i] =
          '<a target="' +
          $_(kn_defaults_get("kn_htmlsafe",
                     "LinkTarget",
                     'ChatLink${0}'),
             Math.random().toString().replace('.','_')) +
          '" href="' +
          tokens[i] +
          '">' +
          tokens[i] +
          '<' + '/a>';
      }
    }
    else if (showEmoticons && tokens[i].match(kn_htmlsafe_smileys))
    {
      tokens[i] = kn_htmlsafe_smileyTag('smiley', tokens[i]);
    }
    else if (showEmoticons && tokens[i].match(kn_htmlsafe_winkeys))
    {
      tokens[i] = kn_htmlsafe_smileyTag('winkey', tokens[i]);
    }
    else if (kn_htmlsafe_isHtmlCloseLastTag(tokens[i]))
    {
      tokens[i] = '';
      if (aStack.length)
      {
        tokens[i] = kn_htmlsafe_closeElement(aStack);
      }
    }
    else if (kn_htmlsafe_isHtmlOpenTag(tokens[i]))
    {
      var token = tokens[i];
      tokens[i] = '';

      var tag = kn_htmlsafe_htmlTagName(token);
      if (token.match(kn_htmlsafe_htmlemptytags))
      {
        token = kn_htmlsafe_htmlEmptyTag(tag);
        if (tag == "br")
        {
          token = "&" + "nbsp;" + token + "&" + "nbsp;";
        }
      }
      else if (token.match(kn_htmlsafe_htmlimgtags))
      {
        // Passed verbatim.
      }
      else
      {
        // "A" tags can't be nested.
        if (tag == "a")
        {
          var j = kn_htmlsafe_findTagInStack(tag, aStack);

          if (typeof(j) != typeof(null))
          {
            while (aStack.length > j)
            {
              tokens[i] += kn_htmlsafe_closeElement(aStack);
            }
          }
        }
        aStack[aStack.length] = tag;
        if (tag == 'a')
        {
          token = token.replace(/([ \r\n])/i,
                  ' target="' +
                      $_(kn_defaults_get("kn_htmlsafe",
                               "LinkTarget",
                               'ChatLink${0}'),
                       Math.random().toString().replace('.','_')) +
                      '"$1');
        }
        else if (tag == 'font')
        {
          // "FONT" open tags are passed verbatim to preserve attributes.
        }
        else
        {
          token = kn_htmlsafe_htmlOpenTag(tag);
        }
      }
      tokens[i] += token;
    }
    else if (kn_htmlsafe_isHtmlCloseTag(tokens[i]))
    {
      var tag = kn_htmlsafe_htmlTagName(tokens[i]);
      tokens[i] = '';
      while (aStack.length)
      {
        var topTag = aStack[aStack.length - 1];

        tokens[i] += kn_htmlsafe_closeElement(aStack);
        if (topTag == tag)
        {
          break;
        }
      }
    }
    else
    {
      tokens[i] = tokens[i].
      replace(/\r\n|\n\r/g, "&" + "nbsp;<br />&" + "nbsp;").
      replace(/[\r\n]/g, "&" + "nbsp;<br />&" + "nbsp;");
    }
  }
  
  while (aStack.length)
  {
    tokens[tokens.length] = kn_htmlsafe_closeElement(aStack);
  }
  
  return tokens.join('');
}

// Is the given character code a dangerous or invalid one?

function kn_htmlsafe_isEvilCharCode(code)
{
  // C0
  if (code < 32)
    return true;
  // DEL or C1
  if ((code > 126) && (code < 160))
    return true;
  // Surrogates
  if ((code & ~0x7FF) == 0xD800)
    return true;
  // Private Use Area
  if ((code >= 0xE000) && (code <= 0xF8FF))
    return true;
  // Specials
  if ((code >= 0xFFF0) && (code <= 0xFFFD))
    return true;
  // ..FFFE / ..FFFF
  if ((code & 0xFFFE) == 0xFFFE)
    return true;
  // High Private Use Area
  if ((code >= 0xF0000) && (code <= 0x10FFFF))
    return true;
  return false;
}

// Characters we have abbreviations for:

kn_htmlsafe_visible_abbrevs = {
  'U+0000': 'NUL', 'U+0001': 'SOH', 'U+0002': 'STX', 'U+0003': 'ETX',
  'U+0004': 'EOT', 'U+0005': 'ENQ', 'U+0006': 'ACK', 'U+0007': 'BEL',
  'U+0008': 'BS', 'U+0009': 'HT', 'U+000A': 'LF', 'U+000B': 'VT',
  'U+000C': 'FF', 'U+000D': 'CR', 'U+000E': 'SO', 'U+000F': 'SI',
  'U+0010': 'DLE', 'U+0011': 'DC1', 'U+0012': 'DC2', 'U+0013': 'DC3',
  'U+0014': 'DC4', 'U+0015': 'NAK', 'U+0016': 'SYN', 'U+0017': 'ETB',
  'U+0018': 'CAN', 'U+0019': 'EM', 'U+001A': 'SUB', 'U+001B': 'ESC',
  'U+001C': 'FS', 'U+001D': 'GS', 'U+001E': 'RS', 'U+001F': 'US',
  'U+0020': 'SP',
  'U+007F': 'DEL',
  'U+0080': 'PAD', 'U+0081': 'HOP', 'U+0082': 'BPH', 'U+0083': 'NBH',
  'U+0084': 'IND', 'U+0085': 'NEL', 'U+0086': 'SSA', 'U+0087': 'ESA',
  'U+0088': 'HTS', 'U+0089': 'HTJ', 'U+008A': 'VTS', 'U+008B': 'PLD',
  'U+008C': 'PLU', 'U+008D': 'RI', 'U+008E': 'SS2', 'U+008F': 'SS3',
  'U+0090': 'DCS', 'U+0091': 'PU1', 'U+0092': 'PU2', 'U+0093': 'STS',
  'U+0094': 'CCH', 'U+0095': 'MW', 'U+0096': 'SPA', 'U+0097': 'EPA',
  'U+0098': 'SOS', 'U+0099': 'SGCI', 'U+009A': 'SCI', 'U+009B': 'CSI',
  'U+009C': 'ST', 'U+009D': 'OSC', 'U+009E': 'PM', 'U+009F': 'APC',
  'U+00A0': 'NBSP',
  'U+00AD': 'SHY',
  'U+2000': 'NQSP', 'U+2001': 'MQSP', 'U+2002': 'ENSP', 'U+2003': 'EMSP',
  'U+2004': '3/MSP', 'U+2005': '4/MSP', 'U+2006': '6/MSP', 'U+2007': 'FSP',
  'U+2008': 'PSP', 'U+2009': 'THSP', 'U+200A': 'HSP', 'U+200B': 'ZWSP',
  'U+200C': 'ZWNJ', 'U+200D': 'ZWJ', 'U+200E': 'LRM', 'U+200F': 'RLM',
  'U+2011': 'NBHY',
  'U+2028': 'LSEP', 'U+2029': 'PSEP', 'U+202A': 'LRE', 'U+202B': 'RLE',
  'U+202C': 'PDF', 'U+202D': 'LRO', 'U+202E': 'RLO', 'U+202F': 'NNBSP',
  'U+206A': 'ISS', 'U+206B': 'ASS', 'U+206C': 'IAFS', 'U+206D': 'AAFS',
  'U+206E': 'NADS', 'U+206F': 'NODS',
  'U+3000': 'IDSP',
  'U+3164': 'HF',
  'U+FFA0': 'HWHF',
  'U+FEFF': 'ZWNBSP',
  'U+FFF9': 'IAA', 'U+FFFA': 'IAS', 'U+FFFB': 'IAT', 'U+FFFC': 'OBJ',
  'U+E0001': 'LANG&' + 'gt;',
  'U+E0020': '&' + 'gt;SP&' + 'gt;',
  'U+E007F': '&' + 'gt;CAN'
};

// Generate a U[+-].... hexadecimal character code reference.

function kn_htmlsafe_uniref(code)
{
  if (! ((code >= 0) && (code <= KN.ucsMaxChar)))
  {
    return '' + code;
  }
  if (code <= KN.ucs2max)
  {
    return 'U+' + (code + 0x10000).toString(16).substring(1).toUpperCase();
  }
  return (code <= KN.utf16max ? 'U+' : 'U-') + code.toString(16).toUpperCase();
}

// visibleString = kn_htmlsafe_visibleEscape(aString)

// Produce a version of string with most special characters made
// human-visible; in DOM-capable browsers long strings have hidden
// tails which can be displayed by clicking ">>" buttons, and
// re-hidden by clicking "<<" buttons. the resulting strings may be
// placed inside hyperlinks, and have some CSS formatting for control codes.

/*!
 * Produces a version of string with most special characters made human-visible.
 * <p>
 * In DOM-capable browsers long strings have hidden
 * tails which can be displayed by clicking "&gt;&gt;" buttons, and
 * re-hidden by clicking "&lt;&lt;" buttons.
 *
 * @tparam string aString The string to be converted.
 * @treturn string The converted string.
 */
function kn_htmlsafe_visibleEscape(aString)
{
  var codes = kn_charCodesFromString(aString);
  var visible = [];
  // pre must start with '<' for SP (space) to be displayed properly.
  var pre = '<em' +
    ' style="background-color:#ffffbb;color:black;font-style:normal;text-decoration:none"' +
    '> <small>';
  var post = '<' + '/small> <' + '/em>';
  // hidepre prefixes a string with a sometimes-hidden section.
  var hidepre = '';
  // hidemid and hidepost surround a sometimes-hidden section of the string.
  var hidemid = '';
  var hidepost = '';
  // DOM compatibility check, thanks to Scott.

  // FIXME: maybe this needs the same check as used in
  // kn_browser_setStyle().

  if (document.getElementById &&
    document.body &&
    document.body.style &&
    typeof(document.body.style.display) == "string")
  {
    hidepre =
      '<span style="display: inline">';
    hidemid =
      '<span style="display: none">';
    hidepost =
      '<' + '/span>' +
      '<' + '/span>' +
      '<span style="display: none">' +
      '<button' +
      ' onclick="with(parentNode){style.display=previousSibling.lastChild.style.display=\'none\';nextSibling.style.display=\'inline\';}"' +
      '>' +
      '<small><nobr>&' + 'lt;&' + 'lt;' + '<' + '/nobr><' + '/small>' +
      '<' + '/button>' +
      '<' + '/span>' +
      '<span style="display: inline">' +
      '<button' +
      ' onclick="with(parentNode){style.display=\'none\';previousSibling.style.display=previousSibling.previousSibling.lastChild.style.display=\'inline\';}"' +
      '>' +
      '<small><nobr>&' + 'gt;&' + 'gt;' + '<' + '/nobr><' + '/small>' +
      '<' + '/button>' +
      '<' + '/span>';
  }
  if (codes.length == 0)
  {
    return pre + "(empty)" + post;
  }
  for (var i = 0; i < codes.length; i ++)
  {
    var code = codes[i];
    var picture = '';
    var ref = kn_htmlsafe_uniref(code);
    // Visible abbreviation.
    var abbrev = kn_htmlsafe_visible_abbrevs[ref];
    // Tag ASCII.
    if ((! abbrev) && (code >= 0xE0020) && (code <= 0xE007E))
      abbrev = '&' + 'gt;' +
        kn_htmlEscape(kn_stringFromCharCode(code & 0x7F)) +
        '&' + 'gt;';
    if (abbrev)
      picture = abbrev;
    if (kn_htmlsafe_isEvilCharCode(code) && ! picture)
      picture = ref;
    if (picture)
    {
      picture = pre + picture + post;
    }
    else
    {
      picture = kn_htmlEscape(kn_stringFromCharCode(code));
    }
    visible[i] = picture;
  }

  // SP (space) is treated in a very special way: a single space between
  // two non-SP, non-special characters is displayed as a regular space.
  for (var i = 1; i < (visible.length - 1); i ++)
  {
    if ((codes[i] == 0x0020) &&
      (visible[i - 1].charAt(0) != '<') &&
      (visible[i + 1].charAt(0) != '<'))
    {
      visible[i] = " ";
      i ++;
    }
  }

  // Elide the tails of long strings.
  if (visible.length >= 20)
  {
    visible[0] = hidepre + visible[0];
    visible[17] += hidemid;
    visible[visible.length - 1] += hidepost;
  }
  return visible.join('');
}

// End of kn_htmlsafe.js
