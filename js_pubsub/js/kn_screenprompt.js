/*! @file kn_screenprompt.js PubSub Screen Prompt Component
 * <pre>
 * &lt;script src="../../js/kn_config.js" type="text/javascript"&gt;&lt;/script&gt;
 * &lt;script src="../../js/kn_browser.js" type="text/javascript"&gt;&lt;/script&gt;
 * <b>&lt;script type="text/javascript"&gt;
 * kn_include("kn_screenprompt");
 * &lt;/script&gt;</b>
 * </pre>
 */

// Copyright 2003 KnowNow, Inc.

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

// $Id: kn_screenprompt.js,v 1.1 2003/07/20 07:09:16 ifindkarma Exp $

/*

Screenprompt state transitions:

-> INIT

INIT - service starting, no message visible
onViewChange -> DISPLAY
onServiceStatus -> DISPLAY

DISPLAY - viewing a message/sendStatus(1), or empty/sendStatus(0)
onViewChange -> DELAY
onChoice -> CHOICE
onConfirm -> CONFIRM

DELAY - delay timer started, no message visible
onTimeout -> DISPLAY

CHOICE - choice started, inputs disabled
onChoiceStatus -> CONFIRM

CONFIRM - confirmation started, inputs disabled
onConfirmationStatus -> DISPLAY

*/

kn_include("kn_activepanels");
kn_include("kn_htmlsafe");
kn_include("kn_defaults");
kn_include("kn_lwws.Service");

kn_createContext("kn_screenprompt");

kn_screenprompt_collection = new Object();

/*!
 * Creates the HTML needed to render the Screen Prompt component.
 *
 * @tparam string anID a unique identifier for this Screen Prompt
 * @tparam int aWidth the desired width of the Screen Prompt, in pixels
 * @tparam int aHeight the desired height of the Screen Prompt, in pixels
 * @tparam string aTopic Mail Box for this Screen Prompt component (default: "/who/" + kn.userid + "/alerts")
 * @tparam statusHandlers aStatusCallback aStatusCallback(anEvent) will
 * @tparam int aDelayLength Delay time for displaying a message update, in ms (default: 1000)
 * @treturn kn_screenprompt_obj an object reference to the
 * newly-created Screen Prompt component
 */
function kn_screenprompt(anID, aWidth, aHeight, aTopic, aStatusCallback, aDelayLength)
{
  var aPanel =
      new kn_activepanel(
          anID, aWidth, aHeight, true);
  var aScreenPromptObj =
      new kn_screenprompt_obj(
          anID, aWidth, aHeight, aPanel, aTopic, aStatusCallback, aDelayLength);
  kn_screenprompt_collection[anID] =
      aScreenPromptObj;
  aScreenPromptObj.init();
  return aScreenPromptObj;
}

/*! @class kn_screenprompt_obj
 * This object provides the PubSub Screen Prompt interface.<p>This
 * constructor should never be called directly from application
 * code. Use kn_screenprompt() instead to create an instance of
 * the Screen Prompt component.
 * @ctor An object reference that provides the PubSub Screen Prompt API.
 * @tparam string anID a unique identifier for this Screen Prompt
 * @tparam int aWidth the desired width of the Screen Prompt, in pixels
 * @tparam int aHeight the desired height of the Screen Prompt, in pixels
 * @tparam kn_activepanel_obj aPanel an ActivePanel to serve as the rendering component
 * @tparam string aTopic Mail Box for this Screen Prompt component (default: "/who/" + kn.userid + "/alerts")
 * @tparam statusHandlers aStatusCallback aStatusCallback(anEvent) will
 * @tparam int aDelayLength Delay time for displaying a message update, in ms (default: 1000)
 * be invoked each time the display switches to a new event
 * (anEvent); anEvent will be undefined if the last message has
 * been deleted (default: no callback)
 */
