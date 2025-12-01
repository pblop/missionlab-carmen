/**********************************************************************
 **                                                                  **
 **                              draw.c                              **
 **                                                                  **
 **         Drawing functions for the main drawing area.             **
 **         (Based on gt_world v1.63.)                               **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **               Jonathan M. Cameron                                **
 **                                                                  **
 **  Copyright 1995 - 2006 Georgia Tech Research Corporation         **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: draw.c,v 1.1.1.1 2008/07/14 16:44:22 endo Exp $ */

#include <stdio.h>
#include <math.h>
#include <malloc.h>
#include <X11/Intrinsic.h>
#include <X11/Xlib.h>

#include "gt_create_world.h"
#include "gt_world.h"
#include "gt_scale.h"
#include "console.h"
#include "gt_console_db.h"
#include "gt_sim.h"
#include "draw.h"
#include "shape.hpp"
#include "bitmap_icon_displayer.h"

// The following declarations should be in gt_world.h along with
// the type definition which is currently in gt_create_world.h.
extern obs_t* obs;
extern int num_obs;

//-----------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------
#define SP_WIDTH 20.0
#define SP_HEIGHT 40.0
#define SP_TRIANGLE_HEIGHT 10.0

const double ROBOT_WIDTH_LENGTH_RATIO  = 2.4 / 3.0;
const double ROBOT_CIRCLE_TO_DISPLAY_WIDTH_RATIO = 0.01;

//-----------------------------------------------------------------------
// Macros
//-----------------------------------------------------------------------
#define DRAW_XSTRING(str) XmStringCreateLtoR(str, XmSTRING_DEFAULT_CHARSET)
#define fix_x(c) (int)(c)
//#define fix_y(c) (drawing_area_height_pixels - (int)(c))
#define fix_y(c) ((int)(mission_area_height_meters / meters_per_pixel) - (int)(c))


//-----------------------------------------------------------------------
// Global variables
//-----------------------------------------------------------------------
int gt_show_trails = false;
int gt_circle_robots = false;

//-----------------------------------------------------------------------
void Load_Robot_Points(XPoint* points, Vector pos, double h)
{
    double length,width;

    if (scale_robots)
    {
        length = robot_length_meters / meters_per_pixel;
        width  = robot_length_meters * ROBOT_WIDTH_LENGTH_RATIO / meters_per_pixel;
    }
    else
    {
        length = robot_length_pixels;
        width  = robot_length_pixels * ROBOT_WIDTH_LENGTH_RATIO;
    }

    Vector shape[5] = {
        {      length * (1.0 / 3.0), 0                },
        {                           0, width / 2.0      },
        { -1 * length * (2.0 / 3.0), width / 2.0      },
        { -1 * length * (2.0 / 3.0), -1 * width / 2.0 },
        {                           0, -1 * width / 2.0 }
    };

    div_2d(pos, meters_per_pixel);
    for(int i = 0; i < 5; i++)
    {
        rotate_z(shape[i], h);

        plus_2d(shape[i],pos);
        points[i].x = fix_x(shape[i].x);
        points[i].y = fix_y(shape[i].y);
    }

    // Close the shape for drawpolygon
    points[5].x = fix_x(shape[0].x);
    points[5].y = fix_y(shape[0].y);
}

//-----------------------------------------------------------------------
void DrawPath(
    double px,
    double py,
    double ph,
    double x,
    double y,
    double h,
    GC gc,
    int erase)
{
    XPoint p_points[6];
    XPoint points[6];
    Vector ppos;
    Vector pos;

    pos.x = x;
    pos.y = y;

    ppos.x = px;
    ppos.y = py;

    Load_Robot_Points(p_points, ppos, ph);
    Load_Robot_Points(points, pos, h);

    x  /= meters_per_pixel;
    y  /= meters_per_pixel;
    px /= meters_per_pixel;
    py /= meters_per_pixel;

    if (erase) 
    {
        // delete the last position
        XFillPolygon(
            XtDisplay(drawing_area),
            XtWindow(drawing_area),
            gc,
            p_points, 6,
            Convex,
            CoordModeOrigin);

        XFillPolygon(
            XtDisplay(drawing_area),
            drawing_area_pixmap,
            gc,
            p_points,
            6,
            Convex,
            CoordModeOrigin);

        // Repeate it with a white XOR GC to delete it completely.
        XFillPolygon(
            XtDisplay(drawing_area),
            XtWindow(drawing_area),
            gGCs.whiteXOR,
            p_points,
            6,
            Convex,
            CoordModeOrigin);

        XFillPolygon(
            XtDisplay(drawing_area),
            drawing_area_pixmap,
            gGCs.whiteXOR,
            p_points,
            6,
            Convex,
            CoordModeOrigin);

        if (gt_show_trails && (erase < 2))
        {
            XDrawLine(
                XtDisplay(drawing_area),
                XtWindow(drawing_area),
                gGCs.black,
                fix_x(x), fix_y(y),
                fix_x(px),fix_y(py));

            XDrawLine(
                XtDisplay(drawing_area),
                drawing_area_pixmap,
                gGCs.black,
                fix_x(x), fix_y(y),
                fix_x(px), fix_y(py));
        }

        if (gt_circle_robots)
        {
            DrawCircle(
                ppos.x,
                ppos.y,
                (ROBOT_CIRCLE_TO_DISPLAY_WIDTH_RATIO*mission_area_width_meters)+
                (robot_length_meters/2.0),
                gc);

            DrawCircle(
                ppos.x,
                ppos.y,
                (ROBOT_CIRCLE_TO_DISPLAY_WIDTH_RATIO*mission_area_width_meters)+
                (robot_length_meters/2.0),
                gGCs.whiteXOR);
        }
    }

    // draw the current position
    XFillPolygon(
        XtDisplay(drawing_area),
        XtWindow(drawing_area),
        gc,
        points,
        6,
        Convex,
        CoordModeOrigin);

    XFillPolygon(
        XtDisplay(drawing_area),
        drawing_area_pixmap,
        gc,
        points,
        6,
        Convex,
        CoordModeOrigin);

    // Any Color XOR GC has to be repainted with a white XOR GC to exhibit the
    // right color.
    XFillPolygon(
        XtDisplay(drawing_area),
        XtWindow(drawing_area),
        gGCs.whiteXOR,
        points,
        6,
        Convex,
        CoordModeOrigin);

    XFillPolygon(
        XtDisplay(drawing_area),
        drawing_area_pixmap,
        gGCs.whiteXOR,
        points,
        6,
        Convex,
        CoordModeOrigin);

    if (gt_circle_robots)
    {
        DrawCircle(
            pos.x,
            pos.y,
            (ROBOT_CIRCLE_TO_DISPLAY_WIDTH_RATIO*mission_area_width_meters)+
            (robot_length_meters/2.0),
            gc); 

        DrawCircle(
            pos.x,
            pos.y,
            (ROBOT_CIRCLE_TO_DISPLAY_WIDTH_RATIO*mission_area_width_meters)+
            (robot_length_meters/2.0),
            gGCs.whiteXOR);
    }

    XFlush(XtDisplay(drawing_area));
}

