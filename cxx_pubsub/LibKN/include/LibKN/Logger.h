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

#if !defined(LIBKN_LOGGER_H)
#define LIBKN_LOGGER_H

#include <LibKN\Defs.h>
#include <LibKN\CS.h>
#include <LibKN\LoggerDefs.h>

/**
 * This singleton class logs information to a file.
 * Since the application cannot create an instance of this class, the
 * only way to get an instance is through the static GetLogger() method.
 */
class Logger : public CCriticalSection
{
public:
	typedef LockImpl<Logger> Lock;

	// Note that any changes to this must be kept in sync with the .IDL file
	// and the .Net Component.
	/**
	 * Mask for output
	 */
	enum Mask
	{
		None = 0x0000,
		Critical = 0x0001,
		Warning = 0x0002,
		Information = 0x0004,
		NetworkTraffic = 0x0010,
		MethodEntryExit = 0x0020,
		TunnelInfo = 0x0040,
		All = 0xFFFFFFFF,
	};

	/**
	 * \return The one instance of the logger.
	 */
	static Logger& GetLogger();

	~Logger();

	/**
	 * Logs the message.
	 */
	void Log(const tstring& component, DWORD mask, const tstring& msg);

	/**
	 * Increases the tab level for the output messages. The default is 0.
	 * \return The new tab level.
	 */
	long IncreaseTabLevel();
	
	/**
	 * Decreases the tab level for the output messages. The default is 0.
	 * The lowest tab level is 0.
	 * \return The new tab level.
	 */
	long DecreaseTabLevel();

	/**
	 * This class automatically increments the tab level for the logger
	 * on construction and decrements the tab level on destruction.
	 */
	class TabIncr
	{
	public:
		TabIncr(Logger& l);
		~TabIncr();
	
	private:
		TabIncr(const TabIncr& rhs);
		TabIncr& operator=(const TabIncr& rhs);

		Logger& m_Logger;
	};

	/**
	 * This class automatically logs the component and method name and
	 * increments the tab level for the logger on construction.
	 * It also logs the same information and decrements the tab level on destruction.
	 */
	class MethodInfo
	{
	public:
		MethodInfo(Logger& l, const tstring& component, const tstring& methodname);
		~MethodInfo();

	private:
		MethodInfo(const MethodInfo& rhs);
		MethodInfo& operator=(const MethodInfo& rhs);

		tstring m_Component;
		tstring m_MethodName;
		Logger& m_Logger;
	};

private:
	// Prevent anyone from creating an instance
	//
	Logger();

	// Prevent anyone from copying an instance. Not implemented.
	//
	Logger(const Logger& rhs);

	// Return the current mask level. 
	// This needs to be dynamic so that it can be changed on the fly to affect running behavior.
	//
	DWORD GetCurrentMask();

	// Returns the current tab level.
	//
	long GetCurrentTabLevel();

	tstring GetTabsForOutput();

	// Returns whether the logging is enabled or disabled. 
	// This needs to be dynamic so that it can be changed on the fly to affect running behavior.
	//
	bool IsEnabled();

	void ResizeOutputBuffer(int newSize);
	void Output(const TCHAR* m);

private:
	long m_TabLevel;
	TCHAR* m_OutputBuffer;
	int m_OutputBufSize;

	HKEY m_Key;
	HANDLE m_FileHandle;
	CMutex m_FileMutex;
};

inline Logger::TabIncr::TabIncr(Logger& l) : 
	m_Logger(l)
{
	m_Logger.IncreaseTabLevel();
}
	
inline Logger::TabIncr::~TabIncr()
{
	m_Logger.DecreaseTabLevel();
}

inline Logger::MethodInfo::MethodInfo(Logger& l, const tstring& component, const tstring& methodname) :
	m_Logger(l),
	m_Component(component),
	m_MethodName(methodname)
{
	tstring tmp = _T("Entering ");
	tmp += m_MethodName;
	m_Logger.Log(m_Component, Mask::MethodEntryExit, tmp);
	m_Logger.IncreaseTabLevel();
}

inline Logger::MethodInfo::~MethodInfo()
{
	m_Logger.DecreaseTabLevel();

	tstring tmp = _T("Leaving ");
	tmp += m_MethodName;
	m_Logger.Log(m_Component, Mask::MethodEntryExit, tmp);
}

#define AutoTabIncr() Logger::TabIncr LogTemp___instance____(Logger::GetLogger())
#if (_MSC_VER < 1300 || _MSC_VER > 1300)
#	define AutoMethod(c, m) Logger::MethodInfo LogTemp___instance2____(Logger::GetLogger(), c, m)
#else
#	define AutoMethod(c, m) { Logger::MethodInfo LogTemp___instance2____(Logger::GetLogger(), c, m); }
#endif
#define TheLogger Logger::GetLogger()

#endif
