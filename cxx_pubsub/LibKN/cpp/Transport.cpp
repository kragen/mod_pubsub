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

#include "stdafx.h"
#include <LibKN\Transport.h>
#include <LibKN\Tunnel.h>
#include <LibKN\SimpleParser.h>
#include <LibKN\IRequestStatusHandler.h>

ITransport::Parameters::Parameters()
{
	m_UseProxy = false;
	m_ShowUI = false;
	m_ParentHwnd = 0;

#if !defined(_WIN32_WCE)
	unsigned char buffer[1024];

	HKEY ieSettings = 0;
	if (RegOpenKey(HKEY_CURRENT_USER, _T("Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings"), &ieSettings) == 
		ERROR_SUCCESS)
	{
		DWORD type;
		DWORD bufSize;

		DWORD regValue = 0;
		type = REG_DWORD;
		bufSize = sizeof(regValue);
		if (RegQueryValueEx(ieSettings, _T("ProxyEnable"), 0, &type, (BYTE*)&regValue, &bufSize) == ERROR_SUCCESS)
		{
			if (type == REG_DWORD)
			{
				m_UseProxy = (regValue == 0) ? false : true;
			}
		}

		type = REG_SZ;
		bufSize = 1024;
		ZeroMemory(buffer, bufSize);
		if (RegQueryValueEx(ieSettings, _T("ProxyServer"), 0, &type, buffer, &bufSize) == ERROR_SUCCESS)
		{
			if (type == REG_SZ)
			{
				m_ProxyServer = (char*)buffer;
			}
		}

		type = REG_SZ;
		bufSize = 1024;
		ZeroMemory(buffer, bufSize);
		if (RegQueryValueEx(ieSettings, _T("ProxyOverride"), 0, &type, buffer, &bufSize) == ERROR_SUCCESS)
		{
			if (type == REG_SZ)
			{
				m_ProxyExceptionList = (char*)buffer;
			}
		}

		RegCloseKey(ieSettings);
		ieSettings = 0;
	}
#endif
}

Transport::Transport(Connector* conn) :
	m_Connector(conn),
	m_Session(0),
	m_Connect(0)
{
	m_Https = false;
}

Transport::~Transport()
{
	Lock autoLock(this);

	Disconnect();
}

const ITransport::Parameters& Transport::GetParameters() const
{
	Lock autoLock(this);

	return m_Parameters;
}

bool Transport::Open(const ITransport::Parameters& p)
{
	Lock autoLock(this);

	m_Parameters = p;
	return Connect();
}

bool Transport::IsConnected()
{
	Lock autoLock(this);

	return m_Session != 0 && m_Connect != 0;
}

bool Transport::Close()
{
	Lock autoLock(this);

	return Disconnect();
}

