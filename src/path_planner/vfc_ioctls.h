/**********************************************************************
 **                                                                  **
 **                               vfc_ioctls.h                          **
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

/* $Id: vfc_ioctls.h,v 1.1.1.1 2008/07/14 16:44:24 endo Exp $ */

/* Copyright (c) 1990 by Sun Microsystems, Inc. */

#ifndef _VIDEOPICS_VFC_IOCTLS_H
#define	_VIDEOPICS_VFC_IOCTLS_H

// ENDO - gcc-3.4
//#ident	"@(#)vfc_ioctls.h	1.4	90/12/12 SMI"

#include <sys/ioctl.h>

	/* IOCTLs */
#define VFCGCTRL	_IOR(j, 0, int)	/* get ctrl. attributes */
#define VFCSCTRL	_IOW(j, 1, int)	/* set ctrl. attr. */
#define VFCGVID		_IOR(j, 2, int)	/* get video decoder attr. */
#define VFCSVID		_IOW(j, 3, int)	/* set video decoder attr. */
#define VFCHUE		_IOW(j, 4, int)	/* change hue to that indicated */
#define VFCPORTCHG	_IOW(j, 5, int)	/* change i/p port to that indicated */
#define VFCRDINFO	_IOW(j, 6, int) /* info necessary for reading */


	/* Options for the VFCSCTRL */
#define MEMPRST		0x1	/* reset FIFO ptr. */
#define CAPTRCMD	0x2	/* issue capture cmd. */
#define DIAGMODE	0x3	/* set board into diag mode */
#define NORMMODE	0x4	/* set board into normal mode */

	/* Options for the VFCSVID */
#define STD_NTSC	0x1	/* switch to NTSC std. */
#define STD_PAL		0x2	/* switch to PAL std. */
#define COLOR_ON	0x3	/* force color ON */
#define MONO		0x4	/* force color OFF */
#define ON			0x1
#define OFF			0x0

	/* Values returned by VFCGVID */

#define NO_LOCK	        1
#define NTSC_COLOR	2
#define NTSC_NOCOLOR    3
#define PAL_COLOR	4
#define PAL_NOCOLOR	5

	/* Options for setting Field number */
#define ODD_FIELD	0x1
#define EVEN_FIELD	0x0
#define ACTIVE_ONLY     0x2
#define NON_ACTIVE	0x0

#endif /* !_VIDEOPICS_VFC_IOCTLS_H */



///////////////////////////////////////////////////////////////////////
// $Log: vfc_ioctls.h,v $
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
// Revision 1.1  2000/03/22 04:40:19  saho
// Initial revision
//
///////////////////////////////////////////////////////////////////////

