/**
 * KNSettings - v1.5
 *
 * KNSettings.h
 *
 * Interface for the PubSub Settings registry helper.  This object can store
 * settings for a PubSub Server in both memory and the Windows registry.
 */


/*
Copyright (c) 2000-2003 KnowNow, Inc.  All rights reserved.

@KNOWNOW_LICENSE_START@

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:

1. Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in
the documentation and/or other materials provided with the
distribution.

3. The name "KnowNow" is a trademark of KnowNow, Inc. and may not
be used to endorse or promote any product without prior written
permission from KnowNow, Inc.

THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESSED OR IMPLIED
WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL KNOWNOW, INC. OR ITS CONTRIBUTORS BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

@KNOWNOW_LICENSE_END@
*/


#include <windows.h>
#include <string>

using namespace std;

class CKNSettings  
{
public:

    /**
     * CTOR
     *
     * @param keyName LPCTSTR containing the key in the registry to use
     */

    CKNSettings(LPCTSTR keyName);


    /**
     * DTOR
     */

    virtual ~CKNSettings();



    ////////////////////////////////////////
    // Get Methods
    //

    /**
     * @return Server's hostname
     */

    const char*     getServer();


    /**
     * @return Path to the router (i.e. "/kn")
     */

    const char*     getPath();


    /**
     * @return Username to login to router with
     */

    const char*     getUser();


    /**
     * @return Password to login to router with
     */

    const char*     getPass();


    /**
     * @return Port router listens on
     */

    int             getPort();



    ////////////////////////////////////////
    // Set Methods
    //      
    
    /**
     * @param strServer String containing server hostname
     * @return boolean whether setting the hostname in the registry worked or not
     */

    bool    setServer(string strServer);


    /**
     * @param strServer String containing server hostname
     * @return boolean whether setting the hostname in the registry worked or not
     */

    bool    setPath(string strPath);


    /**
     * @param strServer String containing server hostname
     * @return boolean whether setting the hostname in the registry worked or not
     */

    bool    setUser(string strUser);


    /**
     * @param strServer String containing server hostname
     * @return boolean whether setting the hostname in the registry worked or not
     */
    
    bool    setPass(string strPass);


    /**
     * @param strServer String containing server hostname
     * @return boolean whether setting the hostname in the registry worked or not
     */

    bool    setPort(int iPort);


protected:

    /**
     * Strings to store the server hostname, router path, router username, and router password
     */

    string  m_strServer,
            m_strPath,
            m_strUser,
            m_strPass;

    /**
     * Int to store the router's port
     */

    int         m_iPort;


    /**
     * Boolean for determining whether an instance of KNSettings should use the registry or not
     */

    bool    m_bUseRegistry;


    /**
     * Handle to the registry key
     */

    HKEY    m_hKey;


    /**
     * LPCTSTR containing the registry key's name
     */

    LPCTSTR m_lpKeyName;


    /**
     * Opens the key associated w/ the instance of KNSettings.
     *
     * @param bForReading Boolean stating whether the key is for reading or writing
     *
     * @return Boolean whether method succeeded or not
     */

    bool OpenKey(bool bForReading);


    /**
     * Closes the key associated w/ the instance of KNSettings.
     *
     * @return Boolean whether method succeeded or not
     */

    bool CloseKey();


    /**
     * Gets a text value from a sub key
     *
     * @param lpSubKey LPCTSTR containing the name of the sub key
     * @param str String to store the results in
     *
     * @return Boolean whether method succeeded or not
     */

    bool GetRegValue(LPCTSTR lpSubKey, string &str);


    /**
     * Gets a numeric value from a sub key
     *
     * @param lpSubKey LPCTSTR containing the name of the sub key
     * @param num DWORD to store result in
     *
     * @return Boolean whether method succeeded or not
     */

    bool GetRegValueNum(LPCTSTR lpSubKey, DWORD& num);
    

    /**
     * Gets all the settings from the registry and stores them in their corresponding 
     * vars.
     *
     * @return Boolean whether method succeeded or not
     */

    bool GetSettingsFromRegistry();


    /**
     * Sets a text value for a sub key
     *
     * @param lpSubKey LPCTSTR containing the name of the sub key
     * @param str String with the value to put in the sub key
     *
     * @return Boolean whether method succeeded or not
     */

    bool SetRegValue(LPCTSTR lpSubKey, string &str);


    /**
     * Sets a numeric value for a sub key
     *
     * @param lpSubKey LPCTSTR containing the name of the sub key
     * @param num SWORD with the value to put in the sub key
     *
     * @return Boolean whether method succeeded or not
     */

    bool SetRegValueNum(LPCTSTR lpSubKey, DWORD num);
};
