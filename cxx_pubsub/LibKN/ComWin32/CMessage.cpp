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

// CMessage.cpp : Implementation of CMessage
#include "stdafx.h"
#include "LibKNCom.h"
#include "CMessage.h"
#include "CMessageEntry.h"
#include <LibKN\StrUtil.h>

/////////////////////////////////////////////////////////////////////////////
// CMessage
CMessage::CMessage()
{
}

void CMessage::Copy(Message* rhs)
{
	if (rhs != 0)
		m_Impl = *rhs;
}

template <class EnumType, class CollType>
HRESULT CreateSTLEnumerator(IUnknown** ppUnk, IUnknown* pUnkForRelease, CollType& collection)
{
	if (ppUnk == NULL)
		return E_POINTER;

	*ppUnk = NULL;

	CComObject<EnumType>* pEnum = NULL;
	HRESULT hr = CComObject<EnumType>::CreateInstance(&pEnum);

	if (FAILED(hr))
		return hr;

	hr = pEnum->Init(pUnkForRelease, collection);

	if (SUCCEEDED(hr))
		hr = pEnum->QueryInterface(ppUnk);

	if (FAILED(hr))
		delete pEnum;

	return hr;

} // CreateSTLEnumerator

STDMETHODIMP CMessage::get__NewEnum(IUnknown** pVal)
{
	const Message::Container& container = m_Impl.GetContainer();
	int sz = container.size();

	m_Coll.clear();

	for (Message::Container::const_iterator it = container.begin(); it != container.end(); ++it)
	{
		CComObject<CMessageEntry>* obj = 0;
		HRESULT hr = CComObject<CMessageEntry>::CreateInstance(&obj);

		if (SUCCEEDED(hr))
		{
			(*obj).SetFV((*it).first.c_str(), (*it).second.c_str());
		}

		CComVariant v = obj;
		m_Coll.push_back(v);
	}

	return CreateSTLEnumerator<VarVarEnum>(pVal, static_cast<IDispatch*>(this), m_Coll);
}


STDMETHODIMP CMessage::Set(BSTR field, BSTR value)
{
	m_Impl.Set(field, value);
	return S_OK;
}

STDMETHODIMP CMessage::Get(BSTR field, BSTR* pVal)
{
	wstring v;
	 
	if (m_Impl.Get(field, v))
	{
		CComBSTR _v(v.c_str());
		return _v.CopyTo(pVal);
	}
	else
		return S_FALSE;
}

STDMETHODIMP CMessage::Remove(BSTR field)
{
	m_Impl.Remove(field);
	return S_OK;
}

STDMETHODIMP CMessage::Empty()
{
	m_Impl.Empty();
	return S_OK;
}

STDMETHODIMP CMessage::IsEmpty(VARIANT_BOOL* pVal)
{
	*pVal = m_Impl.IsEmpty() ? VARIANT_TRUE : VARIANT_FALSE;
	return S_OK;
}

STDMETHODIMP CMessage::Copy(IMessage* msg)
{
	if (msg == 0)
		return S_FALSE;

	long pMsgImpl = 0;

	if (FAILED(msg->_GetImpl(&pMsgImpl)))
		return E_INVALIDARG;

	if (pMsgImpl == 0)
		return S_FALSE;

	Message* rhs = reinterpret_cast<Message*>(pMsgImpl);
	m_Impl = *rhs;

	return S_OK;
}

STDMETHODIMP CMessage::_GetImpl(long* pVal)
{
	if (pVal == 0)
		return E_INVALIDARG;

	*pVal = reinterpret_cast<long>(&m_Impl);
	return S_OK;
}

STDMETHODIMP CMessage::GetAsSimpleFormat(BSTR* pVal)
{
	if (pVal == 0)
		return E_INVALIDARG;

	string v = m_Impl.GetAsSimpleFormat();

	CComBSTR _v(v.c_str());
	return _v.CopyTo(pVal);
}

STDMETHODIMP CMessage::InitFromSimple(BSTR str, VARIANT_BOOL* pVal)
{
	if (str == 0)
		return E_INVALIDARG;

	wstring ws = str;

	bool b = m_Impl.InitFromSimple(ConvertToNarrow(ws));

	if (pVal)
		*pVal = b ? VARIANT_TRUE : VARIANT_FALSE;

	return S_OK;
}


