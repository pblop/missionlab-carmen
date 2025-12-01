/**********************************************************************
 **                                                                  **
 **                              apm_module.c                        **
 **                                                                  **
 **                                                                  **
 **  Written by:  William Halliburton                                **
 **                                                                  **
 **  APM module for HServer                                          **
 **                                                                  **
 **  Copyright 2000, Georgia Tech Research Corporation               **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: apm_module.c,v 1.1.1.1 2006/07/12 13:37:56 endo Exp $ */

// Since Fedora Core 4, /usr/include/apm.h is no longer included in the
// distribution
#if APM_SUPPORT
#include <pthread.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include "statusbar.h"
#include "hserver.h"
#include "message.h"
#include "apm_module.h"

#define SEC_PER_DAY  (60*60*24)
#define SEC_PER_HOUR (60*60)
#define SEC_PER_MIN  (60)

/* If APM support of the right version exists in kernel, return zero.
 * Otherwise, return 1 if no support exists, or 2 if it is the wrong
 * version.  *NOTE* The sense of the return value is not intuitive.
 */
int Apm::apm_exists(void)
{
    int rtn = 1;
    apm_info *apm_i = NULL;

    if (access(APM_PROC, R_OK))
    {
        return rtn;
    }

    apm_i = new apm_info;
    rtn = apm_read(apm_i);
    delete apm_i;

    return rtn;
}

/* Read information from /proc/apm.  Return 0 on success, 1 if APM not
 * installed, 2 if APM installed, but old version. 
 */
int Apm::apm_read(apm_info * i)
{
    FILE *str;
    char units[10];
    char buffer[100];
    int retcode = 0;
    unsigned int apm_flags, ac_line_status, battery_status, battery_flags; // ENDO - gcc 3.4

    if (!(str = fopen(APM_PROC, "r")))
        return 1;
    fgets(buffer, sizeof(buffer) - 1, str);
    buffer[sizeof(buffer) - 1] = '\0';

    /* Should check for other driver versions; driver 1.9 (and some
     * others) uses this format, which doesn't expose # batteries.
     */
    sscanf(buffer, "%s %d.%d %x %x %x %x %d%% %d %s\n",
           (char *)(i->driver_version),
           &i->apm_version_major,
           &i->apm_version_minor,
           &apm_flags,
           &ac_line_status,
           &battery_status,
           &battery_flags,
           &i->battery_percentage,
           &i->battery_time,
           units);
    i->apm_flags = apm_flags;
    i->ac_line_status = ac_line_status;
    i->battery_status = battery_status;
    i->battery_flags = battery_flags;
    i->using_minutes = !strncmp(units, "min", 3) ? 1 : 0;
    /* Fix possible kernel bug -- percentage
     * set to 0xff (==255) instead of -1. 
     */
    if (i->battery_percentage > 100)
        i->battery_percentage = -1;
    
    fclose(str);
    if (i->driver_version[0] == 'B') retcode = -1;

    return retcode;
}

const char *Apm::apm_time_nosec(time_t t)
{
    static char buffer[128];
    unsigned long s, m, h, d;

    d = t / SEC_PER_DAY;
    t -= d * SEC_PER_DAY;
    h = t / SEC_PER_HOUR;
    t -= h * SEC_PER_HOUR;
    m = t / SEC_PER_MIN;
    t -= m * SEC_PER_MIN;
    s = t;

    if (s > 30)
        ++m;

    if (d)
        sprintf(buffer, "%lu day%s, %lu:%02lu",
                d, d > 1 ? "s" : "", h, m);
    else
        sprintf(buffer, "%lu:%02lu", h, m);

    if (t == -1)
        sprintf(buffer, "unknown");

    return buffer;
}

void Apm::updateStatusBar_(void)
{
    statusStr[0] = 0;
    strcat(statusStr,"Battery: ");
    strcat(statusStr,apmString);
    statusbar->update(statusLine);
}


void Apm::calculate(void)
{
    struct rusage rusage;
    int rtn;
    char buf[100];

    rtn = getrusage(RUSAGE_SELF,&rusage);
    if (rtn) {
        printTextWindow("Usage error getrusage");
        return;
    }
    sprintf(buf,"Usage");
    printTextWindow(buf);
    sprintf(buf,"   user time %ld %ld  system time %ld %ld",rusage.ru_utime.tv_sec,rusage.ru_utime.tv_usec,rusage.ru_stime.tv_sec,rusage.ru_stime.tv_usec);
    printTextWindow(buf);
    sprintf(buf,"   maximum resident set size %ld",rusage.ru_maxrss);
    printTextWindow(buf);
    sprintf(buf,"   shared memory %ld  unshared data %ld  unshared stack %ld",
            rusage.ru_ixrss,rusage.ru_idrss,rusage.ru_isrss);
    printTextWindow(buf); 
    sprintf(buf,"   page reclaims %ld  page faults %ld  swaps %ld",
            rusage.ru_minflt,rusage.ru_majflt,rusage.ru_nswap);
    printTextWindow(buf);
    sprintf(buf,"   block input operations %ld  block output operations %ld",
            rusage.ru_inblock,rusage.ru_oublock);
    printTextWindow(buf); 
    sprintf(buf,"   messages send received %ld  signals recieved %ld",
            rusage.ru_msgsnd,rusage.ru_msgrcv);
    printTextWindow(buf);
    sprintf(buf,"   voluntary context switches %ld  involuntary %ld",
            rusage.ru_nvcsw,rusage.ru_nivcsw);
    printTextWindow(buf); 
}

