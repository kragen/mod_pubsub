#include "stdafx.h"
#include "LoggerCfg.h"
#include "MainWnd.h"
#include <LibKN\LoggerDefs.h>

long CreateKey(HKEY root, HKEY* key)
{
	DWORD dispo = 0;
	return RegCreateKeyEx(root, g_LogRegRoot, 0, 0, REG_OPTION_NON_VOLATILE, KEY_READ | KEY_WRITE, 0, key, &dispo);
}

TMainWindow::TMainWindow() :
	m_Key(0)
{
	m_RefreshMsg = ::RegisterWindowMessage(_T("KNLoggerCfgRefresh"));

	LONG result = 0;

	if (m_Key == 0 && CreateKey(HKEY_LOCAL_MACHINE, &m_Key) != ERROR_SUCCESS)
	{
		m_Key = 0;
	}

	if (m_Key != 0)
	{
		OutputDebugString(_T("Now using local machine\r\n"));
	}
	else
	{
		if (CreateKey(HKEY_CURRENT_USER, &m_Key) != ERROR_SUCCESS)
		{
			m_Key = 0;
		}
		else
		{
			OutputDebugString(_T("Now using current user\r\n"));
		}
	}
}

TMainWindow::~TMainWindow()
{
	if (m_Key != 0)
	{
		RegCloseKey(m_Key);
		m_Key = 0;
	}
}

void TMainWindow::OnSize(UINT nType, CSize size)
{
	Resize(nType, size);
}

BOOL TMainWindow::OnCreate(LPCREATESTRUCT cs)
{
	if (m_Key == 0)
	{
		MessageBox(_T("Cannot open registry key. Now exiting."), _T("Fail\n"), MB_OK | MB_ICONEXCLAMATION);
		PostMessage(WM_CLOSE);
		return FALSE;
	}

	if (m_RefreshMsg == 0)
	{
		MessageBox(_T("Error calling RegisterWindowMessage(). Now exiting."), _T("Fail\n"), MB_OK | MB_ICONEXCLAMATION);
		return FALSE;
	}

	DWORD childStyle = WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;

	TLayoutMetrics lm;

	m_stEnabled.Create(*this, 0, _T(" "), childStyle);
	lm.X.SameAs(lmParent, lmLeft);
	lm.Y.SameAs(lmParent, lmTop);
	lm.Width.Absolute(ButtonWidth);
	lm.Height.Absolute(ButtonHeight);
	SetChildLayoutMetrics(m_stEnabled, lm);

	m_chkEnabled.Create(*this, 0, _T("Enabled"), childStyle | WS_TABSTOP | BS_CHECKBOX, 0, EnabledId);
	lm.X.RightOf(m_stEnabled);
	lm.Y.SameAs(m_stEnabled, lmTop);
	lm.Width.Absolute(100);
	lm.Height.Absolute(ButtonHeight);
	SetChildLayoutMetrics(m_chkEnabled, lm);

	m_stFile.Create(*this, 0, _T("File:"), childStyle);
	lm.X.SameAs(lmParent, lmLeft);
	lm.Y.Below(m_stEnabled);
	lm.Width.Absolute(ButtonWidth);
	lm.Height.Absolute(ButtonHeight);
	SetChildLayoutMetrics(m_stFile, lm);

	m_File.Create(*this, 0, _T("File"), childStyle | WS_TABSTOP | WS_BORDER | ES_AUTOHSCROLL, 0, FileId);
	lm.X.RightOf(m_stFile);
	lm.Y.SameAs(m_stFile, lmTop);
	lm.Width.SameAs(lmParent, lmRight);
	lm.Height.Absolute(ButtonHeight);
	SetChildLayoutMetrics(m_File, lm);

	m_stMask.Create(*this, 0, _T("Mask:"), childStyle);
	lm.X.SameAs(lmParent, lmLeft);
	lm.Y.Below(m_stFile);
	lm.Width.Absolute(ButtonWidth);
	lm.Height.Absolute(ButtonHeight);
	SetChildLayoutMetrics(m_stMask, lm);

	m_Mask.Create(*this, 0, _T("Mask"), childStyle | WS_TABSTOP | WS_BORDER | ES_AUTOHSCROLL, 0, MaskId);
	lm.X.RightOf(m_stMask);
	lm.Y.SameAs(m_stMask, lmTop);
//	lm.Width.Absolute(180);
	lm.Width.SameAs(lmParent, lmRight);
	lm.Height.Absolute(ButtonHeight);
	SetChildLayoutMetrics(m_Mask, lm);

	m_ExitButton.Create(*this, NULL, _T("Close"), childStyle | WS_TABSTOP, 0, ExitId);
	lm.X.SameAs(lmParent, lmCenter);
	lm.Y.Below(m_Mask);
	lm.Width.Absolute(ButtonWidth);
	lm.Height.Absolute(ButtonHeight);
	SetChildLayoutMetrics(m_ExitButton, lm);

	m_chkEnabled.SetFocus();
	UpdateFromRegistry();

	return TRUE;
}

