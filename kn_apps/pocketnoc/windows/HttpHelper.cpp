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

#include "HttpHelper.h"

CHttpHelper::CHttpHelper(RouterSettings &settings, char *topic)
{
	m_hConnect = NULL;
	m_hSession = NULL;
	m_routerSettings = &settings;

	m_topic = new char[strlen(topic) + 1];

	strcpy(m_topic, topic);
}

CHttpHelper::~CHttpHelper(void)
{
	delete[] m_topic;
}


bool CHttpHelper::connect(void)
{
	m_hConnect = InternetOpen("pocketnoc", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);

	if(m_hConnect == NULL)
		return false;
	
	m_hSession = InternetConnect(m_hConnect,
							   m_routerSettings->routerServer,
							   m_routerSettings->routerPort,
							   m_routerSettings->routerUser,
							   m_routerSettings->routerPass,
							   INTERNET_SERVICE_HTTP,
							   0,
							   0);

	if(m_hSession == NULL) {
		InternetCloseHandle(m_hConnect);
		return false;
	}

	return true;
}


void CHttpHelper::disconnect(void)
{
	if(m_hConnect) {
		InternetCloseHandle(m_hConnect);
		m_hConnect = NULL;
	}

	if(m_hSession) {
		InternetCloseHandle(m_hSession);
		m_hSession = NULL;
	}
}


bool CHttpHelper::publish(char *payload)
{
	bool result = false;

	DWORD flags = INTERNET_FLAG_NO_CACHE_WRITE | INTERNET_FLAG_PRAGMA_NOCACHE;

	char path[1024];

	int len = sprintf(path, "%s", m_routerSettings->routerPath);
	sprintf(path + len, "%s", m_topic);

	HINTERNET httpRequest = HttpOpenRequest(m_hSession, 
											"POST",
											path,
											NULL, 
											"pocketnoc",
											NULL,
											flags,
											0);

	if(httpRequest == NULL)
		return result;

	if(HttpSendRequest(httpRequest, NULL, 0, payload, strlen(payload)))
		result = true;
	
	InternetCloseHandle(httpRequest);

	return result;
}