bool Transport::Connect()
{
	Lock autoLock(this);

	if (m_Parameters.m_ServerUrl.length() == 0)
		return false;

	URL_COMPONENTS url_comp = 
	{	
		sizeof(URL_COMPONENTS),		//dwStructSize
		NULL,						//lpszScheme
		0,							//dwSchemeLength
		INTERNET_SCHEME_UNKNOWN,	//nScheme
		NULL,						//lpszHostName
		-1,							//dwHostNameLength
		0,							//nPort
		NULL,						//lpszUserName
		-1,							//dwUserNameLength
		NULL,						//lpszPassword
		-1,							//dwPasswordLength
		NULL,						//lpszUrlPath
		-1,							//dwUrlPathLength
		NULL,						//lpszExtraInfo
		0							//dwExtraInfoLength
	};						

	if (!InternetCrackUrl((TCHAR*)m_Parameters.m_ServerUrl.c_str(), m_Parameters.m_ServerUrl.length(), 0, &url_comp))
	{
		return false;
	}

	if (url_comp.nScheme == INTERNET_SCHEME_HTTP)
		m_Https = false;
	else if(url_comp.nScheme == INTERNET_SCHEME_HTTPS)
		m_Https = true;
	else 
	{
		return false;
	}

	m_Server.assign(url_comp.lpszHostName, url_comp.dwHostNameLength);
	m_ServerPath.assign(url_comp.lpszUrlPath, url_comp.dwUrlPathLength);

	if (!m_Connect) 
	{
		//connect to the internet
		m_Connect = InternetOpen(_T("libkn"), 
			GetParameters().m_UseProxy ? INTERNET_OPEN_TYPE_PROXY : INTERNET_OPEN_TYPE_PRECONFIG, 
			GetParameters().m_UseProxy ? GetParameters().m_ProxyServer.c_str() : 0, 
			(GetParameters().m_UseProxy && GetParameters().m_ProxyExceptionList.length() > 0) ? 
				GetParameters().m_ProxyExceptionList.c_str() : 
	            0, 
			0);

		if (m_Connect == NULL) 
		{
			return false;
		}
	}

	if (!m_Session)
	{
		//connect to the server
		m_Session = InternetConnect(m_Connect, m_Server.c_str(), url_comp.nPort, 
			GetParameters().m_Username.c_str(), 0, INTERNET_SERVICE_HTTP, 0, 0);

		if (m_Session == 0) 
		{
			InternetCloseHandle(m_Connect);
			m_Connect = 0;
			return false;
		}

		//set the username and password for the proxy if we are using one
		if (GetParameters().m_UseProxy) 
		{
			InternetSetOption(m_Session, INTERNET_OPTION_PROXY_USERNAME, 
				(void*)GetParameters().m_ProxyUsername.c_str(), 
				GetParameters().m_ProxyUsername.length() + sizeof(TCHAR));
			InternetSetOption(m_Session, INTERNET_OPTION_PROXY_PASSWORD, 
				(void*)GetParameters().m_ProxyPassword.c_str(), 
				GetParameters().m_ProxyPassword.length() + sizeof(TCHAR));
		}
	}

	//workaround blank password problem
	//
	InternetSetOption(m_Session, INTERNET_OPTION_PASSWORD, 
		(void*)GetParameters().m_Password.c_str(), 
		GetParameters().m_Password.length() + sizeof(TCHAR));

	return IsConnected();
}

bool Transport::Disconnect()
{
	Lock autoLock(this);

	if (m_Session) 
	{
		InternetCloseHandle(m_Session);
		m_Session = NULL;
	}

	if (m_Connect) 
	{
		InternetCloseHandle(m_Connect);
		m_Connect = NULL;
	}

	return !IsConnected();
}

bool Transport::EnsureConnected()
{
	Lock autoLock(this);

	if (!IsConnected())
	{
		if (!Connect())
			return false;
	}
	return true;
}

const tstring& Transport::GetUrl() const
{
	Lock autoLock(this);

	return GetParameters().m_ServerUrl;
}

void DumpError(TCHAR* msg, DWORD error)
{
	TCHAR buffer[1024];
	_stprintf(buffer, _T("%s, error %d\r\n"), error);
	OutputDebugString(buffer);
}

HWND GetHwndFromParam(const ITransport::Parameters& p)
{
	if (p.m_ShowUI == false)
		return 0;

	HWND retVal = p.m_ParentHwnd;

	if (retVal == 0)
		retVal = GetFocus();

	if (retVal == 0)
		retVal = GetForegroundWindow();

	if (retVal == 0)
		retVal = GetActiveWindow();

	if (retVal == 0)
		retVal = GetDesktopWindow();

	return retVal;
}

#if !defined(ERROR_INTERNET_SEC_CERT_REV_FAILED)
#	define ERROR_INTERNET_SEC_CERT_REV_FAILED      (INTERNET_ERROR_BASE + 57)
#endif

bool Transport::HandleAuth(HINTERNET http_request, bool& triedAuth)
{
	bool toTryAgain = true;

	if (!triedAuth)
	{
		BOOL temp;
		temp = InternetSetOption(http_request, INTERNET_OPTION_USERNAME, 
			(void*)GetParameters().m_Username.c_str(), GetParameters().m_Username.length() + 1);
		temp = InternetSetOption(http_request, INTERNET_OPTION_PASSWORD, 
			(void*)GetParameters().m_Password.c_str(), GetParameters().m_Password.length() + 1);

		triedAuth = true;
	}
	else
	{
		DWORD retVal = InternetErrorDlg(GetHwndFromParam(GetParameters()), http_request, ERROR_INTERNET_INCORRECT_PASSWORD,
			FLAGS_ERROR_UI_FILTER_FOR_ERRORS | FLAGS_ERROR_UI_FLAGS_GENERATE_DATA | FLAGS_ERROR_UI_FLAGS_CHANGE_OPTIONS,
			NULL);

		toTryAgain = (retVal == ERROR_INTERNET_FORCE_RETRY) ? true : false;

		if (toTryAgain)
		{
			const int buflen = 10240;
			TCHAR buffer[buflen];
			DWORD len = buflen;
			BOOL temp;

			len = buflen;
			ZeroMemory(buffer, buflen);
			temp = InternetQueryOption(http_request, INTERNET_OPTION_USERNAME, buffer, &len);
			m_Parameters.m_Username.assign(buffer);

			len = buflen;
			ZeroMemory(buffer, buflen);
			temp = InternetQueryOption(http_request, INTERNET_OPTION_PASSWORD, buffer, &len);
			m_Parameters.m_Password.assign(buffer);
		}
	}

	return toTryAgain;
}

