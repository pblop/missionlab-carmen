/**********************************************************************
 **                                                                  **
 **                         TwoWorldPoints.hpp                       **
 **                                                                  **
 **                                                                  **
 **  Written by: Eric Matinson                                       **
 **                                                                  **
 **  Copyright 2006 - 2007, Georgia Tech Research Corporation        **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **********************************************************************/

/* $Id: TwoWorldPoints.hpp,v 1.1.1.1 2008/07/14 16:44:21 endo Exp $ */

#ifndef TwoWorldPointsType_HPP
#define TwoWorldPointsType_HPP

//-----------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------
#include <string>
#include <X11/X.h>
#include <Xm/Xm.h>

#include "TypeInfo.hpp"
#include "symbol.hpp"

using std::string;

class TwoWorldPointsType : public TypeInfo {

protected:
    Widget wlat_;
    Widget wlon_;

    static const string POINT_FILENAME_PREFIX_;

    void runOvlPickPointButton_(void);
    bool readPointFromOverlay_(double *lat, double *lon);

    static void cbRunOvlPickPointButton_(Widget w, XtPointer clientData, XtPointer callbackData);

public:

    TwoWorldPointsType(const char *TypeName);
    ~TwoWorldPointsType() {}

    Widget CreateDisplayWidget(Widget parent, Symbol *parm);
    char *PrintValue(struct Symbol *parm, const char *separator);
    void UpdateFromDisplayWidget(Widget w, Symbol *parm);
};

#endif

/**********************************************************************
 * $Log: TwoWorldPoints.hpp,v $
 * Revision 1.1.1.1  2008/07/14 16:44:21  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.3  2007/09/07 23:12:05  endo
 * The overlay name is now remembered when the coordinates are picked from an overlay.
 *
 * Revision 1.2  2006/12/05 01:47:02  endo
 * cdl parser now accepts {& a b} and Symbol *pair added.
 *
 * Revision 1.1  2006/11/28 04:13:28  ebeowulf
 * An extension to the TwoPoints selection system, only applied to
 * selecting latitude and longitude.
 **********************************************************************/
