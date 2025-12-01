/**********************************************************************
 **                                                                  **
 **                               vfc_lib.h                          **
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

/* $Id: vfc_lib.h,v 1.1.1.1 2008/07/14 16:44:24 endo Exp $ */

/* Copyright (c) 1990 by Sun Microsystems, Inc. */

#ifndef _VIDEOPICS_VFC_LIB_H
#define	_VIDEOPICS_VFC_LIB_H

// ENDO - gcc-3.4
//#ident	"@(#)vfc_lib.h	1.6	90/12/12 SMI"

#include <sys/types.h>
#include "vfc_ioctls.h"

/* 
 * PRIVATE
 */
#define	VFC_NTSC_SQ_FACTOR	9	/* Sqaure pixel factor for NTSC */
#define	VFC_PAL_SQ_FACTOR	15	/* Sqaure pixel factor for PAL */

/* Y from the fram data */
#define VFC_YUV_TO_Y(f,o)	((((f)&0xFF00) >> 9)+(o)) 
#define VFC_YUV_TO_Y8(f)	(((f)&0xFF00) >> 9) 
#define VFC_PREVIEW_Y(f,o)	((f)>>25)+(o)  /* Preview mode Y */
#define VFC_PREVIEW_Y8(f)	(f)>>24  /* Preview mode Y, no offset */
#define VFC_PREVIEW_Y0(f)	(f)>>25  /* Preview mode Y, zero add offset */

#define VFC_Y_MIN		0	/* Min value of Y from 1.0 chip set */
#define VFC_Y_MAX		127	/* Max value of Y from 1.0 chip set */
#define VFC_U_MIN		(-64)	/* Min value of U from 1.0 chip set */
#define VFC_U_MAX		63	/* Max value of U from 1.0 chip set */
#define VFC_V_MIN		(-64)	/* Min value of V from 1.0 chip set */
#define VFC_V_MAX		63	/* Max value of V from 1.0 chip set */

/* Convert 32 bit hw data to 16 bit data */
#define	VFC_INT_TO_YUV(f)	(((f)&0xFFFF0000)>>16)

/* Clipping when converting to RGB from PYUV */
#define VFC_RGB_MAX		255	/* Maximum value for 8 bit RGB */
#define VFC_RGB_MIN		0	/* Minimum value for 8 bits RGB */
#define	VFC_CLIP_RGB_OLD(v)	( (v)<0 ? 0 : ( (v)>VFC_Y_MAX ? 255 : (v)<<1) )
#define	VFC_CLIP_RGB(v)		( (v)<0 ? 0 : \
					( (v)>VFC_RGB_MAX ? VFC_RGB_MAX : (v)) )

/* 
 * PUBLIC 
 */

/*
 * Tell the open call to lock the device if required
 */
#define	VFC_LOCKDEV		0	/* Lock it */
#define	VFC_NOLOCKDEV		1	/* Don't lock it */

/*
 * Some defines specifically for hw control
 */
#define	VFC_AUTO		2	/* Auto detect mode */

#define	VFC_COL			0	/* hw colour detect on */
#define	VFC_MONO		1	/* hw colour detect off */

#define	VFC_PORT1		1	/* port 1 */
#define	VFC_PORT2		2	/* port 2 */
#define	VFC_SVIDEO		3	/* s-video port */

#define	VFC_NTSC		0	/* Input signal is NTSC format */
#define	VFC_PAL			1	/* Input signal is PAL format */

/*
 * lines in to the picture start the NTSC odd/even field 
 * (17*VFC_YUV_WIDTH) || (17.5*VFC_YUV_WIDTH). 
 * Hardware misses 6H
 */
#ifdef ALPHA_HWARE
#define VFC_OSKIP_NTSC		(17*VFC_YUV_WIDTH)
#define VFC_ESKIP_NTSC		(VFC_OSKIP_NTSC+(VFC_YUV_WIDTH>>1))
#else
#define VFC_OSKIP_NTSC		(12*VFC_YUV_WIDTH)
#define VFC_ESKIP_NTSC		(VFC_OSKIP_NTSC+(VFC_YUV_WIDTH>>1))
#endif

/*
 * lines in to the picture start the PAL odd/even field 
 * Hardware misses 6H.
 */
