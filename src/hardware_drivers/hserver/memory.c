/**********************************************************************
 **                                                                  **
 **                              memory.c                            **
 **                                                                  **
 **                                                                  **
 **  Written by:     William Halliburton                             **
 **                                                                  **
 **  malloc wrapper for HServer                                      **
 **                                                                  **
 **  Copyright 2000 - 2006, Georgia Tech Research Corporation        **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: memory.c,v 1.1.1.1 2006/07/12 13:37:56 endo Exp $ */

#include <pthread.h>


pthread_mutex_t malloc_mutex;

void malloc2Init(){
    pthread_mutex_init(&malloc_mutex,NULL);
}


void *malloc2(size_t size){
    pthread_mutex

/**********************************************************************
# $Log: memory.c,v $
# Revision 1.1.1.1  2006/07/12 13:37:56  endo
# MissionLab 7.0
#
# Revision 1.2  2006/06/05 21:35:52  endo
# PoseCalculator from MARA 2020 migrated
#
# Revision 1.1.1.1  2005/02/06 22:59:41  endo
# AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
#
# Revision 1.2  2000/09/19 03:45:31  endo
# RCS log added.
#
#
#**********************************************************************/

