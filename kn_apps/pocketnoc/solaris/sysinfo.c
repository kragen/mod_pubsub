/**
 * sysinfo.c - v1.0
 * author: Paul Sharpe
 *
 * Contains the functions to retrieve system statistics
 * and publish them as PubSub events.
 *
 * These functions are for Solaris 7 or greater ONLY.
 **/

/**
 * Copyright 2001-2004 KnowNow, Inc.  All rights reserved.
 *
 * @KNOWNOW_LICENSE_START@
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 
 * 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 * 
 * 3. Neither the name of the KnowNow, Inc., nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * @KNOWNOW_LICENSE_END@
 * 
 **/

#include <sys/kstat.h>
#include <sys/cpuvar.h>
#include <sys/swap.h>
#include <sys/loadavg.h>
#include <vm/anon.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <kstat.h>
#include <utmpx.h>

#include "sysinfo.h"


/* kstat structs for getting cpu stats */

static kstat_ctl_t *kc          = 0;
static kstat_t     **cpuKspList = 0;

/* int for number of cpus */

static int         numOfCpus    = 0;


/* InitSys
 *
 * Setup the kstat structs and info for usage.
 */

void InitSys(void) 
{
    /* kstat struct to contain cpu info. */

    kstat_t *ksp = 0;

    /* ints for init'ing the kstat_ctl and for loops. */

    static int bootstrap = 1;
    int i = 0;

    /* init the kstat_ctl struct. */

    if(bootstrap) {
        if ((kc = kstat_open()) == NULL) {
            perror("can't open /dev/kstat");
            exit(1);
        }

        bootstrap = 0;
    }
    
    /* get the number of cpus. */

    for(i = 0, ksp = kc->kc_chain; ksp; ksp = ksp->ks_next) {
        if(strcmp(ksp->ks_module, "cpu_stat") == 0) { 
            i++; 
        }

        numOfCpus = i;
    }
    
    /* clear cpuKspList. */

    if(cpuKspList) { 
        free(cpuKspList); 
    }
    
    /* create array of cpu stats called cpuKspList. */

    cpuKspList = (kstat_t **) calloc(numOfCpus * sizeof(kstat_t *), 1);
    
    /* get ksp for each cpu, store in cpuKspList. */

    for(i = 0, ksp = kc->kc_chain; ksp; ksp = ksp->ks_next) {
        if(strcmp(ksp->ks_module, "cpu_stat") == 0) {
            cpuKspList[i] = ksp;
            i++;
        }
    }
}


/* CpuUsage
 *
 * Gets cpu usage percentages from kstat info and puts them in the
 * PubSub dictionary passed to the function.
 */

void CpuUsage(knevent_mref knEvent) 
{
    /* ints to store cpu usage info and one for the for loop. */

    static int prevTotal = 0, prevUsed = 0; 
    int total = 0, used = 0, idle = 0, i = 0, cpuUsage = 0;

    /* cpu stat struct to put kstat info into. */

    cpu_stat_t stat;

    /* Temp string to store results for putting in the PubSub dictionary. */

    char tmpStr[1024];

    knstr_ref headerValue;

    /* Check to see if kstat_ctl has been setup. */

    if(kc == NULL || kstat_chain_update(kc)) { 
        InitSys(); 
    }

    /* Get usage stats for each cpu and add them to our used and idle ints. */

    for(i = 0; i < numOfCpus; i++) {
        if(kstat_read(kc, cpuKspList[i], (void *)&stat) == -1) {
            InitSys();
            /* return (CpuUsage(knEvent)); */
            CpuUsage(knEvent);
        }
        
        used += stat.cpu_sysinfo.cpu[CPU_USER];
        used += stat.cpu_sysinfo.cpu[CPU_KERNEL];
        used += stat.cpu_sysinfo.cpu[CPU_WAIT];
        idle += stat.cpu_sysinfo.cpu[CPU_IDLE];
    }

    /* Total of all the cpu usage. */

    total = used + idle;

    /* Get an actual percentage. */

    cpuUsage = 100 * (double)(used - prevUsed) / 
      (double)(total - prevTotal);
    
    prevTotal = total;
    prevUsed = used;

    /* Copy results into temp string and add them to the PubSub
     * dictionary as the "cpuusage" parameter.
     */

    sprintf(tmpStr, "%d", cpuUsage);
    
    headerValue = knstr_create_with_cstr(tmpStr);

    knevent_set(knEvent, KNSTR("cpuusage"), headerValue);

    kn_release(headerValue);
}


/*
 * MemoryUsage
 *
 * Gets memory usage stats from sysconf() and puts them into the PubSub
 * dictionary passed to the function.
 */

