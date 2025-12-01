/**********************************************************************
 **                                                                  **
 **                            basket.hpp                            **
 **                                                                  **
 **  class of basket shapes                                          **
 **                                                                  **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1995 - 2005 Georgia Tech Research Corporation         **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **********************************************************************/

/* $Id: basket.hpp,v 1.1.1.1 2008/07/14 16:44:22 endo Exp $ */


#ifndef BASKET_HPP
#define BASKET_HPP

#include "circle.hpp"

class basket : public circle {
public:
   basket(double x, double y, double r, const char *color)
   {
      x_ = x;
      y_ = y;
      r_ = r;
      color_ = strdup(color);
      //gc_ = lookup_color(color);
      gc_ = getGCByColorName(color);
      assert( gc_ );
   }

   ~basket()
   {
      free((void *)color_);
   }

   void draw()
   { 
      DrawFilledCircle(x_, y_, r_, gc_);
   }

   void erase()
   { 
      DrawFilledCircle(x_, y_, r_, gGCs.erase);
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
      rtn.objshape = CircleObject;
      rtn.x1 = v.x;
      rtn.y1 = v.y;
      rtn.r = r_;
      rtn.ismovable = ismovable();

      // Unused
      rtn.x2 = 0;
      rtn.y2 = 0;

      return rtn;
   }

   bool ismovable() { return false; }


   void bind(shape *owner)
   { 
      cerr << "Error: Object is not movable\n";
   }

   void unbind(shape *owner)
   { 
      cerr << "Error: Object is not movable\n";
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
// $Log: basket.hpp,v $
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
// Revision 1.1.1.1  2005/02/06 23:00:08  endo
// AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
//
// Revision 1.8  2000/10/16 19:39:40  endo
// Modified due to the compiler upgrade.
//
// Revision 1.7  1999/12/16 22:50:13  mjcramer
// rh 6 port
//
// Revision 1.6  1997/02/12  05:41:37  zchen
// *** empty log message ***
//
// Revision 1.5  1996/02/27  05:00:04  doug
// *** empty log message ***
//
// Revision 1.4  1996/02/16  00:05:14  doug
// *** empty log message ***
//
// Revision 1.3  1996/02/08  19:24:07  doug
// handle objects of any color
//
// Revision 1.2  1995/11/07  14:31:57  doug
// added baskets and command to drop oranges into them
//
// Revision 1.1  1995/11/06  19:44:18  doug
// Initial revision
//
///////////////////////////////////////////////////////////////////////
