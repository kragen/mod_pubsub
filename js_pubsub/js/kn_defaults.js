/*! @file kn_defaults.js PubSub Defaults Component
 * <pre>
 * &lt;script src="../../js/kn_config.js" type="text/javascript"&gt;&lt;/script&gt;
 * &lt;script src="../../js/kn_browser.js" type="text/javascript"&gt;&lt;/script&gt;
 * <b>&lt;script type="text/javascript"&gt;
 * kn_include("kn_defaults");
 * &lt;/script&gt;</b>
 * </pre>
 */

// Copyright 2003-2004 KnowNow, Inc.  All rights reserved.

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

// $Id: kn_defaults.js,v 1.2 2004/04/19 05:39:10 bsittler Exp $

// aValue = kn_defaults_get(aSection, aKey, aDefaultValue) - search
// URL arguments and 'self' window properties for a configuration
// string and return its value (aValue); if present as a window
// property, the string is expected to be UTF-8 encoded. The URL
// parameter or window property name is constructed by appending the
// key part (aKey) to the section part (aSection.) By convention the
// section part is in "kn_lowerCamelCase" (that is, a maker prefix
// ['kn' for KnowNow, 'fooCorp' for Foo Corporation, etc.], an
// underscore '_', and a "lowerCamelCase) and the key part is in
// "CapitalCamelCase", so that appending the two will produce
// "kn_lowerCamelCaseButLonger". Returns a default value
// (aDefaultValue) if no configuration string is found.

// An example may serve to illustrate this better: suppose that the
// Anti-Gravity widget (antiGravity) from Meson, Inc. (mesonInc) has a
// parameter controlling gravitational acceleration at sea level
// (Acceleration,) with a default value of 9.8 m/s\u00B2 [that's a
// superscript 2 (Unicode U+00B2, UTF-8 sequence C2 B2) written as a
// Unicode character escape -- keep in mind that kn_defaults_get works
// with string values.] To allow this parameter to be customized, the
// widget would use

// var acceleration =
//   kn_defaults_get(
//     "mesonInc_antiGravity",
//     "Acceleration",
//     kn_utf8decode("9.8 m/s\xC2\xB2")
//    );

// Note that the default value is written in UTF-8 for cross-browser
// portability (\u00B2-style Unicode character escapes don't work in
// some older browsers e.g. Netscape 4.x.) Now to customize
// acceleration (say to work on Mars, where gravitational acceleration
// at the surface is 3.7 m/s\u00B2,) an application might use a 'self'
// window property:

// self.mesonInc_antiGravityAcceleration =
//   kn_utf8decode("3.7 m/s\xC2\xB2");

// Now suppose a particular instance of the application needed to be
// customized for Earth's moon, where gravitational acceleration at
// the surface is only 1.61 m/s\u00B2, without modifying the
// application code. To do this, one could simply add a URL parameter,
// as follows:

// ...?mesonInc_antiGravityAcceleration=1.61%20m%2Fs%C2%B2

// No changes to the widget or application would be needed. Note that
// URL parameters parsed by the JavaScript PubSub Library are always
// URL-encoded UTF-8, as produced by, for example, kn_escape("1.61 m/s\u00B2").

/*!
 * Creates the HTML needed to render the ActivePanel component.
 *
 * @tparam string aSection The library associated with this default
 * @tparam string aKey The name of the default
 * @tparam string aDefaultValue A default value to be returned if no other value is determined.
 * @treturn string The determined value.
 */
function kn_defaults_get(aSection, aKey, aDefaultValue)
{
  // 0. Append the key (aKey) to the section (aSection) to construct
  //    the parameter name.

  var knparam = aSection + aKey;

  // 1. Default to the value from the third parameter (aDefaultValue)

  var value = aDefaultValue;

  // 2. Allow a 'self' window property to override 1.

  if (self[knparam] != null)
  {
      value = kn_utf8decode(self[knparam]);
  }

  // 3. Allow a URL parameter to override 1. and 2.

  if (this.kn_argv && this.kn_argv[knparam] != null)
  {
      value = kn_argv[knparam];
  }

  return value;
}

// End of kn_defaults.js
