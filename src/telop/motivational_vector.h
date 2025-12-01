/**********************************************************************
 **                                                                  **
 **                  motivational_vector.h                           **
 **                                                                  **
 **  Written by:  Alexander Stoytchev                                **
 **                                                                  **
 **  Copyright 2000, Georgia Tech Research Corporation               **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: motivational_vector.h,v 1.1.1.1 2008/07/14 16:44:24 endo Exp $ */

#ifndef _MOTIVATIONAL_VECTOR_
#define _MOTIVATIONAL_VECTOR_

#include <X11/Intrinsic.h>

#include "shared.h"

void  gt_create_motivational_vector_interface(Widget top_level, XtAppContext app);
void  gt_popup_motivational_vector_interface(void);
void  gt_end_motivational_vector(int window);

extern motivational_vector_type gMotivationalVector_vectors[];
extern int gMotivationalVector_numVectors;

#endif /* _SOUND_SIMULATION_ */





/**********************************************************************
 * $Log: motivational_vector.h,v $
 * Revision 1.1.1.1  2008/07/14 16:44:24  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/20 17:17:52  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:38:01  endo
 * MissionLab 7.0
 *
 * Revision 1.1.1.1  2005/02/06 23:00:21  endo
 * AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
 *
 * Revision 1.3  2002/07/18 17:02:58  endo
 * Fixed the problem of not displaying correctly with OpenMotif.
 *
 * Revision 1.2  2001/12/22 16:14:29  endo
 * RH 7.1 porting.
 *
 * Revision 1.1  2000/02/29 21:16:46  saho
 * Initial revision
 *
 **********************************************************************/
