
/**********************************************************************
 **                                                                  **
 **                            robot.hpp                             **
 **                                                                  **
 **  class of robot shapes                                           **
 **                                                                  **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1995 - 2007 Georgia Tech Research Corporation         **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **********************************************************************/

/* $Id: robot.hpp,v 1.1.1.1 2008/07/14 16:44:22 endo Exp $ */


#ifndef ROBOT_HPP
#define ROBOT_HPP

#include "circle.hpp"

class robot : public circle {
public:

    //this is needed for using a different color for robots which are 
    //in 'line-of-sight'

    GC LOSgc;
    bool LOS;

    robot(double x, double y, double r, const char *color) {
        x_ = x;
        y_ = y;
        r_ = r;
        color_ = strdup(color);
        //gc_ = lookup_color(color, true);
        gc_ = getGCByColorName(color, true);
        assert( gc_ );

        // Make it xor style
        //XSetFunction(XtDisplay(drawing_area), gc_, GXxor);

        //this is needed for using a different color for robots which are 
        //in 'line-of-sight'

        LOS = false;
        //LOSgc = lookup_color("yellow", true);
        LOSgc = getGCByColorName("yellow", true);
        assert( LOSgc );

        // Make it xor style
        //XSetFunction(XtDisplay(drawing_area), LOSgc, GXxor);
    }

    // 3D version
    robot(double x, double y, double z, double r, const char *color) {
        x_ = x;
        y_ = y;
        z_ = z;
        r_ = r;
        color_ = strdup(color);
        //gc_ = lookup_color(color, true);
        gc_ = getGCByColorName(color, true);
        assert( gc_ );

        // Make it xor style
        //XSetFunction(XtDisplay(drawing_area), gc_, GXxor);

        //this is needed for using a different color for robots which are 
        //in 'line-of-sight'

        LOS = false;
        //LOSgc = lookup_color("yellow", true);
        LOSgc = getGCByColorName("yellow", true);
        assert( LOSgc );

        // Make it xor style
        //XSetFunction(XtDisplay(drawing_area), LOSgc, GXxor);
    }

    ~robot() {
        free((void *)color_);
    }

    void change_color(const char *color) {
        color_ = strdup(color);
        //gc_ = lookup_color(color, true);
        gc_ = getGCByColorName(color, true);
        assert( gc_ );
        //XSetFunction(XtDisplay(drawing_area), gc_, GXxor);
    }

    void draw() {
        // DrawFilledCircle(x_, y_, r_, theRobotGC);
    }

    void erase() {
        // DrawFilledCircle(x_, y_, r_, eraseGC);
    }

    ObjectReading to_reading(double x, double y, double heading) {
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
        rtn.x2 = x_;
        rtn.y2 = y_;

        return rtn;
    }

    bool ismovable() { return true; }

    void bind(shape *owner) {
        cerr << "Can't pickup robots\n";
    }

    void unbind(shape *owner) {
        cerr << "Can't pickup robots\n";
    }

    gt_Point whereis(void) {

        gt_Point rtn;

        rtn.x = x_;
        rtn.y = y_;
        rtn.y = z_;

        return rtn;
    }

    bool is_up() {
        return false;
    }

    virtual bool is_robot() const {return true;} // is this a robot?

};

#endif


///////////////////////////////////////////////////////////////////////
// $Log: robot.hpp,v $
// Revision 1.1.1.1  2008/07/14 16:44:22  endo
// MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
//
// Revision 1.3  2007/08/30 18:33:39  endo
// BitmapIconDisplayer class added.
//
// Revision 1.2  2007/08/16 19:19:12  endo
// XOR GC bug fixed.
//
// Revision 1.1.1.1  2006/07/20 17:17:50  endo
// MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
//
// Revision 1.1.1.1  2006/07/12 13:37:59  endo
// MissionLab 7.0
//
// Revision 1.3  2006/05/02 04:19:59  endo
// TrackTask improved for the experiment.
//
// Revision 1.2  2005/04/08 01:56:04  pulam
// addition of cnp behaviors
//
// Revision 1.1.1.1  2005/02/06 23:00:11  endo
// AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
//
// Revision 1.8  2002/10/31 20:05:29  ebeowulf
// Added the ability to change the color of a robot.
//
// Revision 1.7  2000/11/20 19:33:54  sgorbiss
// Add visualization of line-of-sight communication
//
// Revision 1.6  2000/10/16 19:39:40  endo
// Modified due to the compiler upgrade.
//
// Revision 1.5  1999/12/16 22:50:21  mjcramer
// rh6 port
//
// Revision 1.4  1997/02/12  05:41:37  zchen
// *** empty log message ***
//
// Revision 1.3  1996/03/04  22:51:58  doug
// *** empty log message ***
//
// Revision 1.2  1996/02/27  05:00:04  doug
// *** empty log message ***
//
// Revision 1.1  1996/02/26  16:04:47  doug
// Initial revision
//
///////////////////////////////////////////////////////////////////////
