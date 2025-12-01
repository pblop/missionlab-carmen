/**********************************************************************
 **                                                                  **
 **                            attractor.hpp                         **
 **                                                                  **
 **  class of attractor shapes                                       **
 **                                                                  **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1995 - 2007 Georgia Tech Research Corporation         **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **********************************************************************/

/* $Id: attractor.hpp,v 1.1.1.1 2008/07/14 16:44:22 endo Exp $ */


#ifndef attractor_HPP
#define attractor_HPP

#include "circle.hpp"

class attractor : public circle {
public:
    attractor(double x, double y, double r, const char *color)
        {
            x_ = x;
            y_ = y;
            r_ = r;
            color_ = strdup(color);
            //gc_ = lookup_color(color);
            gc_ = getGCByColorName(color);
            assert(gc_);
        }

    ~attractor()
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

    bool ismovable() { return true; }

    void bind(shape *owner)
        { 
            if(bound_to_ == NULL)
            {
                double dist = owner->distance(x_, y_);
                if(dist < (MAX_PICKUP_DIST*2))
                {
                    erase();
                    bound_to_ = owner;
                    is_hidden_ = true;
                }
                else
                {
                    warn_once("The robot is not close enough to the object to pick it up");
                }
            }
        }

    void unbind(shape *owner)
        { 
            if(bound_to_ && bound_to_ == owner)
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
// $Log: attractor.hpp,v $
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
// Revision 1.4  2006/07/12 06:16:42  endo
// Clean-up for MissionLab 7.0 release.
//
// Revision 1.3  2005/08/22 22:10:28  endo
// For Pax River demo.
//
// Revision 1.2  2005/02/07 23:12:36  endo
// Mods from usability-2004
//
// Revision 1.1.1.1  2005/02/06 23:00:08  endo
// AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
//
// Revision 1.16  2000/10/16 19:39:40  endo
// Modified due to the compiler upgrade.
//
// Revision 1.15  1999/12/16 22:49:51  mjcramer
// rh6 port
//
// Revision 1.14  1997/02/14  15:51:29  zchen
// *** empty log message ***
//
// Revision 1.13  1996/03/04  00:04:30  doug
// *** empty log message ***
//
// Revision 1.12  1996/02/27  05:00:04  doug
// *** empty log message ***
//
// Revision 1.11  1996/02/22  00:56:22  doug
// *** empty log message ***
//
// Revision 1.10  1996/02/08  19:24:07  doug
// handle objects of any color
//
// Revision 1.9  1996/02/02  18:43:54  doug
// fixed can't bind message
//
// Revision 1.8  1995/11/07  14:31:57  doug
// added baskets and command to drop oranges into them
//
// Revision 1.7  1995/10/23  19:54:34  doug
// *** empty log message ***
//
// Revision 1.6  1995/10/23  18:21:57  doug
// Extend object stuff to support obstacles and also add 3d code
//
// Revision 1.5  1995/10/16  21:33:52  doug
// *** empty log message ***
//
// Revision 1.4  1995/10/11  22:05:34  doug
// *** empty log message ***
//
// Revision 1.3  1995/10/09  15:26:43  doug
// changed ObjectClasses to ObjectColors
//
// Revision 1.2  1995/10/03  21:25:06  doug
// add support for get_objects call
//
// Revision 1.1  1995/10/03  15:31:45  doug
// Initial revision
//
// Revision 1.1  1995/10/03  15:10:18  doug
// Initial revision
//
///////////////////////////////////////////////////////////////////////