bool Transport::HandleProxyAuth(HINTERNET http_request, bool& triedProxyStuff)
{
	bool toTryAgain = true;

	if (!triedProxyStuff)
	{
		BOOL temp;
		temp = InternetSetOption(http_request, INTERNET_OPTION_PROXY_USERNAME, 
			(void*)GetParameters().m_ProxyUsername.c_str(), GetParameters().m_ProxyUsername.length() + 1);
		temp = InternetSetOption(http_request, INTERNET_OPTION_PROXY_PASSWORD, 
			(void*)GetParameters().m_ProxyPassword.c_str(), GetParameters().m_ProxyPassword.length() + 1);

		triedProxyStuff = true;
	}
	else
	{
		DWORD retVal = InternetErrorDlg(GetHwndFromParam(GetParameters()), http_request, ERROR_INTERNET_INCORRECT_PASSWORD,
			FLAGS_ERROR_UI_FILTER_FOR_ERRORS | FLAGS_ERROR_UI_FLAGS_GENERATE_DATA | FLAGS_ERROR_UI_FLAGS_CHANGE_OPTIONS,
			NULL);

		toTryAgain = (retVal == ERROR_INTERNET_FORCE_RETRY) ? true : false;

		if (toTryAgain)
		{
			const int buflen = 10240;
			TCHAR buffer[buflen];
			DWORD len = buflen;
			BOOL temp;

			len = buflen;
			ZeroMemory(buffer, buflen);
			temp = InternetQueryOption(http_request, INTERNET_OPTION_PROXY_USERNAME, buffer, &len);
			m_Parameters.m_ProxyUsername.assign(buffer);

			len = buflen;
			ZeroMemory(buffer, buflen);
			temp = InternetQueryOption(http_request, INTERNET_OPTION_PROXY_PASSWORD, buffer, &len);
			m_Parameters.m_ProxyPassword.assign(buffer);
		}
	}

	return toTryAgain;
}

bool Transport::HandleSecurity(HINTERNET http_request)
{
	bool toTryAgain = false;
	
	DWORD retVal = InternetErrorDlg(GetHwndFromParam(GetParameters()), http_request, GetLastError(),
		FLAGS_ERROR_UI_FILTER_FOR_ERRORS | FLAGS_ERROR_UI_FLAGS_GENERATE_DATA | FLAGS_ERROR_UI_FLAGS_CHANGE_OPTIONS,
		NULL);

	toTryAgain = (retVal == ERROR_INTERNET_FORCE_RETRY) ? true : false;

	return toTryAgain;
}

void GetExtendedError(tstring& error_string, Message& status_map)
{
	//extended error info
	DWORD extended_error;
	DWORD size = 512;
	LPTSTR errinfo = new TCHAR[size];
	ZeroMemory(errinfo, size * sizeof(TCHAR));

	if (!InternetGetLastResponseInfo(&extended_error, &errinfo[0], &size)) 
	{
		if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) 
		{
			delete[] errinfo;
			errinfo = new TCHAR[size];
			if (InternetGetLastResponseInfo(&extended_error, &errinfo[0], &size))
			{
				status_map.Set(_T("kn_payload"), errinfo);
				error_string.append(errinfo);
			}
		}
	}
	else
		error_string.append(errinfo);

	delete[] errinfo;
}

void GetSystemError(DWORD error, tstring& error_string, Message& status_map)
{
	TCHAR buf[512];
	LPTSTR lpMsgBuf = &buf[0];

	DWORD ret = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_HMODULE,
		GetModuleHandle(_T("WinInet")), error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
		lpMsgBuf, 512, NULL);

	// convert the error number to a string
	TCHAR number[11];
	if (!_ultot(error, &number[0], 10))
		number[0] = NULL;

	//put the error number in the status obj
	status_map.Set(_T("local_error"), number);

	if (ret != 0) 
	{
		// Set the error
		error_string.append(lpMsgBuf);
		status_map.Set(_T("kn_payload"), lpMsgBuf);
	}
	else 
	{
		error_string.append(_T("System error number: "));
		error_string.append(&number[0]);
	}
}

