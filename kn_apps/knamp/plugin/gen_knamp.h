/**
 * KNAmp - v1.5
 *
 * gen_knamp.h
 *
 * Contains public methods for the dll and the global variables.
 */


/*
Copyright 2000-2004 KnowNow, Inc.  All rights reserved.

@KNOWNOW_LICENSE_START@

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:

1. Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.

3. Neither the name of the KnowNow, Inc., nor the names of its
contributors may be used to endorse or promote products derived from
this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

@KNOWNOW_LICENSE_END@

*/


#include <windows.h>
#include <winuser.h>
#include <winbase.h>
#include <process.h>
#include <string>


// PubSub includes

#include <kn_dll.h>
#include "KNSettings.h"


// Winamp API includes

#include "gen.h"
#include "frontend.h"
#include "resource.h"

using namespace std;


/**
 * Called when the plugin is initiated.  Gets router settings, connects to a
 * PubSub Server, publishes the playlist, subscribes to the commands topic.
 *
 * Required initalization function for the Winamp API.
 *
 * @return 0, not used, but needs to return an int as defined by the API.
 */

int init();


/**
 * Pops up the config dialog, required config function for the Winamp API.
 */

void config();


/**
 * Called when the plugin closes, required quit function for the Winamp API.
 */

void quit();


/**
 * Pointer to Winamp's WndProc function.
 */

void *wndProcOld;


/**
 * Handle for the Winamp window.
 */

HWND winampWnd;


/**
 * Pointer to a PubSub win32 client.
 */

CKN_MServer* knWMS;


/**
 * String to hold the currently playing song title.
 */

string currentSongTitle = "";
