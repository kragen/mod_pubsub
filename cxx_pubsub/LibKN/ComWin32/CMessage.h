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

// CMessage.h : Declaration of the CMessage

#ifndef __MESSAGE_H_
#define __MESSAGE_H_

#include "resource.h"       // main symbols

#include <LibKN\Message.h>
#include "CMessageEntry.h"

typedef CComEnumOnSTL<IEnumVARIANT, &IID_IEnumVARIANT, VARIANT,
	_Copy<VARIANT>, vector<CComVariant> > VarVarEnum;

/////////////////////////////////////////////////////////////////////////////
// CMessage
class ATL_NO_VTABLE CMessage : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CMessage, &CLSID_Message>,
#if !defined(_WIN32_WCE)
	public IObjectSafetyImpl<CMessage, INTERFACESAFE_FOR_UNTRUSTED_CALLER | INTERFACESAFE_FOR_UNTRUSTED_DATA>,
#endif
	public IDispatchImpl<IMessage, &IID_IMessage, &LIBID_LIBKNCOMLib>
{
public:
	CMessage();

	void Copy(Message* rhs);

DECLARE_REGISTRY_RESOURCEID(IDR_MESSAGE)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CMessage)
#if !defined(_WIN32_WCE)
	COM_INTERFACE_ENTRY(IObjectSafety)
#endif
	COM_INTERFACE_ENTRY(IMessage)
	COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

// IMessage
public:
	STDMETHOD(Copy)(/*[in]*/ IMessage* msg);
	STDMETHOD(get__NewEnum)(IUnknown** pVal);
	STDMETHOD(IsEmpty)(/*[out, retval]*/ VARIANT_BOOL* pVal);
	STDMETHOD(Empty)();
	STDMETHOD(Remove)(/*[in]*/ BSTR field);
	STDMETHOD(Get)(/*[in]*/ BSTR field, /*[out, retval]*/ BSTR* pVal);
	STDMETHOD(Set)(/*[in]*/ BSTR field, /*[in]*/ BSTR value);
	STDMETHOD(GetAsSimpleFormat)(BSTR* pVal);
	STDMETHOD(InitFromSimple)(BSTR str, VARIANT_BOOL* pVal);
	STDMETHOD(_GetImpl)(long* pVal);

private:
	Message m_Impl;
	vector<CComVariant> m_Coll;
};

#endif //__MESSAGE_H_
