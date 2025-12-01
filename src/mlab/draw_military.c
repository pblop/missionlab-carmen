/**********************************************************************
 **                                                                  **
 **                           draw_military.c                        **
 **                                                                  **
 ** Written by: Patrick Ulam                                         **
 **                                                                  **
 ** Copyright 2007 Georgia Tech Research Corporation                 **
 ** Atlanta, Georgia  30332-0415                                     **
 ** ALL RIGHTS RESERVED, See file COPYRIGHT for details.             **
 **                                                                  ** 
 **********************************************************************/

/* $Id: draw_military.c,v 1.1.1.1 2008/07/14 16:44:22 endo Exp $ */

#include "gt_message_types.h"
#include "console.h"
#include "symbol_table.hpp"
#include "draw_military.h"
#include "shape.hpp"
#include "gt_simulation.h"
#include "gt_scale.h"
#include "draw.h"

#define NearXY(a, b) (fabs(a - b) < 0.0001)

void Draw_MilitaryStyle(float x, float y, float nx, float ny, int mobility, GC gc, int erase, char *color) {

    const bool SHOW_DRAW_MILITARY_STYLE_WARNING = false;

    if ((x == -1) && (y == -1)) {
        erase = 0;
    }


    x  /= meters_per_pixel;
    y  /= meters_per_pixel;
    nx  /= meters_per_pixel;
    ny  /= meters_per_pixel;

    if (!strcmp(color, "green")) {
        Draw_Neutral(x, y, nx, ny, mobility, erase, gc);
        return;
    }

  

    if (!strcmp(color, "red")) {
        Draw_Enemy(x, y, nx, ny, mobility, erase, gc);
        return;
    }
  
  
  
    if (!strcmp(color, "cyan")) {
        Draw_Friendly(x, y, nx, ny, mobility, erase,gc);
        return;
    }
  
    if (debug || SHOW_DRAW_MILITARY_STYLE_WARNING)
    {
        fprintf(stderr, "Warning(mlab): Draw_MilitaryStyle(): Color '%s' does not correspond to known unit type!\n", color);
    }
  
    // Unknown unit type. Draw as a friendly.
    Draw_Friendly(x, y, nx, ny, mobility, erase,gc);
}