#ifdef ALPHA_HWARE
#define VFC_ESKIP_PAL		(21*VFC_YUV_WIDTH)
#define VFC_OSKIP_PAL		(VFC_ESKIP_PAL+(VFC_YUV_WIDTH>>1))
#else
#define VFC_ESKIP_PAL		(16*VFC_YUV_WIDTH)
#define VFC_OSKIP_PAL		(VFC_ESKIP_PAL+(VFC_YUV_WIDTH>>1))
#endif


/* PAL & NTSC defines */
#define VFC_YUV_WIDTH		720	/* YUV width for the 1.0 chip set */
#define	VFC_NTSC_WIDTH		640	/* Square pixel NTSC width */
#define	VFC_PAL_WIDTH		768	/* Square pixel PAL width */
#define	VFC_NTSC_HEIGHT		480	/* # of lines in an NTSC image */
#define	VFC_PAL_HEIGHT		575	/* # of lines in a PAL image */

#define VFC_FBWIDTH		1152	/* Width of a frame buffer */

#define	VFC_GRAY_MAPLENGTH	128	/* 7 to 8-bit greyscale map */
#define	VFC_CCUBE_MAPLENGTH	217	/* Colorcube map length */


/* 
 * Macros that will return the correct width and height 
 * given the video format
 */
#define vfc_format_width(f)	((f)==VFC_PAL ? VFC_PAL_WIDTH : \
						VFC_NTSC_WIDTH)
#define vfc_format_height(f)	((f)==VFC_PAL ? VFC_PAL_HEIGHT : \
						VFC_NTSC_HEIGHT)
/*
 * These macros can be used in place of direct calls to
 * the routines for converting YUV411 data to a line
 * of YUV, 8-bit colour image, 24-bit colour image, 7-bit
 * grey scale or 8-bit grey scale. They also do square
 * pixel resampling. 
 */

/* A line of YUV data in three arrays */
#define	vfc_yuv411_to_yuv_sq(buf, y, u, v, f) \
	(f == VFC_PAL ? vfc_yuv411_to_yuv_pal((buf), (y), (u), (v) ) : \
		vfc_yuv411_to_yuv_ntsc((buf), (y), (u), (v)) )

/* Convert YUV data to 8-bit color */
#define	vfc_yuv2rgb8_sq(s, d, f)	((f) == VFC_PAL  \
			? vfc_yuv2rgb8_pal((s), (d)) \
			: vfc_yuv2rgb8_ntsc((s), (d)))

#define	vfc_yuv2rgb_sq(s, d, f)		((f) == VFC_PAL  \
			? vfc_yuv2rgb_pal((s), (d)) \
			: vfc_yuv2rgb_ntsc((s), (d)))

#define	vfc_yuv2rgb_sq_line(s, d, f)	((f) == VFC_PAL  \
			? vfc_yuv2rgb_pal_line((s), (d)) \
			: vfc_yuv2rgb_ntsc_line((s), (d)))

#define	vfc_yuv2y_sq(s, d, o, f)	((f) == VFC_PAL  \
			? vfc_yuv2y_pal((s), (d), (o)) \
			: vfc_yuv2y_ntsc((s), (d), (o)))

#define	vfc_yuv2y8_sq(s, d, f)		((f) == VFC_PAL  \
			? vfc_yuv2y8_pal((s), (d)) \
			: vfc_yuv2y8_ntsc((s), (d)))

/* Read the data from the h/ware */
#define	vfc_yuvread_sq(fd, d, f)	((f) == VFC_PAL  \
			? vfc_yuvread_pal((fd), (d)) \
			: vfc_yuvread_ntsc((fd), (d)))

/* Full 1/2 and 1/4 screen preview modes */
#define	vfc_preview_sq(fd, d, o, f)	((f) == VFC_PAL  \
			? vfc_preview_pal((fd), (d), (o)) \
			: vfc_preview_ntsc((fd), (d), (o)))

#define	vfc_preview_half_sq(fd, d, o, f)	((f) == VFC_PAL  \
			? vfc_preview_half_pal((fd), (d), (o)) \
			: vfc_preview_half_ntsc((fd), (d), (o)))

#define	vfc_preview_quarter_sq(fd, d, o, f)	((f) == VFC_PAL  \
			? vfc_preview_quarter_pal((fd), (d), (o)) \
			: vfc_preview_quarter_ntsc((fd), (d), (o)) )

#define	vfc_cpreview_sq(fd, d, f)	((f) == VFC_PAL  \
			? vfc_cpreview_pal((fd), (d)) \
			: vfc_cpreview_ntsc((fd), (d)) )

