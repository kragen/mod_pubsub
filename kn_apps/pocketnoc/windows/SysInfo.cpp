/**
 * PocketNOC - v1.2
 * Author: Paul Sharpe
 *
 * SysInfo.cpp
 *
 * Implementation of the SysInfo class, which collects system stats
 * for posting to a PubSub Server.  Uses the PDH library for
 * Windows 2000.
 */


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


#include "SysInfo.h"


CSysInfo::CSysInfo()
{
	// Clear query and counter.

	m_hQuery = 0;
	m_hCPUCounter = 0;
	

	// Initialize query, if successful, add a counter to keep track
	// of cpu usage.

	if(Initialize()) {
		PdhAddCounter( m_hQuery, CPU_USAGE, 0, &m_hCPUCounter );
	}
}


CSysInfo::~CSysInfo()
{
	// clean up

	Release();
}



BOOL CSysInfo::Initialize()
{
	// Open the pdh query with the query handle

	if(PdhOpenQuery(NULL, 1, &m_hQuery) != ERROR_SUCCESS)
		return false;

	return true;
}



void CSysInfo::Release()
{
	// Close PDH query

	PdhCloseQuery(&m_hQuery);


	// Remove counter to clean up

	PdhRemoveCounter(m_hCPUCounter);	
}



BOOL CSysInfo::GetStats()
{
	// Collect data from the query

	if(PdhCollectQueryData(m_hQuery) != ERROR_SUCCESS)
		return false;


	// Get a formatted value of the cpu usage and store in the corresponding
	// variable.

	if(PdhGetFormattedCounterValue(m_hCPUCounter, PDH_FMT_LONG, 0, &m_CPUUsage) != ERROR_SUCCESS)
		return false;

	
	// Get memory usage stats.

	GetMemoryStats();

	return true;
}


void CSysInfo::GetMemoryStats()
{
	// Memory status object.

	MEMORYSTATUS memoryStatus;


	// Setup memory status's buffer

	memset(&memoryStatus, sizeof(MEMORYSTATUS), 0);
	memoryStatus.dwLength = sizeof(MEMORYSTATUS);


	// Get system memory usage

	GlobalMemoryStatus (&memoryStatus);


	// Get physical memory stats in MB

	m_lMemTotal = memoryStatus.dwTotalPhys / 1048576;
	m_lMemAvail = memoryStatus.dwAvailPhys / 1048576;
	

	// Get swap memory stats in MB

	m_lSwapTotal = memoryStatus.dwTotalPageFile / 1048576;
	m_lSwapAvail = memoryStatus.dwAvailPageFile / 1048576;
}


long CSysInfo::GetCPUUsage()
{
	return m_CPUUsage.longValue;
}


long CSysInfo::GetMemAvail()
{
	return m_lMemAvail;
}


long CSysInfo::GetMemTotal()
{
	return m_lMemTotal;
}


long CSysInfo::GetSwapAvail()
{
	return m_lSwapAvail;
}


long CSysInfo::GetSwapTotal()
{
	return m_lSwapTotal;
}
