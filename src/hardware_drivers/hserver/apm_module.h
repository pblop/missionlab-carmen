/**********************************************************************
 **                                                                  **
 **                              apm_module.h                        **
 **                                                                  **
 **                                                                  **
 **  Written by:  William Halliburton                                **
 **                                                                  **
 **  APM module for HServer                                          **
 **                                                                  **
 **  Copyright 2000 - 2006, Georgia Tech Research Corporation        **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: apm_module.h,v 1.1.1.1 2006/07/12 13:37:56 endo Exp $ */

#ifndef APM_MODULE_H
#define APM_MODULE_H

#include <pthread.h>

// Since Fedora Core 4, /usr/include/apm.h is no longer included in the
// distribution
#if APM_SUPPORT
#include <apm.h>
#endif

#include "module.h"

class Apm : public Module
{
  protected:
#if APM_SUPPORT
    bool useApm,update_running;
    char *apmString;
    int apm_exists();
    int apm_read(apm_info *i);
    const char *apm_time_nosec(time_t t);
    void disconnect();
    void update_apm();
    void calculate();
    pthread_t update;
#endif
    void updateStatusBar_(void);

  public:
    void update_thread();
    void control();
    Apm(Apm **a);
    ~Apm();
};

extern Apm *apm;

#endif


/**********************************************************************
# $Log: apm_module.h,v $
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
# Revision 1.2  2000/09/19 03:07:11  endo
# RCS log added.
#
#
#**********************************************************************/