// Draw neutral units
void Draw_Neutral(float x, float y, float nx, float ny, int mobility, int erase, GC gc) {
    int length, width;

    if ((mobility == UGV) || (mobility == USV) || (mobility == UUV) || (mobility == UAV)) {

        if (scale_robots) {
            length = width = (int)(robot_length_meters / meters_per_pixel);
        }
        else {
            length = width = (int)robot_length_pixels;
        }


        //GC bluegc = lookup_color("blue", true);
        GC bluegc = getGCByColorName("blue", true);

        // all neutral units are a box with different borders
        XPoint border[5];

        if (erase) {

            // erase the old box
 
            border[0].x = (int)x - (width / 2);
            border[0].y = ((int)(mission_area_height_meters / meters_per_pixel)) - (int)y - (width / 2);
            border[1].x = (int)x + (width / 2);
            border[1].y = ((int)(mission_area_height_meters / meters_per_pixel)) - (int)y - (width / 2);
            border[2].x = (int)x + (width / 2);
            border[2].y = ((int)(mission_area_height_meters / meters_per_pixel)) - (int)y + (width / 2);
            border[3].x = (int)x - (width / 2);
            border[3].y = ((int)(mission_area_height_meters / meters_per_pixel)) - (int)y + (width / 2);
            border[4].x = (int)x - (width / 2);
            border[4].y = ((int)(mission_area_height_meters / meters_per_pixel)) - (int)y - (width / 2);
    
            XFillPolygon(XtDisplay(drawing_area),
                         XtWindow(drawing_area),
                         gc,
                         border, 5, 
                         Convex,
                         CoordModeOrigin);
    
            XFillPolygon(XtDisplay(drawing_area),
                         drawing_area_pixmap,
                         gc,
                         border, 5, 
                         Convex,
                         CoordModeOrigin);
    
            XFillPolygon(XtDisplay(drawing_area),
                         XtWindow(drawing_area),
                         gGCs.whiteXOR,
                         border, 5, 
                         Convex,
                         CoordModeOrigin);
    
            XFillPolygon(XtDisplay(drawing_area),
                         drawing_area_pixmap,
                         gGCs.whiteXOR,
                         border, 5, 
                         Convex,
                         CoordModeOrigin);
    
            // erase the side lines
            XDrawLine(XtDisplay(drawing_area),
                      XtWindow(drawing_area),
                      bluegc,
                      (int)x - (width / 2) - 1, ((int)(mission_area_height_meters / meters_per_pixel)) - (int)y - (width / 2) - 1,
                      (int)x - (width / 2) - 1, ((int)(mission_area_height_meters / meters_per_pixel)) - (int)y + (width / 2));
            XDrawLine(XtDisplay(drawing_area),
                      drawing_area_pixmap,
                      bluegc,
                      (int)x - (width / 2) - 1, ((int)(mission_area_height_meters / meters_per_pixel)) - (int)y - (width / 2) - 1,
                      (int)x - (width / 2) - 1, ((int)(mission_area_height_meters / meters_per_pixel)) - (int)y + (width / 2));	      
            XDrawLine(XtDisplay(drawing_area),
                      XtWindow(drawing_area),
                      gGCs.whiteXOR,
                      (int)x - (width / 2) - 1, ((int)(mission_area_height_meters / meters_per_pixel)) - (int)y - (width / 2) - 1,
                      (int)x - (width / 2) - 1, ((int)(mission_area_height_meters / meters_per_pixel)) - (int)y + (width / 2));
            XDrawLine(XtDisplay(drawing_area),
                      drawing_area_pixmap,
                      gGCs.whiteXOR,
                      (int)x - (width / 2) - 1, ((int)(mission_area_height_meters / meters_per_pixel)) - (int)y - (width / 2) - 1,
                      (int)x - (width / 2) - 1, ((int)(mission_area_height_meters / meters_per_pixel)) - (int)y + (width / 2));
       
            XDrawLine(XtDisplay(drawing_area),
                      XtWindow(drawing_area),
                      bluegc,
                      (int)x + (width / 2) + 1, ((int)(mission_area_height_meters / meters_per_pixel)) - (int)y - (width / 2) - 1,
                      (int)x + (width / 2) + 1, ((int)(mission_area_height_meters / meters_per_pixel)) - (int)y + (width / 2));
            XDrawLine(XtDisplay(drawing_area),
                      drawing_area_pixmap,
                      bluegc,
                      (int)x + (width / 2) + 1, ((int)(mission_area_height_meters / meters_per_pixel)) - (int)y - (width / 2) - 1,
                      (int)x + (width / 2) + 1, ((int)(mission_area_height_meters / meters_per_pixel)) - (int)y + (width / 2));	      
            XDrawLine(XtDisplay(drawing_area),
                      XtWindow(drawing_area),
                      gGCs.whiteXOR,
                      (int)x + (width / 2) + 1, ((int)(mission_area_height_meters / meters_per_pixel)) - (int)y - (width / 2) - 1,
                      (int)x + (width / 2) + 1, ((int)(mission_area_height_meters / meters_per_pixel)) - (int)y + (width / 2));
            XDrawLine(XtDisplay(drawing_area),
                      drawing_area_pixmap,
                      gGCs.whiteXOR,
                      (int)x + (width / 2) + 1, ((int)(mission_area_height_meters / meters_per_pixel)) - (int)y - (width / 2) - 1,
                      (int)x + (width / 2) + 1, ((int)(mission_area_height_meters / meters_per_pixel)) - (int)y + (width / 2));
  
  
  
            // erase the top line if not a UUV
            if (mobility != UUV) {
                XDrawLine(XtDisplay(drawing_area),
                          XtWindow(drawing_area),
                          bluegc,
                          (int)x - (width / 2), ((int)(mission_area_height_meters / meters_per_pixel)) - (int)y - (width / 2) - 1,
                          (int)x + (width / 2), ((int)(mission_area_height_meters / meters_per_pixel)) - (int)y - (width / 2) - 1);
                XDrawLine(XtDisplay(drawing_area),
                          drawing_area_pixmap,
                          bluegc,
                          (int)x - (width / 2), ((int)(mission_area_height_meters / meters_per_pixel)) - (int)y - (width / 2) - 1,
                          (int)x + (width / 2), ((int)(mission_area_height_meters / meters_per_pixel)) - (int)y - (width / 2) - 1);	      
                XDrawLine(XtDisplay(drawing_area),
                          XtWindow(drawing_area),
                          gGCs.whiteXOR,
                          (int)x - (width / 2), ((int)(mission_area_height_meters / meters_per_pixel)) - (int)y - (width / 2) - 1,
                          (int)x + (width / 2), ((int)(mission_area_height_meters / meters_per_pixel)) - (int)y - (width / 2) - 1);
                XDrawLine(XtDisplay(drawing_area),
                          drawing_area_pixmap,
                          gGCs.whiteXOR,
                          (int)x - (width / 2), ((int)(mission_area_height_meters / meters_per_pixel)) - (int)y - (width / 2) - 1,
                          (int)x + (width / 2), ((int)(mission_area_height_meters / meters_per_pixel)) - (int)y - (width / 2) - 1);
            }

            // erase the bottom line if not a UAV
            if (mobility != UAV) {
                XDrawLine(XtDisplay(drawing_area),
                          XtWindow(drawing_area),
                          bluegc,
                          (int)x - (width / 2), ((int)(mission_area_height_meters / meters_per_pixel)) - (int)y + (width / 2),
                          (int)x + (width / 2), ((int)(mission_area_height_meters / meters_per_pixel)) - (int)y + (width / 2));
                XDrawLine(XtDisplay(drawing_area),
                          drawing_area_pixmap,
                          bluegc,
                          (int)x - (width / 2), ((int)(mission_area_height_meters / meters_per_pixel)) - (int)y + (width / 2),
                          (int)x + (width / 2), ((int)(mission_area_height_meters / meters_per_pixel)) - (int)y + (width / 2));	      
                XDrawLine(XtDisplay(drawing_area),
                          XtWindow(drawing_area),
                          gGCs.whiteXOR,
                          (int)x - (width / 2), ((int)(mission_area_height_meters / meters_per_pixel)) - (int)y + (width / 2),
                          (int)x + (width / 2), ((int)(mission_area_height_meters / meters_per_pixel)) - (int)y + (width / 2));
                XDrawLine(XtDisplay(drawing_area),
                          drawing_area_pixmap,
                          gGCs.whiteXOR,
                          (int)x - (width / 2), ((int)(mission_area_height_meters / meters_per_pixel)) - (int)y + (width / 2),
                          (int)x + (width / 2), ((int)(mission_area_height_meters / meters_per_pixel)) - (int)y + (width / 2));
            }
    
        }

        border[0].x = (int)nx - (width / 2);
        border[0].y = ((int)(mission_area_height_meters / meters_per_pixel)) - (int)ny - (width / 2);
        border[1].x = (int)nx + (width / 2);
        border[1].y = ((int)(mission_area_height_meters / meters_per_pixel)) - (int)ny - (width / 2);
        border[2].x = (int)nx + (width / 2);
        border[2].y = ((int)(mission_area_height_meters / meters_per_pixel)) - (int)ny + (width / 2);
        border[3].x = (int)nx - (width / 2);
        border[3].y = ((int)(mission_area_height_meters / meters_per_pixel)) - (int)ny + (width / 2);
        border[4].x = (int)nx - (width / 2);
        border[4].y = ((int)(mission_area_height_meters / meters_per_pixel)) - (int)ny - (width / 2);
    
    
    
        XFillPolygon(XtDisplay(drawing_area),
                     XtWindow(drawing_area),
                     gc,
                     border, 5, 
                     Convex,
                     CoordModeOrigin);
    
        XFillPolygon(XtDisplay(drawing_area),
                     drawing_area_pixmap,
                     gc,
                     border, 5, 
                     Convex,
                     CoordModeOrigin);
    
        XFillPolygon(XtDisplay(drawing_area),
                     XtWindow(drawing_area),
                     gGCs.whiteXOR,
                     border, 5, 
                     Convex,
                     CoordModeOrigin);
    
        XFillPolygon(XtDisplay(drawing_area),
                     drawing_area_pixmap,
                     gGCs.whiteXOR,
                     border, 5, 
                     Convex,
                     CoordModeOrigin);


        // draw the border lines
  
        XDrawLine(XtDisplay(drawing_area),
                  XtWindow(drawing_area),
                  bluegc,
                  (int)nx - (width / 2) - 1, ((int)(mission_area_height_meters / meters_per_pixel)) - (int)ny - (width / 2) - 1,
                  (int)nx - (width / 2) - 1, ((int)(mission_area_height_meters / meters_per_pixel)) - (int)ny + (width / 2));
        XDrawLine(XtDisplay(drawing_area),
                  drawing_area_pixmap,
                  bluegc,
                  (int)nx - (width / 2) - 1, ((int)(mission_area_height_meters / meters_per_pixel)) - (int)ny - (width / 2) - 1,
                  (int)nx - (width / 2) - 1, ((int)(mission_area_height_meters / meters_per_pixel)) - (int)ny + (width / 2));	      
        XDrawLine(XtDisplay(drawing_area),
                  XtWindow(drawing_area),
                  gGCs.whiteXOR,
                  (int)nx - (width / 2) - 1, ((int)(mission_area_height_meters / meters_per_pixel)) - (int)ny - (width / 2) - 1,
                  (int)nx - (width / 2) - 1, ((int)(mission_area_height_meters / meters_per_pixel)) - (int)ny + (width / 2));
        XDrawLine(XtDisplay(drawing_area),
                  drawing_area_pixmap,
                  gGCs.whiteXOR,
                  (int)nx - (width / 2) - 1, ((int)(mission_area_height_meters / meters_per_pixel)) - (int)ny - (width / 2) - 1,
                  (int)nx - (width / 2) - 1, ((int)(mission_area_height_meters / meters_per_pixel)) - (int)ny + (width / 2));
    
        XDrawLine(XtDisplay(drawing_area),
                  XtWindow(drawing_area),
                  bluegc,
                  (int)nx + (width / 2) + 1, ((int)(mission_area_height_meters / meters_per_pixel)) - (int)ny - (width / 2) - 1,
                  (int)nx + (width / 2) + 1, ((int)(mission_area_height_meters / meters_per_pixel)) - (int)ny + (width / 2));
        XDrawLine(XtDisplay(drawing_area),
                  drawing_area_pixmap,
                  bluegc,
                  (int)nx + (width / 2) + 1, ((int)(mission_area_height_meters / meters_per_pixel)) - (int)ny - (width / 2) - 1,
                  (int)nx + (width / 2) + 1, ((int)(mission_area_height_meters / meters_per_pixel)) - (int)ny + (width / 2));	      
        XDrawLine(XtDisplay(drawing_area),
                  XtWindow(drawing_area),
                  gGCs.whiteXOR,
                  (int)nx + (width / 2) + 1, ((int)(mission_area_height_meters / meters_per_pixel)) - (int)ny - (width / 2) - 1,
                  (int)nx + (width / 2) + 1, ((int)(mission_area_height_meters / meters_per_pixel)) - (int)ny + (width / 2));
        XDrawLine(XtDisplay(drawing_area),
                  drawing_area_pixmap,
                  gGCs.whiteXOR,
                  (int)nx + (width / 2) + 1, ((int)(mission_area_height_meters / meters_per_pixel)) - (int)ny - (width / 2) - 1,
                  (int)nx + (width / 2) + 1, ((int)(mission_area_height_meters / meters_per_pixel)) - (int)ny + (width / 2));
  
  
  


        if (mobility != UUV) {
            XDrawLine(XtDisplay(drawing_area),
                      XtWindow(drawing_area),
                      bluegc,
                      (int)nx - (width / 2), ((int)(mission_area_height_meters / meters_per_pixel)) - (int)ny - (width / 2) - 1,
                      (int)nx + (width / 2), ((int)(mission_area_height_meters / meters_per_pixel)) - (int)ny - (width / 2) - 1);
            XDrawLine(XtDisplay(drawing_area),
                      drawing_area_pixmap,
                      bluegc,
                      (int)nx - (width / 2), ((int)(mission_area_height_meters / meters_per_pixel)) - (int)ny - (width / 2) - 1,
                      (int)nx + (width / 2), ((int)(mission_area_height_meters / meters_per_pixel)) - (int)ny - (width / 2) - 1);	      
            XDrawLine(XtDisplay(drawing_area),
                      XtWindow(drawing_area),
                      gGCs.whiteXOR,
                      (int)nx - (width / 2), ((int)(mission_area_height_meters / meters_per_pixel)) - (int)ny - (width / 2) - 1,
                      (int)nx + (width / 2), ((int)(mission_area_height_meters / meters_per_pixel)) - (int)ny - (width / 2) - 1);
            XDrawLine(XtDisplay(drawing_area),
                      drawing_area_pixmap,
                      gGCs.whiteXOR,
                      (int)nx - (width / 2), ((int)(mission_area_height_meters / meters_per_pixel)) - (int)ny - (width / 2) - 1,
                      (int)nx + (width / 2), ((int)(mission_area_height_meters / meters_per_pixel)) - (int)ny - (width / 2) - 1);
        }
        if (mobility != UAV) {
            XDrawLine(XtDisplay(drawing_area),
                      XtWindow(drawing_area),
                      bluegc,
                      (int)nx - (width / 2), ((int)(mission_area_height_meters / meters_per_pixel)) - (int)ny + (width / 2),
                      (int)nx + (width / 2), ((int)(mission_area_height_meters / meters_per_pixel)) - (int)ny + (width / 2));
            XDrawLine(XtDisplay(drawing_area),
                      drawing_area_pixmap,
                      bluegc,
                      (int)nx - (width / 2), ((int)(mission_area_height_meters / meters_per_pixel)) - (int)ny + (width / 2),
                      (int)nx + (width / 2), ((int)(mission_area_height_meters / meters_per_pixel)) - (int)ny + (width / 2));	      
            XDrawLine(XtDisplay(drawing_area),
                      XtWindow(drawing_area),
                      gGCs.whiteXOR,
                      (int)nx - (width / 2), ((int)(mission_area_height_meters / meters_per_pixel)) - (int)ny + (width / 2),
                      (int)nx + (width / 2), ((int)(mission_area_height_meters / meters_per_pixel)) - (int)ny + (width / 2));
            XDrawLine(XtDisplay(drawing_area),
                      drawing_area_pixmap,
                      gGCs.whiteXOR,
                      (int)nx - (width / 2), ((int)(mission_area_height_meters / meters_per_pixel)) - (int)ny + (width / 2),
                      (int)nx + (width / 2), ((int)(mission_area_height_meters / meters_per_pixel)) - (int)ny + (width / 2));
        }

    }


    XFlush(XtDisplay(drawing_area));
}