void Transport::HandleError(DWORD error, Message& status_map)
{
	TCHAR buffer[80];

	_stprintf(buffer, _T("%d"), error);
	// put the error information into the status also.
	status_map.Set(_T("status_code"), buffer);

	tstring error_string;
	error_string = _T("Unable to send request to server.\n");

	if (error == ERROR_INTERNET_EXTENDED_ERROR) 
		GetExtendedError(error_string, status_map);
	else 
		GetSystemError(error, error_string, status_map);

	status_map.Set(_T("ErrorString"), error_string);
}

/** @param http_request a handle returned by HttpSendRequest to test the status on
 *  @param error_msg a string to prepend to the error message if there is an error
 *  @return true if the request was successful.
 *
 * Checks the http status of an internet request.  Sets an error object and returns false if
 * the status is 300 or greater.
 *
 */
bool Transport::CheckStatus(HINTERNET http_request, Message& status_map)
{
	tstring error_string;
	tstring status_string;
	bool retval = false;

	if (http_request) 
	{
		DWORD status = 0;
		DWORD size = sizeof(DWORD);
		DWORD header = 0;

		if (HttpQueryInfo(http_request, HTTP_QUERY_STATUS_CODE|HTTP_QUERY_FLAG_NUMBER, &status, &size, &header)) 
		{
			size = 255;
			LPTSTR errinfo = new TCHAR[size];

			//append the status number
			_itot(status, errinfo, 10);
			status_string += errinfo;
			status_string += _T(" ");

			status_map.Set(StatusCodeField, errinfo);
			
			error_string += errinfo;
			error_string += _T(" ");

			//get the status message
			if (!HttpQueryInfo(http_request, HTTP_QUERY_STATUS_TEXT, (LPVOID) errinfo, &size, &header)) 
			{
				DWORD err = GetLastError();
				if (err == ERROR_INSUFFICIENT_BUFFER) 
				{
					delete[] errinfo;
					errinfo = new TCHAR[size];
					if (HttpQueryInfo(http_request, HTTP_QUERY_STATUS_TEXT, &errinfo, &size, &header)) 
					{
						status_string.append(errinfo);
						error_string.append(errinfo);
					}
				}
			}
			else 
			{
				status_string.append(errinfo);
				error_string.append(errinfo);
			}

			if (status != HTTP_STATUS_OK)
			{
				status_map.Set(_T("ErrorString"), error_string);
			}

			status_map.Set(_T("status"), status_string.c_str());
			delete[] errinfo;

            if (status < 300) 
			{
                if (false)
				{
                    //check the content type header
                    size = 1023;
                    char content[1024];
                    if (HttpQueryInfo(http_request, HTTP_QUERY_CONTENT_TYPE, &content[0], &size, &header)) 
					{
                        //check the content type
                        if (0 != strncmp("text/plain", &content[0], strlen("text/plain"))) 
						{
							status_map.Set(_T("ErrorString"), _T("Response is not from server, check ServerURL."));
                            retval = false;
                        }
                    }
                    else
                        retval = false;
                }

				retval = true;
            }
			else
				retval = false;
		}
		else
			retval = false;
	}
	else 
	{
		HandleError(GetLastError(), status_map);
		retval = false;
	}

	return retval;
}


DWORD Transport::GetINetFlags()
{
	DWORD flags = INTERNET_FLAG_NO_CACHE_WRITE | INTERNET_FLAG_PRAGMA_NOCACHE | INTERNET_FLAG_NO_UI | 
		INTERNET_FLAG_KEEP_CONNECTION;

	if (m_Https) 
	{
		flags |= INTERNET_FLAG_SECURE;
	}

	if (GetParameters().m_UseProxy)
	{
		flags |= INTERNET_FLAG_KEEP_CONNECTION | INTERNET_FLAG_RELOAD;
	}

	return flags;
}

