// Logger.cpp : Implementation of CLogger
#include "stdafx.h"
#include "LibKNCom.h"
#include "CLogger.h"
#include <LibKN\Logger.h>

/////////////////////////////////////////////////////////////////////////////
// CLogger

CLogger::CLogger()
{
}

STDMETHODIMP CLogger::IncreaseTab(long* tl)
{
	long _tl = Logger::GetLogger().IncreaseTabLevel();

	if (*tl)
		*tl = _tl;

	return S_OK;
}

STDMETHODIMP CLogger::DecreaseTab(long* tl)
{
	long _tl = Logger::GetLogger().DecreaseTabLevel();

	if (*tl)
		*tl = _tl;

	return S_OK;
}

tstring ConvertString(BSTR b);

STDMETHODIMP CLogger::Log(BSTR component, long mask, BSTR msg)
{
	tstring c = ConvertString(component);
	tstring m = ConvertString(msg);

	Logger::GetLogger().Log(c, (DWORD)mask, m);

	return S_OK;
}

