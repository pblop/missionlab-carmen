/*----------------------------------------------------------------------------
 
        compass.h
 
        All the functions that mlab needs to know about to create and start
        the Compass viewer interface.
 
        Author: Jonathan F. Diaz hacked together from the 
	        telop interface by Khaled S. Ali
 
        Copyright 1999, Georgia Tech Research Corporation
        Atlanta, Georgia  30332-0415
        ALL RIGHTS RESERVED, See file COPYRIGHT for details.
 
----------------------------------------------------------------------------*/

#ifndef COMPASS_HEADER_FILE
#define COMPASS_HEADER_FILE


#ifndef _XtIntrinsic_h
extern "C" {
#include <X11/Intrinsic.h>
}
#endif /* _XtIntrinsic_h */

void  gt_draw_compass_heading();

void  gt_create_compass_interface(Widget top_level,
				  XtAppContext app); 

void  gt_popup_compass_interface(void);

#endif  /* COMPASS_HEADER_FILE */
