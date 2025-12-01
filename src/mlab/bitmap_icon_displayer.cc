/**********************************************************************
 **                                                                  **
 **                      bitmap_icon_displayer.cc                    **
 **                                                                  **
 **  Written by:  Yoichiro Endo                                      **
 **                                                                  **
 **  Copyright 2007 Georgia Tech Research Corporation                **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: bitmap_icon_displayer.cc,v 1.1.1.1 2008/07/14 16:44:22 endo Exp $ */

//-----------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------
#include <Xm/IconG.h>
#include <Xm/LabelG.h>
#include <Xm/PushBG.h>
#include <Xm/XmP.h>
#include <algorithm>
#include <cctype>

#include "bitmap_icon_displayer.h"
#include "draw.h"
#include "console.h"
#include "shape.hpp"
#include "bitmaps/UnknownBtn.bit"

using std::transform;
using std::tolower;

//-----------------------------------------------------------------------
// Externs
//-----------------------------------------------------------------------
extern double meters_per_pixel;

//-----------------------------------------------------------------------
// Macros
//-----------------------------------------------------------------------
#define BID_CREATE_PIXMAP(name,fg,bg,parent) XCreatePixmapFromBitmapData(XtDisplay(parent),\
        RootWindowOfScreen(XtScreen(parent)),\
        (char *)name##_bits, name##_width, name##_height, fg, bg, \
        DefaultDepthOfScreen(XtScreen(parent)))
#define BID_FIX_DISPLAY_X(c) (int)(c)
#define BID_FIX_DISPLAY_Y(c) ((int)(mission_area_height_meters / meters_per_pixel) - (int)(c))

//-----------------------------------------------------------------------
// Constatns
//-----------------------------------------------------------------------
const string BitmapIconDisplayer::BITMAP_DIRECTORY_ = "/src/mlab/bitmaps/";
const string BitmapIconDisplayer::BITMAP_EXTENSION_ = ".bit";
const string BitmapIconDisplayer::EMPTY_STRING_ = "";
const int BitmapIconDisplayer::LABEL_Y_OFFSET_ = 2;

//-----------------------------------------------------------------------
// Constructor for BitmapIconDisplayer class.
//-----------------------------------------------------------------------
BitmapIconDisplayer::BitmapIconDisplayer(
    Display *display,
    Widget parent,
    XtAppContext app,
    Widget drawingArea,
    const symbol_table<rc_chain> &rcTable) :
    display_(display),
    appContext_(app),
    parentWidget_(parent),
    drawingArea_(drawingArea),
    exposeDrawingAreaCallbackIsActive_(false)
{
    XGCValues xgcv;

    rcTable_ = rcTable;

    XtVaGetValues(
        parentWidget_,
        XmNforeground, &defaultFgPixel_,
        XmNbackground, &defaultBgPixel_,
        NULL);

    xgcv.foreground = defaultFgPixel_;
    defaultFgGC_ = XtGetGC(drawingArea_, GCForeground, &xgcv);

    xgcv.foreground = defaultBgPixel_;
    defaultBgGC_ = XtGetGC(drawingArea_, GCForeground, &xgcv);
}

//-----------------------------------------------------------------------
// This function displays the icon.
//-----------------------------------------------------------------------
void BitmapIconDisplayer::displayIcon(
    float x,
    float y,
    string bitmapName,
    string label,
    string enforcedFgColor,
    string enforcedBgColor)
{
    IconInfo_t iconInfo;
    XmString labelXmString;
    Pixel fgPixel, bgPixel;
    GC fgGC, bgGC;
    string fgColorName, bgColorName;
    map<string, IconInfo_t>::iterator itr;
    int dispX, dispY, w, h;
    bool createIcon = false;

    itr = iconTable_.find(label);

    if (itr == iconTable_.end())
    {
        createIcon = true;
    }
    else
    {
        iconInfo = itr->second;

        if ((iconInfo.bitmapName != bitmapName) ||
            (iconInfo.enforcedFgColor != enforcedFgColor) ||
            (iconInfo.enforcedBgColor != enforcedBgColor))
        {
            createIcon = true;

            if (iconInfo.iconIsUp)
            {
                if (XtIsManaged(iconInfo.iconGadget))
                {
                    XtUnmanageChild(iconInfo.iconGadget);
                }

                XtDestroyWidget(iconInfo.iconGadget);
                iconInfo.iconIsUp = false;

                XFreePixmap(display_, iconInfo.pixmap);
            }
        }
    }

    if (createIcon)
    {
        if (enforcedFgColor != EMPTY_STRING_)
        {
            fgPixel = getPixelByColorName(enforcedFgColor);
            fgGC = getGCByColorName(enforcedFgColor);
        }
        else
        {
            fgPixel = defaultFgPixel_;
            fgGC = defaultFgGC_;
        }

        if (enforcedBgColor != EMPTY_STRING_)
        {
            bgPixel = getPixelByColorName(enforcedBgColor);
            bgGC = getGCByColorName(enforcedBgColor);
        }
        else
        {
            bgPixel = defaultBgPixel_;
            bgGC = defaultBgGC_;
        }

        iconInfo.bitmapName = bitmapName;
        iconInfo.enforcedFgColor = enforcedFgColor;
        iconInfo.enforcedBgColor = enforcedBgColor;
        iconInfo.fgPixel = fgPixel;
        iconInfo.bgPixel = bgPixel;
        iconInfo.fgGC = fgGC;
        iconInfo.bgGC = bgGC;
        iconInfo.pixmap = createPixmap_(bitmapName, fgPixel, bgPixel);

        labelXmString = XmStringCreateLtoR
            ((String)label.c_str(),
             XmSTRING_DEFAULT_CHARSET);

        iconInfo.iconGadget = XtVaCreateWidget
            ("iconInfo.iconGadget",
             xmPushButtonGadgetClass, drawingArea_,
             XmNlabelPixmap, iconInfo.pixmap,
             XmNlabelType, XmPIXMAP,
             XmNhighlightThickness, 0,
             XmNborderWidth, 0,
             XmNmarginHeight, 1,
             XmNmarginWidth, 1,
             XmNbackground, iconInfo.fgPixel,
             XmNarmColor, iconInfo.fgPixel,
             NULL);
    }

    w = XtWidth(iconInfo.iconGadget);
    h = XtHeight(iconInfo.iconGadget);
    dispX = BID_FIX_DISPLAY_X(x/meters_per_pixel) - w/2;
    dispY = BID_FIX_DISPLAY_Y(y/meters_per_pixel) - h/2;

    XtVaSetValues(
        iconInfo.iconGadget,
        XmNx, dispX,
        XmNy, dispY,
        NULL);

    iconInfo.iconIsUp = true;
    iconInfo.labelX = x;
    iconInfo.labelY = y + ((DRAW_TEXT_VERTICAL_OFFSET - LABEL_Y_OFFSET_ - ((float)h)/2.0)*meters_per_pixel);
    iconTable_[label] = iconInfo;

    clear_map();
    draw_world();

    XFlush(XtDisplay(drawingArea_));

    while (XtAppPending(appContext_))
    {
        XtAppProcessEvent(appContext_, XtIMAll);
    }
}

