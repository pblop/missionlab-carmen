/**********************************************************************
 **                                                                  **
 **                               window.c                          **
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

/* $Id: window.c,v 1.1.1.1 2008/07/14 16:44:24 endo Exp $ */

/******************************************************************************/
/* window.c - An X11 graphics interface package.              */
/******************************************************************************/

/* define to make server run synchronously */
#define DEBUG_X 1 

#define DEBUG_EXIT 0 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <X11/Xlib.h>
#include <X11/extensions/XShm.h>
#include <X11/X.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <X11/cursorfont.h>
#include <X11/Xatom.h>
#include <math.h>
#include <malloc.h>
#include "vfc_lib.h"
#include "std.h"
#include "window.h"

// ENDO - gcc-3.4
//extern void     vfc_destroy_shmximage(Display *, XImage *, XShmSegmentInfo);
//extern XImage  *vfc_create_ximage();
//extern XImage  *vfc_create_shmximage();
extern void vfc_destroy_shmximage(
    Display *display,
    XImage *ximage,
    XShmSegmentInfo *shminfo);
extern XImage  *vfc_create_shmximage(
    Display *display,
    int w,
    int h,
    int depth,
    XShmSegmentInfo *shminfo);

#define TEXT_X 0
#define TEXT_Y 0

/****************************************************************************/

static WIN_STRUCT *win_list_head = 0;

/****************************************************************************/
/* Close the window                                                         */
/****************************************************************************/

static void
do_close(WIN_STRUCT * window)
{
   /*
    * this piece of code doesn't seem to work without the server running
    * synchronously because it frees the memory before the window closes.
    */
   if(DEBUG_EXIT)  printf("calling XSynchronize\n");
   XSynchronize(window->the_display, 1);

   if(DEBUG_EXIT)printf("calling vfc_destroy_shmximage\n");
   // ENDO - gcc-3.4
   //vfc_destroy_shmximage(window->the_display,window->ximage,window->pw_shminfo);
   vfc_destroy_shmximage(
       window->the_display,
       window->ximage,
       &(window->pw_shminfo));

   if(DEBUG_EXIT) printf("calling XCloseDisplay\n");
   XCloseDisplay(window->the_display);

   if(DEBUG_EXIT)printf("calling free\n");
   /* free the window structure */
   free(window);
}

/****************************************************************************/
/* Close the specified window                                               */
/****************************************************************************/

int
window_close(WIN_STRUCT * window)
{
   WIN_STRUCT     *cur;

   /*
    * handle special case: could be problem if call with 0 and there are no
    * windows open, because win_list_head will be 0 too.
    */

   if (window == 0)
   {
      return 1;
   }

   cur = win_list_head;

   /* if head of list, handle specially since must change head pointer */
   if (cur == window)
   {
      win_list_head = win_list_head->next;
      do_close(window);
   }

   /* handle rest by searching along the list for the record */
   else
   {
      while (cur != 0 && cur->next != window)
      {
	 cur = cur->next;
      }

      if (cur == 0 || cur->next != window)
      {
	 return 1;
      }

      cur->next = cur->next->next;
      do_close(window);
   }

   return 0;
}

/****************************************************************************/
/* window_close_all                                                         */
/****************************************************************************/

void
window_close_all(void)
{
   WIN_STRUCT     *cur;

   while (win_list_head != 0)
   {
      cur = win_list_head;
      win_list_head = win_list_head->next;

      printf("calling do_close(%lX)\n",(unsigned long)cur);
      do_close(cur);
   }
}

/****************************************************************************/

void sig_catch(int sig)
{
   fprintf(stderr, "caught signal\n");

   window_close_all();

   exit(1);
}

/****************************************************************************/
/* Window manager thread                                                    */
/****************************************************************************/