void Transport::SetupHttpRequest(HINTERNET http_request)
{
	// Set the username and password for the proxy if we are using one
	//
	if (GetParameters().m_UseProxy) 
	{
		BOOL temp = InternetSetOption(http_request, INTERNET_OPTION_PROXY_USERNAME, 
			(void*)GetParameters().m_ProxyUsername.c_str(), GetParameters().m_ProxyUsername.length() + 1);
		temp = InternetSetOption(http_request, INTERNET_OPTION_PROXY_PASSWORD, 
			(void*)GetParameters().m_ProxyPassword.c_str(), GetParameters().m_ProxyPassword.length() + 1);

		DWORD to = 30 * 1000;
		temp = InternetSetOption(http_request, INTERNET_OPTION_RECEIVE_TIMEOUT, &to, sizeof(to));
	}

	if (1)
	{
		// Always do this to prevent a strange timing error that causes multiple requests to be sent
		// to the server in one packet.
		//
		tstring content;
		content = _T("Connection: timeout=1\r\n");
		if (!HttpAddRequestHeaders(http_request, content.c_str(), -1, HTTP_ADDREQ_FLAG_ADD_IF_NEW))
			DumpError(_T("HttpAddRequestHeaders connection failed"), GetLastError());
	}

	if (GetParameters().m_UseProxy)
	{
		tstring content;
		content = _T("Proxy-Connection: Keep-Alive\r\n");
		if (!HttpAddRequestHeaders(http_request, content.c_str(), -1, HTTP_ADDREQ_FLAG_ADD_IF_NEW))
			DumpError(_T("HttpAddRequestHeaders proxy-connection failed"), GetLastError());
	}

	if (GetParameters().m_CustomHeader.length() > 0)
	{
		if (!HttpAddRequestHeaders(http_request, GetParameters().m_CustomHeader.c_str(), -1, HTTP_ADDREQ_FLAG_ADD_IF_NEW))
			DumpError(_T("HttpAddRequestHeaders custom_header failed"), GetLastError());
	}
}

HINTERNET Transport::Send(const string& data, Message& status_map)
{
	Lock autoLock(this);

	if (!EnsureConnected())
		return 0;

	DWORD flags = GetINetFlags();

	int overallTry = 0;
	HINTERNET http_request = 0;

	BOOL success = FALSE;
	int attempts = 0;
	bool triedProxyStuff = false;
	bool triedAuth = false;

	const int MaxAttempts = 10;

tryAgain:
	http_request = 0;
	success = FALSE;
	attempts = 0;
	triedProxyStuff = false;
	triedAuth = false;

	if (overallTry > 0)
		goto exit;

	// build a request
	//
	http_request = HttpOpenRequest(m_Session, _T("POST"), m_ServerPath.c_str(), NULL, _T("libkn"), 
		NULL, flags, 0);

	if (http_request == 0) 
	{
		goto exit;
	}

	SetupHttpRequest(http_request);


	status_map.Empty();

	while (!success && attempts < MaxAttempts)
	{
		// send the request, using the parameters as the optional data (http payload)
		// 
		success = HttpSendRequest(http_request, NULL, 0, (void*)data.c_str(), data.length());

		if (success) 
		{
			// check to make sure we got back a valid status
			//
			DWORD status = 0;
			DWORD size = sizeof(DWORD);
			DWORD header = 0;

			if (HttpQueryInfo(http_request, HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER, &status, &size, &header)) 
			{
				if (status == HTTP_STATUS_DENIED)
				{
					if (HandleAuth(http_request, triedAuth))
					{
						success = FALSE;
						continue;
					}
				}

				if (status == HTTP_STATUS_PROXY_AUTH_REQ)
				{
					if (HandleProxyAuth(http_request, triedProxyStuff))
					{
						success = FALSE;
						continue;
					}
				}
			}

			if (!CheckStatus(http_request, status_map)) 
			{
				InternetCloseHandle(http_request);
				http_request = 0;
				goto exit;
			}	
		}
		else 
		{
			DWORD error = GetLastError();

			if (error == ERROR_HTTP_INVALID_SERVER_RESPONSE || error == ERROR_INTERNET_CANNOT_CONNECT)
			{
				attempts++;

				if (attempts < MaxAttempts)
					continue;
				else
				{
					if (error == ERROR_INTERNET_CANNOT_CONNECT)
					{
						InternetCloseHandle(http_request);
						overallTry++;
						goto tryAgain;
					}
				}
			}

			if (ERROR_INTERNET_SEC_CERT_DATE_INVALID <= error && error <= ERROR_INTERNET_SEC_CERT_REV_FAILED)
			{
				if (HandleSecurity(http_request))
				{
					attempts++;

					if (attempts < MaxAttempts)
						continue;
				}
			}

			// Error condition
			//
			HandleError(error, status_map);

			InternetCloseHandle(http_request);
			http_request = 0;
			goto exit;
		}
	}
	
exit:
	return http_request;
}