void MemoryUsage(knevent_mref knEvent)
{
    /* Temp string to store results for putting in the PubSub dictionary. */

    char tmpStr[1024];

    knstr_ref headerValue;

    
    /* Page size multiplier to convert to bytes. */

    uint_t pages_multiplier = sysconf(_SC_PAGESIZE);
    
    /* Total memory converted to megabytes. */

    uint64_t memTotal = ((uint64_t)sysconf(_SC_PHYS_PAGES) * pages_multiplier) / 1048576;
    
    /* Free memory converted to megabytes. */

    uint64_t memFree = ((uint64_t)sysconf(_SC_AVPHYS_PAGES) * pages_multiplier) / 1048576;
    
    /* Copy results into the temp string and add them to the PubSub dictionary
     * as the "memtotal" and "memfree" parameters.
     */

    sprintf(tmpStr, "%llu", memTotal);

    headerValue = knstr_create_with_cstr(tmpStr);

    knevent_set(knEvent, KNSTR("memtotal"), headerValue);

    sprintf(tmpStr, "%llu", memFree);

    headerValue = knstr_create_with_cstr(tmpStr);

    knevent_set(knEvent, KNSTR("memfree"), headerValue);

    kn_release(headerValue);
}



/*
 * SwapUsage
 *
 * Uses sysconf() and swapctl() to put swap usage results into the PubSub
 * dictionary passed to the function.
 */

void SwapUsage(knevent_mref knEvent)
{
    /* Vars for total and free swap. */

    uint64_t swapTotal, swapFree;
    
    /* Anonymous page info struct. */

    struct anoninfo anoninfo;
    
    /* Temp string to hold the stats. */

    char tmpStr[1024];

    knstr_ref headerValue;

    /* Page size multiplier to convert to bytes. */

    uint_t pages_multiplier = sysconf(_SC_PAGESIZE);
    
    /* Fill out anoninfo, if error, just set values of vars to 
     * 1 to avoid problems.
     */

    if(swapctl(SC_AINFO, &anoninfo) == -1) {
        perror("function swapctl failed");
        swapTotal = swapFree = 1;
        return;
    }
    
    /* Total swap converted to megabytes. */

    swapTotal = (anoninfo.ani_max * pages_multiplier) / 1048576;
    
    /* Free swap converted to megabytes. */

    swapFree = ((anoninfo.ani_max - anoninfo.ani_resv) * pages_multiplier) / 1048576;
    
    /* Copy results to the temp string and add them to the PubSub dictionary
     * as the "swaptotal" and "swapfree" parameters.
     */

    sprintf(tmpStr, "%llu", swapTotal);

    headerValue = knstr_create_with_cstr(tmpStr);

    knevent_set(knEvent, KNSTR("swaptotal"), headerValue);
    
    sprintf(tmpStr, "%llu", swapFree);

    headerValue = knstr_create_with_cstr(tmpStr);

    knevent_set(knEvent, KNSTR("swapfree"), headerValue);

    kn_release(headerValue);
}



/* LoadAvg
 *
 * Uses getloadavg() to get load average from a 1 minute period and store 
 * results in the PubSub dictionary passed to the function.
 */

void LoadAvg(knevent_mref knEvent)
{
    /* double array to pass to getloadavg(). */

    double loadAvg[1];

    /* Number of samples to get from getloadavg(). */

    int nelem = 1;
    
    /* Temp string to hold results. */

    char tmpStr[1024];

    knstr_ref headerValue;

    /* Get load avg and store in loadAvg array. */

    if(getloadavg(loadAvg, nelem) < 0) {
        perror("function getloadavg failed");
        return;
    }
    
    /* Copy results to the temp string and add them to the PubSub dictionary 
     * as the "loadavg" parameter.
     */

    sprintf(tmpStr, "%.2f", loadAvg[LOADAVG_1MIN]);

    headerValue = knstr_create_with_cstr(tmpStr);

    knevent_set(knEvent, KNSTR("loadavg"), headerValue);

    kn_release(headerValue);
}


/* NumUsers
 *
 * Uses utmpx info to get the number of users logged on to the system and
 * stores the results in the PubSub dictionary passed to the function.
 */

void NumUsers(knevent_mref knEvent)
{
    knstr_ref headerValue;

    /* int to store number of users. */

    int numUsers = 0;

    /* utmpx struct to get info from. */

    struct utmpx *utmpx;

    /* Temp string to hold results. */

    char tmpStr[1024];

    /* Set stream to the beginning of the utmp file. */

    setutxent();

    /* Loop through and get number of users. */

    while((utmpx = getutxent()) != NULL) {
        if(utmpx->ut_type == USER_PROCESS && utmpx->ut_user[0] != '\0') { 
            numUsers++; 
        }
    }

    /* Close the utmp file. */

    endutxent();

    /* Copy results to the temp string and add them to the PubSub dictionary
     * as the "numusers" parameter.
     */

    sprintf(tmpStr, "%d", numUsers);

    headerValue = knstr_create_with_cstr(tmpStr);

    knevent_set(knEvent, KNSTR("numusers"), headerValue);
    
    kn_release(headerValue);
}
