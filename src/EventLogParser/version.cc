/**********************************************************************
 **                                                                  **
 **                           version.cc                             **
 **                                                                  **
 ** Compile the version and compile time strings separately so they  **
 ** can be updated efficiently everytime console is compiled.        **
 **                                                                  **
 **  By: Jonathan M. Cameron                                          **
 **                                                                  **
 **  Copyright 1995, Georgia Tech Research Corporation               **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **********************************************************************/

/* $Id: version.cc,v 1.1.1.1 2008/07/14 16:44:14 endo Exp $ */


extern "C" {
#include <stdio.h>
}


char *version_str = "0.5";

char compile_time_str[40];


char *compile_time()
{
   sprintf(compile_time_str, "%s", DATE);
   return compile_time_str;
}



///////////////////////////////////////////////////////////////////////
// $Log: version.cc,v $
// Revision 1.1.1.1  2008/07/14 16:44:14  endo
// MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
//
// Revision 1.1.1.1  2006/07/20 17:17:45  endo
// MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
//
// Revision 1.1.1.1  2006/07/12 13:37:53  endo
// MissionLab 7.0
//
// Revision 1.1.1.1  2005/02/06 22:59:28  endo
// AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
//
// Revision 1.1  1996/03/05  22:24:28  doug
// Initial revision
//
////////////////////////////////////////////////////////////////////////
