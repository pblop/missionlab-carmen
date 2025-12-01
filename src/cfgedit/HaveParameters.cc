/**********************************************************************
 **                                                                  **
 **                           HaveParameters                         **
 **                                                                  **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1995, 1996, 1997 Georgia Tech Research Corporation    **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **********************************************************************/

/* $Id: HaveParameters.cc,v 1.1.1.1 2008/07/14 16:44:15 endo Exp $ */

#include "load_cdl.h"
#include "HaveParameters.h"

/*-----------------------------------------------------------------------*/

bool
HaveParameters(Symbol *rec)
{
   if( rec == NULL )
   {
      return false;
   }

   if( rec->symbol_type != GROUP_NAME ||
       rec->children.isempty() )
   {  
      return false;
   }

   void *cur;
   Symbol *p;
   if( (cur = rec->parameter_list.first(&p)) != NULL )
   {
      do
      {
	 if( p->name && p->name[0] == '%' )
	 {
	    if( p->symbol_type == PARM_HEADER )
	    {
	       Symbol *lp;
	       void *cur_lp;

	       if ((cur_lp = p->parameter_list.first(&lp)) != NULL)
	       {
	          do
	          {
                     // If is a constant, then we are interested
		     if (lp->input_generator &&
			 lp->input_generator != (Symbol *) UP &&
		         lp->input_generator->symbol_type == INITIALIZER)
		     {
			return true;
		     }
	          }
	          while ((cur_lp = p->parameter_list.next(&lp, cur_lp)) != NULL);
	       }
	    }
	    else
	    {
               // If is a constant, then we are interested
	       if (p->input_generator &&
		   p->input_generator != (Symbol *) UP &&
		   p->input_generator->symbol_type == INITIALIZER)
               {
		  return true;
	       }
	    }
         }
      } while((cur = rec->parameter_list.next(&p,cur)) != NULL);
   }

   return false;
}


///////////////////////////////////////////////////////////////////////
// $Log: HaveParameters.cc,v $
// Revision 1.1.1.1  2008/07/14 16:44:15  endo
// MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
//
// Revision 1.1.1.1  2006/07/20 17:17:45  endo
// MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
//
// Revision 1.1.1.1  2006/07/12 13:37:54  endo
// MissionLab 7.0
//
// Revision 1.1.1.1  2005/02/06 22:59:31  endo
// AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
//
// Revision 1.3  1997/02/14  16:53:35  zchen
// *** empty log message ***
//
// Revision 1.2  1996/05/08  15:16:39  doug
// fixing compile warnings
//
// Revision 1.1  1996/02/26  16:14:52  doug
// Initial revision
//
//
///////////////////////////////////////////////////////////////////////
