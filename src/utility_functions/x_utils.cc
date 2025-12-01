/**********************************************************************
 **                                                                  **
 **                            x_utils.cc                            **
 **                                                                  **
 **                                                                  **
 **  Written by: Yoichiro Endo                                       **
 **                                                                  **
 **  Copyright 2003 Georgia Tech Research Corporation                **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: x_utils.cc,v 1.1.1.1 2008/07/14 16:44:25 endo Exp $ */

#include "x_utils.h"

//-----------------------------------------------------------------------
// This function creates GC
//-----------------------------------------------------------------------
GC createGC(
    Pixel fg,
    Pixel bg,
    Widget w,
    int lineWidth,
    bool setArcMode,
    bool setXor)
{
    GC newGC;
    XGCValues xgcv;

    xgcv.foreground = fg;
    xgcv.background = bg;

    newGC = XtGetGC(w, GCForeground | GCBackground, &xgcv);

    if (setXor)
    {
        XSetFunction(XtDisplay(w), newGC, GXxor);
    }
    else
    {
        if (lineWidth > 0)
        {
            XSetLineAttributes(
                XtDisplay(w),
                newGC,
                lineWidth,
                LineSolid,
                CapButt,
                JoinMiter);
        }

        if (setArcMode)
        {
            XSetArcMode(XtDisplay(w), newGC, ArcPieSlice);
        }
    }

    return newGC;
}

//-----------------------------------------------------------------------
// This function creates pixel
//-----------------------------------------------------------------------
Pixel createPixel(Display *display, Colormap colormap, char *colorName)
{
    Pixel newPixel;
    XColor color, ignore;

    if (XAllocNamedColor(display, colormap, colorName, &color, &ignore))
    {
	    newPixel = color.pixel;
    }
    else
    {
        fprintf(stderr, "Warnign: Failed to create a \"%s\" color pixel.\n");
    }

    return newPixel;
}

//-----------------------------------------------------------------------
// This function creates a font and append it in the list.
//-----------------------------------------------------------------------
Font createAndAppendFont(
    Display *display,
    char *fontName,
    char *fontTag,
    XmFontList *fontList)
{
    XmFontListEntry entry;
    XFontStruct *font;

    font = XLoadQueryFont(display, fontName);

    entry = XmFontListEntryCreate(
        fontTag,
        XmFONT_IS_FONT,
        font);

    *fontList = XmFontListAppendEntry(*fontList, entry);
    
    XtFree((char *)entry);

    return (font->fid);
}

/**********************************************************************
 * $Log: x_utils.cc,v $
 * Revision 1.1.1.1  2008/07/14 16:44:25  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/20 17:17:52  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:38:01  endo
 * MissionLab 7.0
 *
 * Revision 1.1  2005/02/07 19:09:12  endo
 * *** empty log message ***
 *
 **********************************************************************/