void
window_thread(WIN_STRUCT * window)
{
   XEvent          event;
   int             i;
#if 0
   char            msg[80];
   int             xr, yr, x, y, kbstat;
   Window          rw, cw;
#endif

   while (1)
   {
      XNextEvent(window->the_display, &event);
      switch (event.type)
      {
	 case Expose:		/* Previously obsured window or part of
				 * window exposed */
	    /* redraw the window */
	    window_clear(window);

	    for (i = 0; i < window->num_lines; i++)
	    {
	       XSetForeground(window->the_display, window->gc, window->lines[i].color);

	       XDrawLine(window->the_display, window->window, window->gc,
			 scale_x(window->lines[i].x1),
			 scale_y(window->lines[i].y1),
			 scale_x(window->lines[i].x2),
			 scale_y(window->lines[i].y2));
	    }
	    break;

#if 0
	 case MotionNotify:
	 case EnterNotify:
	    XQueryPointer(window->the_display, event.xmotion.window,
			  &rw, &cw, &xr, &yr, &x, &y, &kbstat);

	    /* print the location of the cursor */
	    sprintf(msg, "x=%4.1f y=%4.1f", inv_scale_x(x), inv_scale_y(y));
	    XDrawImageString(window->the_display, window->window,
			     window->text_gc, window->height - 10, 0, msg,
			     strlen(msg));
	    break;

	 case LeaveNotify:
	    strcpy(msg, "                 ");
	    XDrawImageString(window->the_display, window->window,
			     window->text_gc, window->height - 10, 0, msg,
			     strlen(msg));
	    break;
#endif
      }
   }
}

/****************************************************************************/
/* pick_visual - select a visual of appropriate "class" and "depth" from    */
/* the available visuals.  returns TRUE if successful.	    */
/****************************************************************************/

static int
pick_visual(Display * the_display, int screen, int depth_wanted,
            Visual ** visual, int *depth)
{
    XVisualInfo    *vis_list;
    XVisualInfo     vis_template;
    int             i;
    int             found_one;
    int             num_visuals;
    int             min_depth;

    /* get a list of all the visuals for this screen */
    vis_template.screen = screen;

    vis_list = XGetVisualInfo(the_display, VisualScreenMask, &vis_template,
                              &num_visuals);

    if (num_visuals == 0)
    {
        fprintf(stderr, "window:pick_visual - No visuals found\n");
        exit(1);
    }

    /* find the first matching visual */
    found_one = FALSE;
    min_depth = PATH_PLAN_INFINITY;
    for (i = 0; i < num_visuals; i++)
    {
        // ENDO - gcc-3.4
        //if (vis_list->class == PseudoColor && vis_list->depth >= depth_wanted)
        if (vis_list->depth >= depth_wanted)
        {
            if (vis_list->depth < min_depth)
            {
                *visual = vis_list->visual;
                found_one = TRUE;
                min_depth = vis_list->depth;
                *depth = min_depth;
            }
        }
        vis_list++;
    }

    return found_one;
}

/****************************************************************************/
/* Create and initialize the window.                                        */
/* Returns the window ID if successful.  0 otherwise                        */
/****************************************************************************/