void Apm::update_apm(void)
{
    int rtn;
    apm_info *apm_i = NULL;
    char buf[100];

    apm_i = new apm_info;

    rtn = apm_read(apm_i);

    apmString[0] = 0;

    if (rtn) {
        printTextWindow("APM old version not supported");
        delete apm_i;
        return;
    }
    switch (apm_i->ac_line_status) {

    case 0:
        strcat(apmString,"AC off");
        break;
    case 1:
        strcat(apmString,"AC on");
        break;
    case 2:
        strcat(apmString,"On backup power");
        break;
    }

    switch (apm_i->battery_status) {

    case 0:
        strcat(apmString,", battery status high");
        break;
    case 1:
        strcat(apmString,", battery status low");
        break;
    case 2:
        strcat(apmString,", battery status critical");
        break;
    case 3:
        strcat(apmString,", battery charging");
        break;
    }

    if (apm_i->battery_percentage >= 0){
        sprintf(buf,": %d%%", apm_i->battery_percentage);
        strcat(apmString,buf);
    }

    if (apm_i->battery_time >= 0){
        int secs = apm_i->using_minutes ? apm_i->battery_time * 60 : apm_i->battery_time;
        sprintf(buf," (%s)", apm_time_nosec(secs));
        strcat(apmString,buf);
    }

    delete apm_i;
}    


static void start_thread(void *apm_instance)
{
    ((Apm*)apm_instance)->update_thread();
}


void Apm::update_thread(void)
{
    while(1)
    {
        pthread_testcancel();
        if (useApm)
        {
            update_apm();
        }
        updateStatusBar_();
        usleep(100000);
    }
}

Apm::~Apm(void)
{
    if (update_running)
    {
        pthread_cancel(update);
        pthread_join(update,NULL);
        printTextWindow("Battery usage disconnected");
    }
    delete [] apmString;
}

Apm::Apm(Apm** a) : Module((Module**)a, HS_MODULE_NAME_APM)
{
    apm_info *apm_i = NULL;

    update_running = false;
    apmString = new char[100];

    switch(apm_exists()) {

    case 1: 
        useApm = false;
        break;
    case 2:
        useApm = false;
        break;
    default:
        useApm = true;
        break;
    }

    if (!useApm)
    {
        printTextWindow("No APM support");
        delete this;
        return;
    }
    else if (report_level >= HS_REP_LEV_DEBUG)
    {
        apm_i = new apm_info;
        apm_read(apm_i);

        printfTextWindow(
            "APM: driver version %s  apm version %d.%d",
            apm_i->driver_version,
            apm_i->apm_version_major,
            apm_i->apm_version_minor );
        delete apm_i;
        update_apm();
    }
    updateStatusBar_();

    update_running = true;
    pthread_create(&update,NULL,(void * (*)(void *))&start_thread,(void*)this);
}


void Apm::control(void)
{
    bool bDone = 0;

    messageDrawWindow( EnMessageType_USAGE_CONTROL, EnMessageErrType_NONE, NULL );
    do
    {
        int c = getch();

        switch(c) {

        case 'c':
            calculate();
            break;
        case 'd':
            messageHide();
            delete this;
            return;
            break;
        case 'x':
        case 'Q':
        case KEY_ESC:
            bDone = true;
            break;
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
        case KEY_UP:
        case KEY_DOWN:
        case KEY_LEFT:
        case KEY_RIGHT:
            messageMovePanelKey(c);
            break;
        }
    } while ( !bDone );
    messageHide();
}

#else // APM_SUPPORT

#include "hserver.h"
#include "apm_module.h"

void Apm::updateStatusBar_(void) {}

void Apm::update_thread(void) {}

void Apm::control(void)
{
    printTextWindow("No APM support");
}

Apm::Apm(Apm** a) : Module((Module**)a, HS_MODULE_NAME_APM)
{
    printTextWindow("No APM support");
    delete this;
    return;
}

Apm::~Apm(void){}

#endif // APM_SUPPORT

/**********************************************************************
# $Log: apm_module.c,v $
# Revision 1.1.1.1  2006/07/12 13:37:56  endo
# MissionLab 7.0
#
# Revision 1.4  2006/06/24 21:51:20  endo
# gcc 4.1 upgrade
#
# Revision 1.3  2006/06/05 21:35:51  endo
# PoseCalculator from MARA 2020 migrated
#
# Revision 1.2  2006/05/15 06:01:03  endo
# gcc-3.4 upgrade
#
# Revision 1.1.1.1  2005/02/06 22:59:43  endo
# AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
#
# Revision 1.3  2000/12/12 23:22:45  blee
# Changed #defines for the user interface to enumerated types.
#
# Revision 1.2  2000/09/19 03:07:11  endo
# RCS log added.
#
#
#**********************************************************************/
