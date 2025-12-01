/**********************************************************************
 **                                                                  **
 **                       TwoWorldPointsType.cc                      **
 **                                                                  **
 **                                                                  **
 **  Written by: Yoichiro Endo                                       **
 **                                                                  **
 **  Copyright 2007, Georgia Tech Research Corporation               **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **********************************************************************/

/* $Id: TwoWorldPoints.cc,v 1.1.1.1 2008/07/14 16:44:21 endo Exp $ */

//-----------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------
#include <unistd.h>
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
#include "string_utils.h"
#include "gt_std.h"
#include "convert_scale.h"
#include "TwoWorldPoints.hpp"

//-----------------------------------------------------------------------
// Externs
//-----------------------------------------------------------------------
extern void gSavePickedOverlayName(string overlayName);

//-----------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------
const string TwoWorldPointsType::POINT_FILENAME_PREFIX_ = "/tmp/TwoWorldPointsType-point";

//-----------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------
TwoWorldPointsType::TwoWorldPointsType(const char *TypeName) :
    TypeInfo(TypeName),
    wlat_(NULL),
    wlon_(NULL)
{
}

//-----------------------------------------------------------------------
Widget TwoWorldPointsType::CreateDisplayWidget(Widget parent, Symbol *parm)
{
    Widget w, latLabel, lonLabel, ovl_pick_pt_button;
    XmString str;
    string latVal = "0";
    string lonVal = "0";

    // Get the lat value
    if(((parm->input_generator) != NULL) &&
       ((parm->input_generator->name) != NULL))
    {
        latVal = parm->input_generator->name;
    }

    // Get the lat value
    if(((parm->pair) != NULL) &&
       ((parm->pair->input_generator) != NULL) &&
       ((parm->pair->input_generator->name) != NULL))
    {
        lonVal = parm->pair->input_generator->name;
    }

    // Create a blank line
    str = XmStringCreateLocalized(" ");
    XtVaCreateManagedWidget(
        "",
        xmLabelGadgetClass, parent,
        XmNlabelString, str,
        NULL);
    XmStringFree(str);

    w = XtVaCreateWidget(
        "",
        xmFormWidgetClass, parent,
        XmNwidth, TYPEINFO_DEFAULT_WIDTH,
        XmNleftAttachment,	XmATTACH_FORM,
        XmNrightAttachment,	XmATTACH_FORM,
        NULL);
               
    // Create the Latitude text widget
    wlat_ = XtVaCreateManagedWidget(
        "",
        xmTextWidgetClass, w,
        XmNvalue, (char *)(latVal.c_str()),
        XmNrightAttachment, XmATTACH_FORM,
        XmNtopAttachment, XmATTACH_FORM,
        NULL);

    // Create the Y text widget
    wlon_ = XtVaCreateManagedWidget(
        "",
        xmTextWidgetClass, w,
        XmNvalue, (char *)(lonVal.c_str()),
        XmNtopAttachment, XmATTACH_WIDGET,
        XmNtopWidget, wlat_,
        XmNrightAttachment, XmATTACH_FORM,
        XmNbottomAttachment, XmATTACH_FORM,
        NULL);

    // Create the ovl_pick_pt_button.
    str = XmStringCreateLocalized("Pick from Overlay");
    ovl_pick_pt_button = XtVaCreateManagedWidget(
        "",
        xmPushButtonGadgetClass, w,
        XmNlabelString, str,
        XmNtopAttachment, XmATTACH_FORM,
        XmNbottomAttachment, XmATTACH_FORM,
        XmNleftAttachment, XmATTACH_FORM,
        XmNshowAsDefault, False,
        XmNdefaultButtonShadowThickness,1,
        NULL);
    XmStringFree(str);
    XtAddCallback(
        ovl_pick_pt_button,
        XmNactivateCallback,
        (XtCallbackProc)cbRunOvlPickPointButton_,
        this);
 
    latLabel = XtVaCreateManagedWidget(
        "Latitude",
        xmLabelGadgetClass, w,
        XmNtopAttachment, XmATTACH_FORM,
        XmNrightAttachment, XmATTACH_WIDGET,
        XmNrightWidget, wlat_,
        XmNleftWidget, ovl_pick_pt_button,
        XmNtopOffset, 10,
        NULL);

    lonLabel = XtVaCreateManagedWidget(
        "Longitude",
        xmLabelGadgetClass, w,
        XmNtopWidget, latLabel,
        XmNtopAttachment, XmATTACH_WIDGET,
        XmNbottomAttachment, XmATTACH_FORM,
        XmNrightAttachment, XmATTACH_WIDGET,
        XmNrightWidget, wlon_,
        XmNleftWidget, ovl_pick_pt_button,
        XmNtopOffset, 10,
        NULL);

    XtManageChild(w);
               
    return w;
}

