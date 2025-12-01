/**********************************************************************
 **                                                                  **
 **  gap.cc                                                          **
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

/* $Id: gap.cc,v 1.1.1.1 2008/07/14 16:44:16 endo Exp $ */

/**********************************************************************
* $Log: gap.cc,v $
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
* Revision 1.5  2004/05/11 19:34:39  doug
* massive changes to integrate with USC and GaTech
*
* Revision 1.4  2004/03/08 14:52:28  doug
* cross compiles on visual C++
*
* Revision 1.3  2002/09/20 22:34:44  doug
* *** empty log message ***
*
* Revision 1.1  2002/09/20 13:05:19  doug
* Initial revision
*
**********************************************************************/

#include "mic.h"
#include "Vector.h"
#include "gap.h"
#include "lines.h"
#include <math.h>

namespace sara
{
static double DEFAULT_GAP_WIDTH = 40.0;
static string DEFAULT_COLOR = "black";

// *********************************************************************
// Constructor
gap::gap(shape *theShape) :
   env_object(theShape)
{ /* empty */ }

// *********************************************************************
// Constructor
gap::gap(shape *theShape, const strings *label) :
   env_object(theShape, label)
{ /* empty */ }

// *********************************************************************
// Constructor
gap::gap(shape *theShape, const strings *label, const string &new_color) :
   env_object(theShape, label, new_color)
{ /* empty */ }

// *********************************************************************
// destructor
gap::~gap()
{
}

// *********************************************************************
/// build a set of lines that are the gap
shape *
gap::makeGap(const Vector &beg, const Vector &end)
{
   return makeGap(beg, end, DEFAULT_GAP_WIDTH);
}

// *********************************************************************
/// build a set of lines that are the gap
shape *
gap::makeGap(const Vector &beg, const Vector &end, const double width)
{
#define LIP_WIDTH (4.0/6.0)
#define LIP_LNG (0.2)

   Vector left = beg;
   Vector right = end;

   // create the outline 
   Vector axis = end - beg;
   Vector unit_axis = axis.unit();
   double length = axis.length_2d();

   // offset back to left
   Vector h_offset = unit_axis;
   h_offset *= -1 * LIP_LNG*length;

   Vector ul = unit_axis;
   ul.rotateZ(90);
   ul *= LIP_WIDTH * width;
   ul += left;
   ul += h_offset;

   Vector ll = unit_axis;
   ll.rotateZ(-90);
   ll *= LIP_WIDTH * width;
   ll += left;
   ll += h_offset;

   // offset to right
   h_offset = unit_axis;
   h_offset *= LIP_LNG * length;

   Vector ur = unit_axis;
   ur.rotateZ(90);
   ur *= LIP_WIDTH * width;
   ur += right;
   ur += h_offset;

   Vector lr = unit_axis;
   lr.rotateZ(-90);
   lr *= LIP_WIDTH * width;
   lr += right;
   lr += h_offset;


   Vector uml = unit_axis;
   uml.rotateZ(90);
   uml *= width/2;
   uml += left;

   Vector lml = unit_axis;
   lml.rotateZ(-90);
   lml *= width/2;
   lml += left;

   Vector umr = unit_axis;
   umr.rotateZ(90);
   umr *= width/2;
   umr += right;

   Vector lmr = unit_axis;
   lmr.rotateZ(-90);
   lmr *= width/2;
   lmr += right;

   // build the outline
   Vectors line1;
   line1.push_back(ul);
   line1.push_back(uml);
   line1.push_back(umr);
   line1.push_back(ur);

   Vectors line2;
   line2.push_back(ll);
   line2.push_back(lml);
   line2.push_back(lmr);
   line2.push_back(lr);

   Vectorss theLines;
   theLines.push_back(line1);
   theLines.push_back(line2);

   shape *rtn = new lines(theLines);
   return rtn;
}

/**********************************************************************/
}
