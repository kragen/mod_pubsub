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

// CConnector.cpp : Implementation of CConnector
#include "stdafx.h"
#include "LibKNCom.h"
#include "CConnector.h"
#include "CParameters.h"

/////////////////////////////////////////////////////////////////////////////
// CConnector
const TCHAR* CConnector::GetClassName()
{
	return _T("LibKNConnectorWndClass");
}


CConnector::CConnector() : 
	m_Hwnd(0),
	m_Atom(0)
{
}

LRESULT WINAPI CConnector::WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
	switch (msg)
	{
		case HandlerMsg:
		{
			switch (wp)
			{
				case OnStatusMsg:
				{
					RequestHandlerMsgParam* hmp = reinterpret_cast<RequestHandlerMsgParam*>(lp);
		
					if (hmp == 0)
						return 0;
		
					CComObject<CMessage>* obj = 0;
					HRESULT hr = CComObject<CMessage>::CreateInstance(&obj);
					obj->Copy(hmp->GetMsg());
		
					obj->AddRef();
		
					if (hmp->GetHandler() && hmp->GetHandler()->GetComHandler())
						hmp->GetHandler()->GetComHandler()->OnStatus(obj);
					if (hmp->GetConnector())
						hmp->GetConnector()->Fire_OnStatus(obj);

					obj->Release();
					delete hmp;

					break;
				}

				case OnUpdateMsg:
				{
					ListenerHandlerMsgParam* lmp = reinterpret_cast<ListenerHandlerMsgParam*>(lp);
					if (lmp == 0)
						return 0;

					CComObject<CMessage>* obj = 0;
					HRESULT hr = CComObject<CMessage>::CreateInstance(&obj);
					obj->Copy(lmp->GetMsg());
		
					obj->AddRef();
		
					if (lmp->GetHandler() && lmp->GetHandler()->GetComHandler())
						lmp->GetHandler()->GetComHandler()->OnUpdate(obj);
					if (lmp->GetConnector())
						lmp->GetConnector()->Fire_OnUpdate(obj);

					obj->Release();
					delete lmp;
					break; 
				}


				case OnConnectionStatusMsg:
				{
					ConnectionStatusMsgParam* csmp = reinterpret_cast<ConnectionStatusMsgParam*>(lp);
					if (csmp == 0)
						return 0;

					CComObject<CMessage>* obj = 0;
					HRESULT hr = CComObject<CMessage>::CreateInstance(&obj);
					obj->Copy(csmp->GetMsg());
		
					obj->AddRef();
		
					if (csmp->GetConnector())
						csmp->GetConnector()->Fire_OnConnectionStatus(obj);

					obj->Release();
					delete csmp;
					break; 
				}

				default:
					break;
			}
		}

		default:
			return DefWindowProc(hwnd, msg, wp, lp);
	}

	return 0;
}

void CConnector::FromHandlerImpl(RequestStatusHandlerAdapter* sha, CallbackReason cbReason, const Message& msg)
{
	if (IsWindow(m_Hwnd))
	{
		RequestHandlerMsgParam* hmp = new RequestHandlerMsgParam(this, new Message(msg), sha);
		PostMessage(m_Hwnd, HandlerMsg, cbReason, reinterpret_cast<LPARAM>(hmp));
	}
}

void CConnector::FromHandlerImpl(ListenerHandlerAdapter* sha, CallbackReason cbReason, const Message& msg)
{
	if (IsWindow(m_Hwnd))
	{
		ListenerHandlerMsgParam* lmp = new ListenerHandlerMsgParam(this, new Message(msg), sha);
		PostMessage(m_Hwnd, HandlerMsg, cbReason, reinterpret_cast<LPARAM>(lmp));
	}
}

void CConnector::OnStatusFromHandler(RequestStatusHandlerAdapter* sha, const Message& msg)
{
	FromHandlerImpl(sha, OnStatusMsg, msg);
}

void CConnector::OnUpdateFromHandler(ListenerHandlerAdapter* sha, const Message& msg)
{
	FromHandlerImpl(sha, OnUpdateMsg, msg);
}

void CConnector::OnConnectionStatus(const Message& msg)
{
	if (IsWindow(m_Hwnd))
	{
		ConnectionStatusMsgParam* csmp = new ConnectionStatusMsgParam(this, new Message(msg), 0);
		PostMessage(m_Hwnd, HandlerMsg, OnConnectionStatusMsg, reinterpret_cast<LPARAM>(csmp));
	}
}

