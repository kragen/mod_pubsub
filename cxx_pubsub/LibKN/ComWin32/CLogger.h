// Logger.h : Declaration of the CLogger

#ifndef __LOGGER_H_
#define __LOGGER_H_

#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// CLogger
class ATL_NO_VTABLE CLogger : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CLogger, &CLSID_Logger>,
	public IDispatchImpl<ILogger, &IID_ILogger, &LIBID_LIBKNCOMLib>
{
public:
	CLogger();

DECLARE_REGISTRY_RESOURCEID(IDR_LOGGER)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CLogger)
	COM_INTERFACE_ENTRY(ILogger)
	COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

// ILogger
public:
	STDMETHOD(Log)(/*[in]*/ BSTR component, /*[in]*/ long mask, /*[in]*/ BSTR msg);
	STDMETHOD(DecreaseTab)(/*[out, retval]*/ long* tl);
	STDMETHOD(IncreaseTab)(/*[out, retval]*/ long* tl);
};

#endif //__LOGGER_H_
