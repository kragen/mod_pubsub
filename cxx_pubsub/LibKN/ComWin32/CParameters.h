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

// CParameters.h : Declaration of the CParameters

#ifndef __PARAMETERS_H_
#define __PARAMETERS_H_

#include "resource.h"       // main symbols

#include <LibKN\ITransport.h>

extern ITransport::Parameters ConvertParameter(IParameters* p);
extern void ConvertParameter(const ITransport::Parameters& p, IParameters** pVal);

/////////////////////////////////////////////////////////////////////////////
// CParameters
class ATL_NO_VTABLE CParameters : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CParameters, &CLSID_Parameters>,
#if !defined(_WIN32_WCE)
	public IObjectSafetyImpl<CParameters, INTERFACESAFE_FOR_UNTRUSTED_CALLER | INTERFACESAFE_FOR_UNTRUSTED_DATA>,
#endif
	public IDispatchImpl<IParameters, &IID_IParameters, &LIBID_LIBKNCOMLib>
{
public:
	CParameters();
	CParameters(const ITransport::Parameters& p);

DECLARE_REGISTRY_RESOURCEID(IDR_PARAMETERS)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CParameters)
#if !defined(_WIN32_WCE)
	COM_INTERFACE_ENTRY(IObjectSafety)
#endif
	COM_INTERFACE_ENTRY(IParameters)
	COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

// IParameters
public:
	STDMETHOD(get_ShowUI)(/*[out, retval]*/ VARIANT_BOOL *pVal);
	STDMETHOD(put_ShowUI)(/*[in]*/ VARIANT_BOOL newVal);
	STDMETHOD(get_CustomHeader)(/*[out, retval]*/ BSTR *pVal);
	STDMETHOD(put_CustomHeader)(/*[in]*/ BSTR newVal);
	STDMETHOD(get_ProxyExceptionList)(/*[out, retval]*/ BSTR *pVal);
	STDMETHOD(put_ProxyExceptionList)(/*[in]*/ BSTR newVal);
	STDMETHOD(get_ProxyPassword)(/*[out, retval]*/ BSTR *pVal);
	STDMETHOD(put_ProxyPassword)(/*[in]*/ BSTR newVal);
	STDMETHOD(get_ProxyUsername)(/*[out, retval]*/ BSTR *pVal);
	STDMETHOD(put_ProxyUsername)(/*[in]*/ BSTR newVal);
	STDMETHOD(get_ProxyServer)(/*[out, retval]*/ BSTR *pVal);
	STDMETHOD(put_ProxyServer)(/*[in]*/ BSTR newVal);
	STDMETHOD(get_UseProxy)(/*[out, retval]*/ VARIANT_BOOL *pVal);
	STDMETHOD(put_UseProxy)(/*[in]*/ VARIANT_BOOL newVal);
	STDMETHOD(get_Password)(/*[out, retval]*/ BSTR *pVal);
	STDMETHOD(put_Password)(/*[in]*/ BSTR newVal);
	STDMETHOD(get_Username)(/*[out, retval]*/ BSTR *pVal);
	STDMETHOD(put_Username)(/*[in]*/ BSTR newVal);
	STDMETHOD(get_ServerUrl)(/*[out, retval]*/ BSTR *pVal);
	STDMETHOD(put_ServerUrl)(/*[in]*/ BSTR newVal);

private:
	ITransport::Parameters m_Impl;
};

#endif //__PARAMETERS_H_
