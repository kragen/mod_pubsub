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

// CConnector.h : Declaration of the CConnector

#ifndef __CONNECTOR_H_
#define __CONNECTOR_H_

#include "resource.h"       // main symbols
#include <LibKN\Connector.h>
#include "StatusHandlerAdapter.h"
#include "CMessage.h"
#include "LibKNComCP.h"

class CConnector;

template <class T, bool b>
class MsgParam
{
public:
	MsgParam(CConnector* c, Message* msg, T* sha) :
		m_Connector(c),
		m_Msg(msg),
		m_HandlerAdapter(sha)
	{
	}
	~MsgParam()
	{
		if (m_Msg)
		{
			delete m_Msg;
			m_Msg = 0;
		}

		if (b && m_HandlerAdapter)
		{
			delete m_HandlerAdapter;
			m_HandlerAdapter = 0;
		}
	}

	Message* GetMsg()
	{
		return m_Msg;
	}

	T* GetHandler()
	{
		return m_HandlerAdapter;
	}

	CConnector* GetConnector()
	{
		return m_Connector;
	}

private:
	CConnector* m_Connector;
	Message* m_Msg;
	T* m_HandlerAdapter;
};

typedef MsgParam<RequestStatusHandlerAdapter, true> RequestHandlerMsgParam;
typedef MsgParam<ListenerHandlerAdapter, false> ListenerHandlerMsgParam;
typedef MsgParam<int, false> ConnectionStatusMsgParam;

#if 0
class RequestHandlerMsgParam
{
public:
	RequestHandlerMsgParam(CConnector* c, Message* msg, RequestStatusHandlerAdapter* sha);
	~RequestHandlerMsgParam();

	Message* GetMsg();
	RequestStatusHandlerAdapter* GetHandler();
	CConnector* GetConnector();

private:
	CConnector* m_Connector;
	Message* m_Msg;
	RequestStatusHandlerAdapter* m_HandlerAdapter;
};
#endif


/////////////////////////////////////////////////////////////////////////////
// CConnector
class ATL_NO_VTABLE CConnector : 
	public IConnectionStatusHandler,
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CConnector, &CLSID_Connector>,
	public IConnectionPointContainerImpl<CConnector>,
	public IDispatchImpl<IConnector, &IID_IConnector, &LIBID_LIBKNCOMLib>,
	public IProvideClassInfo2Impl<&CLSID_Connector, &DIID__IConnectorEvents, &LIBID_LIBKNCOMLib, 1, 0>,
#if !defined(_WIN32_WCE)
	public IObjectSafetyImpl<CConnector, INTERFACESAFE_FOR_UNTRUSTED_CALLER | INTERFACESAFE_FOR_UNTRUSTED_DATA>,
#endif
	public CProxy_IConnectorEvents< CConnector >
{
public:
	CConnector();

	static const TCHAR* GetClassName();

	HRESULT FinalConstruct();
	void FinalRelease();

DECLARE_REGISTRY_RESOURCEID(IDR_CONNECTOR)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CConnector)
#if !defined(_WIN32_WCE)
	COM_INTERFACE_ENTRY(IObjectSafety)
#endif
	COM_INTERFACE_ENTRY(IProvideClassInfo2)
	COM_INTERFACE_ENTRY(IProvideClassInfo)
	COM_INTERFACE_ENTRY(IConnector)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(IConnectionPointContainer)
	COM_INTERFACE_ENTRY_IMPL(IConnectionPointContainer)
END_COM_MAP()

BEGIN_CONNECTION_POINT_MAP(CConnector)
CONNECTION_POINT_ENTRY(DIID__IConnectorEvents)
END_CONNECTION_POINT_MAP()

// IConnector
public:
	STDMETHOD(Subscribe)(/*[in]*/ BSTR topic, /*[in]*/ IComListener* listener, IMessage* options, /*[in]*/ IComRequestStatusHandler* sh, BSTR* pVal);
	STDMETHOD(Publish)(/*[in]*/ IMessage* m, /*[in]*/ IComRequestStatusHandler* sh, VARIANT_BOOL* pVal);
	STDMETHOD(EnsureConnected)(/*[out, retval]*/ VARIANT_BOOL* pVal);
	STDMETHOD(Close)(/*[out, retval]*/ VARIANT_BOOL* pVal);
	STDMETHOD(GetParameters)(/*[out, retval]*/ IParameters** p);
	STDMETHOD(Open)(/*[in]*/ IParameters* p, VARIANT_BOOL* pVal);
	STDMETHOD(IsConnected)(/*[out, retval]*/ VARIANT_BOOL* pVal);
	STDMETHOD(Unsubscribe)(/*[in]*/ BSTR rid, /*[in]*/ IComRequestStatusHandler* sh, /*[out, retval]*/ VARIANT_BOOL* pVal);
	STDMETHOD(get_Queueing)(/*[out, retval]*/ VARIANT_BOOL* pVal);
	STDMETHOD(put_Queueing)(/*[in]*/ VARIANT_BOOL newVal);
	STDMETHOD(SaveQueue)(/*[in]*/ BSTR filename, /*[out, retval]*/ VARIANT_BOOL* pVal);
	STDMETHOD(LoadQueue)(/*[in]*/ BSTR filename, /*[out, retval]*/ VARIANT_BOOL* pVal);
	STDMETHOD(Clear)(/*[out, retval]*/ VARIANT_BOOL* pVal);
	STDMETHOD(Flush)(/*[out, retval]*/ VARIANT_BOOL* pVal);
	STDMETHOD(HasItems)(/*[out, retval]*/ VARIANT_BOOL* pVal);

public:
	void OnStatusFromHandler(RequestStatusHandlerAdapter* sha, const Message& msg);
	void OnUpdateFromHandler(ListenerHandlerAdapter* sha, const Message& msg);

private:
	void OnConnectionStatus(const Message& msg);

	enum
	{
		HandlerMsg = WM_USER + 0x1234,
	};

	enum CallbackReason
	{
		OnStatusMsg,
		OnUpdateMsg,
		OnConnectionStatusMsg,
	};

	void FromHandlerImpl(RequestStatusHandlerAdapter* sha, CallbackReason cbReason, const Message& msg);
	void FromHandlerImpl(ListenerHandlerAdapter* sha, CallbackReason cbReason, const Message& msg);

private:
	Connector m_Connector;

	HWND m_Hwnd;
	ATOM m_Atom;

	static LRESULT WINAPI WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);

	typedef map<wstring, ListenerHandlerAdapter*> ListenerAdapters;
	ListenerAdapters m_ListenerAdapters;

	void ClearListenerAdapters();
};

#endif //__CONNECTOR_H_
