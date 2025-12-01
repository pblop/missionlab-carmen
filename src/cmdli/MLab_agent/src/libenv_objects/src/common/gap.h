#ifndef gap_H
#define gap_H
/**********************************************************************
 **                                                                  **
 **  gap.h                                                           **
 **                                                                  **
 **  Implement a gap-like environmental object.                      **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-1998.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Alto, Michigan, USA                                             **
 **                                                                  **
 **********************************************************************/

/* $Id: gap.h,v 1.1.1.1 2008/07/14 16:44:16 endo Exp $ */

/**********************************************************************
* $Log: gap.h,v $
* Revision 1.1.1.1  2008/07/14 16:44:16  endo
* MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
*
* Revision 1.1.1.1  2006/07/20 17:17:46  endo
* MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
*
* Revision 1.1.1.1  2006/07/12 13:37:54  endo
* MissionLab 7.0
*
* Revision 1.1  2006/07/01 00:21:03  endo
* CMDLi from MARS 2020 migrated into AO-FNC repository.
*
* Revision 1.1.1.1  2006/06/29 20:42:07  endo
* cmdli local repository.
*
* Revision 1.4  2004/05/11 19:34:39  doug
* massive changes to integrate with USC and GaTech
*
* Revision 1.3  2004/03/08 14:52:28  doug
* cross compiles on visual C++
*
* Revision 1.2  2002/09/20 22:34:44  doug
* *** empty log message ***
*
* Revision 1.1  2002/09/20 13:05:19  doug
* Initial revision
*
**********************************************************************/

#include "mic.h"
#include "env_object.h"
#include "Vector.h"

/**********************************************************************/

namespace sara
{
class gap : public env_object
{
public:
   // gaps are not movable.
   bool is_movable() {return false;}

   // Constructors
   gap(shape *theShape);
   gap(shape *theShape, const strings *label);
   gap(shape *theShape, const strings *label, const string &color);

   // destructor
   ~gap();

   /// build a set of lines that are the gap
   static class shape *makeGap(const Vector &beg, const Vector &end, const double width);
   static class shape *makeGap(const Vector &beg, const Vector &end);
};

}
/**********************************************************************/
#endif

