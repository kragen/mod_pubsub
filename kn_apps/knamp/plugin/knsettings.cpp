/**
 * KNSettings - v1.5
 *
 * KNSettings.cpp
 *
 * Implementation for the PubSub Settings registry helper.  This object can
 * store settings for a PubSub Server in both memory and the Windows registry.
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



#include "KNSettings.h"


CKNSettings::CKNSettings(LPCTSTR keyName)
{
    // we're using the registry

    m_bUseRegistry = true;


    // set reg key's name to passed string

    m_lpKeyName = keyName;


    // Get the settings from the reg key, if it succeeds, return

    if(GetSettingsFromRegistry())
        return;


    // If not, make these the settings

    m_strServer = "";
    m_strPath = "";
    m_strUser = "";
    m_strPass = "";
    m_iPort = 0;
}


CKNSettings::~CKNSettings()
{

}


const char* 
CKNSettings::getServer()
{
    return m_strServer.c_str();
}


const char* 
CKNSettings::getPath()
{
    return m_strPath.c_str();
}


const char* 
CKNSettings::getUser()
{
    return m_strUser.c_str();
}


const char* 
CKNSettings::getPass()
{
    return m_strPass.c_str();
}


int 
CKNSettings::getPort()
{
    return m_iPort;
}


bool 
CKNSettings::setServer(string strServer)
{
    bool bReturn = true;

    m_strServer = strServer;
    
    // If we're using the registry, write the passed string to the Server subkey

    if(m_bUseRegistry) 
    {
        // If the key is open, close it.

        if(m_hKey)
            CloseKey();

        // Opening the key for writing, so we pass false for the ForRead value.

        if(OpenKey(false)) 
        {
            bReturn = SetRegValue("Server", strServer);
    
            CloseKey();
        }
    }

    return bReturn;
}


bool 
CKNSettings::setPath(string strPath)
{
    bool bReturn = true;

    m_strPath = strPath;
    
    // If we're using the registry, write the passed string to the Path subkey

    if(m_bUseRegistry) 
    {

        // If the key is open, close it.

        if(m_hKey)
            CloseKey();

        // Opening the key for writing, so we pass false for the ForRead value.

        if(OpenKey(false)) 
        {
            bReturn = SetRegValue("Path", strPath);
    
            CloseKey();
        }
    }

    return bReturn;
}


bool
CKNSettings::setUser(string strUser)
{
    bool bReturn = true;

    m_strUser = strUser;
    
    // If we're using the registry, write the passed string to the User subkey

    if(m_bUseRegistry) 
    {

        // If the key is open, close it.

        if(m_hKey)
            CloseKey();

        // Opening the key for writing, so we pass false for the ForRead value.

        if(OpenKey(false)) 
        {
            bReturn = SetRegValue("User", strUser);
    
            CloseKey();
        }
    }

    return bReturn;
}


bool 
CKNSettings::setPass(string strPass)
{
    bool bReturn = true;

    m_strPass = strPass;
    
    // If we're using the registry, write the passed string to the Pass subkey

    if(m_bUseRegistry) {

        // If the key is open, close it.

        if(m_hKey)
            CloseKey();

        // Opening the key for writing, so we pass false for the ForRead value.

        if(OpenKey(false)) {
            bReturn = SetRegValue("Pass", strPass);
    
            CloseKey();
        }
    }

    return bReturn;
}


bool 
CKNSettings::setPort(int iPort)
{
    bool bReturn = true;

    m_iPort = iPort;
    
    // If we're using the registry, write the passed int to the Port subkey

    if(m_bUseRegistry) {

        // If the key is open, close it.

        if(m_hKey)
            CloseKey();

        // Opening the key for writing, so we pass false for the ForRead value.

        if(OpenKey(false)) {
            bReturn = SetRegValueNum("Port", iPort);
    
            CloseKey();
        }
    }

    return bReturn;
}



bool 
CKNSettings::OpenKey(bool bForReading)
{
    // If reg key is open, close it

    if(m_hKey)
        CloseKey();


    // Stores result of reg operations

    LONG result;


    // If the key is for reading, open it w/ read perms, otherwise open w/ 
    // write perms, either way store key in it's handle

    if(bForReading) 
    {
        result = RegOpenKeyEx(HKEY_CURRENT_USER,
                              m_lpKeyName,
                              0,
                              KEY_QUERY_VALUE,
                              &m_hKey);
    } 
    else 
    {
        result = RegOpenKeyEx(HKEY_CURRENT_USER,
                              m_lpKeyName,
                              0,
                              KEY_WRITE,
                              &m_hKey);
    }


    // If opening didn't succeed, create the key

    if(result != ERROR_SUCCESS) 
    {
        DWORD disp;

        result = RegCreateKeyEx(HKEY_CURRENT_USER,
                                m_lpKeyName,
                                0,
                                NULL,
                                REG_OPTION_NON_VOLATILE,
                                KEY_WRITE,
                                NULL,
                                &m_hKey,
                                &disp);

        // If creating didn't succed, set handle to null and return false

        if(result != ERROR_SUCCESS) 
        {
            m_hKey = NULL;
            return false;
        }


        // Otherwise set settings to default.

        string str = "";

        setServer(str);
        setPort(0);
        setPath(str);
        setUser(str);
        setPass(str);
    }

    return true;
}


bool 
CKNSettings::CloseKey()
{
    // If a handle to a key esists, close it

    if(m_hKey) 
    {
        LONG result = RegCloseKey(m_hKey);

        // If close didn't succeed, null out the handle and return false

        if(result != ERROR_SUCCESS) 
        {
            m_hKey = NULL;

            return false;
        }
    }

    return true;
}


bool 
CKNSettings::GetRegValue(LPCTSTR lpSubKey, string &str)
{
    // If handle to the key doesn't exist, return false

    if(!m_hKey)
        return false;


    // Stores type of the retrieved value

    DWORD type;


    // Length of the buffer to store the value in

    DWORD dwBufLen = 257;

    // Buffer to store value in

    LPBYTE lpBuf = new BYTE[dwBufLen];


    // Get value in sub key, store it in buffer

    LONG result = RegQueryValueEx(m_hKey,
                                  lpSubKey,
                                  NULL,
                                  &type,
                                  lpBuf,
                                  &dwBufLen);


    // If the query is successful and value is a string, store result
    // in passed string, delete the buffer, and return true

    if(result == ERROR_SUCCESS && type == REG_SZ) 
    {
            str = (char *)lpBuf;

            delete[] lpBuf;

            return true;
    }


    // Clean up buffer

    delete[] lpBuf;

    return false;   
}


bool 
CKNSettings::GetRegValueNum(LPCTSTR lpSubKey, DWORD& num)
{
    // If handle to the key doesn't exist, return false

    if(!m_hKey)
        return false;


    // Stores type of the retrieved value

    DWORD type;


    // Length of the buffer to store the value in

    DWORD dwBufLen = sizeof(DWORD);


    // Get value from sub key, store it in the passed DWORD

    LONG result = RegQueryValueEx(m_hKey,
                                  lpSubKey,
                                  NULL,
                                  &type,
                                  (LPBYTE) &num,
                                  &dwBufLen);


    // If the query fails or the type returned isn't a DWORD, return false

    if(result != ERROR_SUCCESS || type != REG_DWORD)
        return false;

    return true;
}


bool 
CKNSettings::SetRegValue(LPCTSTR lpSubKey, string &str)
{
    // If handle to the key doesn't exist, return false

    if(!m_hKey)
        return false;


    // Set value in passed sub key to the passed string

    LONG result = RegSetValueEx(m_hKey, 
                                lpSubKey, 
                                0, 
                                REG_SZ, 
                                (CONST BYTE*) str.c_str(), 
                                (str.length() + 1) * sizeof(TCHAR));
    

    // If setting value doesn't succeed, return false

    if(result != ERROR_SUCCESS)
        return false;

    return true;
}


bool 
CKNSettings::SetRegValueNum(LPCTSTR lpSubKey, DWORD num)
{
    // If handle to the key doesn't exist, return false

    if(!m_hKey)
        return false;


    // Set value in passed sub key to the passed DWORD

    LONG result = RegSetValueEx(m_hKey,
                                lpSubKey,
                                0,
                                REG_DWORD,
                                (CONST BYTE*) &num,
                                sizeof(DWORD));

    
    // If setting value doesn't succeed, return false

    if(result != ERROR_SUCCESS)
        return false;

    return true;
}


bool 
CKNSettings::GetSettingsFromRegistry()
{
    // If opening the key for reading succeeds, get the registry values for
    // Server, Path, User, and Pass, and store them in their appropriate
    // variables.

    if(OpenKey(true)) 
    {
        if(GetRegValue("Server", m_strServer) && 
           GetRegValueNum("Port", (DWORD&) m_iPort) && 
           GetRegValue("Path", m_strPath) && 
           GetRegValue("User", m_strUser) && 
           GetRegValue("Pass", m_strPass))
        {
            // If getting all of the above values succeeds, close the key

            CloseKey();

            return true;
        }
    }

    return false;
}
