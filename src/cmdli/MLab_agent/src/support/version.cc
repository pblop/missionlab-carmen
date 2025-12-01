/**********************************************************************
 **                                                                  **
 **                           version.cc                             **
 **                                                                  **
 ** Save the version, build number, and build date in globals.       **
 ** The build version, number and compile date is set in the makefile**
 ** via -DBUILDVER ``the version''  -DBUILDDATE ``the current time'' **
 ** and -DBUILDNUM ``build number''.                                 **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  ** 
 **  Copyright 1997, Mobile Intelligence Corporation                 **  
 **  7963 Pine Hollow Drive, SE                                      **   
 **  Alto, Michigan  49302-9724                                      **    
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **     
 **********************************************************************/

/* $Id: version.cc,v 1.1.1.1 2008/07/14 16:44:17 endo Exp $ */

#include "version.h"

const char build_version[] = BUILDVER;
// Integer left of decimal point is the major release number
// Integer right of decimal point is the minor release number
// lower case letters after minor release number indicate bug fix releases
// A leading letter 'I' indicates an internal development release

const char build_number[] = BUILDNUM;
// A positive integer which is incremented each time the system is built.
// It is reset to 1 on changes to the version string.

const char build_date[] = BUILDDATE;
// The date and time that the build occured.

///////////////////////////////////////////////////////////////////////
// $Log: version.cc,v $
// Revision 1.1.1.1  2008/07/14 16:44:17  endo
// MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
//
// Revision 1.1.1.1  2006/07/20 17:17:46  endo
// MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
//
// Revision 1.1.1.1  2006/07/12 13:37:55  endo
// MissionLab 7.0
//
// Revision 1.1  2006/07/01 00:37:43  endo
// CMDLi from MARS 2020 migrated into AO-FNC repository.
//
// Revision 1.1.1.1  2006/06/29 20:42:07  endo
// cmdli local repository.
//
// Revision 1.2  1997/12/15 18:27:35  doug
// switch to user BUILDVER
//
// Revision 1.1  1997/12/04 15:45:09  doug
// Initial revision
//
//
////////////////////////////////////////////////////////////////////////