/**********************************************************************/

void DrawArrowHead(Vector suOrigin, Vector suDisplacement)
{
    const double dROTATION_AMOUNT = 45; // how much the sides of the arrow head
                                        // are offset from the center line in degrees

    Vector suLeftSide, suRightSide;

    // this length will make the arrow head ~3 pixels
    double dLength = 3.0 * meters_per_pixel;

    // create two vectors pointing back
    suLeftSide.x = -dLength;
    suLeftSide.y = 0;
    suRightSide = suLeftSide;

    // rotate the sides to form an arrow head
    rotate_z(suLeftSide, -dROTATION_AMOUNT);
    rotate_z(suRightSide, dROTATION_AMOUNT);

    // move the arrow head out the same distance as the length of the vector
    double dMagnitude = len_2d(suDisplacement);
    suLeftSide.x  += dMagnitude;
    suRightSide.x += dMagnitude;

    // rotate the arrow head to the end of the vector
    double dAngle = AngleOfVector(suDisplacement);
    rotate_z(suLeftSide,  dAngle);
    rotate_z(suRightSide, dAngle);

    // move the arrow head so its origin is suOrigin
    add_2d(suLeftSide,  suOrigin, suLeftSide );
    add_2d(suRightSide, suOrigin, suRightSide);

    // find the tip of the vector for which the arrow head is being drawn
    Vector suEndPoint;
    add_2d(suOrigin, suDisplacement, suEndPoint);

    // convert meters to pixels
    suEndPoint.x  /= meters_per_pixel;
    suEndPoint.y  /= meters_per_pixel;
    suLeftSide.x  /= meters_per_pixel;
    suLeftSide.y  /= meters_per_pixel;
    suRightSide.x /= meters_per_pixel;
    suRightSide.y /= meters_per_pixel;

    // draw the arrow head
    XDrawLine(
        XtDisplay(drawing_area),
        XtWindow(drawing_area),
        gGCs.redXOR,
        fix_x(suEndPoint.x), fix_y(suEndPoint.y),
        fix_x(suLeftSide.x), fix_y(suLeftSide.y));

    XDrawLine(
        XtDisplay(drawing_area),
        drawing_area_pixmap,
        gGCs.redXOR,
        fix_x(suEndPoint.x), fix_y(suEndPoint.y),
        fix_x(suLeftSide.x), fix_y(suLeftSide.y));

    XDrawLine(
        XtDisplay(drawing_area),
        XtWindow(drawing_area),
        gGCs.whiteXOR,
        fix_x(suEndPoint.x), fix_y(suEndPoint.y),
        fix_x(suLeftSide.x), fix_y(suLeftSide.y));

    XDrawLine(
        XtDisplay(drawing_area),
        drawing_area_pixmap,
        gGCs.whiteXOR,
        fix_x(suEndPoint.x), fix_y(suEndPoint.y),
        fix_x(suLeftSide.x), fix_y(suLeftSide.y));

    XDrawLine(
        XtDisplay(drawing_area),
        XtWindow(drawing_area),
        gGCs.redXOR,
        fix_x(suEndPoint.x), fix_y(suEndPoint.y),
        fix_x(suRightSide.x), fix_y(suRightSide.y));

    XDrawLine(
        XtDisplay(drawing_area),
        drawing_area_pixmap,
        gGCs.redXOR,
        fix_x(suEndPoint.x),fix_y(suEndPoint.y),
        fix_x(suRightSide.x),fix_y(suRightSide.y));

    XDrawLine(
        XtDisplay(drawing_area),
        XtWindow(drawing_area),
        gGCs.whiteXOR,
        fix_x(suEndPoint.x), fix_y(suEndPoint.y),
        fix_x(suRightSide.x), fix_y(suRightSide.y));

    XDrawLine(
        XtDisplay(drawing_area),
        drawing_area_pixmap,
        gGCs.whiteXOR,
        fix_x(suEndPoint.x), fix_y(suEndPoint.y),
        fix_x(suRightSide.x), fix_y(suRightSide.y));
}

