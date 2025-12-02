/**********************************************************************
 **                                                                  **
 **                           TwoPoints.cc                           **
 **                                                                  **
 **                                                                  **
 **  Written by: Yoichiro Endo                                       **
 **                                                                  **
 **  Copyright 2007, Georgia Tech Research Corporation               **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **********************************************************************/

/* $Id: TwoPoints.cc,v 1.1.1.1 2008/07/14 16:44:21 endo Exp $ */

//-----------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------
#include <unistd.h>
#include <string>
#include <X11/X.h>
#include <Xm/Xm.h>
#include <Xm/Text.h>
#include <Xm/Frame.h>
#include <Xm/Form.h>
#include <Xm/DialogS.h>
#include <Xm/PanedW.h>
#include <Xm/TextF.h>
#include <Xm/SelectioB.h>
#include <Xm/LabelG.h>
#include <Xm/RowColumn.h>
#include <Xm/PushBG.h>
#include <Xm/Scale.h>
#include <Xm/ToggleB.h>

#include "utilities.h"
#include "gt_std.h"
#include "TypeInfo.hpp"
#include "TwoPoints.hpp"

using std::string;

//-----------------------------------------------------------------------
// Externs
//-----------------------------------------------------------------------
extern void gSavePickedOverlayName(string overlayName);

//-----------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------
const string TwoPointsType::POINT_FILENAME_PREFIX_ = "/tmp/TwoPointsType-point";

//-----------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------
TwoPointsType::TwoPointsType(const char *TypeName) :
    TypeInfo(TypeName),
    wx_(NULL),
    wy_(NULL)
{
}

//-----------------------------------------------------------------------
Widget TwoPointsType::CreateDisplayWidget(Widget parent, Symbol *parm)
{
    Widget w, xlabel, ylabel, ovl_pick_pt_button;
    XmString xmstr;
    char *xval = NULL, *yval = NULL;
    char *label = "";

    if( parm->name )
    {
        if( parm->name[0] == '%' )
        {
            label = &parm->name[1];
        }
        else
        {
            label = parm->name;
        }
    }

    // Get the value
    if( parm->input_generator && parm->input_generator->name )
    {
        xval = strdup(parm->input_generator->name);
    }
    else
    {
        xval = strdup("0,0");
    }

    yval = strchr(xval,',');

    if( yval == NULL )
    {
        free(xval);
        xval = strdup("0");
        yval = "0";
    }
    else
    {
        // Right now, xval is "x.xx, y.yy" and yval is ", y.yy". Replace, ","
        // of yval with NULL terminate, so that, xval string ends before ",".
        *yval = '\0';
        yval++;

        // Trim any leading whitespace of yval.
        while((*yval)&&(isspace(*yval))) yval++;
    }

    // Create a blank line
    xmstr = XmStringCreateLocalized(" ");
    XtVaCreateManagedWidget(
        "",
        xmLabelGadgetClass, parent,
        XmNlabelString, xmstr,
        NULL);
    XmStringFree(xmstr);

    // Create the frame widget; Lilia - added width parameter to display correctly
    w = XtVaCreateWidget(
        "",
        xmFormWidgetClass, parent,
        XmNwidth, TYPEINFO_DEFAULT_WIDTH,
        XmNleftAttachment,	XmATTACH_FORM,
        XmNrightAttachment,	XmATTACH_FORM,
        NULL);
               
    // Create the X text widget
    wx_ = XtVaCreateManagedWidget(
        "", xmTextWidgetClass, w,
        XmNvalue,   xval,
        XmNrightAttachment, XmATTACH_FORM,
        XmNtopAttachment, XmATTACH_FORM,
        NULL);

    // Create the Y text widget
    wy_ = XtVaCreateManagedWidget(
        "", xmTextWidgetClass, w,
        XmNvalue,   yval,
        XmNtopAttachment, XmATTACH_WIDGET,
        XmNtopWidget, wx_,
        XmNrightAttachment, XmATTACH_FORM,
        XmNbottomAttachment, XmATTACH_FORM,
        NULL);

    // Create the ovl_pick_pt_button.
    xmstr = XmStringCreateLocalized("Pick from Overlay");
    ovl_pick_pt_button = XtVaCreateManagedWidget(
        "", xmPushButtonGadgetClass, w,
        XmNlabelString, xmstr,
        XmNtopAttachment,     XmATTACH_FORM,
        XmNbottomAttachment,  XmATTACH_FORM,
        XmNleftAttachment,    XmATTACH_FORM,
        XmNshowAsDefault,     False,
        XmNdefaultButtonShadowThickness,1,
        NULL);
    XmStringFree(xmstr);
    XtAddCallback(
        ovl_pick_pt_button,
        XmNactivateCallback,
        (XtCallbackProc)cbRunOvlPickPointButton_,
        this);
 
    xlabel = XtVaCreateManagedWidget(
        "X",
        xmLabelGadgetClass, w,
        XmNtopAttachment, XmATTACH_FORM,
        XmNrightAttachment, XmATTACH_WIDGET,
        XmNrightWidget, wx_,
        XmNleftWidget, ovl_pick_pt_button,
        XmNtopOffset,        10,
        NULL);

    ylabel = XtVaCreateManagedWidget(
        "Y",
        xmLabelGadgetClass, w,
        XmNtopWidget, xlabel,
        XmNtopAttachment, XmATTACH_WIDGET,
        XmNbottomAttachment, XmATTACH_FORM,
        XmNrightAttachment, XmATTACH_WIDGET,
        XmNrightWidget, wy_,
        XmNleftWidget, ovl_pick_pt_button,
        XmNtopOffset,        10,
        NULL);

    XtManageChild(w);
               
    return w;
}