void TMainWindow::OnDestroy()
{
	PostQuitMessage(0);
}

void TMainWindow::OnExit(UINT code, int id, HWND ctl)
{
	DestroyWindow();
}

void TMainWindow::OnEnabled(UINT code, int id, HWND ctl)
{
	BOOL b = !m_chkEnabled.GetCheck();

	m_chkEnabled.SetCheck(b);

	{
		DWORD type = REG_DWORD;
		DWORD regValue = b;
		DWORD bufSize = sizeof(regValue);

		RegSetValueEx(m_Key, g_LogIsEnabled, 0, type, (BYTE*)&regValue, bufSize);
	}

	SendRefreshMsg();
}

void TMainWindow::OnFileKillFocus(UINT code, int id, HWND ctl)
{
	{
		DWORD type = REG_SZ;
		DWORD bufSize = m_File.GetWindowTextLength() + sizeof(TCHAR);
		TCHAR* regValue = new TCHAR[bufSize];
		m_File.GetWindowText(regValue, bufSize);

		RegSetValueEx(m_Key, g_LogFile, 0, type, (BYTE*)regValue, bufSize);

		delete[] regValue;
	}
	
	SendRefreshMsg();
}

void TMainWindow::OnMaskKillFocus(UINT code, int id, HWND ctl)
{
	{
		DWORD type = REG_DWORD;
		DWORD bufSize = m_File.GetWindowTextLength() + sizeof(TCHAR);
		TCHAR* regValue = new TCHAR[bufSize];
		m_Mask.GetWindowText(regValue, bufSize);
		DWORD mask = 0;

		if (_stscanf(regValue, _T("0x%X"), &mask) == 1)
		{
			RegSetValueEx(m_Key, g_LogMask, 0, type, (BYTE*)&mask, sizeof(mask));
		}

		delete[] regValue;
	}
	
	SendRefreshMsg();
}


LRESULT TMainWindow::OnRefreshMsg(UINT u, WPARAM wp, LPARAM lp)
{
	UpdateFromRegistry();
	return 0;
}

void TMainWindow::OnLButtonUp(UINT u, CPoint p)
{
	UpdateFromRegistry();
}

void TMainWindow::SendRefreshMsg()
{
	::PostMessage(HWND_BROADCAST, m_RefreshMsg, 0, 0);
}

void TMainWindow::OnPaint(HDC)
{
	PAINTSTRUCT ps;
	HDC dc = BeginPaint(&ps);
	EndPaint(&ps);
}

BOOL TMainWindow::PreTranslateMessage(MSG* pMsg)
{
	return IsDialogMessage(pMsg);
}

void TMainWindow::UpdateFromRegistry()
{
	{
		DWORD type = REG_DWORD;
		DWORD regValue = 0;
		DWORD bufSize = sizeof(regValue);

		bool retVal = false;

		if (RegQueryValueEx(m_Key, g_LogIsEnabled, 0, &type, (BYTE*)&regValue, &bufSize) == ERROR_SUCCESS)
		{
			if (type == REG_DWORD)
			{
				retVal = (regValue == 0) ? false : true;
			}
		}

		m_chkEnabled.SetCheck(retVal);
	}

	{
		DWORD type = REG_SZ;
		TCHAR regValue[1024];
		DWORD bufSize = sizeof(regValue);
		ZeroMemory(regValue, bufSize);

		if (RegQueryValueEx(m_Key, g_LogFile, 0, &type, (BYTE*)&regValue, &bufSize) == ERROR_SUCCESS)
		{
			if (type != REG_SZ)
			{
				ZeroMemory(regValue, bufSize);
			}

		}
		
		m_File.SetWindowText(regValue);
	}
	
	{
		DWORD type = REG_DWORD;
		DWORD regValue = 0;
		DWORD bufSize = sizeof(regValue);

		if (RegQueryValueEx(m_Key, g_LogMask, 0, &type, (BYTE*)&regValue, &bufSize) == ERROR_SUCCESS)
		{
			if (type != REG_DWORD)
				regValue = 0;
		}


		TCHAR buffer[32];
		_stprintf(buffer, _T("0x%08X"), regValue);
		m_Mask.SetWindowText(buffer);
	}
}