/**********************************************************************/
// Draw the vector indicating the direction in which the robot should move
void DrawVector(Vector c, Vector v, bool bDrawArrowHead/*=false*/)
{
    // don't draw anything if we have a zero vector
    if (len_2d(v) <= EPS_ZERO)
    {
        return;
    }

    double mult = 2.0;

    // scale it with the robot
    if (scale_robots)
    {
        mult *= robot_length_meters;
    }
    else
    {
        mult *= robot_length_pixels * meters_per_pixel;
    }

    double cx = c.x / meters_per_pixel;
    double cy = c.y / meters_per_pixel;

    double dx = mult * v.x / meters_per_pixel;
    double dy = mult * v.y / meters_per_pixel;

    XDrawLine(
        XtDisplay(drawing_area),
        XtWindow(drawing_area),
        gGCs.redXOR,
        fix_x(cx), fix_y(cy),
        fix_x(cx+dx), fix_y(cy+dy));

    XDrawLine(
        XtDisplay(drawing_area),
        drawing_area_pixmap,
        gGCs.redXOR,
        fix_x(cx), fix_y(cy),
        fix_x(cx+dx), fix_y(cy+dy));

    // Any Color XOR GC has to be repainted with a white XOR GC to exhibit the
    // right color.
    XDrawLine(
        XtDisplay(drawing_area),
        XtWindow(drawing_area),
        gGCs.whiteXOR,
        fix_x(cx), fix_y(cy),
        fix_x(cx+dx), fix_y(cy+dy));

    XDrawLine(
        XtDisplay(drawing_area),
        drawing_area_pixmap,
        gGCs.whiteXOR,
        fix_x(cx), fix_y(cy),
        fix_x(cx+dx), fix_y(cy+dy));

    // see if we should draw the arrow head
    if (bDrawArrowHead)
    {
        DrawArrowHead(c, v);
    }
}



/**********************************************************************/

void DrawArc(double x, double y, double r,
              double start_theta, double end_theta, GC thisGC)
{
    // scale
    x /= meters_per_pixel;
    y /= meters_per_pixel;
    r /= meters_per_pixel;

    XDrawArc(XtDisplay(drawing_area), XtWindow(drawing_area), thisGC,
              fix_x(x - r), fix_y(y + r), (int) (r * 2), (int) (r * 2),
              (int) start_theta, (int) end_theta);
    XDrawArc(XtDisplay(drawing_area), drawing_area_pixmap, thisGC,
              fix_x(x - r), fix_y(y + r), (int) (r * 2), (int) (r * 2),
              (int) start_theta, (int) end_theta);
}

/**********************************************************************/

void FillArc(double x, double y, double r,
              double start_theta, double end_theta, GC thisGC)
{
    // scale
    x /= meters_per_pixel;
    y /= meters_per_pixel;
    r /= meters_per_pixel;

    XFillArc(XtDisplay(drawing_area), XtWindow(drawing_area), thisGC,
              fix_x(x - r), fix_y(y + r), (int) (r * 2), (int) (r * 2),
              (int) start_theta, (int) end_theta);
    XFillArc(XtDisplay(drawing_area), drawing_area_pixmap, thisGC,
              fix_x(x - r),fix_y(y + r), (int) (r * 2), (int) (r * 2),
              (int) start_theta, (int) end_theta);
}

/**********************************************************************/

void DrawCircle(double x, double y, double r, GC thisGC)
{
    DrawArc(x, y, r, 0, 360 * 64, thisGC);
}



/**********************************************************************/

void DrawImpact(double x, double y, double r)
{
    DrawCircle(x, y, r, gGCs.redXOR);

    // Any Color XOR GC has to be repainted with a white XOR GC to exhibit the
    // right color.
    DrawCircle(x, y, r, gGCs.whiteXOR);
}



/**********************************************************************/

void DrawHalo(double x, double y, double r)
{
    DrawCircle(x, y, r, gGCs.XOR);

    // Any Color XOR GC has to be repainted with a white XOR GC to exhibit the
    // right color.
    DrawCircle(x, y, r, gGCs.whiteXOR);
}



/**********************************************************************/

void DrawFilledCircle(double x, double y, double r, GC thisGC)
{
    FillArc(x, y, r, 0, 360 * 64, thisGC);
}



/**********************************************************************/

//-----------------------------------------------------------------------
// This function draws a line with the specified GC.
//-----------------------------------------------------------------------
void DrawLine(double x1, double y1, double x2, double y2)
{
    DrawLine(x1, y1, x2, y2, gGCs.black);
}

