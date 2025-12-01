/*      
	Copyright 1995, Georgia Tech Research Corporation
        Atlanta, Georgia  30332-0415
        ALL RIGHTS RESERVED, See file COPYRIGHT for details.
*/

/* $Id: assert.c,v 1.1.1.1 2008/07/14 16:44:24 endo Exp $ */

#include <stdlib.h>

void _assert(int exp)
{
   if( !exp )
      abort();
}



/**********************************************************************
 * $Log: assert.c,v $
 * Revision 1.1.1.1  2008/07/14 16:44:24  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/20 17:17:52  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:38:01  endo
 * MissionLab 7.0
 *
 * Revision 1.1.1.1  2005/02/06 23:00:20  endo
 * AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
 *
 * Revision 1.3  2002/07/18 22:04:16  endo
 * #include <stdlib.h> added.
 *
 * Revision 1.2  1995/06/28 19:29:06  jmc
 * Added RCS id and log strings.
 **********************************************************************/
