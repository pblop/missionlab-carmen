/**********************************************************************
 **                                                                  **
 **                            line.hpp                              **
 **                                                                  **
 **  class of line shapes                                            **
 **                                                                  **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1995, 1996, 1997 Georgia Tech Research Corporation    **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **********************************************************************/

/* $Id: line.hpp,v 1.1.1.1 2008/07/14 16:44:22 endo Exp $ */


#ifndef line_HPP
#define line_HPP

#include "shape.hpp"

class line : public shape {
public:
   double x1_, y1_, x2_, y2_;

   double distance(double x, double y)
   {
      double dist = vlength(x1_ - x, y1_ - y);
      dist = min(dist, vlength(x2_ - x, y2_ - y));

      // create a unit vector along the line 
      double Ux = x2_ - x1_;
      double Uy = y2_ - y1_;
      double length = vlength(Ux,Uy);
      Ux /= length;
      Uy /= length;

      // create a vector to the point
      double Vx = x - x1_;
      double Vy = y - y1_;

      /* calculate the dot product, to see if is along the line segment */
      /* recall: dot product gives parallel component of resultant vector */
      double h = Ux * Vx + Uy * Vy;

      if (h < 0 || h > length)
      {
	 /* The perpendicular from the point to the line does
	    not intersect the line between the two end points */
      }
      else
      {
         // now calculate the distance via the cross product.
         // recall cross product gives perpendicular component of resultant 
	 // vector
         dist = min(dist, abs(Ux * Vy - Uy * Vx));
      }

      // return distance from line to the point
      return dist;
   }

   void center(double &x, double &y)
   {
      x = (x1_ + x2_) / 2;
      y = (y1_ + y2_) / 2;
   }
};

#endif


///////////////////////////////////////////////////////////////////////
// $Log: line.hpp,v $
// Revision 1.1.1.1  2008/07/14 16:44:22  endo
// MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
//
// Revision 1.1.1.1  2006/07/20 17:17:50  endo
// MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
//
// Revision 1.1.1.1  2006/07/12 13:37:59  endo
// MissionLab 7.0
//
// Revision 1.1.1.1  2005/02/06 23:00:11  endo
// AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
//
// Revision 1.3  1997/02/14  15:51:29  zchen
// *** empty log message ***
//
// Revision 1.2  1995/11/07  14:31:57  doug
// added baskets and command to drop oranges into them
//
// Revision 1.1  1995/10/03  18:59:20  doug
// Initial revision
//
// Revision 1.1  1995/10/03  15:10:18  doug
// Initial revision
//
///////////////////////////////////////////////////////////////////////