//-----------------------------------------------------------------------
// This function draws a line with a black GC.
//-----------------------------------------------------------------------
void DrawLine(double x1, double y1, double x2, double y2, GC thisGC)
{
    // scale
    x1 /= meters_per_pixel;
    y1 /= meters_per_pixel;
    x2 /= meters_per_pixel;
    y2 /= meters_per_pixel;

    XDrawLine(
        XtDisplay(drawing_area),
        XtWindow(drawing_area),
        thisGC,
        fix_x(x1), fix_y(y1),
        fix_x(x2), fix_y(y2));

    XDrawLine(
        XtDisplay(drawing_area),
        drawing_area_pixmap,
        thisGC,
        fix_x(x1), fix_y(y1),
        fix_x(x2), fix_y(y2));
}

//-----------------------------------------------------------------------
// This functions DrawText() below with the default gc (black).
//-----------------------------------------------------------------------
void DrawText(double x, double y, char* text)
{
    DrawText(x, y, gGCs.black, text);
}

//-----------------------------------------------------------------------
// This functions prints the text on the screeen.
//-----------------------------------------------------------------------
void DrawText(double x, double y, GC gc, char* text)
{
    XmString textString;
    Dimension width, height;

    textString = DRAW_XSTRING(text);

    // scale
    x /= meters_per_pixel;
    y /= meters_per_pixel;

      // Get the dimension of the text box.
    XmStringExtent(
        default_fonts,
        textString,
        &width,
        &height);

    // Adjust the position of the text.
    x -= (width/2.0);
    y -= DRAW_TEXT_VERTICAL_OFFSET;

    // Draw
    XmStringDraw(
        XtDisplay(drawing_area),
        XtWindow(drawing_area),
        default_fonts,
        textString,
        gc,
        fix_x(x),
        fix_y(y),
        width,
        XmALIGNMENT_BEGINNING,
        XmSTRING_DIRECTION_L_TO_R,
        NULL);
	

    XmStringDraw(
        XtDisplay(drawing_area),
        drawing_area_pixmap,
        default_fonts,
        textString,
        gc,
        fix_x(x),
        fix_y(y),
        width,
        XmALIGNMENT_BEGINNING,
        XmSTRING_DIRECTION_L_TO_R,
        NULL);

    XmStringFree(textString);
}

//-----------------------------------------------------------------------
// Previous version of DrawText. It does not support \n etc.
//-----------------------------------------------------------------------
void DrawTextOld(double x, double y, GC gc, char* text)
{
    XCharStruct extent;
    XFontStruct* font_struct = NULL;
    int direction, ascent, descent, len;

    // scale
    x /= meters_per_pixel;
    y /= meters_per_pixel;

    len = strlen(text);

    XGCValues* theGCValues = (XGCValues*)malloc(sizeof(XGCValues));

    XGetGCValues(
        XtDisplay(drawing_area),
        gGCs.black,
        GCFont,
        theGCValues);

    XID this_font = theGCValues->font;
    free(theGCValues);

    font_struct = XQueryFont(XtDisplay(drawing_area), this_font);

    XTextExtents(
        font_struct,
        text,
        len,
        &direction,
        &ascent,
        &descent,
        &extent);

    x -= extent.width/2.0;
    fprintf(stderr, "before y = %f ascent = %d\n", y, ascent);
    y -= ascent*1.1;
    fprintf(stderr, "after y = %f\n", y);

    XDrawString(
        XtDisplay(drawing_area),
        XtWindow(drawing_area),
        gc,
        fix_x(x), fix_y(y),
        text, len);

    XDrawString(
        XtDisplay(drawing_area),
        drawing_area_pixmap,
        gc,
        fix_x(x), fix_y(y),
        text, len);
}



/**********************************************************************/

void draw_polygon(gt_Measure* measure)
{
    int i;

    if (measure == NULL)
    {
        return;
    }

    gt_Multipoint* edge = &(measure->data);
    if (edge->num_points > 0)
    {
        if (edge->num_points == 1)
        {
            DrawCircle(
                edge->point[0].x,
                edge->point[0].y,
                (double)((measure->width)/2.0),
                gGCs.black);
        }
        else
        {
            for (i = 1; i < (int)(edge->num_points); i++)
            {
                DrawLine(edge->point[i - 1].x, edge->point[i - 1].y,
                          edge->point[i].x, edge->point[i].y);
            }
            DrawLine(edge->point[edge->num_points - 1].x,
                      edge->point[edge->num_points - 1].y,
                      edge->point[0].x, 
                      edge->point[0].y);
        }
    }
}

/**********************************************************************/

void draw_polyline(gt_Measure* measure)
{
    int i;

    if (measure == NULL)
    {
        return;
    }

    gt_Multipoint* line = &(measure->data);
    if (line->num_points > 0)
    {
        if (line->num_points == 1)
        {
            DrawCircle(
                line->point[0].x,
                line->point[0].y,
                25.0,
                gGCs.black);
        }
        else
        {
            for (i = 1; i < (int)(line->num_points); i++)
            {
                DrawLine(line->point[i - 1].x, line->point[i - 1].y,
                          line->point[i].x, line->point[i].y);
            }
        }
    }
}


/**********************************************************************/

void draw_label(char* name, gt_Measure* measure)
{
    if (measure->draw_label)
    {
        gt_Point center;
        gt_center_of_measure(measure, &center);
        DrawText(center.x, center.y, name);
    }
}