//-----------------------------------------------------------------------
void TwoPointsType::UpdateFromDisplayWidget(Widget w, Symbol *parm)
{
    char *xvalue = NULL;
    char *yvalue = NULL;

    if (parm == NULL)
    {
        fprintf(
            stderr,
            "Warning: TwoPoints::UpdateFromDisplayWidget(). parm is NULL.\n");
        return;
    }

    XtVaGetValues(
        wx_,
        XmNvalue, &xvalue,
        NULL);

    XtVaGetValues(
        wy_,
        XmNvalue, &yvalue,
        NULL);

    // Trim any leading whitespace of xvalue and yvalue.
    while((*xvalue)&&(isspace(*xvalue))) xvalue++;
    while((*yvalue)&&(isspace(*yvalue))) yvalue++;

    char buf[2048];
    sprintf(buf,"%s, %s", xvalue, yvalue);
        
    if ((parm->input_generator) != NULL)
    {
        if ((parm->input_generator->name) != NULL)
        {
            free(parm->input_generator->name);
        }

        parm->input_generator->name = strdup(buf);
    }
}

//-----------------------------------------------------------------------
char *TwoPointsType::PrintValue(struct Symbol *parm, const char *separator)
{

    char buf[2048];
    buf[0] = '\0';  

    char *val = "Unknown, Unknown";

    // Get the value
    if(parm->input_generator && parm->input_generator->name)
    {
        val = parm->input_generator->name;
    }

    if( parm->name[0] == '%' )
    {
        sprintf(buf,"%s <%s>", &parm->name[1], val);
    }
    else
    {
        sprintf(buf,"%s <%s>", parm->name, val);
    }
  
    return strdup(buf);
}

//-----------------------------------------------------------------------
// This function is to read the coordinates from overlay by running mlab
//-----------------------------------------------------------------------
void TwoPointsType::runOvlPickPointButton_(void)
{
    float x, y;
    char xstr[80];
    char ystr[80];

    if (readPointFromOverlay_(&x, &y))
    {
        sprintf(xstr, "%.2f", x);
        sprintf(ystr, "%.2f", y);

        XtUnmanageChild(wx_);
        XtUnmanageChild(wy_);

        XmTextSetString( wx_, xstr);
        XmTextSetString( wy_, ystr);

        XtManageChild(wx_);
        XtManageChild(wy_);
    }
}

//-----------------------------------------------------------------------
// This function is to read the coordinates from overlay by running mlab
//-----------------------------------------------------------------------
void TwoPointsType::cbRunOvlPickPointButton_(Widget w, XtPointer clientData, XtPointer callbackData)
{
     TwoPointsType *twoPointsTypeInstance = NULL;

     twoPointsTypeInstance = (TwoPointsType *)clientData;
     twoPointsTypeInstance->runOvlPickPointButton_();
}

//-----------------------------------------------------------------------
// This function is to read the coordinates from overlay by running mlab
//-----------------------------------------------------------------------
bool TwoPointsType::readPointFromOverlay_(float *x, float *y)
{
    FILE *pointFile = NULL;
    string overlayNameStr;
    char pointFileName[1024];
    char cmd[1024];
    char label[1024], overlayName[1024];
    int status, fd;

    // Create the output file name.
    sprintf(
        pointFileName,
        "%s-%s-XXXXXX",
        POINT_FILENAME_PREFIX_.c_str(),
        getenv("USER"));
    fd = mkstemp(pointFileName);
    unlink(pointFileName);

    // Execute mlab.
    // Single quotes around filename to handle weird chars in overlay names, for
    // example, when USER is (null). This WILL fail if the username contains
    // a single quote, but that is unlikely, as POSIX disallows it.
    sprintf(cmd, "mlab -m '%s' -n", pointFileName);
    system(cmd);

    if ((pointFile = fopen(pointFileName, "r")) == NULL)
    {
        //can't open this file
        return false;
    } 
    else 
    {
        status = fscanf(pointFile, "%f, %f", x, y);

        if (status != EOF)
        {
            status = fscanf(pointFile, "%s %s", label, overlayName);

            overlayNameStr = overlayName;

            if (status != EOF)
            {
                gSavePickedOverlayName(overlayNameStr);
            }
        }

        fclose(pointFile);
        unlink(pointFileName);

        return true;
    }
}

/**********************************************************************
 * $Log: TwoPoints.cc,v $
 * Revision 1.1.1.1  2008/07/14 16:44:21  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.1  2007/09/07 23:12:05  endo
 * The overlay name is now remembered when the coordinates are picked from an overlay.
 *
 **********************************************************************/
