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

// CParameters.cpp : Implementation of CParameters
#include "stdafx.h"
#include "LibKNCom.h"
#include "CParameters.h"
#include <LibKN\Message.h>
#include <LibKN\StrUtil.h>

/////////////////////////////////////////////////////////////////////////////
// CParameters

CParameters::CParameters()	
{
}

CParameters::CParameters(const ITransport::Parameters& p) :
	m_Impl(p)
{
}

STDMETHODIMP CParameters::get_ServerUrl(BSTR* pVal)
{
	CComBSTR v(m_Impl.m_ServerUrl.c_str());
	return v.CopyTo(pVal);
}

STDMETHODIMP CParameters::put_ServerUrl(BSTR newVal)
{
	if (newVal == 0)
		m_Impl.m_ServerUrl.erase();
	else
		m_Impl.m_ServerUrl = ConvertToTString(newVal);
	return S_OK;
}

STDMETHODIMP CParameters::get_Username(BSTR* pVal)
{
	CComBSTR v(m_Impl.m_Username.c_str());
	return v.CopyTo(pVal);
}

STDMETHODIMP CParameters::put_Username(BSTR newVal)
{
	if (newVal == 0)
		m_Impl.m_Username.erase();
	else
		m_Impl.m_Username = ConvertToTString(newVal);
	return S_OK;
}

STDMETHODIMP CParameters::get_Password(BSTR* pVal)
{
	CComBSTR v(m_Impl.m_Password.c_str());
	return v.CopyTo(pVal);
}

STDMETHODIMP CParameters::put_Password(BSTR newVal)
{
	if (newVal == 0)
		m_Impl.m_Password.erase();
	else
		m_Impl.m_Password = ConvertToTString(newVal);
	return S_OK;
}

STDMETHODIMP CParameters::get_UseProxy(VARIANT_BOOL* pVal)
{
	*pVal = (m_Impl.m_UseProxy) ? VARIANT_TRUE : VARIANT_FALSE;
	return S_OK;
}

STDMETHODIMP CParameters::put_UseProxy(VARIANT_BOOL newVal)
{
	m_Impl.m_UseProxy = (newVal == VARIANT_TRUE) ? true : false;
	return S_OK;
}

STDMETHODIMP CParameters::get_ProxyServer(BSTR* pVal)
{
	CComBSTR v(m_Impl.m_ProxyServer.c_str());
	return v.CopyTo(pVal);
}

STDMETHODIMP CParameters::put_ProxyServer(BSTR newVal)
{
	if (newVal == 0)
		m_Impl.m_ProxyServer.erase();
	else
		m_Impl.m_ProxyServer = ConvertToTString(newVal);
	return S_OK;
}

STDMETHODIMP CParameters::get_ProxyUsername(BSTR* pVal)
{
	CComBSTR v(m_Impl.m_ProxyUsername.c_str());
	return v.CopyTo(pVal);
}

STDMETHODIMP CParameters::put_ProxyUsername(BSTR newVal)
{
	if (newVal == 0)
		m_Impl.m_ProxyUsername.erase();
	else
		m_Impl.m_ProxyUsername = ConvertToTString(newVal);
	return S_OK;
}

STDMETHODIMP CParameters::get_ProxyPassword(BSTR* pVal)
{
	CComBSTR v(m_Impl.m_ProxyPassword.c_str());
	return v.CopyTo(pVal);
}

STDMETHODIMP CParameters::put_ProxyPassword(BSTR newVal)
{
	if (newVal == 0)
		m_Impl.m_ProxyPassword.erase();
	else
		m_Impl.m_ProxyPassword = ConvertToTString(newVal);
	return S_OK;
}

STDMETHODIMP CParameters::get_ProxyExceptionList(BSTR* pVal)
{
	CComBSTR v(m_Impl.m_ProxyExceptionList.c_str());
	return v.CopyTo(pVal);
}

STDMETHODIMP CParameters::put_ProxyExceptionList(BSTR newVal)
{
	if (newVal == 0)
		m_Impl.m_ProxyExceptionList.erase();
	else
		m_Impl.m_ProxyExceptionList = ConvertToTString(newVal);
	return S_OK;
}

STDMETHODIMP CParameters::get_CustomHeader(BSTR* pVal)
{
	CComBSTR v(m_Impl.m_CustomHeader.c_str());
	return v.CopyTo(pVal);
}