//-----------------------------------------------------------------------
void draw_world(void)
{
    shape* p;
    void* pos = NULL;

    // Draw the overlay.
    gt_draw_db();

    // Draw the icons on the overlay.
    if (gBitmapIconDisplayer != NULL)
    {
        gBitmapIconDisplayer->displayAll();
    }

    // Draw the object.
    if (pos = object_list.first(&p))
    {
        do
        {
            p->draw();
        } while(pos = object_list.next(&p, pos));
    }
}

/**********************************************************************/

void draw_photo(Map_info* map)
{
    // first, figure out where the top left corner of the photo goes
    int dest_x = fix_x(map->photo_origin_x / meters_per_pixel);
    int dest_y = fix_y(map->photo_origin_y / meters_per_pixel) - map->photo_pixmap_height;

    // Now adjust y as necessary, since the photo pixmap may be larger
    // than the drawing_area_pixmap
    int src_y = 0;
    if (dest_y < 0)
    {
        src_y = -dest_y;
        dest_y = 0;
    }

    // Now adjust x as necessary, since the photo pixmap may be larger
    // than the drawing_area_pixmap
    int src_x = 0;
    if (dest_x < 0)
    {
        src_x = -dest_x;
        dest_x = 0;
    }
    int wid = (int)(mission_area_width_meters / meters_per_pixel);
    int hi = (int)(mission_area_width_meters / meters_per_pixel);  
    
    XCopyArea(XtDisplay(drawing_area),
	      map->photo_pixmap,
	      XtWindow(drawing_area),
	      gGCs.black,
	      0, 0,
	      wid, hi,
	      0, 0);

    XCopyArea(
        XtDisplay(drawing_area),
        map->photo_pixmap,
        drawing_area_pixmap,
        gGCs.black,
        0, 0,
        wid, hi,
        0, 0);
    
    /*
    XCopyArea(
        XtDisplay(drawing_area),
        map->photo_pixmap,
        XtWindow(drawing_area),
        gGCs.black,
        src_x, src_y,
        width, height,
        dest_x, dest_y);

    XCopyArea(
        XtDisplay(drawing_area),
        map->photo_pixmap,
        drawing_area_pixmap,
        gGCs.black,
        src_x, src_y,
        width, height,
        dest_x, dest_y);
    */
}

/**********************************************************************/

void draw_mission_name(char* name)
{
    // Write the mission name (on the bottom, centered)
    DrawText(mission_area_width_meters / 2.0, (meters_per_pixel * 20.0), name);
}

/**********************************************************************/

void draw_assembly_area(char* name, gt_Measure* measure)
{
    draw_polygon(measure);
    draw_label(name, measure);
}

/**********************************************************************/

void draw_attack_position(char* name, gt_Measure* measure)
{
    draw_polygon(measure);
    draw_label(name, measure);
}



/**********************************************************************/

void draw_axis(char* name, gt_Measure* measure)
{
    int i;

    // error
    if (measure == NULL)
    {
        return;
    }

    // get the beginning and end of the axis
    gt_Multipoint* line = &(measure->data);
    if (line->num_points < 2)
    {
        warn_userf("Error in draw_axis: axis has %d points!", line->num_points);
        return;
    }
    gt_Point* beg = &(line->point[0]);
    gt_Point* end = &(line->point[line->num_points - 1]);

    // Draw the cap at the start
    gt_Point* pt = &(line->point[1]);
    double theta_a = atan2(pt->y - beg->y, pt->x - beg->x);
    double dx = measure->width * sin(theta_a) / 2.0;
    double dy = measure->width * cos(theta_a) / 2.0;

    Vector a, al, au, b, bl, bu, cu, cl;
    al.x = beg->x + dx;
    al.y = beg->y - dy;
    au.x = beg->x - dx;
    au.y = beg->y + dy;
    a.x = beg->x;
    a.y = beg->y;
    b.x = pt->x;
    b.y = pt->y;
    DrawLine(al.x, al.y, au.x, au.y);

    // Draw the segments up to the end
    for (i = 1; i < (int)(line->num_points - 1); i++)
    {
        // find the next set of intersection points
        pt = &(line->point[i + 1]);
        double theta_b = atan2(pt->y - b.y, pt->x - b.x);
        double phi = (theta_a + theta_b + M_PI) / 2.0;
        double length = measure->width / (2.0 * sin(phi - theta_b));
        dx = length * cos(phi);
        dy = length * sin(phi);

        bl.x = b.x - dx;
        bl.y = b.y - dy;

        bu.x = b.x + dx;
        bu.y = b.y + dy;

        // Draw the edges for this segment
        DrawLine(au.x, au.y, bu.x, bu.y);
        DrawLine(al.x, al.y, bl.x, bl.y);

        // update for the next round
        theta_a = theta_b;
        au = bu;
        al = bl;
        a = b;
        b.x = pt->x;
        b.y = pt->y;
    }

    // Draw the last segment and the arrow head
    dx = end->x - a.x;
    dy = end->y - a.y;
    double head_length = vlength(dx, dy);
    head_length = ((head_length < (measure->width * 2.0)) ? 
                    (0.5 * head_length) : 
                    measure->width);
    dx = head_length * cos(theta_a);
    dy = head_length * sin(theta_a);
    b.x = end->x - dx;
    b.y = end->y - dy;
    dx = measure->width * sin(theta_a) / 2.0;
    dy = measure->width * cos(theta_a) / 2.0;
    bl.x = b.x + dx;
    bl.y = b.y - dy;
    bu.x = b.x - dx;
    bu.y = b.y + dy;

    double head_width = measure->width * 1.4;
    dx = head_width * sin(theta_a) / 2.0;
    dy = head_width * cos(theta_a) / 2.0;
    cl.x = b.x + dx;
    cl.y = b.y - dy;
    cu.x = b.x - dx;
    cu.y = b.y + dy;
    DrawLine(al.x, al.y, bl.x, bl.y);
    DrawLine(bl.x, bl.y, cl.x, cl.y);
    DrawLine(cl.x, cl.y, end->x, end->y);
    DrawLine(end->x, end->y, cu.x, cu.y);
    DrawLine(cu.x, cu.y, bu.x, bu.y);
    DrawLine(bu.x, bu.y, au.x, au.y);

    // Draw the axis label
    if (measure->draw_label)
    {
        char msg[256];
        sprintf(msg, "Axis %s", name);
        draw_label(msg, measure);
    }
}



