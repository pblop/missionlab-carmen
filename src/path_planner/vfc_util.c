/**********************************************************************
 **                                                                  **
 **                               vfc_util.c                          **
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

/* $Id: vfc_util.c,v 1.1.1.1 2008/07/14 16:44:24 endo Exp $ */

/* Copyright (c) 1990 by Sun Microsystems, Inc. */

// ENDO - gcc-3.4
//#ident	"@(#)vfc_util.c	1.6	90/12/12 SMI"

/*
 * This file contains a set of utilities that will be useful for users
 * programming for VideoPix in the OpenWindows environment.
 */
#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/extensions/XShm.h>
#include <malloc.h>
#include "std.h"

/*  Do not use these with LINUX 
int             shmget(key_t key, int size, int shmflg);
char           *shmat(int shmid, char *shmaddr, int shmflg);
int             shmdt(char *shmaddr);
int             shmctl(int shmid, int cmd, struct shmid_ds * buf);
*/


/*
 * Create an X11 image given a desired width, height and depth.
 */
XImage         *
vfc_create_ximage(
    Display *display,
    int w,
    int h,
    int depth,
    int create_mem)
{
   register int    i;
   register long   howmuch;
   register u_char *imdata;
   Visual         *visual;
   XImage         *r_ximage;

   visual = DefaultVisual(display, DefaultScreen(display));
   if (depth == 1)
      r_ximage = XCreateImage(display, visual, 1, XYBitmap,
			      0, 0, w, h, 16, 0);
   else
      r_ximage = XCreateImage(display, visual, depth, ZPixmap,
			      0, 0, w, h, 16, 0);
   if (r_ximage == NULL)
   {
      fprintf(stderr, "XCreateImage failed\n");
      return (NULL);
   }

   howmuch = r_ximage->height * r_ximage->bytes_per_line;
   r_ximage->data = (char *) malloc(howmuch);
   if (r_ximage->data == NULL)
   {
      perror("malloc");
      return (NULL);
   }

   /* Clear the memory out */
   imdata = (u_char *) r_ximage->data;
   for (i = 0; i < howmuch; i++)
      *imdata++ = 0;
   return (r_ximage);
}

/*
 * Create a shared memory X11 image
 */
XImage         *
vfc_create_shmximage(
    Display *display,
    int w,
    int h,
    int depth,
    XShmSegmentInfo *shminfo)
{
   register int    i;
   register u_char *imdata;
   XImage         *ximage;
   Visual         *visual;
   int             majorv, minorv, howmuch;
   Bool            sharedPixmaps;

   if (!XShmQueryVersion(display, &majorv, &minorv, &sharedPixmaps))
   {
      fprintf(stderr, "X11 Shared memory images not available");
      return (NULL);
   }

   visual = DefaultVisual(display, DefaultScreen(display));
   if (depth == 1)
      ximage = XShmCreateImage(display, visual, 1,
			       XYBitmap, 0, shminfo, w, h);
   else
      ximage = XShmCreateImage(display, visual, depth,
			       ZPixmap, 0, shminfo, w, h);

   howmuch = h * ximage->bytes_per_line;

   shminfo->shmid = shmget(IPC_PRIVATE, howmuch, IPC_CREAT | 0777);

   if (shminfo->shmid < 0)
   {
      perror("shmget");
      return (NULL);
   }
   shminfo->shmaddr = (char *) shmat(shminfo->shmid, 0, 0);

   if (shminfo->shmaddr == ((char *) -1))
   {
      perror("shmat");
      return (NULL);
   }
   ximage->data = shminfo->shmaddr;

   /* Clear the memory out */
   imdata = (u_char *) ximage->data;
   for (i = 0; i < howmuch; i++)
      *imdata++ = 0;

   shminfo->readOnly = False;
   XShmAttach(display, shminfo);
   return (ximage);
}

/*
 * Destroy the X11 shared memory image specified
 */
void vfc_destroy_shmximage(
    Display *display,
    XImage *ximage,
    XShmSegmentInfo *shminfo)
{
   int             rc;

   if (ximage)
   {
      XShmDetach(display, shminfo);
      rc = shmdt(shminfo->shmaddr);
      if (rc < 0)
	 perror("shmdt");
      rc = shmctl(shminfo->shmid, IPC_RMID, 0);
      if (rc < 0)
	 perror("shmctl");
   }
   return;
}

