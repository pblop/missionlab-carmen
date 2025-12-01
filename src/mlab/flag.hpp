/**********************************************************************
 **                                                                  **
 **                            attractor.hpp                         **
 **                                                                  **
 **  class of attractor shapes                                       **
 **                                                                  **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1995 - 2005, Georgia Tech Research Corporation        **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **********************************************************************/

/* $Id: flag.hpp,v 1.1.1.1 2008/07/14 16:44:22 endo Exp $ */


#ifndef flag_HPP
#define flag_HPP

#include "shape.hpp"

class flag : public shape {

public:
   double x_, y_, d_;  // Center x,y and the diagonal for the flag.

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

   double distance(double x, double y)
   {
      double dx = x_ - x;
      double dy = y_ - y;
      double sqr_dist = dx*dx + dy*dy;

      // Kick out case where points are coincident
      if( sqr_dist <= 0.0001 )
	 return 0;

      return sqrt(sqr_dist);
   }

   void center(double &x, double &y)
   {
      x = x_;
      y = y_;
   }

   flag(double x, double y, double r, const char *color)
   {
      x_ = x;
      y_ = y;
      d_ = r;
      color_ = strdup(color);
      //gc_ = lookup_color(color);
      gc_ = getGCByColorName(color);
      assert( gc_ );
   }

   ~flag()
   {
      free((void *)color_);
   }

   void draw()
   { 
      DrawFilledCircle(x_, y_, d_, gc_);
   }

   void erase()
   { 
      DrawFilledCircle(x_, y_, d_, gGCs.erase);
   }

   ObjectReading to_reading(double x, double y, double heading)
   { 
      ObjectReading rtn;
      Vector  v;

      // Make position egocentric
      v.x = x_ - x;
      v.y = y_ - y;
      rotate_z(v, -1 * heading);

      rtn.setColor((char *)color_);
      rtn.id = id_;
      rtn.objshape = PointObject; // Just has a center location.
      rtn.x1 = v.x;
      rtn.y1 = v.y;
      rtn.ismovable = ismovable();

      // Unused
      rtn.x2 = 0;
      rtn.y2 = 0;
      rtn.r = 0;

      return rtn;
   }

   bool ismovable() { return true; }

   void bind(shape *owner)
   { 
      if( bound_to_ == NULL )
      {
	 double dist = owner->distance(x_, y_);
	 if( dist < MAX_PICKUP_DIST )
	 {
            erase();
            bound_to_ = owner;
	    is_hidden_ = true;
	 }
         else
	 {
	    warn_once("The robot is not close enough to the flag to pick it up");
         }
      }
   }

   void unbind(shape *owner)
   { 
      if( bound_to_ && bound_to_ == owner )
      {
         owner->center(x_, y_);
         bound_to_ = NULL;
	 is_hidden_ = false;
         draw();
      }
   }

   gt_Point whereis()
   { 
      gt_Point rtn;

      rtn.x = x_;
      rtn.y = y_;

      return rtn;
   }

   bool is_hidden()
   { 
      return is_hidden_;
   }
};

#endif


///////////////////////////////////////////////////////////////////////
// $Log: flag.hpp,v $
// Revision 1.1.1.1  2008/07/14 16:44:22  endo
// MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
//
// Revision 1.2  2007/08/30 18:33:39  endo
// BitmapIconDisplayer class added.
//
// Revision 1.1.1.1  2006/07/20 17:17:50  endo
// MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
//
// Revision 1.1.1.1  2006/07/12 13:37:59  endo
// MissionLab 7.0
//
// Revision 1.2  2005/02/07 23:12:36  endo
// Mods from usability-2004
//
// Revision 1.1.1.1  2005/02/06 23:00:09  endo
// AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
//
// Revision 1.5  2000/10/16 19:39:40  endo
// Modified due to the compiler upgrade.
//
// Revision 1.4  1999/12/16 22:49:58  mjcramer
// rh6 port
//
// Revision 1.3  1997/02/12  05:41:37  zchen
// *** empty log message ***
//
// Revision 1.2  1996/03/09  01:08:43  doug
// *** empty log message ***
//
// Revision 1.1  1996/03/09  00:19:41  doug
// Initial revision
//
//
///////////////////////////////////////////////////////////////////////