STDMETHODIMP CParameters::put_CustomHeader(BSTR newVal)
{
	if (newVal == 0)
		m_Impl.m_CustomHeader.erase();
	else
		m_Impl.m_CustomHeader = ConvertToTString(newVal);
	return S_OK;
}

STDMETHODIMP CParameters::get_ShowUI(VARIANT_BOOL* pVal)
{
	*pVal = (m_Impl.m_ShowUI) ? VARIANT_TRUE : VARIANT_FALSE;
	return S_OK;
}

STDMETHODIMP CParameters::put_ShowUI(VARIANT_BOOL newVal)
{
	m_Impl.m_ShowUI = (newVal == VARIANT_TRUE) ? true : false;
	return S_OK;
}

tstring ConvertString(BSTR b);

ITransport::Parameters ConvertParameter(IParameters* p)
{
	ITransport::Parameters retVal;
	CComBSTR b;
	HRESULT hr = S_OK;
	VARIANT_BOOL vb;

	b.Empty();
	if (SUCCEEDED(p->get_ServerUrl(&b)))
		retVal.m_ServerUrl = ConvertToTString(wstring(b));
	b.Empty();
	if (SUCCEEDED(p->get_Username(&b)))
		retVal.m_Username = ConvertToTString(wstring(b));
	b.Empty();
	if (SUCCEEDED(p->get_Password(&b)))
		retVal.m_Password = ConvertToTString(wstring(b));
	b.Empty();
	if (SUCCEEDED(p->get_UseProxy(&vb)))
		retVal.m_UseProxy = vb == VARIANT_TRUE ? true : false;
	b.Empty();
	if (SUCCEEDED(p->get_ProxyServer(&b)))
		retVal.m_ProxyServer = ConvertToTString(wstring(b));
	b.Empty();
	if (SUCCEEDED(p->get_ProxyUsername(&b)))
		retVal.m_ProxyUsername = ConvertToTString(wstring(b));
	b.Empty();
	if (SUCCEEDED(p->get_ProxyPassword(&b)))
		retVal.m_ProxyPassword = ConvertToTString(wstring(b));
	b.Empty();
	if (SUCCEEDED(p->get_ProxyExceptionList(&b)))
		retVal.m_ProxyExceptionList = ConvertToTString(wstring(b));
	b.Empty();
	if (SUCCEEDED(p->get_CustomHeader(&b)))
		retVal.m_CustomHeader = ConvertToTString(wstring(b));
	b.Empty();
	if (SUCCEEDED(p->get_ShowUI(&vb)))
		retVal.m_ShowUI = vb == VARIANT_TRUE ? true : false;
	b.Empty();
	return retVal;
}

void ConvertParameter(const ITransport::Parameters& p, IParameters** pVal)
{
	if (pVal == 0)
		return;

	CComObject<CParameters>* obj = 0;
	HRESULT hr = S_OK;

	if (*pVal == 0)
	{
		hr = CComObject<CParameters>::CreateInstance(&obj);

		if (SUCCEEDED(hr))
			hr = obj->QueryInterface(pVal);
	}

	CComBSTR b;
	
	b = p.m_ServerUrl.c_str();
	hr = (*pVal)->put_ServerUrl(b);
	b = p.m_Username.c_str();
	hr = (*pVal)->put_Username(b);
	b = p.m_Password.c_str();
	hr = (*pVal)->put_Password(b);
	hr = (*pVal)->put_UseProxy(p.m_UseProxy ? VARIANT_TRUE : VARIANT_FALSE);
	b = p.m_ProxyServer.c_str();
	hr = (*pVal)->put_ProxyServer(b);
	b = p.m_ProxyUsername.c_str();
	hr = (*pVal)->put_ProxyUsername(b);
	b = p.m_ProxyPassword.c_str();
	hr = (*pVal)->put_ProxyPassword(b);
	b = p.m_ProxyExceptionList.c_str();
	hr = (*pVal)->put_ProxyExceptionList(b);
	b = p.m_CustomHeader.c_str();
	hr = (*pVal)->put_CustomHeader(b);
	hr = (*pVal)->put_ShowUI(p.m_ShowUI ? VARIANT_TRUE : VARIANT_FALSE);
}

