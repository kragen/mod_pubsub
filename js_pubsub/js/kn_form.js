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

// $Id: kn_form.js,v 1.2 2004/04/19 05:39:10 bsittler Exp $

// To make this form 2-way, uncomment the following line:
// window.kn__form2way = true;

// Note that we put this line in pubsub.cgi so you don't have to here.

// FIXME: 2-way forms doesn't work entirely correctly in Mozilla and Galeon.

function kn__formOnMessage(e) {
    for (var i=0; i < this.elements.length; i++)
        this.elements[i].value =
            (e[this.elements[i].name]) ?
            e[this.elements[i].name]  :
        this.elements[i].value = "";
    // 2-way forms don't support onsubmit handlers yet.
    if (! kn__form2way)
    {
        setTimeout('document.forms['+this.kn_index+'].onsubmit();',1);
    }
}

function kn__formInit() 
{
    if (watchdog) 
    {
        clearInterval(watchdog);
        watchdog=null;
    }
    for (var i=0; i < document.forms.length; i++) 
    {
        f = document.forms[i];
        a = f.action; 
        if (a) 
        {
            // We are looking for kn:topic URLs.
            // In Netscape, though, they prepend the base URL to
			// anything that moves (er, that doesn't begin with "http").

            var prefix = "kn:";
            var l = document.location;
            var lprefix = l.protocol + "//" + l.host + 
                kn_resolvePath(l.pathname, prefix);
            if (a.substring(0,lprefix.length).toLowerCase() 
                == lprefix.toLowerCase())
            {
                prefix = lprefix;
            }
            if (a.substring(0,prefix.length).toLowerCase() 
                == prefix.toLowerCase())
            {
                var topic = a.substring(prefix.length);

                // Reduces confusion upon erroneous user onSubmit handlers.

                f.action = "javascript:void 0 //";

				// GET indicates a listener.

                if (f.method.toLowerCase() == "get")
                {
                    f.onMessage=kn__formOnMessage;
                    f.kn_index=i;
                    kn.subscribe(topic, f,
                                 kn__form2way ? ({ do_max_n: 1 }) : (void 0));
                }

                // POST indicates a speaker.

                if ((f.method.toLowerCase() != "get") || kn__form2way)
                {
                    if (kn__form2way)
                    {
                        f.onsubmit = function ()
                        {
                            kn_publishForm(topic, this);
                            return false;
                        };
                    }
                    else
                    {
                        f.useronsubmit = f.onsubmit;
                        f.onsubmit = function () 
                        {
                            window.errorFlag=false;
                            var oldOE = onerror;
                            onerror = kn__formOnError;
                            if ((this.useronsubmit() != false) && !window.errorFlag)
                            kn_publishForm(topic,this);
                            onerror = oldOE;
                            return false;
                        };

                        // The logic above does not quite work: if
						// useronsubmit() fails, our onsubmit aborts,
						// returning *true* (which submits the form).
                    }
                }
            }
        }
    }
}

function kn__formOnError (m,u,l) 
{
    window.errorFlag=true; 
    alert("The original onSubmit() handler of this form reported this error:\n"
          +m+"\nURL: "+u+"\nLine:"+l); 
    return true;
}

function kn__formWatchdog()
{
    // We've been replaced by user code, so run kn__formInit() ourselves.
    if (window.onload != kn__formInit) {
        clearInterval(watchdog);
        watchdog=null;
        setTimeout("kn__formInit()",500);
    }
}

// Patching into the onLoad loop to trap kn:/topic FORMs
// begins with the same conditional as kn__formInitMicroserver().

if (parent && (parent.kn != (void 0)) && (parent.kn != null)
    && (parent != window)) {
    window.onload = kn__formInit;
    kn__form2way = window.kn__form2way;
    var watchdog = setInterval("kn__formWatchdog()",10);
}

// End of kn_form.js