bool Transport::SendImpl(const string& data, const Message& msg, IRequestStatusHandler* sh)
{
	Message result;

	HINTERNET hReq = Send(data, result);

	if (sh)
	{
		wstring rid;

		if (msg.Get(L"LIBKN_RID", rid))
		{
			result.Set(L"LIBKN_RID", rid);
		}

		sh->OnStatus(result);
	}
	
	if (hReq != 0)
	{
		InternetCloseHandle(hReq);
		return true;
	}

	return false;
}

bool Transport::Send(const string& data, IRequestStatusHandler* sh)
{
	Message m;
	return SendImpl(data, m, sh);
}

bool Transport::Send(const Message& msg, IRequestStatusHandler* sh)
{
	return SendImpl(msg.GetAsHttpParam(), msg, sh);
}

Tunnel* Transport::OpenTunnel(const Message& msg)
{
	Lock autoLock(this);

	Message status_map;
	Tunnel* tunnel = 0;

	HINTERNET hRes = Send(msg.GetAsHttpParam(), status_map);
	if (hRes)
	{
		if (GetTunnelStatus(hRes, status_map))
			tunnel = new Tunnel(hRes, status_map);
	}
	return tunnel;
}

bool Transport::GetTunnelStatus(HINTERNET hTunnel, Message& status_map)
{
	Lock autoLock(this);

	if (!hTunnel)
		return false;

	mb_buf_ptr buf_ptr;
	const int buffer_size = 256;            	// size of the buffer
	unsigned char read_buffer[buffer_size];     // buffer for the data
	string len_string;
	int len = 0;
	DWORD available = 0;
	DWORD dwSize = 0;              // size of the data available
	DWORD dwDownloaded = 0;        // size of the downloaded data
	bool found = false;
	string err_string;

	// This loop handles reading the data.
	while (!found) 
	{
		//put the pointer and length into a mb_buf_ptr
		buf_ptr.m_Ptr = &read_buffer[0];
		buf_ptr.m_Max = buffer_size;
		buf_ptr.m_Len = 0;

		// Read the data from the tunnel.
		if (available == 0) 
		{
			// This will block until there is data avalable or the connection is closed
			if (!InternetQueryDataAvailable(hTunnel, &dwSize, 0, 0)) 
			{
				//check for close
				return false;
			}

			available = dwSize;
		}

		// setup for reading
		//
		unsigned char* buffer = buf_ptr.m_Ptr;
		DWORD total_read = 0;
		err_string = "";

		while (available > total_read)
		{
			// Read the data from the HINTERNET handle.
			int read = InternetReadFile(hTunnel, (void*)buffer, 1, &dwDownloaded);
			if (!read)
				return false;
			else
				total_read += read;

			if (*buffer >= '0' && *buffer <= '9') 
			{
				len_string += *buffer;
			}
			else if (*buffer == '\n') 
			{
				len = atoi(len_string.c_str());
				found = true;
				break;
			}
			else
				err_string += *buffer;
		}
	}

	found = false;
	string event;

	//get the rest of the event
	while (!found) 
	{
		// put the pointer and length into a mb_buf_ptr
		buf_ptr.m_Ptr = &read_buffer[0];
		buf_ptr.m_Max = buffer_size;
		buf_ptr.m_Len = 0;

		// Read the data from the tunnel.
		if (available == 0) 
		{
			// This will block until there is data avalable or the connection is closed
			if (!InternetQueryDataAvailable(hTunnel, &dwSize, 0, 0)) 
			{
				//check for close
				return false;
			}
			
			available = dwSize;
		}

		//setup for reading
		DWORD total_read = 0;
		unsigned char* buffer = buf_ptr.m_Ptr;
		DWORD to_read = (available < (len - total_read)) ? available : (len - total_read);

		// Read the data from the HINTERNET handle.
		if (InternetReadFile(hTunnel, (void*)buffer, to_read, &dwDownloaded))
			total_read += to_read;
		else
			return false;

		event.append((char*)buffer, to_read);

		if (total_read >= (unsigned)len)
			found = true;
	}

	if (found) 
	{
		SimpleParser sp(0);
		sp.make_map_from_simple(event, status_map);
		return true;
	}

	return false;
}

