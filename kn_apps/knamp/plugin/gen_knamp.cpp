/**
 * KNAmp - v1.5
 *
 * gen_knamp.cpp
 *
 * Contains all methods for the dll.  Contains all PubSub functionality as well.
 *
 * The plug-in uses the following PubSub topic structure:
 *
 * Publish topics:
 *
 * /what/knamp/playlist - Stores the current playlist.
 * /what/knamp/songs - When a song is played, the title is published here.
 *
 * Subscribe topics:
 *
 * /what/knamp/commands - Commands sent to the plug-in from clients are sent here.
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


#include "gen_knamp.h"


/**
 * Main entry point for the .dll.  Not used for anything, so just returns true.
 */

BOOL 
APIENTRY 
DllMain(HANDLE hModule, 
    DWORD  ul_reason_for_call, 
    LPVOID lpReserved)
{
    return TRUE;
}


/** 
 * General purpose plug-in struct.  Used to define the plugin name, init,
 * config, and quit functions.  See gen.h and winamp's API documentation.
 */

winampGeneralPurposePlugin plugin = {
    GPPHDR_VER,
    "KNAmp",
    init,
    config,
    quit,
};


/**
 * Publishes the current playlist to the configured PubSub Server.
 */

void 
PublishPlayList()
{
    // PubSub Params collection for publishing an event.

    CKN_Params params;


    // Strings to store the position and title of a song on the list, for publishing.

    string playListPosition, playListTitle;


    // Buffer to store the song title we get from the winamp API.

    char *songTitle;


    // Buffer to store the playlist position we get from the winamp API.

    char listPos[1024];


    // Get the length of the playlist.

    int listLen = SendMessage(plugin.hwndParent, WM_WA_IPC, 0, IPC_GETLISTLENGTH);


    // Get the title and position for each song in the playlist.

    for(int i = 0; i < listLen; i++)
    {
        // Get the title of the song from the winamp API.  See the API docs.

        songTitle = (char *) SendMessage(plugin.hwndParent, WM_WA_IPC, i, IPC_GETPLAYLISTTITLE);
    

        // Copy playlist position to its buffer.

        sprintf(listPos, "%i", i);

        playListPosition += listPos;
        playListPosition += '\n';

        playListTitle += songTitle;
        playListTitle += '\n';
    }

    // Put the position and title in their own headers in our event to be published.

    params["position"] = playListPosition.c_str();
    params["title"] = playListTitle.c_str();


    // Publish the event.

    knWMS->publish("/what/knamp/playlist", params);
}


/**
 * Handles all the events coming in from the commands topic.
 * Parses the command and acts accordingly.
 */

void 
CommandHandler(CKN_Params params)
{
    // Stores a parameter from a collection.

    CKN_Params::iterator pos;


    // Finds the command parameter and parse command.

    if((pos = params.find("command")) != params.end())
    {
        // The value from the command parameter.

        string command = pos->value();


        // Update the selected song on winamp's playlist
        // using the position parameter and play it.

        if(command == "playfromplaylist")
        {
            CKN_Params::iterator tmpPos;

            if((tmpPos = params.find("position")) != params.end())
            {
                int playlistPos = atoi(tmpPos->value());

                // Set playlist position and play, see Winamp API docs.

                SendMessage(plugin.hwndParent, WM_WA_IPC, playlistPos, IPC_SETPLAYLISTPOS);
                SendMessage(plugin.hwndParent, WM_COMMAND, 40045, 0);
            }
        }


        // Play current selected song.

        if(command =="play")
            SendMessage(plugin.hwndParent, WM_COMMAND, 40045, 0);


        // Pause song.

        if(command == "pause")
            SendMessage(plugin.hwndParent, WM_COMMAND, 40046, 0);


        // Stop song.

        if(command == "stop")
            SendMessage(plugin.hwndParent, WM_COMMAND, 40047, 0);


        // Play previous track.

        if(command == "prevtrack")
            SendMessage(plugin.hwndParent, WM_COMMAND, 40044, 0);


        // Play next track.

        if(command == "nexttrack")
            SendMessage(plugin.hwndParent, WM_COMMAND, 40048, 0);


        // Re-publish winamp's current playlist.

        if(command == "updateplaylist")
            PublishPlayList();
    }
}


/**
 * Creates a URL to be used to connect to a PubSub Server.  For instance:
 * "http://alien:80/kn"
 *
 * @return std::string containing the created url
 */

string 
CreateURL(const char *server, 
          int port, 
          const char *path,
          const char *user,
          const char *pass)
{
    char portBuf[20];           // Buffer for storing itoa result.

    string url = "http://";     // String containing created url, start w/ "http://".

    // If there's a username, append username and pass to url.

    if(strcmp(user, "") != 0)
    {
        url.append(user);
        url.append("@");
        url.append(pass);
        url.append(":");
    }

    url.append(server);
    url.append(":");

    // Convert port to str.

    _itoa(port, portBuf, 10);

    url.append(portBuf);
    url.append(path);

    return url;
}


/**
 * We override winamp's normal WndProc function to intercept messages that say
 * when a song has been selected and is starting to play.  We set the WndProc
 * back to winamp's when finished.
 *
 * @return The result of the CallWindowProc message
 */