HRESULT CConnector::FinalConstruct()
{
	if (m_Atom == 0)
	{
		WNDCLASS wc;
		ZeroMemory(&wc, sizeof WNDCLASS);
		wc.style = 0;
		wc.hInstance = _Module.GetModuleInstance();
		wc.lpszClassName = GetClassName();
		wc.lpfnWndProc = WndProc;
	
		m_Atom = RegisterClass(&wc);
	}

	if (m_Hwnd == 0)
	{
		TCHAR buffer[80];

		wsprintf(buffer, _T("CConWnd%08X"), this);

		m_Hwnd = CreateWindow(GetClassName(), buffer, WS_POPUP, 0, 0, 0, 0,
			0, 0, _Module.GetModuleInstance(), 0);

		if (IsWindow(m_Hwnd))
		{
			ATLTRACE(_T("Successfully created window\r\n"));
		}
		else
		{
			ATLTRACE(_T("Failed to create window\r\n"));
		}
	}

	return S_OK;
}

void CConnector::FinalRelease()
{
	m_Connector.RemoveConnectionStatusHandler(this);

	ClearListenerAdapters();

	if (IsWindow(m_Hwnd))
	{
		DestroyWindow(m_Hwnd);
		m_Hwnd = 0;
	}
}

STDMETHODIMP CConnector::IsConnected(VARIANT_BOOL* pVal)
{
	if (pVal == 0)
		return E_INVALIDARG;

	*pVal = m_Connector.IsConnected() ? VARIANT_TRUE : VARIANT_FALSE;

	return S_OK;
}

STDMETHODIMP CConnector::Open(IParameters* p, VARIANT_BOOL* pVal)
{
	if (p == 0 || pVal == 0)
		return E_INVALIDARG;

	ITransport::Parameters tp = ConvertParameter(p);

	bool b = m_Connector.Open(tp);

	if (b)
	{
		m_Connector.AddConnectionStatusHandler(this);
	}

	*pVal = b ? VARIANT_TRUE : VARIANT_FALSE;

	return S_OK;
}

STDMETHODIMP CConnector::GetParameters(IParameters** p)
{
	if (p == 0)
		return E_INVALIDARG;

	ITransport::Parameters tp = m_Connector.GetParameters();
	ConvertParameter(tp, p);

	return S_OK;
}

void CConnector::ClearListenerAdapters()
{
	for (ListenerAdapters::iterator it = m_ListenerAdapters.begin();
		it != m_ListenerAdapters.end(); ++it)
	{
		delete (*it).second;
	}

	m_ListenerAdapters.clear();
}

STDMETHODIMP CConnector::Close(VARIANT_BOOL* pVal)
{
	if (pVal == 0)
		return E_INVALIDARG;

	*pVal = m_Connector.Close() ? VARIANT_TRUE : VARIANT_FALSE;

	ClearListenerAdapters();
	m_Connector.RemoveConnectionStatusHandler(this);

	return S_OK;
}

STDMETHODIMP CConnector::EnsureConnected(VARIANT_BOOL* pVal)
{
	if (pVal == 0)
		return E_INVALIDARG;

	*pVal = m_Connector.EnsureConnected() ? VARIANT_TRUE : VARIANT_FALSE;
	
	return S_OK;
}

STDMETHODIMP CConnector::Publish(IMessage* m, IComRequestStatusHandler* sh)
{
	if (m == 0)
		return S_OK;

	long l = 0;

	if (FAILED(m->_GetImpl(&l)))
		return E_INVALIDARG;

	if (l == 0)
		return E_INVALIDARG;

	Message* mImpl = reinterpret_cast<Message*>(l);

	m_Connector.Publish(*mImpl, new RequestStatusHandlerAdapter(this, sh));

	return S_OK;
}

STDMETHODIMP CConnector::Subscribe(BSTR topic, IComListener* listener, IComRequestStatusHandler* sh, 
	BSTR* pVal)
{
	if (topic == 0)
		return E_INVALIDARG;

	RequestStatusHandlerAdapter* rsha = new RequestStatusHandlerAdapter(this, sh);
	ListenerHandlerAdapter* la = new ListenerHandlerAdapter(this, listener);

	wstring rid = m_Connector.Subscribe(topic, la, rsha);

	ListenerAdapters::iterator it = m_ListenerAdapters.find(rid);

	if (it != m_ListenerAdapters.end())
	{
		// What to do if it already exists?
		delete (*it).second;
	}

	m_ListenerAdapters[rid] = la;

	CComBSTR v(rid.c_str());
	v.CopyTo(pVal);

	return S_OK;
}


STDMETHODIMP CConnector::Unsubscribe(BSTR rid, IComRequestStatusHandler* sh, VARIANT_BOOL* pVal)
{
	if (rid == 0)
		return E_INVALIDARG;

	ListenerAdapters::iterator it = m_ListenerAdapters.find(rid);

	if (it == m_ListenerAdapters.end())
	{
		if (pVal)
			*pVal = VARIANT_TRUE;

		return S_OK;
	}

	RequestStatusHandlerAdapter* rsha = new RequestStatusHandlerAdapter(this, sh);
	bool b = m_Connector.Unsubscribe(rid, rsha);

	if (pVal)
	{
		*pVal = b ? VARIANT_TRUE : VARIANT_FALSE;
	}

	m_ListenerAdapters.erase(it);

	return S_OK;
}