WIN_STRUCT     *
window_open(char *title, int width, int height, int num_colors,
	    int shared_colors, char *ctext[], unsigned long cnum[],
	    WIN_COLORS colors[])
{
   Window          parent;
   int             screen;
   Visual         *visual;
   XColor         *Xcolors;
   int             colormap_size;
   int             i;
   int             req_depth;
   int             depth;
   XSizeHints      myhint;
   XWMHints        wmhints;
   Atom            atom_WM_COLORMAP_WINDOWS;
   WIN_STRUCT     *rtn_win;
   Colormap        new_colormap;
   Colormap        default_colormap;
   XColor          color;
   XSetWindowAttributes xswa;


   if ((rtn_win = (WIN_STRUCT *) calloc(sizeof(WIN_STRUCT), 1)) == NULL)
   {
      fprintf(stderr, "window:window_open - Error allocating memory for WIN_STRUCT\n");
      return NULL;
   }

   /* create some window location hints from the users parms */
   myhint.x = 0;
   myhint.y = 0;
   myhint.width = width;
   myhint.height = height;
   myhint.flags = PPosition | PSize;

   /* open the display */
   rtn_win->the_display = XOpenDisplay((char *) NULL);

#ifdef DEBUG_X
   XSynchronize(rtn_win->the_display, 1);
#endif

   screen = DefaultScreen(rtn_win->the_display);
   parent = DefaultRootWindow(rtn_win->the_display);

   if (shared_colors)
   {

      default_colormap = DefaultColormap(rtn_win->the_display, screen);

      /* try to allocate the colors */
      for (i = 0; i < num_colors; i++)
      {
	if ((XParseColor(rtn_win->the_display, default_colormap, ctext[i], &color) == 0) ||
	    (XAllocColor(rtn_win->the_display, default_colormap, &color) == 0))
	 {
	    printf("unable to find color '%s': using Black\n", ctext[i]);
	    /* failed to get color */
	    cnum[i] = BlackPixel(rtn_win->the_display, DefaultScreen(rtn_win->the_display));
	 }
	 else
	 {
	    /* save color number */
	    cnum[i] = color.pixel;
	 }
      }

      if ((rtn_win->window = XCreateSimpleWindow(rtn_win->the_display, parent,
						 myhint.x, myhint.y, myhint.width, myhint.height, 2,
						 BlackPixel(rtn_win->the_display, screen),
						 WhitePixel(rtn_win->the_display, screen))) == 0)
      {
	 fprintf(stderr, "Unable to create window!!\n");
	 free(rtn_win);
	 return (NULL);
      }
   }
   else
   {
      /* determine number of planes required */
      if (num_colors > 256)
      {
	 fprintf(stderr, "%d colors asked for, only %d available\n",
		 num_colors, 256);
	 free(rtn_win);
      }
      if (num_colors < 1)
      {
	 fprintf(stderr, "%d colors asked for, min should be 1\n", num_colors);
	 free(rtn_win);
      }
      for (req_depth = 8; req_depth > 0; req_depth--)
      {
	 if ((num_colors & (1 << (req_depth - 1))) != 0)
	    break;
      }

      if (pick_visual(rtn_win->the_display, screen, req_depth, &visual, &depth) == FALSE)
      {
	 fprintf(stderr, "Failed to get visual\n");
	 free(rtn_win);
	 exit(1);
      }

      /* setup the color map */
      colormap_size = DisplayCells(rtn_win->the_display, screen);

      if (colormap_size < num_colors)
      {
	 fprintf(stderr, "%d colors asked for, only %d available\n",
		 num_colors, colormap_size);
	 free(rtn_win);
	 return (NULL);
      }

      /* allocate the XColor array for the colormap */
      if ((Xcolors = (XColor *) calloc(colormap_size, sizeof(XColor))) == NULL)
      {
	 fprintf(stderr, "Failed to allocate memory for colormap\n");
	 free(rtn_win);
	 return (NULL);
      }

      /* initialize parts of the colors array */
      for (i = 0; i < num_colors; i++)
      {
	 Xcolors[i].pixel  = i;
	 Xcolors[i].red    = colors[i].r;
	 Xcolors[i].green  = colors[i].g;
	 Xcolors[i].blue   = colors[i].b;
	 Xcolors[i].flags  = DoRed | DoGreen | DoBlue;
      }

      /* create the new color map */
      new_colormap = XCreateColormap(rtn_win->the_display, parent, visual, AllocAll);

      /* store the new colors into the new colormap */
      XStoreColors(rtn_win->the_display, new_colormap, Xcolors, num_colors);

      /*
       * new release the memory used by the colors, since the server has a
       * copy
       */
      free(Xcolors);

      /* now setup the window's attributes to use the new color map */
      xswa.colormap = new_colormap;
      xswa.background_pixel = WhitePixel(rtn_win->the_display, screen);
      xswa.border_pixel = BlackPixel(rtn_win->the_display, screen);

      /* open the window */
      if ((rtn_win->window = XCreateWindow(rtn_win->the_display, parent,
			 myhint.x, myhint.y, myhint.width, myhint.height, 2,
					   depth, InputOutput, visual,
		     CWColormap | CWBackPixel | CWBorderPixel, &xswa)) == 0)
      {
	 fprintf(stderr, "Unable to create window!!\n");
	 free(rtn_win);
	 return (NULL);
      }
   }

   /* Catch control-c and others to ensure that we free the shared memory. */
   signal(SIGINT,  sig_catch);
   signal(SIGBUS,  sig_catch);
   signal(SIGHUP,  sig_catch);
   signal(SIGSEGV, sig_catch);

   XSetStandardProperties(rtn_win->the_display, rtn_win->window, title, title, None, 0, 0, &myhint);

   wmhints.flags = InputHint;
   wmhints.input = True;
   XSetWMHints(rtn_win->the_display, rtn_win->window, &wmhints);

   /* inform the window manager of our new color map */
   atom_WM_COLORMAP_WINDOWS = XInternAtom(rtn_win->the_display,
					  "WM_COLORMAP_WINDOWS", 0);

   XChangeProperty(rtn_win->the_display, parent, atom_WM_COLORMAP_WINDOWS,
     XA_WINDOW, 32, PropModeReplace, (unsigned char *) &rtn_win->window, 1);

   /* select which events generate an interupt */
   /* window exposure and button press */
   XSelectInput(rtn_win->the_display, rtn_win->window,
		EnterWindowMask | LeaveWindowMask | ExposureMask | ButtonPressMask | PointerMotionHintMask);

   /* create a normal gc */
   rtn_win->gc = XCreateGC(rtn_win->the_display, rtn_win->window, 0, 0);

   /* create an XOR gc */
   rtn_win->xor_gc = XCreateGC(rtn_win->the_display, rtn_win->window, 0, 0);
   XSetFunction(rtn_win->the_display, rtn_win->xor_gc, GXxor);

   /* create text gc with black text on white background */
   rtn_win->text_gc = XCreateGC(rtn_win->the_display, rtn_win->window, 0, 0);
   XSetForeground(rtn_win->the_display, rtn_win->text_gc, 1);
   XSetBackground(rtn_win->the_display, rtn_win->text_gc, 0);

   /* map the window */
   XSetNormalHints(rtn_win->the_display, rtn_win->window, &myhint);
   XMapWindow(rtn_win->the_display, rtn_win->window);

   if (shared_colors)
   {
      rtn_win->buf = NULL;
      rtn_win->ximage = NULL;
      rtn_win->type = 1;

      /* zero number of lines */
      rtn_win->num_lines = 0;

   }
   else
   {
      rtn_win->type = 0;

      /* init ximage, used for writing images to the window */
      rtn_win->ximage = vfc_create_shmximage(rtn_win->the_display, width,
				       height, depth, &rtn_win->pw_shminfo);
      if (rtn_win->ximage == NULL)
      {
	 fprintf(stderr, "Failed to get shared memory\n");
	 free(rtn_win);
	 XDestroyWindow(rtn_win->the_display, rtn_win->window);
	 return NULL;
      }

      /* make a copy of the buffer's address for ease of access */
      rtn_win->buf = (unsigned char *) rtn_win->ximage->data;
   }

   /* add window structure to the chain */
   rtn_win->next = win_list_head;
   win_list_head = rtn_win;

   /* remember window's size */
   rtn_win->width    = width;
   rtn_win->height   = height;
   rtn_win->scale    = 1;
   rtn_win->x_offset = 0;
   rtn_win->y_offset = 0;

   return rtn_win;
}

