/**********************************************************************
 **                                                                  **
 **                            shape.hpp                             **
 **                                                                  **
 **  Abstract class used to draw stuff in the simulation world.      **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1995 - 2007 Georgia Tech Research Corporation         **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **********************************************************************/

/* $Id: shape.hpp,v 1.1.1.1 2008/07/14 16:44:22 endo Exp $ */


#ifndef SHAPE_HPP
#define SHAPE_HPP

#include <string>

#include "gt_message_types.h"
#include "console.h"
#include "symbol_table.hpp"

using std::string;

#define MAX_PICKUP_DIST (100.0)

inline double length(double x, double y)
{
    double sqrval = x*x + y*y;
    if(sqrval <= 0)
        return 0;

    return sqrt(sqrval);
}

/* Rewrote as getGCByColorName() in console.c.

inline GC lookup_color(const char *color, bool isXor = false)
{
    Display *dpy = NULL;
    XColor colorRec, ignore;
    XGCValues xgcv;
    Colormap cmap;
    GC gc;
    int scr;
    static symbol_table<_XGC> colorTable;
    static symbol_table<_XGC> xorColorTable;

    if (isXor)
    {
        gc = (_XGC *)xorColorTable.get(color);
    }
    else
    {
        gc = (_XGC *)colorTable.get(color);
    }

    // Allocate a new color
    if(gc == NULL)
    {
        dpy = XtDisplay(main_window);
        scr = DefaultScreen(dpy);
        cmap = DefaultColormap(dpy, scr);

        if(!XAllocNamedColor(dpy, cmap, color, &colorRec, &ignore))
        {
            warn_userf(
                "Unable to allocate object color '%s': using black\nNote:\nonly color names existing in the file \"/usr/lib/X11/rgb.txt\" are legal.",
                color);
            colorRec.pixel = BlackPixel(dpy, scr);
        }

        xgcv.foreground = colorRec.pixel;

        if (isXor)
        {
            gc = XtGetGC(drawing_area, GCForeground | GCBackground, &xgcv);
            XSetFunction(dpy, gc, GXxor);
            xorColorTable.put(color, gc);
        }
        else
        {
            gc = XtGetGC(drawing_area, GCForeground, &xgcv);
            colorTable.put(color, gc);
        }
    }

    return gc;
}
*/

class shape {

protected:
    static int last_shape_id;

public:
    int         id_;
    shape*      bound_to_;	// Null if not bound
    bool        is_hidden_;
    const char* color_;
    GC          gc_;

    shape(void) {
        id_ = last_shape_id ++; 
        bound_to_ = NULL;
        is_hidden_ = false;
        color_ = NULL;
        gc_ = NULL;
    }

    virtual ~shape(void) {} // ENDO - gcc 3.4
    virtual void bind(shape* owner) = 0;   // hides this object in owner
    virtual void unbind(shape* owner) = 0; // unhides object

    // generate an obj reading
    virtual ObjectReading to_reading(double x, double y, double heading) = 0;

    virtual void move(double x, double y) = 0;       // move the object delta x,y
    virtual double distance(double x, double y) = 0; // calc distance to pnt
    virtual void center(double &x, double &y) = 0;   // return center of obj
    virtual bool ismovable() = 0;                    // is object movable?

    virtual void draw() = 0;     // draw the object
    virtual void erase() = 0;    // erase the object

    GC gc() const { return(gc_); }
    virtual bool is_robot() const { return false; } // is this a robot?

    // Change the color of an object
    void set_color(shape *robot, char *new_color) {
        double x,y;
        center(x, y);
        double dist = robot->distance(x, y);

        if(dist < MAX_PICKUP_DIST)
        {
            erase();

            // Get rid of the old color
            if(color_)
                free((void*)color_);
            color_ = strdup(new_color);

            // Get a new gc for the new color
            // (The gc's are shared, so don't delete the old one)
            //gc_ = lookup_color(color_);
            gc_ = getGCByColorName(color_);
            draw();
        }
    };

    // Return the color of the object.
    string color(void) {
        string colorCopy = color_;
        return colorCopy;
    };
};

#endif


///////////////////////////////////////////////////////////////////////
// $Log: shape.hpp,v $
// Revision 1.1.1.1  2008/07/14 16:44:22  endo
// MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
//
// Revision 1.5  2007/08/30 18:33:39  endo
// BitmapIconDisplayer class added.
//
// Revision 1.4  2007/08/16 19:19:12  endo
// XOR GC bug fixed.
//
// Revision 1.3  2007/08/10 04:41:06  pulam
// Added scale fix and military unit drawing fix.
//
// Revision 1.2  2007/08/03 22:45:34  pulam
// Modified for military iconography
//
// Revision 1.1.1.1  2006/07/20 17:17:50  endo
// MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
//
// Revision 1.1.1.1  2006/07/12 13:37:59  endo
// MissionLab 7.0
//
// Revision 1.4  2006/05/15 01:50:11  endo
// gcc-3.4 upgrade
//
// Revision 1.3  2005/07/27 20:38:10  endo
// 3D visualization improved.
//
// Revision 1.2  2005/05/18 21:09:49  endo
// AuRA.naval added.
//
// Revision 1.1.1.1  2005/02/06 23:00:11  endo
// AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
//
// Revision 1.16  2002/04/04 18:39:51  blee
// Made last_shape_id a static member of shape.
//
// Revision 1.15  2000/04/25 07:21:45  endo
// MAX_PICKUP_DIST changed to 20.
//
// Revision 1.14  1999/12/16 22:49:45  mjcramer
// rh6 port
//
// Revision 1.13  1997/02/14  15:51:29  zchen
// *** empty log message ***
//
// Revision 1.12  1996/03/09  01:08:43  doug
// *** empty log message ***
//
// Revision 1.11  1996/03/04  22:51:58  doug
// *** empty log message ***
//
// Revision 1.10  1996/02/28  03:54:51  doug
// *** empty log message ***
//
// Revision 1.9  1996/02/27  05:00:04  doug
// *** empty log message ***
//
// Revision 1.8  1996/02/22  00:56:22  doug
// *** empty log message ***
//
// Revision 1.7  1996/02/16  00:06:44  doug
// *** empty log message ***
//
// Revision 1.6  1996/02/08  19:24:07  doug
// handle objects of any color
//
// Revision 1.5  1995/11/07  14:31:57  doug
// added baskets and command to drop oranges into them
//
// Revision 1.4  1995/10/23  18:21:57  doug
// Extend object stuff to support obstacles and also add 3d code
//
// Revision 1.3  1995/10/16  21:33:52  doug
// *** empty log message ***
//
// Revision 1.2  1995/10/03  21:25:06  doug
// add support for get_objects call
//
// Revision 1.1  1995/10/03  15:09:49  doug
// Initial revision
//
///////////////////////////////////////////////////////////////////////
