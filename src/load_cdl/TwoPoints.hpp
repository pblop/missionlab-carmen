/**********************************************************************
 **                                                                  **
 **                           TwoPoints.hpp                          **
 **                                                                  **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1995 - 2007, Georgia Tech Research Corporation        **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **********************************************************************/

/* $Id: TwoPoints.hpp,v 1.1.1.1 2008/07/14 16:44:21 endo Exp $ */

#ifndef TwoPointsType_HPP
#define TwoPointsType_HPP

//-----------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------
#include <unistd.h>
#include <string>

#include "TypeInfo.hpp"
#include "symbol.hpp"

using std::string;

class TwoPointsType : public TypeInfo {

protected:
    Widget wx_;
    Widget wy_;

    static const string POINT_FILENAME_PREFIX_;

    void runOvlPickPointButton_(void);
    bool readPointFromOverlay_(float *x, float *y);

    static void cbRunOvlPickPointButton_(Widget w, XtPointer clientData, XtPointer callbackData);

public:
    TwoPointsType(const char *TypeName);
    ~TwoPointsType() {};

    Widget CreateDisplayWidget(Widget parent, Symbol *parm);
    char *PrintValue(struct Symbol *parm, const char *separator);
    void UpdateFromDisplayWidget(Widget w, Symbol *parm);
};

#endif

/**********************************************************************
 * $Log: TwoPoints.hpp,v $
 * Revision 1.1.1.1  2008/07/14 16:44:21  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.5  2007/09/07 23:12:05  endo
 * The overlay name is now remembered when the coordinates are picked from an overlay.
 *
 * Revision 1.4  2006/12/05 01:47:02  endo
 * cdl parser now accepts {& a b} and Symbol *pair added.
 *
 * Revision 1.3  2006/08/29 15:13:57  endo
 * Advanced parameter flag added to symbol.
 *
 * Revision 1.2  2006/08/08 17:34:07  lilia
 * TwoPoints width fixed.
 *
 * Revision 1.1.1.1  2006/07/20 17:17:49  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:37:59  endo
 * MissionLab 7.0
 *
 * Revision 1.1.1.1  2005/02/06 23:00:06  endo
 * AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
 *
 * Revision 1.8  2003/04/06 12:58:48  endo
 * gcc 3.1.1
 *
 * Revision 1.7  2002/01/13 01:24:31  endo
 * list ->GTList
 *
 * Revision 1.6  2000/10/16 19:28:28  endo
 * Modified due to the compiler upgrade.
 *
 * Revision 1.5  2000/04/13 22:31:17  endo
 * Checked in for Doug.
 *
 * Revision 1.4  1999/07/03 22:04:03  endo
 * Pick-point-from-Overlay feature added.
 *
 * Revision 1.3  1996/10/04 21:10:59  doug
 * changes to get to version 1.0c
 *
 * Revision 1.3  1996/10/01 13:01:32  doug
 * got library writes working
 *
 * Revision 1.2  1996/05/14  23:17:39  doug
 * added twopoints case
 *
 * Revision 1.1  1996/05/14  18:56:27  doug
 * Initial revision
 **********************************************************************/