/****************************************************************************/
/* Draw an image buffer in the window.                                      */
/****************************************************************************/

void
window_draw_image(WIN_STRUCT * window, char *image)
{
   if( (long)image != (long)window->buf )
      memcpy(window->buf,image,window->ximage->width * window->ximage->height);

   /* copy image to the pixmap */
   XPutImage(window->the_display, window->window, window->gc, window->ximage,
	     0, 0, 0, 0, window->ximage->width, window->ximage->height);
}


/****************************************************************************/
/* Draw the shared memory buffer to the window                              */
/****************************************************************************/

void
window_draw(WIN_STRUCT * window)
{
   /* copy image to the pixmap */
   XPutImage(window->the_display, window->window, window->gc, window->ximage,
	     0, 0, 0, 0, window->ximage->width, window->ximage->height);
}


/****************************************************************************/
/* write text to the window                                                 */
/****************************************************************************/

void
window_print(WIN_STRUCT * window, float x, float y, char *str)
{
   XDrawImageString(window->the_display, window->window, window->text_gc,
		    scale_x(x), scale_y(y), str, strlen(str));
}


/****************************************************************************/
/* Draw a line on the window                                                */
/****************************************************************************/

void
window_drawline(WIN_STRUCT * window, float x1, float y1,
		float x2, float y2, ushort color)
{
   int             pos;

   /* draw the line on the screen in the given color */
   XSetForeground(window->the_display, window->gc, color);

   XDrawLine(window->the_display, window->window, window->gc, scale_x(x1),
	     scale_y(y1), scale_x(x2), scale_y(y2));

   if (window->type)
   {
      pos = window->num_lines++;
      window->lines[pos].x1 = x1;
      window->lines[pos].x2 = x2;
      window->lines[pos].y1 = y1;
      window->lines[pos].y2 = y2;
      window->lines[pos].color = color;
   }
}


/****************************************************************************/
/* Draw a circle on the window                                                */
/****************************************************************************/