void Draw_Friendly(float x, float y, float nx, float ny, int mobility, int erase, GC gc) {
  int length, width;



  if (scale_robots) {
    length = width = (int)(robot_length_meters / meters_per_pixel);
  }
  else {
    length = width = (int)robot_length_pixels;
  }

  int rx = (int)(nx - (width / 2.0));
  int ry = (int)(ny - (length / 2.0));
  int ox = (int)(x - (width / 2.0));
  int oy = (int)(y - (length / 2.0));

  //GC bluegc = lookup_color("blue", true);
  GC bluegc = getGCByColorName("blue", true);

  if (mobility == UAV) {
    
    // icon is too small...
    width *= 2;
    length *= 2;

    int orx = (int)(x - (width / 6.0));
    int ory = (int)(y + (width / 2.0));

    if (erase) {
      // erase the old arc
      XFillArc(XtDisplay(drawing_area),
	       XtWindow(drawing_area),
	       gc,
	       orx, ((int)(mission_area_height_meters / meters_per_pixel)) - (int)ory,
	       (int)(width / 1.5), (int)width * 2,
	       0, 180 * 64);
      XFillArc(XtDisplay(drawing_area),
	       drawing_area_pixmap,
	       gc,
	       (int)orx, ((int)(mission_area_height_meters / meters_per_pixel)) - (int)ory,
	       (int)(width / 1.5), (int)width * 2,
	       0, 180 * 64);	     
      XFillArc(XtDisplay(drawing_area),
	       XtWindow(drawing_area),
	       gGCs.whiteXOR,
	       (int)orx, ((int)(mission_area_height_meters / meters_per_pixel)) - (int)ory,
	       (int)(width / 1.5), (int)width * 2,
	       0, 180 * 64);
      XFillArc(XtDisplay(drawing_area),
	       drawing_area_pixmap,
	       gGCs.whiteXOR,
	       (int)orx, ((int)(mission_area_height_meters / meters_per_pixel)) - (int)ory,
	       (int)(width / 1.5), (int)width * 2,
	       0, 180 * 64);

      // erase the outline
      orx--;
      ory++;

      XDrawArc(XtDisplay(drawing_area),
	       XtWindow(drawing_area),
	       bluegc,
	       orx, ((int)(mission_area_height_meters / meters_per_pixel)) - (int)ory,
	       (int)(width / 1.5), (int)width * 2,
	       0, 180 * 64);
      XDrawArc(XtDisplay(drawing_area),
	       drawing_area_pixmap,
	       bluegc,
	       (int)orx, ((int)(mission_area_height_meters / meters_per_pixel)) - (int)ory,
	       (int)(width / 1.5), (int)width * 2,
	       0, 180 * 64);	     
      XDrawArc(XtDisplay(drawing_area),
	       XtWindow(drawing_area),
	       gGCs.whiteXOR,
	       (int)orx, ((int)(mission_area_height_meters / meters_per_pixel)) - (int)ory,
	       (int)(width / 1.5), (int)width * 2,
	       0, 180 * 64);
      XDrawArc(XtDisplay(drawing_area),
	       drawing_area_pixmap,
	       gGCs.whiteXOR,
	       (int)orx, ((int)(mission_area_height_meters / meters_per_pixel)) - (int)ory,
	       (int)(width / 1.5), (int)width * 2,
	       0, 180 * 64);
    }



    // draw the new arc
    orx = (int)(nx - (width / 6.0));
    ory = (int)(ny + (width / 2.0));

    XFillArc(XtDisplay(drawing_area),
	     XtWindow(drawing_area),
	     gc,
	     orx, ((int)(mission_area_height_meters / meters_per_pixel)) - (int)ory,
	     (int)(width / 1.5), (int)width * 2,
	     0, 180 * 64);
    XFillArc(XtDisplay(drawing_area),
	     drawing_area_pixmap,
	     gc,
	     (int)orx, ((int)(mission_area_height_meters / meters_per_pixel)) - (int)ory,
	     (int)(width / 1.5), (int)width * 2,
	     0, 180 * 64);	     
    XFillArc(XtDisplay(drawing_area),
	     XtWindow(drawing_area),
	     gGCs.whiteXOR,
	     (int)orx, ((int)(mission_area_height_meters / meters_per_pixel)) - (int)ory,
	     (int)(width / 1.5), (int)width * 2,
	     0, 180 * 64);
    XFillArc(XtDisplay(drawing_area),
	     drawing_area_pixmap,
	     gGCs.whiteXOR,
	     (int)orx, ((int)(mission_area_height_meters / meters_per_pixel)) - (int)ory,
	     (int)(width / 1.5), (int)width * 2,
	     0, 180 * 64);	  

    orx--;
    ory++;

    XDrawArc(XtDisplay(drawing_area),
	     XtWindow(drawing_area),
	     bluegc,
	     orx, ((int)(mission_area_height_meters / meters_per_pixel)) - (int)ory,
	     (int)(width / 1.5), (int)width * 2,
	     0, 180 * 64);
    XDrawArc(XtDisplay(drawing_area),
	     drawing_area_pixmap,
	     bluegc,
	     (int)orx, ((int)(mission_area_height_meters / meters_per_pixel)) - (int)ory,
	     (int)(width / 1.5), (int)width * 2,
	     0, 180 * 64);	     
    XDrawArc(XtDisplay(drawing_area),
	     XtWindow(drawing_area),
	     gGCs.whiteXOR,
	     (int)orx, ((int)(mission_area_height_meters / meters_per_pixel)) - (int)ory,
	     (int)(width / 1.5), (int)width * 2,
	     0, 180 * 64);
    XDrawArc(XtDisplay(drawing_area),
	     drawing_area_pixmap,
	     gGCs.whiteXOR,
	     (int)orx, ((int)(mission_area_height_meters / meters_per_pixel)) - (int)ory,
	     (int)(width / 1.5), (int)width * 2,
	     0, 180 * 64);

  }





  // draw a circle
  if (mobility == USV) {

    ox = (int)x - (width / 2);
    oy = (int)y + (width / 2);
  

    if (erase) {
      // erase the circle
      XFillArc(XtDisplay(drawing_area),
	       XtWindow(drawing_area),
	       gc,
	       ox, ((int)(mission_area_height_meters / meters_per_pixel)) - (int)oy,
	       (int)width, (int)width,
	       0, 23040);
      XFillArc(XtDisplay(drawing_area),
	       drawing_area_pixmap,
	       gc,
	       (int)ox, ((int)(mission_area_height_meters / meters_per_pixel)) - (int)oy,
	       (int)width, (int)width,
	       0, 23040);	     
      XFillArc(XtDisplay(drawing_area),
	       XtWindow(drawing_area),
	       gGCs.whiteXOR,
	       (int)ox, ((int)(mission_area_height_meters / meters_per_pixel)) - (int)oy,
	       (int)width, (int)width,
	       0, 23040);
      XFillArc(XtDisplay(drawing_area),
	       drawing_area_pixmap,
	       gGCs.whiteXOR,
	       (int)ox, ((int)(mission_area_height_meters / meters_per_pixel)) - (int)oy,
	       (int)width, (int)width,
	       0, 23040);	  

      // erase the outline
      XDrawArc(XtDisplay(drawing_area),
	       XtWindow(drawing_area), 
	       bluegc,
	       ox, ((int)(mission_area_height_meters / meters_per_pixel)) - (int)oy,
	       (int)width + 1, (int)width + 1,
	       0, 23040);	 
      XDrawArc(XtDisplay(drawing_area),
	       drawing_area_pixmap, 
	       bluegc,
	       ox, ((int)(mission_area_height_meters / meters_per_pixel)) - (int)oy,
	       (int)width + 1, (int)width + 1,
	       0, 23040);	
      XDrawArc(XtDisplay(drawing_area),
	       XtWindow(drawing_area), 
	       gGCs.whiteXOR,
	       ox, ((int)(mission_area_height_meters / meters_per_pixel)) - (int)oy,
	       (int)width + 1, (int)width + 1,
	       0, 23040);
      XDrawArc(XtDisplay(drawing_area),
	       drawing_area_pixmap, 
	       gGCs.whiteXOR,
	       ox, ((int)(mission_area_height_meters / meters_per_pixel)) - (int)oy,
	       (int)width + 1, (int)width + 1,
	       0, 23040);
    }

    rx = (int)nx  - (width / 2);
    ry = (int)ny  + (width / 2) ;

    // draw the circle
    XFillArc(XtDisplay(drawing_area),
	     XtWindow(drawing_area),
	     gc,
	     (int)rx, ((int)(mission_area_height_meters / meters_per_pixel)) - (int)ry,
	     (int)width, (int)width,
	     0, 23040);
    XFillArc(XtDisplay(drawing_area),
	     drawing_area_pixmap,
	     gc,
	     (int)rx, ((int)(mission_area_height_meters / meters_per_pixel)) - (int)ry,
	     (int)width, (int)width,
	     0, 23040);	     
    XFillArc(XtDisplay(drawing_area),
	     XtWindow(drawing_area),
	     gGCs.whiteXOR,
	     (int)rx, ((int)(mission_area_height_meters / meters_per_pixel)) - (int)ry,
	     (int)width, (int)width,
	     0, 23040);
    XFillArc(XtDisplay(drawing_area),
	     drawing_area_pixmap,
	     gGCs.whiteXOR,
	     (int)rx, ((int)(mission_area_height_meters / meters_per_pixel)) - (int)ry,
	     (int)width, (int)width,
	     0, 23040);	  

    // draw the outline
    XDrawArc(XtDisplay(drawing_area),
	     XtWindow(drawing_area), 
	     bluegc,
	     rx, ((int)(mission_area_height_meters / meters_per_pixel)) - (int)ry,
	     (int)width + 1, (int)width + 1,
	     0, 23040);	 
    XDrawArc(XtDisplay(drawing_area),
	     drawing_area_pixmap, 
	     bluegc,
	     rx, ((int)(mission_area_height_meters / meters_per_pixel)) - (int)ry,
	     (int)width + 1, (int)width + 1,
	     0, 23040);	 
    XDrawArc(XtDisplay(drawing_area),
	     XtWindow(drawing_area), 
	     gGCs.whiteXOR,
	     rx, ((int)(mission_area_height_meters / meters_per_pixel)) - (int)ry,
	     (int)width + 1, (int)width + 1,
	     0, 23040);
    XDrawArc(XtDisplay(drawing_area),
	     drawing_area_pixmap, 
	     gGCs.whiteXOR,
	     rx, ((int)(mission_area_height_meters / meters_per_pixel)) - (int)ry,
	     (int)width + 1, (int)width + 1,
	     0, 23040);

  }






  // draw a rectangle
  if (mobility == UGV) {
    XPoint border[5];


    if (erase) {
      // erase the box
      border[0].x = (int)x - (width / 2) - (width / 4);
      border[0].y = ((int)(mission_area_height_meters / meters_per_pixel)) - (int)y - (width / 2);
      border[1].x = (int)x + (width / 2) + (width / 4);
      border[1].y = ((int)(mission_area_height_meters / meters_per_pixel)) - (int)y - (width / 2);
      border[2].x = (int)x + (width / 2) + (width / 4);
      border[2].y = ((int)(mission_area_height_meters / meters_per_pixel)) - (int)y + (width / 2);
      border[3].x = (int)x - (width / 2) - (width / 4);
      border[3].y = ((int)(mission_area_height_meters / meters_per_pixel)) - (int)y + (width / 2);
      border[4].x = (int)x - (width / 2) - (width / 4);
      border[4].y = ((int)(mission_area_height_meters / meters_per_pixel)) - (int)y - (width / 2);
      XFillPolygon(XtDisplay(drawing_area),
		   XtWindow(drawing_area),
		   gc,
		   border, 5, 
		   Convex,
		   CoordModeOrigin);
    
      XFillPolygon(XtDisplay(drawing_area),
		   drawing_area_pixmap,
		   gc,
		   border, 5, 
		   Convex,
		   CoordModeOrigin);
    
      XFillPolygon(XtDisplay(drawing_area),
		   XtWindow(drawing_area),
		   gGCs.whiteXOR,
		   border, 5, 
		   Convex,
		   CoordModeOrigin);
    
      XFillPolygon(XtDisplay(drawing_area),
		   drawing_area_pixmap,
		   gGCs.whiteXOR,
		   border, 5, 
		   Convex,
		   CoordModeOrigin);    



  
          
      // earase the outline
      border[0].x = (int)x - (width / 2) - (width / 4) - 1;
      border[0].y = ((int)(mission_area_height_meters / meters_per_pixel)) - (int)y - (width / 2) - 1;
      border[1].x = (int)x + (width / 2) + (width / 4);
      border[1].y = ((int)(mission_area_height_meters / meters_per_pixel)) - (int)y - (width / 2) - 1;
      border[2].x = (int)x + (width / 2) + (width / 4);
      border[2].y = ((int)(mission_area_height_meters / meters_per_pixel)) - (int)y + (width / 2);
      border[3].x = (int)x - (width / 2) - (width / 4) - 1;
      border[3].y = ((int)(mission_area_height_meters / meters_per_pixel)) - (int)y + (width / 2);
      border[4].x = (int)x - (width / 2) - (width / 4) - 1;
      border[4].y = ((int)(mission_area_height_meters / meters_per_pixel)) - (int)y - (width / 2) - 1;

      XDrawLines(XtDisplay(drawing_area),
		 XtWindow(drawing_area),
		 bluegc,
		 border, 5, 
		 CoordModeOrigin);
    
      XDrawLines(XtDisplay(drawing_area),
		 drawing_area_pixmap,
		 bluegc,
		 border, 5, 
		 CoordModeOrigin);
    
      XDrawLines(XtDisplay(drawing_area),
		 XtWindow(drawing_area),
		 gGCs.whiteXOR,
		 border, 5, 
		 CoordModeOrigin);
    
      XDrawLines(XtDisplay(drawing_area),
		 drawing_area_pixmap,
		 gGCs.whiteXOR,
		 border, 5, 
		 CoordModeOrigin);

    }

    // draw the rectangle
    border[0].x = (int)nx - (width / 2) - (width / 4);
    border[0].y = ((int)(mission_area_height_meters / meters_per_pixel)) - (int)ny - (width / 2);
    border[1].x = (int)nx + (width / 2) + (width / 4);
    border[1].y = ((int)(mission_area_height_meters / meters_per_pixel)) - (int)ny - (width / 2);
    border[2].x = (int)nx + (width / 2) + (width / 4);
    border[2].y = ((int)(mission_area_height_meters / meters_per_pixel)) - (int)ny + (width / 2);
    border[3].x = (int)nx - (width / 2) - (width / 4);
    border[3].y = ((int)(mission_area_height_meters / meters_per_pixel)) - (int)ny + (width / 2);
    border[4].x = (int)nx - (width / 2) - (width / 4);
    border[4].y = ((int)(mission_area_height_meters / meters_per_pixel)) - (int)ny - (width / 2);



    XFillPolygon(XtDisplay(drawing_area),
		 XtWindow(drawing_area),
		 gc,
		 border, 6, 
		 Convex,
		 CoordModeOrigin);
    
    XFillPolygon(XtDisplay(drawing_area),
		 drawing_area_pixmap,
		 gc,
		 border, 6, 
		 Convex,
		 CoordModeOrigin);
    
    XFillPolygon(XtDisplay(drawing_area),
		 XtWindow(drawing_area),
		 gGCs.whiteXOR,
		 border, 6, 
		 Convex,
		 CoordModeOrigin);
    
    XFillPolygon(XtDisplay(drawing_area),
		 drawing_area_pixmap,
		 gGCs.whiteXOR,
		 border, 6, 
		 Convex,
		 CoordModeOrigin);


    // draw the outline
    border[0].x = (int)nx - (width / 2) - (width / 4) - 1;
    border[0].y = ((int)(mission_area_height_meters / meters_per_pixel)) - (int)ny - (width / 2) - 1;
    border[1].x = (int)nx + (width / 2) + (width / 4);
    border[1].y = ((int)(mission_area_height_meters / meters_per_pixel)) - (int)ny - (width / 2) - 1;
    border[2].x = (int)nx + (width / 2) + (width / 4);
    border[2].y = ((int)(mission_area_height_meters / meters_per_pixel)) - (int)ny + (width / 2);
    border[3].x = (int)nx - (width / 2) - (width / 4) - 1;
    border[3].y = ((int)(mission_area_height_meters / meters_per_pixel)) - (int)ny + (width / 2);
    border[4].x = (int)nx - (width / 2) - (width / 4) - 1;
    border[4].y = ((int)(mission_area_height_meters / meters_per_pixel)) - (int)ny - (width / 2) - 1;

    XDrawLines(XtDisplay(drawing_area),
	       XtWindow(drawing_area),
	       bluegc,
	       border, 5, 
	       CoordModeOrigin);
    
    XDrawLines(XtDisplay(drawing_area),
	       drawing_area_pixmap,
	       bluegc,
	       border, 5, 
	       CoordModeOrigin);
    
    XDrawLines(XtDisplay(drawing_area),
	       XtWindow(drawing_area),
	       gGCs.whiteXOR,
	       border, 5, 
	       CoordModeOrigin);
    
    XDrawLines(XtDisplay(drawing_area),
	       drawing_area_pixmap,
	       gGCs.whiteXOR,
	       border, 5, 
	       CoordModeOrigin);

  }


  if (mobility == UUV) {
    // icon is too small...
    width *= 2;
    length *= 2;

    

    int orx = (int)(x - (width / 6.0));
    int ory = (int)(y + (width * 1.5));


    if (erase) {
      // erase the old arc
      XFillArc(XtDisplay(drawing_area),
	       XtWindow(drawing_area),
	       gc,
	       orx, ((int)(mission_area_height_meters / meters_per_pixel)) - (int)ory,
	       (int)(width / 1.5), (int)width * 2,
	       0, -180 * 64);
      XFillArc(XtDisplay(drawing_area),
	       drawing_area_pixmap,
	       gc,
	       (int)orx, ((int)(mission_area_height_meters / meters_per_pixel)) - (int)ory,
	       (int)(width / 1.5), (int)width * 2,
	       0, -180 * 64);	     
      XFillArc(XtDisplay(drawing_area),
	       XtWindow(drawing_area),
	       gGCs.whiteXOR,
	       (int)orx, ((int)(mission_area_height_meters / meters_per_pixel)) - (int)ory,
	       (int)(width / 1.5), (int)width * 2,
	       0, -180 * 64);
      XFillArc(XtDisplay(drawing_area),
	       drawing_area_pixmap,
	       gGCs.whiteXOR,
	       (int)orx, ((int)(mission_area_height_meters / meters_per_pixel)) - (int)ory,
	       (int)(width / 1.5), (int)width * 2,
	       0, -180 * 64);

      // erase the outline
      orx--;
      ory++;

      XDrawArc(XtDisplay(drawing_area),
	       XtWindow(drawing_area),
	       bluegc,
	       orx, ((int)(mission_area_height_meters / meters_per_pixel)) - (int)ory,
	       (int)(width / 1.5), (int)width * 2,
	       0, -180 * 64);
      XDrawArc(XtDisplay(drawing_area),
	       drawing_area_pixmap,
	       bluegc,
	       (int)orx, ((int)(mission_area_height_meters / meters_per_pixel)) - (int)ory,
	       (int)(width / 1.5), (int)width * 2,
	       0, -180 * 64);	     
      XDrawArc(XtDisplay(drawing_area),
	       XtWindow(drawing_area),
	       gGCs.whiteXOR,
	       (int)orx, ((int)(mission_area_height_meters / meters_per_pixel)) - (int)ory,
	       (int)(width / 1.5), (int)width * 2,
	       0, -180 * 64);
      XDrawArc(XtDisplay(drawing_area),
	       drawing_area_pixmap,
	       gGCs.whiteXOR,
	       (int)orx, ((int)(mission_area_height_meters / meters_per_pixel)) - (int)ory,
	       (int)(width / 1.5), (int)width * 2,
	       0, -180 * 64);


    }

    // draw the new arc
    orx = (int)(nx - (width / 6.0));
    ory = (int)(ny + (width * 1.5));

    XFillArc(XtDisplay(drawing_area),
	     XtWindow(drawing_area),
	     gc,
	     orx, ((int)(mission_area_height_meters / meters_per_pixel)) - (int)ory,
	     (int)(width / 1.5), (int)width * 2,
	     0, -180 * 64);
    XFillArc(XtDisplay(drawing_area),
	     drawing_area_pixmap,
	     gc,
	     (int)orx, ((int)(mission_area_height_meters / meters_per_pixel)) - (int)ory,
	     (int)(width / 1.5), (int)width * 2,
	     0, -180 * 64);	     
    XFillArc(XtDisplay(drawing_area),
	     XtWindow(drawing_area),
	     gGCs.whiteXOR,
	     (int)orx, ((int)(mission_area_height_meters / meters_per_pixel)) - (int)ory,
	     (int)(width / 1.5), (int)width * 2,
	     0, -180 * 64);
    XFillArc(XtDisplay(drawing_area),
	     drawing_area_pixmap,
	     gGCs.whiteXOR,
	     (int)orx, ((int)(mission_area_height_meters / meters_per_pixel)) - (int)ory,
	     (int)(width / 1.5), (int)width * 2,
	     0, -180 * 64);	  

    orx--;
    ory++;

    XDrawArc(XtDisplay(drawing_area),
	     XtWindow(drawing_area),
	     bluegc,
	     orx, ((int)(mission_area_height_meters / meters_per_pixel)) - (int)ory,
	     (int)(width / 1.5), (int)width * 2,
	     0, -180 * 64);
    XDrawArc(XtDisplay(drawing_area),
	     drawing_area_pixmap,
	     bluegc,
	     (int)orx, ((int)(mission_area_height_meters / meters_per_pixel)) - (int)ory,
	     (int)(width / 1.5), (int)width * 2,
	     0, -180 * 64);	     
    XDrawArc(XtDisplay(drawing_area),
	     XtWindow(drawing_area),
	     gGCs.whiteXOR,
	     (int)orx, ((int)(mission_area_height_meters / meters_per_pixel)) - (int)ory,
	     (int)(width / 1.5), (int)width * 2,
	     0, -180 * 64);
    XDrawArc(XtDisplay(drawing_area),
	     drawing_area_pixmap,
	     gGCs.whiteXOR,
	     (int)orx, ((int)(mission_area_height_meters / meters_per_pixel)) - (int)ory,
	     (int)(width / 1.5), (int)width * 2,
	     0, -180 * 64);

  }


  XFlush(XtDisplay(drawing_area));
}

