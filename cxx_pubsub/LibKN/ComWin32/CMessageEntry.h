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

// MessageEntry.h : Declaration of the CMessageEntry

#ifndef __CMESSAGEENTRY_H_
#define __CMESSAGEENTRY_H_

#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// CMessageEntry
class ATL_NO_VTABLE CMessageEntry : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CMessageEntry, &CLSID_MessageEntry>,
#if !defined(_WIN32_WCE)
	public IObjectSafetyImpl<CMessageEntry, INTERFACESAFE_FOR_UNTRUSTED_CALLER | INTERFACESAFE_FOR_UNTRUSTED_DATA>,
#endif
	public IDispatchImpl<IMessageEntry, &IID_IMessageEntry, &LIBID_LIBKNCOMLib>
{
public:
	CMessageEntry();
	CMessageEntry(CComBSTR f, CComBSTR v);

	void SetFV(CComBSTR f, CComBSTR v);

DECLARE_REGISTRY_RESOURCEID(IDR_MESSAGEENTRY)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CMessageEntry)
#if !defined(_WIN32_WCE)
	COM_INTERFACE_ENTRY(IObjectSafety)
#endif
	COM_INTERFACE_ENTRY(IMessageEntry)
	COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

// IMessageEntry
public:
	STDMETHOD(get_Field)(/*[out, retval]*/ BSTR *pVal);
	STDMETHOD(get_Value)(/*[out, retval]*/ BSTR *pVal);

private:
	CComBSTR m_Field;
	CComBSTR m_Value;
};

#endif //__CMESSAGEENTRY_H_