/**********************************************************************/

void draw_battle_position(char* name, gt_Measure* measure)
{
    draw_polygon(measure);
    draw_label(name, measure);
}

/**********************************************************************/

void draw_boundary(char* name, gt_Measure* measure)
{
    draw_polyline(measure);
    draw_label(name, measure);
}

/**********************************************************************/

void draw_wall(char* name, gt_Measure* measure)
{
    draw_polyline(measure);
    draw_label(name, measure);
}

/**********************************************************************/

void draw_door(char* name, gt_Measure* measure)
{
    draw_label(name, measure);
}

/**********************************************************************/

void draw_room(char* name, gt_Measure* measure)
{
   draw_label(name, measure);
}

/**********************************************************************/

void draw_hallway(char* name, gt_Measure* measure)
{
   draw_label(name, measure);
}

/**********************************************************************/

void draw_gap(char* name, gt_Measure* measure)
{
    Vector left, right, axis, unit_axis, h_offset;
    Vector ll, ul, uml, lml, lmr, lr, ur, umr;

#define LIP_WIDTH (4.0 / 6.0)
#define LIP_LNG   (0.2)

    // error
    if (measure == NULL)
    {
        return;
    }

    // get the beginning and end of the gap
    gt_Multipoint* line = &(measure->data);
    if (line->num_points != 2)
    {
        warn_userf("Error in draw_gap: line has %d points!", line->num_points);
        return;
    }
    gt_Point* beg = &(line->point[0]);
    gt_Point* end = &(line->point[line->num_points - 1]);

    left.x  = beg->x;
    left.y  = beg->y;
    right.x = end->x;
    right.y = end->y;

    // create the outline
    axis.x = end->x - beg->x;
    axis.y = end->y - beg->y;

    unit_axis = axis;
    unit_2d(unit_axis);
    double length = len_2d(axis);

    // offset back to left
    h_offset = unit_axis;
    mult_2d(h_offset, -1 * LIP_LNG * length);

    ul = unit_axis;
    rotate_z(ul, 90);
    mult_2d(ul, LIP_WIDTH * measure->width);
    plus_2d(ul, left);
    plus_2d(ul, h_offset);

    ll = unit_axis;
    rotate_z(ll, -90);
    mult_2d(ll, LIP_WIDTH * measure->width);
    plus_2d(ll, left);
    plus_2d(ll, h_offset);

    // offset to right
    h_offset = unit_axis;
    mult_2d(h_offset, LIP_LNG * length);

    ur = unit_axis;
    rotate_z(ur, 90);
    mult_2d(ur, LIP_WIDTH * measure->width);
    plus_2d(ur, right);
    plus_2d(ur, h_offset);

    lr = unit_axis;
    rotate_z(lr, -90);
    mult_2d(lr, LIP_WIDTH * measure->width);
    plus_2d(lr, right);
    plus_2d(lr, h_offset);


    uml = unit_axis;
    rotate_z(uml, 90);
    mult_2d(uml, measure->width / 2);
    plus_2d(uml, left);

    lml = unit_axis;
    rotate_z(lml, -90);
    mult_2d(lml, measure->width / 2);
    plus_2d(lml, left);

    umr = unit_axis;
    rotate_z(umr, 90);
    mult_2d(umr, measure->width / 2);
    plus_2d(umr, right);

    lmr = unit_axis;
    rotate_z(lmr, -90);
    mult_2d(lmr, measure->width / 2);
    plus_2d(lmr, right);

    // draw the outline
    DrawLine(ul.x,  ul.y,  uml.x, uml.y);
    DrawLine(uml.x, uml.y, umr.x, umr.y);
    DrawLine(umr.x, umr.y, ur.x,  ur.y );

    DrawLine(ll.x,  ll.y,  lml.x, lml.y);
    DrawLine(lml.x, lml.y, lmr.x, lmr.y);
    DrawLine(lmr.x, lmr.y, lr.x,  lr.y );

    // draw the name under (or to the right of) the middle of the gap
    if (measure->draw_label)
    {
        gt_Point center;
        gt_center_of_measure(measure, &center);
        double theta = atan2((double) axis.y, (double) axis.x);
        double dx = 0.7 * measure->width * sin(theta);
        double dy = 0.7 * measure->width * cos(theta);
        DrawText(center.x + dx, center.y - dy, name);
    }
}