void Draw_Enemy(float x, float y, float nx, float ny, int mobility, int erase, GC gc) {
  int length, width;

  const bool DRAW_UUV_UAV_OUTLINE = true;


  if (scale_robots) {
    length = width = (int)(robot_length_meters / meters_per_pixel);
  }
  else {
    length = width = (int)robot_length_pixels;
  }

  //bluegc = lookup_color("blue", true);
  GC bluegc = getGCByColorName("blue", true);


  if (mobility == UAV) {
    XPoint border[6];


    if (erase) {
      border[0].x = (int)x;
      border[0].y = ((int)(mission_area_height_meters / meters_per_pixel)) - (int)y - (width / 2);
      border[1].x = (int)x + (width / 2);
      border[1].y = ((int)(mission_area_height_meters / meters_per_pixel)) - (int)y;
      border[2].x = (int)x + (width / 2);
      border[2].y = ((int)(mission_area_height_meters / meters_per_pixel)) - (int)y + (width / 2);
      border[3].x = (int)x - (width / 2);
      border[3].y = ((int)(mission_area_height_meters / meters_per_pixel)) - (int)y + (width / 2);
      border[4].x = (int)x - (width / 2);
      border[4].y = ((int)(mission_area_height_meters / meters_per_pixel)) - (int)y;
      border[5].x = (int)x;
      border[5].y = ((int)(mission_area_height_meters / meters_per_pixel)) - (int)y - (width / 2); 
    


      XFillPolygon(XtDisplay(drawing_area),
		   XtWindow(drawing_area),
		   gc,
		   border, 6, 
		   Convex,
		   CoordModeOrigin);
    
      XFillPolygon(XtDisplay(drawing_area),
		   drawing_area_pixmap,
		   gc,
		   border, 6, 
		   Convex,
		   CoordModeOrigin);
    
      XFillPolygon(XtDisplay(drawing_area),
		   XtWindow(drawing_area),
		   gGCs.whiteXOR,
		   border, 6, 
		   Convex,
		   CoordModeOrigin);
    
      XFillPolygon(XtDisplay(drawing_area),
		   drawing_area_pixmap,
		   gGCs.whiteXOR,
		   border, 6, 
		   Convex,
		   CoordModeOrigin);

      if (DRAW_UUV_UAV_OUTLINE) {
	// erase outline
	border[0].x = (int)x - (width / 2) - 1;
	border[0].y = ((int)(mission_area_height_meters / meters_per_pixel)) - (int)y + (width / 2) ;
	border[1].x = (int)x - (width / 2) - 1;
	border[1].y = ((int)(mission_area_height_meters / meters_per_pixel)) - (int)y - 1;
	border[2].x = (int)x - 1;
	border[2].y = ((int)(mission_area_height_meters / meters_per_pixel)) - (int)y - (width / 2) - 1;
	border[3].x = (int)x + (width / 2);
	border[3].y = ((int)(mission_area_height_meters / meters_per_pixel)) - (int)y - 1;
	border[4].x = (int)x + (width / 2);
	border[4].y = ((int)(mission_area_height_meters / meters_per_pixel)) - (int)y + (width / 2);
      
	XDrawLines(XtDisplay(drawing_area),
		   XtWindow(drawing_area),
		   bluegc,
		   border, 5, 
		   CoordModeOrigin);
      
	XDrawLines(XtDisplay(drawing_area),
		   drawing_area_pixmap,
		   bluegc,
		   border, 5, 
		   CoordModeOrigin);
      
	XDrawLines(XtDisplay(drawing_area),
		   XtWindow(drawing_area),
		   gGCs.whiteXOR,
		   border, 5, 
		   CoordModeOrigin);
      
	XDrawLines(XtDisplay(drawing_area),
		   drawing_area_pixmap,
		   gGCs.whiteXOR,
		   border, 5, 
		   CoordModeOrigin);
      
      }
    }    


    // draw the new position
    // fill in the whole thing with black

    border[0].x = (int)nx;
    border[0].y = ((int)(mission_area_height_meters / meters_per_pixel)) - (int)ny - (width / 2);
    border[1].x = (int)nx + (width / 2);
    border[1].y = ((int)(mission_area_height_meters / meters_per_pixel)) - (int)ny;
    border[2].x = (int)nx + (width / 2);
    border[2].y = ((int)(mission_area_height_meters / meters_per_pixel)) - (int)ny + (width / 2);
    border[3].x = (int)nx - (width / 2);
    border[3].y = ((int)(mission_area_height_meters / meters_per_pixel)) - (int)ny + (width / 2);
    border[4].x = (int)nx - (width / 2);
    border[4].y = ((int)(mission_area_height_meters / meters_per_pixel)) - (int)ny;
    border[5].x = (int)nx;
    border[5].y = ((int)(mission_area_height_meters / meters_per_pixel)) - (int)ny - (width / 2); 
    

    XFillPolygon(XtDisplay(drawing_area),
		 XtWindow(drawing_area),
		 gc,
		 border, 6, 
		 Convex,
		 CoordModeOrigin);
    
    XFillPolygon(XtDisplay(drawing_area),
		 drawing_area_pixmap,
		 gc,
		 border, 6, 
		 Convex,
		 CoordModeOrigin);
    
    XFillPolygon(XtDisplay(drawing_area),
		 XtWindow(drawing_area),
		 gGCs.whiteXOR,
		 border, 6, 
		 Convex,
		 CoordModeOrigin);
    
    XFillPolygon(XtDisplay(drawing_area),
		 drawing_area_pixmap,
		 gGCs.whiteXOR,
		 border, 6, 
		 Convex,
		 CoordModeOrigin);

    // draw new outline
    if (DRAW_UUV_UAV_OUTLINE) { 
      border[0].x = (int)nx - (width / 2) - 1;
      border[0].y = ((int)(mission_area_height_meters / meters_per_pixel)) - (int)ny + (width / 2);
      border[1].x = (int)nx - (width / 2) - 1;
      border[1].y = ((int)(mission_area_height_meters / meters_per_pixel)) - (int)ny - 1;
      border[2].x = (int)nx - 1;
      border[2].y = ((int)(mission_area_height_meters / meters_per_pixel)) - (int)ny - (width / 2) - 1;
      border[3].x = (int)nx + (width / 2);
      border[3].y = ((int)(mission_area_height_meters / meters_per_pixel)) - (int)ny - 1;
      border[4].x = (int)nx + (width / 2);
      border[4].y = ((int)(mission_area_height_meters / meters_per_pixel)) - (int)ny + (width / 2);
      
      XDrawLines(XtDisplay(drawing_area),
		 XtWindow(drawing_area),
		 bluegc,
		 border, 5, 
		 CoordModeOrigin);
      
      XDrawLines(XtDisplay(drawing_area),
		 drawing_area_pixmap,
		 bluegc,
		 border, 5, 
		 CoordModeOrigin);
      
      XDrawLines(XtDisplay(drawing_area),
		 XtWindow(drawing_area),
		 gGCs.whiteXOR,
		 border, 5, 
		 CoordModeOrigin);
      
      XDrawLines(XtDisplay(drawing_area),
		 drawing_area_pixmap,
		 gGCs.whiteXOR,
		 border, 5, 
		 CoordModeOrigin);
    }



  }
  
  else if (mobility == UUV) {
    XPoint border[6];



    if (erase) {
      border[0].x = (int)x;
      border[0].y = ((int)(mission_area_height_meters / meters_per_pixel)) - (int)y + (width / 2);
      border[1].x = (int)x + (width / 2);
      border[1].y = ((int)(mission_area_height_meters / meters_per_pixel)) - (int)y;
      border[2].x = (int)x + (width / 2);
      border[2].y = ((int)(mission_area_height_meters / meters_per_pixel)) - (int)y - (width / 2);
      border[3].x = (int)x - (width / 2);
      border[3].y = ((int)(mission_area_height_meters / meters_per_pixel)) - (int)y - (width / 2);
      border[4].x = (int)x - (width / 2);
      border[4].y = ((int)(mission_area_height_meters / meters_per_pixel)) - (int)y;
      border[5].x = (int)x;
      border[5].y = ((int)(mission_area_height_meters / meters_per_pixel)) - (int)y + (width / 2); 
    

      XFillPolygon(XtDisplay(drawing_area),
		   XtWindow(drawing_area),
		   gc,
		   border, 6, 
		   Convex,
		   CoordModeOrigin);
    
      XFillPolygon(XtDisplay(drawing_area),
		   drawing_area_pixmap,
		   gc,
		   border, 6, 
		   Convex,
		   CoordModeOrigin);
    
      XFillPolygon(XtDisplay(drawing_area),
		   XtWindow(drawing_area),
		   gGCs.whiteXOR,
		   border, 6, 
		   Convex,
		   CoordModeOrigin);
    
      XFillPolygon(XtDisplay(drawing_area),
		   drawing_area_pixmap,
		   gGCs.whiteXOR,
		   border, 6, 
		   Convex,
		   CoordModeOrigin);

      // erase outline
      if (DRAW_UUV_UAV_OUTLINE) {

	border[0].x = (int)x - (width / 2) - 1;
	border[0].y = ((int)(mission_area_height_meters / meters_per_pixel)) - (int)y - (width / 2);
	border[1].x = (int)x - (width / 2) - 1;
	border[1].y = ((int)(mission_area_height_meters / meters_per_pixel)) - (int)y + 1;
	border[2].x = (int)x;
	border[2].y = ((int)(mission_area_height_meters / meters_per_pixel)) - (int)y + (width / 2) + 1;
	border[3].x = (int)x + (width / 2);
	border[3].y = ((int)(mission_area_height_meters / meters_per_pixel)) - (int)y + 1;
	border[4].x = (int)x + (width / 2);
	border[4].y = ((int)(mission_area_height_meters / meters_per_pixel)) - (int)y - (width / 2);
      
	XDrawLines(XtDisplay(drawing_area),
		   XtWindow(drawing_area),
		   bluegc,
		   border, 5, 
		   CoordModeOrigin);
      
	XDrawLines(XtDisplay(drawing_area),
		   drawing_area_pixmap,
		   bluegc,
		   border, 5, 
		   CoordModeOrigin);
      
	XDrawLines(XtDisplay(drawing_area),
		   XtWindow(drawing_area),
		   gGCs.whiteXOR,
		   border, 5, 
		   CoordModeOrigin);
      
	XDrawLines(XtDisplay(drawing_area),
		   drawing_area_pixmap,
		   gGCs.whiteXOR,
		   border, 5, 
		   CoordModeOrigin);
      
      }
    }


          
    // draw the new position
    // fill in the whole thing with black

    border[0].x = (int)nx;
    border[0].y = ((int)(mission_area_height_meters / meters_per_pixel)) - (int)ny + (width / 2);
    border[1].x = (int)nx + (width / 2);
    border[1].y = ((int)(mission_area_height_meters / meters_per_pixel)) - (int)ny;
    border[2].x = (int)nx + (width / 2);
    border[2].y = ((int)(mission_area_height_meters / meters_per_pixel)) - (int)ny - (width / 2);
    border[3].x = (int)nx - (width / 2);
    border[3].y = ((int)(mission_area_height_meters / meters_per_pixel)) - (int)ny - (width / 2);
    border[4].x = (int)nx - (width / 2);
    border[4].y = ((int)(mission_area_height_meters / meters_per_pixel)) - (int)ny;
    border[5].x = (int)nx;
    border[5].y = ((int)(mission_area_height_meters / meters_per_pixel)) - (int)ny + (width / 2); 
    

    XFillPolygon(XtDisplay(drawing_area),
		 XtWindow(drawing_area),
		 gc,
		 border, 6, 
		 Convex,
		 CoordModeOrigin);
    
    XFillPolygon(XtDisplay(drawing_area),
		 drawing_area_pixmap,
		 gc,
		 border, 6, 
		 Convex,
		 CoordModeOrigin);
    
    XFillPolygon(XtDisplay(drawing_area),
		 XtWindow(drawing_area),
		 gGCs.whiteXOR,
		 border, 6, 
		 Convex,
		 CoordModeOrigin);
    
    XFillPolygon(XtDisplay(drawing_area),
		 drawing_area_pixmap,
		 gGCs.whiteXOR,
		 border, 6, 
		 Convex,
		 CoordModeOrigin);

    // draw outline
    if (DRAW_UUV_UAV_OUTLINE) {
      border[0].x = (int)nx - (width / 2) - 1;
      border[0].y = ((int)(mission_area_height_meters / meters_per_pixel)) - (int)ny - (width / 2);
      border[1].x = (int)nx - (width / 2) - 1;
      border[1].y = ((int)(mission_area_height_meters / meters_per_pixel)) - (int)ny + 1;
      border[2].x = (int)nx;
      border[2].y = ((int)(mission_area_height_meters / meters_per_pixel)) - (int)ny + (width / 2) + 1;
      border[3].x = (int)nx + (width / 2);
      border[3].y = ((int)(mission_area_height_meters / meters_per_pixel)) - (int)ny + 1;
      border[4].x = (int)nx + (width / 2);
      border[4].y = ((int)(mission_area_height_meters / meters_per_pixel)) - (int)ny - (width / 2);
      
      XDrawLines(XtDisplay(drawing_area),
		 XtWindow(drawing_area),
		 bluegc,
		 border, 5, 
		 CoordModeOrigin);
      
      XDrawLines(XtDisplay(drawing_area),
		 drawing_area_pixmap,
		 bluegc,
		 border, 5, 
		 CoordModeOrigin);
      
      XDrawLines(XtDisplay(drawing_area),
		 XtWindow(drawing_area),
		 gGCs.whiteXOR,
		 border, 5, 
		 CoordModeOrigin);
      
      XDrawLines(XtDisplay(drawing_area),
		 drawing_area_pixmap,
		 gGCs.whiteXOR,
		 border, 5, 
		 CoordModeOrigin);

    }



  }

  else {


    // erase the old robot 

    XPoint border[5];
  
    int top = (int)(sqrt(width * width + width * width) / 2);

    if (erase) {
      border[0].x = (int)x;
      border[0].y = ((int)(mission_area_height_meters / meters_per_pixel)) - (int)y + top;
      border[1].x = (int)x + top;
      border[1].y = ((int)(mission_area_height_meters / meters_per_pixel)) - (int)y;
      border[2].x = (int)x;
      border[2].y = ((int)(mission_area_height_meters / meters_per_pixel)) - (int)y - top;
      border[3].x = (int)x - top;
      border[3].y = ((int)(mission_area_height_meters / meters_per_pixel)) - (int)y;
      border[4].x = (int)x;
      border[4].y = ((int)(mission_area_height_meters / meters_per_pixel)) - (int)y + top;
    

      XFillPolygon(XtDisplay(drawing_area),
		   XtWindow(drawing_area),
		   gc,
		   border, 5, 
		   Convex,
		   CoordModeOrigin);
    
      XFillPolygon(XtDisplay(drawing_area),
		   drawing_area_pixmap,
		   gc,
		   border, 5, 
		   Convex,
		   CoordModeOrigin);
    
      XFillPolygon(XtDisplay(drawing_area),
		   XtWindow(drawing_area),
		   gGCs.whiteXOR,
		   border, 5, 
		   Convex,
		   CoordModeOrigin);
    
      XFillPolygon(XtDisplay(drawing_area),
		   drawing_area_pixmap,
		   gGCs.whiteXOR,
		   border, 5, 
		   Convex,
		   CoordModeOrigin);


      // erase outline
      border[0].x = (int)x;
      border[0].y = ((int)(mission_area_height_meters / meters_per_pixel)) - (int)y + top + 1;
      border[1].x = (int)x + top + 1;
      border[1].y = ((int)(mission_area_height_meters / meters_per_pixel)) - (int)y;
      border[2].x = (int)x;
      border[2].y = ((int)(mission_area_height_meters / meters_per_pixel)) - (int)y - top - 1;
      border[3].x = (int)x - top - 1;
      border[3].y = ((int)(mission_area_height_meters / meters_per_pixel)) - (int)y;
      border[4].x = (int)x;
      border[4].y = ((int)(mission_area_height_meters / meters_per_pixel)) - (int)y + top + 1;
    

      XDrawLines(XtDisplay(drawing_area),
		   XtWindow(drawing_area),
		   bluegc,
		   border, 5, 
		   CoordModeOrigin);
    
      XDrawLines(XtDisplay(drawing_area),
		   drawing_area_pixmap,
		   bluegc,
		   border, 5, 
		   CoordModeOrigin);
    
      XDrawLines(XtDisplay(drawing_area),
		   XtWindow(drawing_area),
		   gGCs.whiteXOR,
		   border, 5, 
		   CoordModeOrigin);
    
      XDrawLines(XtDisplay(drawing_area),
		   drawing_area_pixmap,
		   gGCs.whiteXOR,
		   border, 5, 
		   CoordModeOrigin);
    }

          
    // draw the new position
    // fill in the whole thing with black
      
    border[0].x = (int)nx;
    border[0].y = ((int)(mission_area_height_meters / meters_per_pixel)) - (int)ny + top;
    border[1].x = (int)nx + top;
    border[1].y = ((int)(mission_area_height_meters / meters_per_pixel)) - (int)ny;
    border[2].x = (int)nx;
    border[2].y = ((int)(mission_area_height_meters / meters_per_pixel)) - (int)ny - top;
    border[3].x = (int)nx - top;
    border[3].y = ((int)(mission_area_height_meters / meters_per_pixel)) - (int)ny;
    border[4].x = (int)nx;
    border[4].y = ((int)(mission_area_height_meters / meters_per_pixel)) - (int)ny + top;
    

    XFillPolygon(XtDisplay(drawing_area),
		 XtWindow(drawing_area),
		 gc,
		 border, 5, 
		 Convex,
		 CoordModeOrigin);
    
    XFillPolygon(XtDisplay(drawing_area),
		 drawing_area_pixmap,
		 gc,
		 border, 5, 
		 Convex,
		 CoordModeOrigin);
    
    XFillPolygon(XtDisplay(drawing_area),
		 XtWindow(drawing_area),
		 gGCs.whiteXOR,
		 border, 5,
		 Convex,
		 CoordModeOrigin);
    
    XFillPolygon(XtDisplay(drawing_area),
		 drawing_area_pixmap,
		 gGCs.whiteXOR,
		 border, 5, 
		 Convex,
		 CoordModeOrigin);

    // draw outline
    border[0].x = (int)nx;
    border[0].y = ((int)(mission_area_height_meters / meters_per_pixel)) - (int)ny + top + 1;
    border[1].x = (int)nx + top + 1;
    border[1].y = ((int)(mission_area_height_meters / meters_per_pixel)) - (int)ny;
    border[2].x = (int)nx;
    border[2].y = ((int)(mission_area_height_meters / meters_per_pixel)) - (int)ny - top - 1;
    border[3].x = (int)nx - top - 1;
    border[3].y = ((int)(mission_area_height_meters / meters_per_pixel)) - (int)ny;
    border[4].x = (int)nx;
    border[4].y = ((int)(mission_area_height_meters / meters_per_pixel)) - (int)ny + top + 1;
    

    XDrawLines(XtDisplay(drawing_area),
		 XtWindow(drawing_area),
		 bluegc,
		 border, 5, 
		 CoordModeOrigin);
    
    XDrawLines(XtDisplay(drawing_area),
		 drawing_area_pixmap,
		 bluegc,
		 border, 5, 
		 CoordModeOrigin);
    
    XDrawLines(XtDisplay(drawing_area),
		 XtWindow(drawing_area),
		 gGCs.whiteXOR,
		 border, 5, 
		 CoordModeOrigin);
    
    XDrawLines(XtDisplay(drawing_area),
		 drawing_area_pixmap,
		 gGCs.whiteXOR,
		 border, 5, 
		 CoordModeOrigin);

  }

  XFlush(XtDisplay(drawing_area));
}

/**********************************************************************
 * $Log: draw_military.c,v $
 * Revision 1.1.1.1  2008/07/14 16:44:22  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.9  2007/08/30 18:33:39  endo
 * BitmapIconDisplayer class added.
 *
 * Revision 1.8  2007/08/24 22:22:34  endo
 * Draw_Neutral ghosting problem fixed.
 *
 * Revision 1.7  2007/08/17 20:36:35  endo
 * Fixing ghosting problem.
 *
 * Revision 1.6  2007/08/17 18:38:45  endo
 * Cleaning up military display.
 *
 * Revision 1.5  2007/08/16 19:54:43  endo
 * Warning message cleaned up.
 *
 * Revision 1.4  2007/08/16 19:19:12  endo
 * XOR GC bug fixed.
 *
 **********************************************************************/
