/**
 * PocketNOC - v1.2
 * Author: Paul Sharpe
 *
 * SysInfo.h
 *
 * Interface to the SysInfo class, which collects system stats
 * for posting to a PubSub Server.  Uses the PDH library for
 * Windows 2000.
 */

#include <pdh.h>
#include <pdhmsg.h>
#include <memory.h>


// Registry location for cpu usage

#define CPU_USAGE			"\\Processor(_Total)\\% Processor Time"

/**
 * A class that contains the methods to get system usage statistics
 * and the variables to store them in.
 */

class CSysInfo  
{
public:

	/**
	 * CTOR 
	 *
	 * Sets up the pdh query
	 */

	CSysInfo();

	
	/**
	 * DTOR
	 */

	virtual ~CSysInfo();


	/**
	 * Collects system statistics and places the results in the appropriate
	 * variables.
	 *
	 * @return Boolean stating whether the collection succeeded for failed.
	 */

	BOOL GetStats();


	/**
	 * @return CPU usage 
	 */

	long GetCPUUsage();
	
	
	/**
	 * @return Total physical memory in MB
	 */

	long GetMemTotal();

	/**
	 * @return Total physical memory available in MB
	 */

	long GetMemAvail();


	/**
	 * @return Total swap memory in MB
	 */

	long GetSwapTotal();
	

	/**
	 * @return Total swap memory available in MB
	 */

	long GetSwapAvail();

protected:

	/**
	 * Opens pdh query
	 *
	 * @return Boolean stating whether the opening succeeded or not
	 */

	BOOL Initialize();


	/**
	 * Closes pdh query and cleans up counters from memory.
	 */

	void Release();


	/**
	 * Gets memory usage stats and stores them in the class's corresponding
	 * variables.
	 */

	void GetMemoryStats();


	/**
	 * PDH query for getting cpu usage
	 */ 

	HQUERY					m_hQuery;


	/**
	 * PDH counter for cpu usage query
	 */

	HCOUNTER				m_hCPUCounter, m_hMemAvailCounter;


	/**
	 * Formatted value of cpu usage.
	 */

	PDH_FMT_COUNTERVALUE	m_CPUUsage;


	/**
	 * longs to store memory and swap usage
	 */

	long	m_lMemTotal, m_lMemAvail, m_lSwapTotal, m_lSwapAvail;
};