/*
 * convert an xcolor array to 3 u_char r,g,b arrays
 */
int
xcol2uchar(
    XColor *xcol,
    int length,
    u_char *r,
    u_char *g,
    u_char *b)
{
   register int    i;

   if (!xcol || !r || !g || !b)
      return (-1);

   for (i = 0; i < length; i++)
   {
      r[i] = xcol[i].red >> 8;
      g[i] = xcol[i].green >> 8;
      b[i] = xcol[i].blue >> 8;
   }
   return (0);
}

/*
 * Use the private color map and install it in the canvas
 */
void
vfc_load_private_cmap(
    Display *display,
    Colormap cmap,
    u_char *r,
    u_char *g,
    u_char *b,
    int map_length,
    int offset)
{
   register int    i;
   u_long          pixels[256], plane_masks[256];
   XColor          colors[256];

   /* Allocate 256 cells again */
   XAllocColorCells(display,
		    cmap, True, plane_masks, 0, pixels, 256);

   /* Now set the values of the colour array */
   for (i = 0; i < map_length; i++)
   {
      colors[i].pixel = i + offset;
      colors[i].red = r[i] << 8;
      colors[i].green = g[i] << 8;
      colors[i].blue = b[i] << 8;
      colors[i].flags = DoRed | DoGreen | DoBlue;
   }

   /* Free the ones we don't need */
   if (offset)
   {
      /* Free window manager colors */
      for (i = 0; i < offset; i++)
	 pixels[i] = i;
      XFreeColors(display, cmap, pixels, offset, 0);

      /* Free colors at the top of the map */
      for (i = (offset + map_length); i < 256; i++)
	 pixels[i - (offset + map_length)] = i;
      XFreeColors(display, cmap, pixels,
		  256 - (offset + map_length), 0);
   }

   /* Store it in the colour map */
   XStoreColors(display, cmap, colors, map_length);

   /* Now install it in the h/ware */
   XInstallColormap(display, cmap);
   return;
}

/*
 * Try to allocate read/write cells from the default color map. This is the
 * preferred choice as the window manager will install the map for us. Need
 * to return the value of the offset so that we can compensate for it
 * elsewhere.
 */
int
vfc_load_default_cmap(
    Display *display,
    Colormap cmap,
    u_char *r,
    u_char *g,
    u_char *b,
    int map_length,
    int *offset)
{
   Status          rc;
   register int    i;
   u_long          pixels[256], plane_masks[256];
   XColor          colors[256];

   rc = XAllocColorCells(display,
			 cmap, True, plane_masks, 0, pixels, map_length);

   if (!rc)
   {
      /* XAllocColorCells failed, return a failure */
      return (-1);
   }

   if ((pixels[0] + map_length - 1) != pixels[map_length - 1])
   {
      /*
       * The map returned wasn't contigous. Free the colors returned in
       * pixels. Return a failure.
       */
      XFreeColors(display, cmap, pixels, map_length, 0);
      return (-1);
   }

   /* Now set the values of the colour array */
   for (i = 0; i < map_length; i++)
   {
      colors[i].pixel = i + pixels[0];
      colors[i].red = r[i] << 8;
      colors[i].green = g[i] << 8;
      colors[i].blue = b[i] << 8;
      colors[i].flags = DoRed | DoGreen | DoBlue;
   }

   /* Return the value of offset to the user */
   *offset = pixels[0];

   /* Store it in the colour map */
   XStoreColors(display, cmap, colors, map_length);
   return (0);
}



///////////////////////////////////////////////////////////////////////
// $Log: vfc_util.c,v $
// Revision 1.1.1.1  2008/07/14 16:44:24  endo
// MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
//
// Revision 1.1.1.1  2006/07/20 17:17:51  endo
// MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
//
// Revision 1.1.1.1  2006/07/12 13:38:00  endo
// MissionLab 7.0
//
// Revision 1.2  2006/05/14 05:57:38  endo
// gcc-3.4 upgrade
//
// Revision 1.1.1.1  2005/02/06 23:00:24  endo
// AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
//
// Revision 1.1  2000/03/22 04:40:21  saho
// Initial revision
//
///////////////////////////////////////////////////////////////////////