/**********************************************************************/

void draw_LDLC(char* name, gt_Measure* measure)
{
    Vector axis, step, pos;

#define MIN_CIRCLE_RAD 20

    // error
    if (measure == NULL)
    {
        return;
    }

    // get the beginning and end of the LDLC
    gt_Multipoint* line = &(measure->data);
    if (line->num_points < 2)
    {
        warn_userf("Error in draw_LDLC: line has %d points!", line->num_points);
        return;
    }
    gt_Point* beg = &(line->point[0]);
    gt_Point* end = &(line->point[line->num_points - 1]);

    // create the outline
    axis.x = end->x - beg->x;
    axis.y = end->y - beg->y;
    double length = len_2d(axis);

    // get the angles to draw at
    double start_theta = RADIANS_TO_DEGREES(atan2(axis.y, axis.x));
    deg_range_pm_360(start_theta);

    // find the radius
    int num = (int) floor(length / MIN_CIRCLE_RAD);
    double d = length / num;

    // offset to center of circles
    step = axis;
    unit_2d(step);
    mult_2d(step, d / 2);
    pos.x = beg->x;
    pos.y = beg->y;
    plus_2d(pos, step);

    step = axis;
    unit_2d(step);
    mult_2d(step, d);
    for(int i = 0; i < num; i++)
    {
        DrawArc(
            pos.x,
            pos.y,
            d/2,
            start_theta*64,
            -180*64,
            gGCs.black);

        plus_2d(pos, step);
    }

    if (measure->draw_label)
    {
        char msg[256];
        sprintf(msg, "LD/LC %s", name);
        if (beg->y < end->y)
        {
            DrawText(beg->x, beg->y - (d / 3.0), msg);
        }
        else
        {
            DrawText(end->x, end->y - (d / 3.0), msg);
        }
    }
}

/**********************************************************************/

void draw_objective(char* name, gt_Measure* measure)
{
    draw_polygon(measure);
    draw_label(name, measure);
}

/**********************************************************************/

void draw_passage_point(char* name, gt_Measure* measure)
{
    gt_Point center;

    gt_center_of_measure(measure, &center);
    DrawCircle(
        center.x,
        center.y,
        (measure->width)/2.0,
        gGCs.black);

    if (measure->draw_label)
    {
        DrawText(center.x, center.y - measure->width / 2.0, name);
    }
}

/**********************************************************************/

void draw_phase_line(char* name, gt_Measure* measure)
{
    draw_polyline(measure);

    if (measure->draw_label)
    {
        char msg[256];
        sprintf (msg, "PL %s", name);
        gt_Point* beg = &(measure->data.point[0]);
        gt_Point* end = &(measure->data.point[measure->data.num_points - 1]);
        if (beg->y < end->y)
        {
            DrawText(beg->x, beg->y, msg);
        }
        else
        {
            DrawText(end->x, end->y, msg);
        }
    }
}

//-----------------------------------------------------------------------
// Draw the starting point. Draw the icon the same size regardless of scale.
// Center the label in the box part of the icon.
//
// NOTE: SP is obsolete. Use "PP StartPlace" instead.
//-----------------------------------------------------------------------
void draw_starting_point(char* name, gt_Measure* measure)
{
    int x = fix_x(nint(measure->data.point[0].x / meters_per_pixel));
    int y = fix_y(nint(measure->data.point[0].y / meters_per_pixel));
    int box_left   = nint(x - SP_WIDTH/2.0);
    int box_right  = nint(x + SP_WIDTH/2.0);
    int box_bottom = nint(y - SP_TRIANGLE_HEIGHT);
    int box_top    = nint(y - SP_HEIGHT);

    // Draw the starting point icon
    XDrawLine(
        XtDisplay(drawing_area),
        XtWindow(drawing_area),
        gGCs.black,
        x, y,
        box_left, box_bottom);

    XDrawLine(
        XtDisplay(drawing_area),
        drawing_area_pixmap,
        gGCs.black,
        x,y,
        box_left, box_bottom);

    XDrawLine(
        XtDisplay(drawing_area),
        XtWindow(drawing_area),
        gGCs.black,
        x, y,
        box_right, box_bottom);

    XDrawLine(
        XtDisplay(drawing_area),
        drawing_area_pixmap,
        gGCs.black,
        x,y,
        box_right, box_bottom);

    XDrawLine(
        XtDisplay(drawing_area),
        XtWindow(drawing_area),
        gGCs.black,
        box_left, box_bottom,
        box_right, box_bottom);

    XDrawLine(
        XtDisplay(drawing_area),
        drawing_area_pixmap,
        gGCs.black,
        box_left, box_bottom,
        box_right, box_bottom);

    XDrawLine(
        XtDisplay(drawing_area),
        XtWindow(drawing_area),
        gGCs.black, 
       box_right, box_bottom,
        box_right, box_top);

    XDrawLine(
        XtDisplay(drawing_area),
        drawing_area_pixmap,
        gGCs.black,
        box_right, box_bottom,
        box_right, box_top);

    XDrawLine(
        XtDisplay(drawing_area),
        XtWindow(drawing_area),
        gGCs.black,
        box_right, box_top,
        box_left, box_top);

    XDrawLine(
        XtDisplay(drawing_area),
        drawing_area_pixmap,
        gGCs.black,
        box_right, box_top,
        box_left, box_top);

    XDrawLine(
        XtDisplay(drawing_area),
        XtWindow(drawing_area),
        gGCs.black,
        box_left, box_top,
        box_left, box_bottom);

    XDrawLine(
        XtDisplay(drawing_area),
        drawing_area_pixmap,
        gGCs.black,
        box_left, box_top,
        box_left, box_bottom);

    // Draw the label
    double cx = measure->data.point[0].x;
    double cy = measure->data.point[0].y + 
        (SP_TRIANGLE_HEIGHT + (SP_HEIGHT - SP_TRIANGLE_HEIGHT) / 2.0 + 8.0) * meters_per_pixel;

    if (measure->draw_label)
    {
        char msg[256];
        sprintf(msg, "SP %s", name);
        DrawText(cx, cy, msg);
    }
    else
    {
        DrawText(cx, cy, "SP");
    }
}

