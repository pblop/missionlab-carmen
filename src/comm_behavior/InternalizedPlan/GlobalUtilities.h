/**********************************************************************
 **                                                                  **
 **                     GlobalUtilities.h                            **
 **                                                                  **
 **                                                                  **
 **  Written by:  Alan Wagner                                        **
 **                                                                  **
 **  Copyright 2002 - 2006 Georgia Tech Research Corporation         **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 ** Purpose: This a lame class I added temporarily. It seems to      **
 ** have stuck around.                                               **
 **                                                                  **
 **                                                                  **
 **********************************************************************/

#ifndef GlOBALUTILITIES_H
#define GlOBALUTILITIES_H

#include "ProjectDefinitions.h"

class GlobalUtilities
{
 public:

  double round( double value )
  {
	double valueAsFloor = floor(value);

	if( (value - valueAsFloor) >= .5 )		//round the current location
	  return (int)ceil(value);
	else
	  return (int)valueAsFloor; 
  }
};

#endif

/**********************************************************************
 * $Log: GlobalUtilities.h,v $
 * Revision 1.1.1.1  2008/07/14 16:44:17  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/20 17:17:47  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:37:55  endo
 * MissionLab 7.0
 *
 * Revision 1.1  2006/06/08 14:47:42  endo
 * CommBehavior from MARS 2020 migrated.
 *
 **********************************************************************/
