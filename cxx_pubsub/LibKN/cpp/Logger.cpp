#include "stdafx.h"
#include <LibKN\Logger.h>
#include <LibKN\LoggerDefs.h>

const TCHAR* g_LogRegRoot = _T("Software\\KnowNow\\Logger");
const TCHAR* g_LogIsEnabled = _T("IsEnabled");
const TCHAR* g_LogMask = _T("Mask");
const TCHAR* g_LogFile = _T("File");

const TCHAR* g_cmpLogger = _T("Logger");
const TCHAR* g_cmpCppConnector = _T("C++ Connector");
const TCHAR* g_cmpCppTunnel = _T("C++ Tunnel");

Logger& Logger::GetLogger()
{
	CCriticalSection cs;
	LockImpl<CCriticalSection> autoLock(&cs);
	static Logger s_Logger;
	return s_Logger;
}

Logger::Logger() :
	m_TabLevel(0),
	m_OutputBufSize(0),
	m_OutputBuffer(0),
	m_Key(0),
	m_FileHandle(0),
	m_FileMutex(_T("LibKNCppFileMutex"))
{
	ResizeOutputBuffer(1024);

	if (m_Key == 0 && RegOpenKeyEx(HKEY_LOCAL_MACHINE, g_LogRegRoot, 0, KEY_READ, &m_Key) != ERROR_SUCCESS)
	{
		Output(_T("Logger::Logger(): Failed to open registry key for local machine.\r\n"));
		m_Key = 0;
	}
	
	if (m_Key == 0 && RegOpenKeyEx(HKEY_CURRENT_USER, g_LogRegRoot, 0, KEY_READ, &m_Key) != ERROR_SUCCESS)
	{
		Output(_T("Logger::Logger(): Failed to open registry key for current user.\r\n"));
		m_Key = 0;
	}

	if (m_Key != 0)
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
			else
			{
				m_FileHandle = CreateFile(regValue, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
					0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
				if (m_FileHandle == INVALID_HANDLE_VALUE)
				{
					m_FileHandle = CreateFile(regValue, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
						0, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);

					if (m_FileHandle == INVALID_HANDLE_VALUE)
					{
						m_FileHandle = 0;
					}
				}
			}
		}
	}

	Log(_T("Logger"), Mask::Information, _T("Logger started"));
}

Logger::~Logger()
{
	Log(_T("Logger"), Mask::Information, _T("Logger ended"));

	if (m_FileHandle)
	{
		CloseHandle(m_FileHandle);
		m_FileHandle = 0;
	}

	if (m_Key)
	{
		RegCloseKey(m_Key);
		m_Key = 0;
	}

	delete[] m_OutputBuffer;
	m_OutputBuffer = 0;
}

void Logger::ResizeOutputBuffer(int newSize)
{
	if (newSize > m_OutputBufSize)
	{
		delete[] m_OutputBuffer;

		m_OutputBufSize = newSize;
		m_OutputBuffer = new TCHAR[m_OutputBufSize];
	}
}

void Logger::Log(const tstring& component, DWORD mask, const tstring& msg)
{
	if (!IsEnabled())
		return;

	if (GetCurrentMask() & mask)
	{
//		ResizeOutputBuffer(component.length());
//		ResizeOutputBuffer(msg.length());

		TCHAR tmp[1024];
		tstring str;

#if !defined(_WIN32_WCE)
		DWORD size;
		size = sizeof tmp;
		ZeroMemory(tmp, sizeof tmp);
		GetUserName(tmp, &size);
		str += tmp;
		str += _T(" ");

		size = sizeof tmp;
		ZeroMemory(tmp, sizeof tmp);
		GetComputerName(tmp, &size);
		str += tmp;
		str += _T(" ");
#endif

		int n = GetTimeFormat(LOCALE_USER_DEFAULT, TIME_FORCE24HOURFORMAT, 0, 0, tmp, 1024);

		str += tmp;
		str += _T(" ");

		_stprintf(tmp, _T("pid:0x%04X "), GetCurrentProcessId());
		str += tmp;

		_stprintf(tmp, _T("tid:0x%04X "), GetCurrentThreadId());
		str += tmp;

		_stprintf(tmp, _T("mask:0x%04X "), mask);
		str += tmp;

		str += GetTabsForOutput();

		str += _T("[");
		str += component;
		str += _T("] ");
		str += msg;
		str += _T("\r\n");

		Output(str.c_str());
	}
}

void Logger::Output(const TCHAR* m)
{
	OutputDebugString(m);

	if (m_FileHandle)
	{
		m_FileMutex.Enter();
		DWORD pos = SetFilePointer(m_FileHandle, 0, NULL, FILE_END);
		DWORD num_written = 0;
		WriteFile(m_FileHandle, m, _tcslen(m), &num_written, NULL);
		m_FileMutex.Leave();
	}
}

long Logger::IncreaseTabLevel()
{
	Lock autoLock(this);

	m_TabLevel++;

	return m_TabLevel;
}

long Logger::DecreaseTabLevel()
{
	Lock autoLock(this);

	m_TabLevel--;

	if (m_TabLevel < 0)
		m_TabLevel = 0;

	return m_TabLevel;
}

tstring Logger::GetTabsForOutput()
{
	tstring retVal;

	for (long i = 0; i < m_TabLevel; i++)
	{
		retVal += _T("    ");
	}

	return retVal;
}

long Logger::GetCurrentTabLevel()
{
	return m_TabLevel;
}

DWORD Logger::GetCurrentMask()
{
	if (m_Key == 0)
		return Mask::None;

	DWORD type = REG_DWORD;
	DWORD regValue = 0;
	DWORD bufSize = sizeof(regValue);

	if (RegQueryValueEx(m_Key, g_LogMask, 0, &type, (BYTE*)&regValue, &bufSize) == ERROR_SUCCESS)
	{
		if (type != REG_DWORD)
			regValue = 0;
	}

	return regValue;
}

bool Logger::IsEnabled()
{
	if (m_Key == 0)
		return false;

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

	return retVal;
}