void
window_drawcircle(WIN_STRUCT * window, float x, float y, float r, ushort color)
{
   int             width;

   /* draw the circle on the screen in the given color */
   XSetForeground(window->the_display, window->gc, color);

   width = scale_dist(2 * r);
   XDrawArc(window->the_display, window->window, window->gc, scale_x(x - r),
	    scale_y(y - r) - width, width, width, 0, 360 * 64);
}

/****************************************************************************/
/* fill a circle on the window                                                */
/****************************************************************************/

void
window_fillcircle(WIN_STRUCT * window, float x, float y, float r, ushort color)
{
   int             width;

   /* draw the circle on the screen in the given color */
   XSetForeground(window->the_display, window->gc, color);

   width = scale_dist(2 * r);
   XFillArc(window->the_display, window->window, window->gc, scale_x(x - r),
	    scale_y(y - r) - width, width, width, 0, 360 * 64);
}

/****************************************************************************/
/* fill a pie section on the window                                         */
/****************************************************************************/

void
window_fillpie(WIN_STRUCT * window, float x, float y, float r,
	       float start_deg, float end_deg, ushort color)
{
   int             width;

   /* draw the circle on the screen in the given color */
   XSetForeground(window->the_display, window->gc, color);

   width = scale_dist(2 * r);
   XFillArc(
       window->the_display,
       window->window,
       window->gc,
       scale_x(x - r),
       scale_y(y - r) - width,
       width,
       width,
       (int)(start_deg * 64),
       (int)(end_deg * 64));
}

/****************************************************************************/
/* Flush requests to the server                                             */
/****************************************************************************/

void
window_flush(WIN_STRUCT * window)
{
   XFlush(window->the_display);
}


/****************************************************************************/
/* clear the window                                                         */
/****************************************************************************/

void
window_clear(WIN_STRUCT * window)
{
   XClearWindow(window->the_display, window->window);
   window->num_lines = 0;
}

/****************************************************************************/
/* scale the window                                                         */
/****************************************************************************/

void
window_scale(WIN_STRUCT * window, double min_x, double max_x, double min_y, double max_y)
{
   double          x_scale;
   double          y_scale;
   double          width;
   double          height;

   width  = (max_x - min_x) + 1;
   height = (max_y - min_y) + 1;

   /* determine the optimal scaling in each direction to fill 94% of window */
   x_scale = 0.94 * (double) (window->width)  / width;
   y_scale = 0.94 * (double) (window->height) / height;

   /* use the smallest scaling factor (is multiplication factor) so fits */
   window->scale = min(x_scale, y_scale);

   /* save the offsets with a 3% border (offsets are subtractive) */
   window->x_offset = min_x - (width * 0.03);
   window->y_offset = min_y - (height * 0.03);
   window->max_x = max_x;

   window_clear(window);
}

/****************************************************************************/

void
window_wait_for_button(WIN_STRUCT * window, int *cursor_x, int *cursor_y)
{
   XEvent          event;
   char            msg[80];
   int             xr, yr, x, y;
   unsigned int    kbstat; // ENDO gcc-3.4
   Window          rw, cw;
   int             cnt;

   /* read and discard any pending events */
   cnt = XPending(window->the_display);
   for (x = 0; x < cnt; x++)
      XNextEvent(window->the_display, &event);

   /* wait for a button press event */
   do
   {
      XNextEvent(window->the_display, &event);
      switch (event.type)
      {
	 case ButtonPress:
	    XQueryPointer(
            window->the_display,
            event.xmotion.window,
            &rw,
            &cw,
            &xr,
            &yr,
            &x,
            &y,
            &kbstat);

	    /* print the location of the cursor */
	    sprintf(msg, "x=%3d y=%3d", x, y);
	    XDrawImageString(window->the_display, window->window,
			     window->text_gc, TEXT_X, TEXT_Y, msg, 11);

	    *cursor_x = x;
	    *cursor_y = y;
	    break;
      }
   } while (event.type != ButtonPress);
}

/**********************************************************************
 * $Log: window.c,v $
 * Revision 1.1.1.1  2008/07/14 16:44:24  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/20 17:17:51  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:38:00  endo
 * MissionLab 7.0
 *
 * Revision 1.2  2006/05/14 05:57:38  endo
 * gcc-3.4 upgrade
 *
 **********************************************************************/
