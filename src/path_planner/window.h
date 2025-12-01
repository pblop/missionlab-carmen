/**********************************************************************
 **                                                                  **
 **                               window.h                          **
 **                                                                  **
 **                                                                  **
 **                                                                  **
 **                                                                  **
 **  Written by:                                                     **
 **                                                                  **
 **  Copyright 2000, Georgia Tech Research Corporation               **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **********************************************************************/

/* $Id: window.h,v 1.1.1.1 2008/07/14 16:44:24 endo Exp $ */

#ifndef WINDOW_H
#define WINDOW_H

/****************************************************/
/* 		window.h                            */
/****************************************************/

#include <X11/Xlib.h>
#include <X11/extensions/XShm.h>

/*****************************************************/
/* 			macros 			     */

/* scale points to window */
#define scale_x(c) (int)(((c) - window->x_offset) * window->scale + 0.5)
#define scale_y(c) (window->height - (int)((((c) - window->y_offset) * window->scale) + 0.5))
#define scale_dist(c) ((int)((c) * window->scale + 0.5))
#define inv_scale_x(c) (((double)(c) / window->scale) + window->x_offset)
#define inv_scale_y(c) (((double)(window->height-(c)) / window->scale) + window->y_offset)

/*****************************************************/

struct LINE_T
{
   float  x1;
   float  y1;
   float  x2;
   float  y2;
   ushort color;
};

#define MAX_WIN_LINES 5000
struct WIN_STRUCT_t
{
   unsigned char  *buf;
   struct WIN_STRUCT_t *next;
   Display        *the_display;
   XImage         *ximage;
   XShmSegmentInfo pw_shminfo;
   Window          window;
   GC              gc;
   GC              xor_gc;
   GC              text_gc;
   double          x_offset;
   double          y_offset;
   double          scale;
   double          max_x;
   int             width;
   int             height;
   struct LINE_T   lines[MAX_WIN_LINES];
   int             num_lines;
   int             type;  /* 0=image, 1=shared */
};

typedef struct WIN_STRUCT_t WIN_STRUCT;

struct WIN_COLORS_t
{
   unsigned short  r;
   unsigned short  g;
   unsigned short  b;
};

typedef struct WIN_COLORS_t WIN_COLORS;

WIN_STRUCT * window_open(char *title, int width, int height, int num_colors,
	      int shared_colors, char *ctext[], unsigned long cnum[],
	      WIN_COLORS colors[]);

   void window_draw_image(WIN_STRUCT * window, char *image);
   void window_draw(WIN_STRUCT * window);
   int  window_close(WIN_STRUCT * window);
   void window_close_all(void);
   void window_drawline(WIN_STRUCT * window, float x1, float y1,
			                float x2, float y2, ushort color);
   void window_drawcircle(WIN_STRUCT * window, float x, float y,
			                float r, ushort color);
   void window_fillcircle(WIN_STRUCT * window, float x, float y,
			                float r, ushort color);
   void window_fillpie(WIN_STRUCT * window, float x, float y, float r,
		       float start_deg, float end_deg, ushort color);

   void window_flush(WIN_STRUCT * window);
   void window_clear(WIN_STRUCT * window);
   void window_scale(WIN_STRUCT * window,double min_x,double max_x,double min_y,double max_y);
   void window_wait_for_button(WIN_STRUCT * window,int *cursor_x,int *cursor_y);
   void window_print(WIN_STRUCT * window, float x, float y, char *str);

#endif



///////////////////////////////////////////////////////////////////////
// $Log: window.h,v $
// Revision 1.1.1.1  2008/07/14 16:44:24  endo
// MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
//
// Revision 1.1.1.1  2006/07/20 17:17:51  endo
// MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
//
// Revision 1.1.1.1  2006/07/12 13:38:00  endo
// MissionLab 7.0
//
// Revision 1.1.1.1  2005/02/06 23:00:25  endo
// AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
//
// Revision 1.1  2000/03/22 04:40:23  saho
// Initial revision
//
///////////////////////////////////////////////////////////////////////

