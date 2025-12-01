/**********************************************************************
 **                                                                  **
 **                            cnp_types.h                           **
 **                                                                  **
 **  Written by: Yoichiro Endo                                       **
 **                                                                  **
 **  Copyright 2006 Georgia Tech Research Corporation                **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/
#ifndef CNP_TYPES_H
#define CNP_TYPES_H

/* $Id: cnp_types.h,v 1.1.1.1 2008/07/14 16:44:17 endo Exp $ */

// Make sure this matches with CNP_MODE_STRING below
typedef enum CNP_Mode_t {
    CNP_MODE_DISABLED,
    CNP_MODE_PREMISSION,
    CNP_MODE_RUNTIME,
    CNP_MODE_PREMISSION_AND_RUNTIME,
    NUM_CNP_MODES
};

// Make sure this matches with CNP_Mode_t above.
const string CNP_MODE_STRING[NUM_CNP_MODES] = 
{
    "CNP_MODE_DISABLED",
    "CNP_MODE_PREMISSION",
    "CNP_MODE_RUNTIME",
    "CNP_MODE_PREMISSION_AND_RUNTIME"
};


#endif
/**********************************************************************
 * $Log: cnp_types.h,v $
 * Revision 1.1.1.1  2008/07/14 16:44:17  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/20 17:17:47  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:37:55  endo
 * MissionLab 7.0
 *
 * Revision 1.1  2006/01/29 04:10:03  endo
 * *** empty log message ***
 *
 **********************************************************************/