//-----------------------------------------------------------------------
// This function creates a pixmap from a bitmap.
//-----------------------------------------------------------------------
Pixmap BitmapIconDisplayer::createPixmap_(
    string bitmapName,
    Pixel fgPixel,
    Pixel bgPixel)
{
    Pixmap pixmap;
    Display *display = NULL;
    Drawable drawable;
    Screen *screen = NULL;
    string fileName;
    unsigned char *bitmapData = NULL;
    int status;
    int hotX, hotY;
    int screenNum;
    unsigned int w, h;

    fileName = MLAB_HOME;
    fileName += BITMAP_DIRECTORY_;
    fileName += bitmapName;
    fileName += BITMAP_EXTENSION_;

    status = XReadBitmapFileData(
        (char *)fileName.c_str(),
        &w,
        &h,
        &bitmapData,
        &hotX, 
        &hotY);

    if (status != BitmapSuccess)
    {
        switch (status) {

        case BitmapOpenFailed:
            fprintf(
                stderr,
                "BitmapIconDisplayer::createPixmap_(): BitmapOpenFailed. [%s]\n",
                fileName.c_str());
            break;

        case BitmapFileInvalid:
            fprintf(
                stderr,
                "BitmapIconDisplayer::createPixmap_(): BitmapFileInvalid. [%s]\n",
                fileName.c_str());
            break;

        case BitmapNoMemory:
            fprintf(stderr, "BitmapIconDisplayer::createPixmap_(): BitmapNoMemory.\n");
            break;

        default:
            fprintf(stderr, "BitmapIconDisplayer::createPixmap_(): Unknown error.\n");
            break;
        }

        pixmap = BID_CREATE_PIXMAP(
            UnknownBtn,
            fgPixel,
            bgPixel,
            parentWidget_);
    }
    else
    {
        display = XtDisplay(parentWidget_);
        screen = XtScreen(parentWidget_);
        drawable = RootWindowOfScreen(screen);
        screenNum = DefaultDepthOfScreen(screen);

        pixmap = XCreatePixmapFromBitmapData(
            display,
            drawable,
            (char *)bitmapData,
            w,
            h,
            fgPixel,
            bgPixel,
            screenNum);

        XFree(bitmapData);
    }

    return pixmap;

}

//-----------------------------------------------------------------------
// This function refreshes the display.
//-----------------------------------------------------------------------
void BitmapIconDisplayer::displayAll(void)
{
    map<string, IconInfo_t>::iterator itr;
    IconInfo_t iconInfo;
    string label;

    // Erase the icons being displayed.
    itr = iconTable_.begin();

    while (itr != iconTable_.end())
    {
        label = itr->first;
        iconInfo = itr->second;

        if (iconInfo.iconIsUp)
        {
            if (XtIsManaged(iconInfo.iconGadget))
            {
                XtUnmanageChild(iconInfo.iconGadget); 
            }

            XtManageChild(iconInfo.iconGadget);

            DrawText(
                iconInfo.labelX,
                iconInfo.labelY,
                iconInfo.fgGC,
                (char *)label.c_str());
        }

        itr++;
    }

    XFlush(XtDisplay(drawingArea_));

    while (XtAppPending(appContext_))
    {
        XtAppProcessEvent(appContext_, XtIMAll);
    }
}

/**********************************************************************
 * $Log: bitmap_icon_displayer.cc,v $
 * Revision 1.1.1.1  2008/07/14 16:44:22  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.1  2007/08/30 18:33:39  endo
 * BitmapIconDisplayer class added.
 *
 **********************************************************************/
