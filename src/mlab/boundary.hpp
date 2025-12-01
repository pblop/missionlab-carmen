/**********************************************************************
 **                                                                  **
 **                            boundary.hpp                          **
 **                                                                  **
 **  class of boundary shapes                                        **
 **                                                                  **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1995, 1996, 1997 Georgia Tech Research Corporation    **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **********************************************************************/

/* $Id: boundary.hpp,v 1.1.1.1 2008/07/14 16:44:22 endo Exp $ */


#ifndef BOUNDARY_HPP
#define BOUNDARY_HPP

#include "line.hpp"

class boundary : public line {
public:
   boundary(double x, double y, double r) : circle()
   {
      is_up_ = false;
      x_ = x;
      y_ = y;
      r_ = r;
   }

   void draw()
   { 
      DrawLine(x1_, y1_, x2_, y2_, theboundaryGC);
   }

   void erase()
   { 
      draw();
   }

   Reading to_reading(double x, double y, double heading)
   { 
      Reading rtn;

      rtn.objclass = FixedObject;
      rtn.id = id_;
      rtn.objshape = LineObject;

      // Make position egocentric
      v.x = x1_ - x;                 
      v.y = y1_ - y;                 
      rotate_z(v, -1 * heading);     

      rtn.x1 = v.x;
      rtn.y1 = v.y;

      // Make position egocentric
      v.x = x2_ - x;                 
      v.y = y2_ - y;                 
      rotate_z(v, -1 * heading);     

      rtn.x2 = v.x;
      rtn.y2 = v.y;

      // Not used
      rtn.r = 0;
   }

   void pickup(double x, double y)
   { 
      cerr << "Error: Boundaries are not movable\n";
   }

   void putdown(double x, double y)
   { 
      x_ = x;
      y_ = y;
      draw();
   }

   bool is_up()
   { 
      return is_up_;
   }
};

#endif


///////////////////////////////////////////////////////////////////////
// $Log: boundary.hpp,v $
// Revision 1.1.1.1  2008/07/14 16:44:22  endo
// MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
//
// Revision 1.1.1.1  2006/07/20 17:17:50  endo
// MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
//
// Revision 1.1.1.1  2006/07/12 13:37:59  endo
// MissionLab 7.0
//
// Revision 1.1.1.1  2005/02/06 23:00:08  endo
// AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
//
// Revision 1.3  1997/02/14  15:51:29  zchen
// *** empty log message ***
//
// Revision 1.2  1995/10/23  18:21:57  doug
// Extend object stuff to support obstacles and also add 3d code
//
// Revision 1.1  1995/10/19  21:02:17  doug
// Initial revision
//
// Revision 1.1  1995/10/03  15:31:45  doug
// Initial revision
//
// Revision 1.1  1995/10/03  15:10:18  doug
// Initial revision
//
///////////////////////////////////////////////////////////////////////