/* Macro to return the correct format from the driver values */
#define	vfc_video_format(f)	( ((f) == PAL_COLOR || (f) == PAL_NOCOLOR) \
			? VFC_PAL : VFC_NTSC )

/* Macro to return the correct colour format from the driver values */
#define	vfc_colmode(f)	( ((f) == PAL_NOCOLOR || (f) == NTSC_NOCOLOR) \
			? VFC_MONO : VFC_COL )
			

/* Absolute value macro */
#define VFC_ABS(n)      ((n) < 0 ? (-1)*(n) : (n))

/* 32 bit XBGR pixel to red, green, blue values */
#define VFC_RED(v)	((v) & 0xFF)
#define VFC_GREEN(v)	(((v)>>8) & 0xFF)
#define VFC_BLUE(v)	(((v)>>16) & 0xFF)

/* The VFC device structure */
typedef struct _VfcDev {
	int	vfc_fd;		/* The file descriptor */
	u_int	*vfc_port1;	/* Port1 for this device */
	u_int	*vfc_port2;	/* Port2 for this device */
} VfcDev;

/*
 * Private Calls 
 */
extern void	vfc_pyuv_to_rgb(), vfc_clip_pyuv(), vfc_rgb_to_py(),
		vfc_init_rgb_tables(), vfc_rgb_tables(), 
		vfc_rgb_od_init(), vfc_build_yuv2rgb_tables();

extern int	vfc_gen_py_ramp(), vfc_gen_rgb_cmap();

/* 
 * Public Calls 
 */

/* Hardware related library calls */
extern	VfcDev	*vfc_open();
extern int	vfc_destroy(), vfc_grab(), vfc_reset_memptr(), 
		vfc_adjust_hue(), vfc_set_colmode(), vfc_set_defaults(), 
		vfc_set_port(), vfc_set_format(),
		vfc_yuvread_ntsc(), vfc_yuvread_pal();

/* Initialisation of tables */
extern	int	vfc_init_lut(), vfc_get_colormaps();

/* Preview mode calls */
extern	int	vfc_preview_ntsc(), vfc_preview_pal(),
		vfc_preview_half_ntsc(), vfc_preview_half_pal(),
		vfc_preview_quarter_ntsc(), vfc_preview_quarter_pal(),
		vfc_preview_ntsc8(), vfc_preview_pal8(),
		vfc_preview_field_ntsc(), vfc_preview_field_pal(),
		vfc_preview_field8_ntsc(), vfc_preview_field8_pal(),
		vfc_preview_col();

/* Color Preview */
extern int	vfc_cpreview_pal(), vfc_cpreview_ntsc(),
		vfc_cpreview_half(), vfc_cpreview_qrtr();

/* YUV to various Gray scale sizes */
extern	int	vfc_yuv2y_ntsc(), vfc_yuv2y_pal(),
		vfc_yuv2y8_ntsc(), vfc_yuv2y8_pal(),
		vfc_yuv2y_half(), vfc_yuv2y_quarter(),
		vfc_yuv2y_field_ntsc(), vfc_yuv2y_field_pal(),
		vfc_yuv2y8_field_ntsc(), vfc_yuv2y8_field_pal();

/* YUV to color */
extern	int	vfc_yuv2rgb_ntsc(), vfc_yuv2rgb_pal(),
		vfc_yuv2rgb_ntsc_line(), vfc_yuv2rgb_pal_line(),
		vfc_yuv2rgb8_ntsc(), vfc_yuv2rgb8_pal(),
		vfc_yuv2rgb8_half(), vfc_yuv2rgb8_quarter(),
		vfc_best_col_dither();

/* X11 Image processing routines */
extern	int	vfc_copy_ximage(), vfc_odither8to1(), vfc_fsdither8to1(),
		vfc_add_offset(), vfc_fs_rgb_dither(), vfc_od_rgb_dither(),
		vfc_rgb_to_y(), vfc_rgb8_to_y(), vfc_rgb8_to_rgb(),
		vfc_remove_motion();

/* YUV 411 to YUV arrays */
extern	void	vfc_yuv411_to_yuv_ntsc(), vfc_yuv411_to_yuv_pal();
#endif /* !_VIDEOPICS_VFC_LIB_H */



///////////////////////////////////////////////////////////////////////
// $Log: vfc_lib.h,v $
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
// Revision 1.1  2000/03/22 04:40:20  saho
// Initial revision
//
///////////////////////////////////////////////////////////////////////

