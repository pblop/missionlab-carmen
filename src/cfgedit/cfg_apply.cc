/**********************************************************************
 **                                                                  **
 **                       cfg_apply.cc                               **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1995, 1996, 1997 Georgia Tech Research Corporation    **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **********************************************************************/

/* $Id: cfg_apply.cc,v 1.1.1.1 2008/07/14 16:44:15 endo Exp $ */


#include <stdio.h>
#include <stdlib.h>
#include <libc.h>

#include <Xm/Xm.h>

#include "load_cdl.h"
#include "configuration.hpp"     
#include "popups.h"     
#include "globals.h"     
#include "cfg_apply.h"

//**************************************************************
// Returns true if success
bool
cfg_apply(Symbol *start, apply_fnc *fnc)
{
   SymbolList *page_stack = new SymbolList;

   if (start->children.len() == 0)
      return true;

   // Clear all the marked nodes
   start->clear_marks();

   Symbol *location = NULL;
   char *message = NULL;

   if( !apply(start, fnc, &location, &message, &page_stack) )
   {
      if( message )
      {
         config->goto_page(page_stack, location);
         warn_user(message);
      }

      delete page_stack;
      return false;
   }

   delete page_stack;

   return true;
}

///////////////////////////////////////////////////////////////////////
// $Log: cfg_apply.cc,v $
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
// Revision 1.6  1997/02/14  16:53:35  zchen
// *** empty log message ***
//
// Revision 1.5  1996/02/26  05:01:33  doug
// *** empty log message ***
//
// Revision 1.4  1996/02/25  19:37:05  doug
// *** empty log message ***
//
// Revision 1.3  1996/02/18  23:31:19  doug
// binding is working!!!!
//
// Revision 1.2  1996/02/18  00:03:13  doug
// binding is working better
//
// Revision 1.1  1996/02/06  18:17:26  doug
// Initial revision
//
//
///////////////////////////////////////////////////////////////////////
