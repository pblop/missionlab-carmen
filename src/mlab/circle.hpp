/**********************************************************************
 **                                                                  **
 **                            circle.hpp                            **
 **                                                                  **
 **  class of circle shapes                                          **
 **                                                                  **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1995, 1996, 1997 Georgia Tech Research Corporation    **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **********************************************************************/

/* $Id: circle.hpp,v 1.1.1.1 2008/07/14 16:44:22 endo Exp $ */


#ifndef CIRCLE_HPP
#define CIRCLE_HPP

#include "shape.hpp"

class circle : public shape {

protected:
   double x_, y_, z_, r_;

public:

   void move(double dx, double dy)
   { 
      erase();
      x_ += dx;
      y_ += dy;
      draw();
   }

   void moveto(double x, double y)
   { 
      erase();
      x_ = x;
      y_ = y;
      draw();
   }

   void moveto(double x, double y, double z)
   { 
      erase();
      x_ = x;
      y_ = y;
      z_ = z;
      draw();
   }

   double distance(double x, double y)
   {
      double dx = x_ - x;
      double dy = y_ - y;
      double sqr_dist = dx*dx + dy*dy;

      // Kick out case where points are coincident
      if( sqr_dist <= 0 )
	 return 0;

      double dist = sqrt(sqr_dist) - r_;

      // Kick out case where point is inside the circle
      if( dist <= 0 )
	 return 0;

      // return distance from edge of circle to the point
      return dist;
   }

   void center(double &x, double &y)
   {
      x = x_;
      y = y_;
   }

   double getX(void)
   {
      return x_;
   }

   double getY(void)
   {
      return y_;
   }

   double getZ(void)
   {
      return z_;
   }

   double getR(void)
   {
      return r_;
   }
};

#endif


///////////////////////////////////////////////////////////////////////
// $Log: circle.hpp,v $
// Revision 1.1.1.1  2008/07/14 16:44:22  endo
// MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
//
// Revision 1.1.1.1  2006/07/20 17:17:50  endo
// MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
//
// Revision 1.1.1.1  2006/07/12 13:37:59  endo
// MissionLab 7.0
//
// Revision 1.2  2006/05/02 04:19:59  endo
// TrackTask improved for the experiment.
//
// Revision 1.1.1.1  2005/02/06 23:00:08  endo
// AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
//
// Revision 1.4  1997/02/14  15:51:29  zchen
// *** empty log message ***
//
// Revision 1.3  1995/11/07  14:31:57  doug
// added baskets and command to drop oranges into them
//
// Revision 1.2  1995/10/03  21:25:06  doug
// add support for get_objects call
//
// Revision 1.1  1995/10/03  15:10:18  doug
// Initial revision
//
///////////////////////////////////////////////////////////////////////