/**********************************************************************
 * $Log: draw.c,v $
 * Revision 1.1.1.1  2008/07/14 16:44:22  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.4  2007/08/30 18:33:39  endo
 * BitmapIconDisplayer class added.
 *
 * Revision 1.3  2007/08/10 04:41:05  pulam
 * Added scale fix and military unit drawing fix.
 *
 * Revision 1.2  2007/01/28 21:00:54  endo
 * MEXP_FEATURE_OPTION_POLYGONS added.
 *
 * Revision 1.1.1.1  2006/07/20 17:17:50  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:37:59  endo
 * MissionLab 7.0
 *
 * Revision 1.5  2006/05/15 01:50:11  endo
 * gcc-3.4 upgrade
 *
 * Revision 1.4  2006/01/30 02:43:17  endo
 * AO-FNC CBR-CNP Type-I check-in.
 *
 * Revision 1.3  2005/07/27 20:38:09  endo
 * 3D visualization improved.
 *
 * Revision 1.2  2005/02/07 23:12:35  endo
 * Mods from usability-2004
 *
 * Revision 1.1.1.1  2005/02/06 23:00:09  endo
 * AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
 *
 * Revision 1.21  2002/07/02 20:48:20  blee
 * changed DrawVector() and added DrawArrowHead()
 *
 * Revision 1.20  2001/11/13 15:01:08  endo
 * The problem of mlab not being able to display a robot with a right color
 * was fixed by painting color XOR GC regions with a white XOR GC.
 *
 * Revision 1.19  2000/11/20 19:33:54  sgorbiss
 * Add visualization of line-of-sight communication
 *
 * Revision 1.18  2000/02/07 05:16:21  jdiaz
 * doorways walls hallways rooms
 *
 * Revision 1.17  1997/02/12 05:42:50  zchen
 * *** empty log message ***
 *
 * Revision 1.16  1996/03/04  22:51:58  doug
 * *** empty log message ***
 *
 * Revision 1.15  1995/10/23  18:21:57  doug
 * Extend object stuff to support obstacles and also add 3d code
 *
 * Revision 1.14  1995/10/09  19:58:57  doug
 * fixed usage of min function
 *
 * Revision 1.13  1995/10/03  21:25:06  doug
 * add support for get_objects call
 *
 * Revision 1.12  1995/07/05  17:56:23  jmc
 * Minor tweak to comment.
 *
 * Revision 1.11  1995/06/09  21:41:52  jmc
 * Added draw_photo() function.  Cleaned up a few odds and ends.
 *
 * Revision 1.10  1995/05/05  15:12:59  jmc
 * Fixed draw_mission_name to put it on the bottom.
 *
 * Revision 1.9  1995/05/04  22:06:15  jmc
 * Added a definition for gt_show_trails (moved here from
 * gt_world.c).
 *
 * Revision 1.8  1995/05/04  19:52:42  jmc
 * Various fixes to accomodate the different style of map scaling.  Renamed
 * magnify_robots to scale_robots.  Renamed user_robot_magnification to
 * robot_length_meters.  Renamed fixed_robot_magnification to
 * robot_length_pixels.  If scale_robots==TRUE, use robot_length_meters,
 * else use robot_length_pixels to compute the robot size.  Converted use
 * of ROBOT_WIDTH and ROBOT_LENGTH to ROBOT_WIDTH_LENGTH_RATIO.  A few
 * other related modifications.
 *
 * Revision 1.7  1995/05/02  19:50:41  jmc
 * Changed work_work_*_meters to mission_area_*_meters.
 *
 * Revision 1.6  1995/04/28  17:29:31  jmc
 * Renamed pixmap variable to drawing_area_pixmap.
 *
 * Revision 1.5  1995/04/27  19:40:33  jmc
 * Converted GRID_HEIGHT/WIDTH_METERS variables to lower case.
 *
 * Revision 1.4  1995/04/21  20:09:15  doug
 * moved mission name to top center of world
 *
 * Revision 1.3  1995/04/19  21:50:38  jmc
 * Modified draw_mission_name to draw it on the bottom of the drawing
 * window since that the default position is scrolled all the way
 * down.
 *
 * Revision 1.2  1995/04/10  17:17:41  jmc
 * Removed the declarations for GCs, grid_*, meters_per_pixel, etc,
 * since they are now in console.h.
 *
 * Revision 1.1  1995/04/07  20:06:08  jmc
 * Initial revision
 **********************************************************************/