LRESULT 
CALLBACK 
WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    // PubSub Params collection for publishing an event.

    CKN_Params params;


    // If a song has been selected and starts playing, publish the song title.

    if((message == WM_USER) && (wParam == 666)) 
    {       
        // Get the playlist position of the current song.

        int currentIndex = SendMessage(plugin.hwndParent, WM_WA_IPC, 0, IPC_GETLISTPOS);


        // Get the song title of the current song.

        char *songTitle = (char *) SendMessage(plugin.hwndParent, WM_WA_IPC, currentIndex, IPC_GETPLAYLISTTITLE);


        // If the song is different from the one currently playing,
        // then publish the title.  We had to check for this because
        // winamp was sending multiple events for the same song, and
        // if you're trying to keep a history of songs played, that's
        // bad. :p

        if(currentSongTitle.compare(songTitle) != 0) 
        {
            currentSongTitle = songTitle;


            // Make the payload of the kn event the song title.

            params["kn_payload"] = currentSongTitle.c_str();


            // Publish to the songs topic.

            knWMS->publish("/what/knamp/songs", params);
        }
    }


    // Set winamp's WndProc back to its original one.

    return CallWindowProc((WNDPROC)wndProcOld, hwnd, message, wParam, lParam);
}


/**
 * The callback function for the config dialog, gets settings and stores
 * them in the registry.
 *
 * @return boolean whether 
 */

BOOL 
CALLBACK 
ConfigPlugin(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch(uMsg)
    {
    case WM_INITDIALOG:
        return FALSE;

    case WM_COMMAND:
        switch(LOWORD (wParam))
        {
        case IDOK:
        {
            // Close the PubSub client, we need to reconnect
            // if we're going to make changes.

            knWMS->close();


            // Buffer to store text from the dialog boxes.

            char buf[1024];


            // String to store the text from the buffer and
            // pass to the registry helped functions.

            string windowText;


            // Handle for the dialog boxes.

            HWND hEditBox;


            // Create a new KNSettings using the reg key Software\KNAmp .

            CKNSettings *knSettings = new CKNSettings("Software\\KNAmp");


            // Get server name.
                
            hEditBox = GetDlgItem(hwndDlg, IDC_SERVER);

            GetWindowText(hEditBox, buf, sizeof(buf));

            windowText = buf;

            knSettings->setServer(windowText);

            // Get server port.

            hEditBox = GetDlgItem(hwndDlg, IDC_PORT);

            GetWindowText(hEditBox, buf, sizeof(buf));

            knSettings->setPort(atoi(buf));

            // Get server path.

            hEditBox = GetDlgItem(hwndDlg, IDC_PATH);

            GetWindowText(hEditBox, buf, sizeof(buf));

            windowText = buf;

            knSettings->setPath(windowText);


            // Get username.

            hEditBox = GetDlgItem(hwndDlg, IDC_USERNAME);

            GetWindowText(hEditBox, buf, sizeof(buf));

            windowText = buf;

            knSettings->setUser(windowText);


            // Get password.

            hEditBox = GetDlgItem(hwndDlg, IDC_PASSWORD);

            GetWindowText(hEditBox, buf, sizeof(buf));

            windowText = buf;

            knSettings->setPass(windowText);


            // Open PubSub client with these new settings.

            string url = CreateURL(knSettings->getServer(),
                                   knSettings->getPort(),
                                   knSettings->getPath(),
                                   knSettings->getUser(),
                                   knSettings->getPass());

            knWMS->open(url.c_str());


            // Re-publish the playlist to the new PubSub Server.

            PublishPlayList();


            // Close dialog box.

            EndDialog(hwndDlg, TRUE);


            // Clean up settings object.

            delete knSettings;

            return TRUE;
        }

        case IDCANCEL:
            EndDialog(hwndDlg, TRUE);
            return TRUE;
        }

    }

    return FALSE;
}


int 
init()
{
    // Get a KNSettings instance using the reg key Software\KNAmp .

    CKNSettings *knSettings = new CKNSettings("Software\\KNAmp");


    // Get an instance of the Windows PubSub client.

    knWMS = GetKN_MServer();


    // Set the callback function for subscriptions.

    knWMS->set_event_callback(&CommandHandler);


    // If the settings don't have a server, port, or path set,
    // run the config dialog, otherwise connect to a PubSub Server
    // w/ the settings from the registry.

    if(knSettings->getServer() == "" || knSettings->getPort() == 0 || knSettings->getPath() == "")
        config();
    else
    {
        string url = CreateURL(knSettings->getServer(),
                                knSettings->getPort(),
                                knSettings->getPath(),
                                knSettings->getUser(),
                                knSettings->getPass());

        knWMS->open(url.c_str());
    }

    // Clean up.

    delete knSettings;


    // Get the winamp handle.

    winampWnd = FindWindow("Winamp PE", NULL);


    // Get a handle to winamp's original WndProc.

    wndProcOld = (void *) GetWindowLong(winampWnd, GWL_WNDPROC);


    // Set the WndProc to our WndProc for message intercepting purposes.

    SetWindowLong(winampWnd, GWL_WNDPROC, (long) WndProc);


    // Publish the playlist.

    PublishPlayList();


    // PubSub parameters collection for subscribing.

    CKN_Params params;


    // Subscribe to the commands topic.

    knWMS->subscribe("/what/knamp/commands", params);

    return 0;
}


void 
config()
{
    // Creates the config dialog box w/ ConfigPlugin as the handler method.

    DialogBox(plugin.hDllInstance, MAKEINTRESOURCE(IDD_CONFIG), plugin.hwndParent, ConfigPlugin);
}


void
quit()
{
    // Close the connection to the PubSub and the win32 client.

    knWMS->close();


    // Make sure winamp gets its old WndProc back.

    SetWindowLong(winampWnd, GWL_WNDPROC, (long) wndProcOld);
}


extern "C" __declspec(dllexport) 
winampGeneralPurposePlugin * winampGetGeneralPurposePlugin() {
    return &plugin;
}