//-----------------------------------------------------------------------
void TwoWorldPointsType::UpdateFromDisplayWidget(Widget w, Symbol *parm)
{
    string latVal, lonVal;
    char *latValue = NULL;
    char *lonValue = NULL;

    if (parm == NULL)
    {
        fprintf(
            stderr,
            "Warning: TwoWorldPoints::UpdateFromDisplayWidget(). parm is NULL.\n");
        return;
    }

    if ((parm->input_generator) != NULL)
    {
        XtVaGetValues(
            wlat_,
            XmNvalue, &latValue,
            NULL);

        if (latValue != NULL)
        {
            latVal = latValue;
            latVal = removeCharInString(latVal, ' ');

            if ((parm->input_generator->name) != NULL)
            {
                free(parm->input_generator->name);
            }

            parm->input_generator->name = strdup(latVal.c_str());

            free(latValue);
            latValue = NULL;
        }
    }

    if (((parm->pair) != NULL) &&
        ((parm->pair->input_generator) != NULL))
    {
        XtVaGetValues(
            wlon_,
            XmNvalue, &lonValue,
            NULL);

        if (lonValue != NULL)
        {
            lonVal = lonValue;
            lonVal = removeCharInString(lonVal, ' ');

            if ((parm->pair->input_generator->name) != NULL)
            {
                free(parm->pair->input_generator->name);
            }

            parm->pair->input_generator->name = strdup(lonVal.c_str());

            free(lonValue);
            lonValue = NULL;
        }
    }
}


char *TwoWorldPointsType::PrintValue(struct Symbol *parm, const char *separator)
{

    char buf[2048];
    buf[0] = '\0';  

    char *val = "Unknown, Unknown";

    // Get the value
    if(parm->input_generator && parm->input_generator->name)
    {
        val = parm->input_generator->name;
    }

    if(parm->name[0] == '%')
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
void TwoWorldPointsType::runOvlPickPointButton_(void)
{
    double lat, lon;
    char xstr[80];
    char ystr[80];

    if (readPointFromOverlay_(&lat, &lon))
    {
        sprintf(xstr, "%.6f", lat);
        sprintf(ystr, "%.6f", lon);

        XtUnmanageChild(wlat_);
        XtUnmanageChild(wlon_);

        XmTextSetString(wlat_, xstr);
        XmTextSetString(wlon_, ystr);

        XtManageChild(wlat_);
        XtManageChild(wlon_);
    }
}

//-----------------------------------------------------------------------
// This function is to read the coordinates from overlay by running mlab
//-----------------------------------------------------------------------
void TwoWorldPointsType::cbRunOvlPickPointButton_(Widget w, XtPointer clientData, XtPointer callbackData)
{
     TwoWorldPointsType *twoWorldPointsTypeInstance = NULL;

     twoWorldPointsTypeInstance = (TwoWorldPointsType *)clientData;
     twoWorldPointsTypeInstance->runOvlPickPointButton_();
}

//-----------------------------------------------------------------------
// This function is to read the coordinates from overlay by running mlab
//-----------------------------------------------------------------------
bool TwoWorldPointsType::readPointFromOverlay_(double *lat, double *lon)
{
    FILE *worldPointFile = NULL;
    string overlayNameStr;
    char worldPointFileName[1024];
    char cmd[1024];
    char label[1024], overlayName[1024];
    int status, fd;

    // Create the output file name.
    sprintf(
        worldPointFileName,
        "%s-%s-XXXXXX",
        POINT_FILENAME_PREFIX_.c_str(),
        getenv("USER"));
    fd = mkstemp(worldPointFileName);
    unlink(worldPointFileName);

    // Execute mlab.
    sprintf(cmd, "mlab -G %s -n", worldPointFileName);
    system(cmd);

    if ((worldPointFile = fopen(worldPointFileName, "r")) == NULL)
    {
        //can't open this file
        return false;
    } 
    else 
    {
        status = fscanf(worldPointFile, "%lf, %lf", lat, lon);

        if (status != EOF)
        {
            status = fscanf(worldPointFile, "%s %s", label, overlayName);

            overlayNameStr = overlayName;

            if (status != EOF)
            {
                gSavePickedOverlayName(overlayNameStr);
            }
        }

        fclose(worldPointFile);
        unlink(worldPointFileName);

        return true;
    }
}

/**********************************************************************
 * $Log: TwoWorldPoints.cc,v $
 * Revision 1.1.1.1  2008/07/14 16:44:21  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.1  2007/09/07 23:12:05  endo
 * The overlay name is now remembered when the coordinates are picked from an overlay.
 *
 **********************************************************************/
