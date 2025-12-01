/**********************************************************************
 **                                                                  **
 **                             error.cc                             **
 **                                                                  **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1995, 1996, 1997 Georgia Tech Research Corporation    **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **********************************************************************/

/* $Id: error.cc,v 1.1.1.1 2008/07/14 16:44:15 endo Exp $ */


/* report_error will print the error message to the user */

#include <iostream>

#include "version.h"

using std::cerr;

bool internal_error = false;

void
print_error(const char *message,const char *file,const int line)
{
   cerr << "\n\nInternal Error in " << file << " at " << line << '\n';

   cerr << "cfgedit version " << version_str << " compiled ";
   cerr << compile_time() << '\n';

   cerr << message << "\n\n";

   internal_error = true;
}


///////////////////////////////////////////////////////////////////////
// $Log: error.cc,v $
// Revision 1.1.1.1  2008/07/14 16:44:15  endo
// MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
//
// Revision 1.1.1.1  2006/07/20 17:17:45  endo
// MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
//
// Revision 1.1.1.1  2006/07/12 13:37:54  endo
// MissionLab 7.0
//
// Revision 1.1.1.1  2005/02/06 22:59:32  endo
// AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
//
// Revision 1.4  2003/04/06 12:25:34  endo
// gcc 3.1.1
//
// Revision 1.3  1997/02/14  16:53:35  zchen
// *** empty log message ***
//
// Revision 1.2  1995/06/29  17:41:13  jmc
// Added header and RCS id and log strings.
///////////////////////////////////////////////////////////////////////