function kn_screenprompt_obj(anID, aWidth, aHeight, aPanel, aTopic, aStatusCallback, aDelayLength)
{
    if (aTopic == null) aTopic = "/who/" + kn.userid + "/alerts";
    if (! aStatusCallback) aStatusCallback = function(e){};
    if (! aDelayLength) aDelayLength = 1000;
    this.id = anID;
    this.width = aWidth;
    this.height = aHeight;
    this.panel = aPanel;
    this.topic = aTopic;
    this.statusCallback = aStatusCallback;
    this.delayLength = aDelayLength;
    this.viewing = null;
    this.messages = new Object;

    // superclass constructor
    this.Super = kn_lwws.Service;
    this.Super(self.kn, aTopic);
}

// Implements kn_lwws.Service.

kn_screenprompt_obj.prototype = new kn_lwws_Service();

kn_screenprompt_obj.prototype.draw = function(anEvent)
{
    this.panel.clear();
    this.panel.write('<div class="screenprompt" align="center">');
    if (anEvent)
    {
        this.panel.write('<h1>');
        this.panel.write(kn_htmlEscape(anEvent.kn_payload));
        this.panel.write('</h1>');

        if (anEvent.choices)
        {
            this.panel.write('<form' +
                             ' name="' +
                             this.id +
                             '_choices"' +
                             ' action="javascript:void(0)//">\n');
            var choices = anEvent.choices.split(",");
            for (var i = 0; i < choices.length; i ++)
            {
                var choice = choices[i];
                var choiceHeader = "choice_" + choice;
                var choiceTitle = anEvent[choiceHeader] || choice;
                this.panel.write(
                    '<input' +
                    ' type="button"' +
                    ' value="' +
                    kn_htmlEscape(choiceTitle) +
                    '"' +
                    ' onclick="kn_screenprompt_collection[\'' +
                    this.id +
                    '\'].onChoice(kn_unescape(\'' +
                    kn_htmlEscape(
                        kn_escape(choice)) +
                    '\'), document);' +
                    'return false;"' +
                    ' />\n');
            }
            this.panel.write('<' + '/form>\n');
        }
        this.panel.write('<form' +
                         ' name="' +
                         this.id +
                         '_confirm"' +
                         ' action="javascript:void(0)//">\n');
        this.panel.write(
            '<input' +
            ' type="button"' +
            ' value="' +
            kn_htmlEscape(
                kn_screenprompt$("Confirm")) +
            '"' +
            ' onclick="kn_screenprompt_collection[\'' +
            this.id +
            '\'].onConfirm(document);' +
            'return false;"' +
            ' />\n');
        this.panel.write('<' + '/form>\n');
    }
    else
    {
        this.panel.write('');
    }
    this.panel.write('</div>');
    this.panel.update();
    
    if (navigator.userAgent.indexOf("Gecko") != -1)
    {
        self.setTimeout("kn_screenprompt_collection['" + this.id + "'].NS62Render();",500);
    }
    
}

kn_screenprompt_obj.prototype.NS62Render = function()
{
    // Especially for NS6.2.
    var p = kn_activepanel_getObj(this.panel.id);
    p.style.width = (parseInt(p.style.width) + 1) + "px";
    p.style.width = (parseInt(p.style.width) - 1) + "px";
    top.resizeBy(0,-1);
    top.resizeBy(0,1);
}

kn_screenprompt_obj.prototype.disableInputs = function(aDocument)
{
    return;  // FIXME?

    if (aDocument && aDocument.forms)
    {
        var aFormList = new Array;
        var aChoicesForm = aDocument.forms[this.id + "_choices"];
        if (aChoicesForm)
        {
            aFormList[aFormList.length] =
                aChoicesForm;
        }
        var aConfirmForm = aDocument.forms[this.id + "_confirm"];
        if (aConfirmForm)
        {
            aFormList[aFormList.length] =
                aConfirmForm;
        }
        for (var aFormIndex = 0;
             aFormIndex < aFormList.length;
             aFormIndex ++)
        {
            var aForm = aFormList[aFormIndex];
            if (aForm.elements)
            {
                for (var anElementIndex = 0;
                     anElementIndex < aForm.elements.length;
                     anElementIndex ++)
                {
                    var anElement = aForm.elements[anElementIndex];
                    anElement.disabled = true;
                }
            }
        }
    }
}

