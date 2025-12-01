/**********************************************************************
 **                                                                  **
 **                     print_overloads.cc                           **  
 **                                                                  **
 **  debug printing functions                                        **  
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1995, Georgia Tech Research Corporation               **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: print_overloads.c,v 1.1.1.1 2006/07/12 13:37:56 endo Exp $ */


#include <memory.h>
#include "gt_message_types.h"

ostream & operator << (ostream & s, ObjectReading &rec)
{
   const char *cls = rec.objcolor == NULL ?  "Invalid" : rec.objcolor;
   s << cls << " ";

   if( rec.objcolor )
   {
      char *shp = (char *)(rec.objshape == CircleObject ? "Circle" :
			   rec.objshape == LineObject ? "Line" :
			   "Unknown");
      s << shp << " ";

      if( rec.id != NOID )
      {
         s << "id=" << rec.id;
         s << " ";
      }

      switch(rec.objshape)
      {
         case LineObject:
            s << '<' << rec.x1 << ' ' << rec.y1 << "> <" << rec.x2 << ' ' << rec.y2 << '>';
            break;
         case CircleObject:
            s << '<' << rec.x1 << ' ' << rec.y1 << "> r=" << rec.r;
            break;
         case PointObject:
            s << "point";
            break;
      }

      if( rec.ismovable )
         s << "  Is movable";
      else
         s << "  Is NOT movable";
   }

   return s;
}



/**********************************************************************
 * $Log: print_overloads.c,v $
 * Revision 1.1.1.1  2006/07/12 13:37:56  endo
 * MissionLab 7.0
 *
 * Revision 1.2  2006/05/15 06:26:46  endo
 * gcc-3.4 upgrade
 *
 * Revision 1.1.1.1  2005/02/06 22:59:39  endo
 * AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
 *
 * Revision 1.10  2000/10/16 19:43:46  endo
 * Modified due to the compiler upgrade.
 *
 * Revision 1.9  1999/12/16 22:28:51  mjcramer
 * fixed for RH6
 *
 * Revision 1.8  1999/07/03 17:42:25  endo
 * The 3D thing disabled because it has a problem on
 * drawing obstacles on the mlab-console.
 *
 * Revision 1.7  1999/06/29 02:40:35  mjcramer
 * Fixed this to print 3D coordinates
 *
 * Revision 1.6  1996/05/30 03:50:44  tucker
 * done
 *
 * Revision 1.5  1996/02/28  03:54:51  doug
 * *** empty log message ***
 *
 * Revision 1.4  1996/02/20  16:47:27  doug
 * made detect_objects work on the dennings
 *
 * Revision 1.3  1996/02/08  19:24:07  doug
 * handle objects of any color
 *
 * Revision 1.2  1995/10/11  22:05:34  doug
 * *** empty log message ***
 *
 * Revision 1.1  1995/10/11  20:35:47  doug
 * Initial revision
 *
 **********************************************************************/