kn_screenprompt_obj.prototype.onChoice = function(aChoice, aDocument)
{
    if (this.state == this.display)
    {
        this.disableInputs(aDocument);
        this.choice(aChoice);
    }
}

kn_screenprompt_obj.prototype.onConfirm = function(aDocument)
{
    if (this.state == this.display)
    {
        this.disableInputs(aDocument);
        this.confirm();
    }
}

// init() - service starting, no message visible.
kn_screenprompt_obj.prototype.init = function()
{
    this.state = this.init;
    var anOptions = new Object;
    anOptions.do_max_age = "infinity";
    anOptions.kn_deletions = "true";
    var aFinisher = new kn_browser_wrapper(this, "onServiceStatus");
    aFinisher.onStatus = aFinisher.onMessage;
    this.start(
        anOptions,
        aFinisher);
}

kn_screenprompt_obj.prototype.onServiceStatus = function(anEvent)
{
    if (this.state == this.init)
    {
        this.display();
    }
}

kn_screenprompt_obj.prototype.onMessage = function(anEvent)
{
    // FIXME: Someday also care about kn_time_t.
    if (!anEvent.kn_payload || anEvent.kn_deleted == "true")
    {
        if (this.messages[anEvent.kn_id])
        {
            delete this.messages[anEvent.kn_id];
            if (this.viewing == anEvent.kn_id)
            {
                this.viewing = null;
                for (var aKey in this.messages)
                {
                    this.viewing = aKey;
                    break;
                }
                this.onViewChange();
            }
        }
    }
    else
    {
        this.messages[anEvent.kn_id] = anEvent;
        if (this.viewing == null || this.viewing == anEvent.kn_id)
        {
            this.viewing = anEvent.kn_id;
            this.onViewChange();
        }
    }
}

kn_screenprompt_obj.prototype.onViewChange = function()
{
    if (this.state == this.init)
    {
        this.display();
    }
    else if (this.state == this.display)
    {
        this.delay();
    }
}

// display() - Viewing a message/statusCallback(1), or empty/statusCallback(0).
kn_screenprompt_obj.prototype.display = function()
{
    this.state = this.display;
    if (this.viewing != null)
    {
        this.statusCallback(this.messages[this.viewing]);
        this.draw(this.messages[this.viewing]);
    }
    else
    {
        this.statusCallback();
        this.draw();
    }
}

// delay - Delay timer started, no message visible.
kn_screenprompt_obj.prototype.delay = function()
{
    this.state = this.delay;
    this.draw();
    setTimeout(
        'kn_screenprompt_collection[\'' +
        this.id +
        '\'].onTimeout()',
        this.delayLength);
}

// choice(aChoice) - Choice started, inputs disabled.
kn_screenprompt_obj.prototype.choice = function(aChoice)
{
    this.state = this.choice;
    var aReply = new Object;
    aReply.kn_payload = aChoice;
    var aFinisher = new kn_browser_wrapper(this, "onChoiceStatus");
    aFinisher.onStatus = aFinisher.onMessage;
    this.reply(this.messages[this.viewing],
               aReply,
               aFinisher);
}

kn_screenprompt_obj.prototype.onChoiceStatus = function(anEvent)
{
    if (this.state == this.choice)
    {
        this.confirm();
    }
}

// confirm - Confirmation started, inputs disabled.
kn_screenprompt_obj.prototype.confirm = function()
{
    this.state = this.confirm;
    var anEvent = new Object();
    anEvent.kn_id = this.viewing;
    anEvent.kn_payload = "";
    anEvent.kn_expires = "+255";
    var aFinisher = new kn_browser_wrapper(this, "onConfirmationStatus");
    aFinisher.onStatus = aFinisher.onMessage;
    kn.publish(this.topic,
               anEvent,
               aFinisher,
               aFinisher);
}

kn_screenprompt_obj.prototype.onConfirmationStatus = function()
{
    if (this.state == this.confirm)
    {
        this.display();
    }
}

kn_screenprompt_obj.prototype.onTimeout = function()
{
    if (this.state == this.delay)
    {
        this.display();
    }
}

// End of kn_screenprompt.js
