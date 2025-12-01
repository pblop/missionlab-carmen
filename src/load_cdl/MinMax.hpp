/**********************************************************************
 **                                                                  **
 **                             MinMax.hpp                           **
 **                                                                  **
 **                                                                  **
 **  Written by:  Yoichiro Endo                                      **
 **                                                                  **
 **  Copyright 2006  Georgia Tech Research Corporation               **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **********************************************************************/

/* $Id: MinMax.hpp,v 1.1.1.1 2008/07/14 16:44:21 endo Exp $ */

#ifndef MIN_MAX_HPP
#define MIN_MAX_HPP

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
#include "string_utils.h"

using std::string;

typedef enum MinMaxTypeStyle_t {
    MINMAXTYPE_INPUT_FORMAT_NUMBER,
    MINMAXTYPE_INPUT_FORMAT_TEXT
};

class MinMaxType : public TypeInfo {

protected:
    Widget wMin_;
    Widget wMax_;
    string description_;
    int inputFormat_;

    static const string MIN_LABEL_;
    static const string MAX_LABEL_;

    Widget createDisplayWidgetFormatNumberDELETE_(Widget parent, Symbol *parm) {

        Widget w, minLabel, maxLabel;
        XmString str;
        char *minVal = NULL, *maxVal = NULL;
        char *label = "";

        if(parm->name)
        {
            if(parm->name[0] == '%')
            {
                label = &parm->name[1];
            }
            else
            {
                label = parm->name;
            }
        }

        // Get the value
        if(parm->input_generator && parm->input_generator->name)
        {
            minVal = parm->input_generator->name;
        }
        else
        {
            minVal = strdup("0,0");
        }

        maxVal = strchr(minVal,',');

        if(maxVal == NULL)
        {
            free(minVal);
            minVal = strdup("0");
            maxVal = "0";
        }
        else
        {
            // Right now, minVal is "x.xx, y.yy" and maxVal is ", y.yy". Replace, ","
            // of maxVal with NULL terminate, so that, minVal string ends before ",".
            *maxVal = '\0';
            maxVal++;

            // Trim any leading whitespace of maxVal.
            while((*maxVal)&&(isspace(*maxVal))) maxVal++;
        }

        // Make the title
        str = XmStringCreateLocalized((char *)(description_.c_str()));
        XtVaCreateManagedWidget(
            "",
            xmLabelGadgetClass, parent,
            XmNlabelString, str,
            NULL);
        XmStringFree(str);

        // Create the frame widget
        w = XtVaCreateWidget(
            "",
            xmFormWidgetClass, parent,
            XmNwidth, TYPEINFO_DEFAULT_WIDTH,
            XmNleftAttachment,	XmATTACH_FORM,
            XmNrightAttachment,	XmATTACH_FORM,
            NULL);
               
        // Create the Min text widget
        wMin_ = XtVaCreateManagedWidget(
            "", xmTextWidgetClass, w,
            XmNvalue, minVal,
            XmNrightAttachment, XmATTACH_FORM,
            XmNtopAttachment, XmATTACH_FORM,
            XmNwidth, TYPEINFO_DEFAULT_WIDTH*3/4,
            NULL);

        // Create the Max text widget
        wMax_ = XtVaCreateManagedWidget(
            "", xmTextWidgetClass, w,
            XmNvalue,   maxVal,
            XmNtopAttachment, XmATTACH_WIDGET,
            XmNtopWidget, wMin_,
            XmNrightAttachment, XmATTACH_FORM,
            XmNbottomAttachment, XmATTACH_FORM,
            XmNwidth, TYPEINFO_DEFAULT_WIDTH*3/4,
            NULL);

        minLabel = XtVaCreateManagedWidget(
            MIN_LABEL_.c_str(),
            xmLabelGadgetClass, w,
            XmNtopAttachment, XmATTACH_FORM,
            XmNrightAttachment, XmATTACH_WIDGET,
            XmNrightWidget, wMin_,
            XmNleftAttachment, XmATTACH_FORM,
            XmNtopOffset, 10,
            NULL);

        maxLabel = XtVaCreateManagedWidget(
            MAX_LABEL_.c_str(),
            xmLabelGadgetClass, w,
            XmNtopWidget, minLabel,
            XmNtopAttachment, XmATTACH_WIDGET,
            XmNbottomAttachment, XmATTACH_FORM,
            XmNrightAttachment, XmATTACH_WIDGET,
            XmNrightWidget, wMax_,
            XmNleftAttachment, XmATTACH_FORM,
            XmNtopOffset, 10,
            NULL);

        XtManageChild(w);
               
        return w;
    }

    Widget createDisplayWidgetFormatNumber_(Widget parent, Symbol *parm) {

        Widget w, minLabel, maxLabel;
        XmString str;
        string value, minValue, maxValue;
        string::size_type index;
        char *label = "";
        const string SEPARATER_KEY = ",";

        if(parm->name)
        {
            if(parm->name[0] == '%')
            {
                label = &parm->name[1];
            }
            else
            {
                label = parm->name;
            }
        }

        // Get the value
        if(parm->input_generator && parm->input_generator->name)
        {
            value = parm->input_generator->name;

            // Get rid of '\n' and white spaces.
            value = removeCharInString(value, '\n');
            value = removeCharInString(value, ' ');
        }
        else
        {
            value = "0,0";
        }

        index = value.find(SEPARATER_KEY);

        if (index == string::npos)
        {
            fprintf(stderr, "Error (MinMax.hpp): No separator found.\n");
            return NULL;
        }

        // Extract the values.
        minValue = value.substr(0, index);
        maxValue = value.substr(index+SEPARATER_KEY.size(), string::npos);

        // Make the title
        str = XmStringCreateLocalized((char *)(description_.c_str()));
        XtVaCreateManagedWidget(
            "",
            xmLabelGadgetClass, parent,
            XmNlabelString, str,
            NULL);
        XmStringFree(str);

        // Create the frame widget
        w = XtVaCreateWidget(
            "",
            xmFormWidgetClass, parent,
            XmNwidth, TYPEINFO_DEFAULT_WIDTH,
            XmNleftAttachment,	XmATTACH_FORM,
            XmNrightAttachment,	XmATTACH_FORM,
            NULL);
               
        // Create the Min text widget
        wMin_ = XtVaCreateManagedWidget(
            "", xmTextWidgetClass, w,
            XmNvalue, minValue.c_str(),
            XmNrightAttachment, XmATTACH_FORM,
            XmNtopAttachment, XmATTACH_FORM,
            XmNwidth, TYPEINFO_DEFAULT_WIDTH*3/4,
            NULL);

        // Create the Max text widget
        wMax_ = XtVaCreateManagedWidget(
            "", xmTextWidgetClass, w,
            XmNvalue,   maxValue.c_str(),
            XmNtopAttachment, XmATTACH_WIDGET,
            XmNtopWidget, wMin_,
            XmNrightAttachment, XmATTACH_FORM,
            XmNbottomAttachment, XmATTACH_FORM,
            XmNwidth, TYPEINFO_DEFAULT_WIDTH*3/4,
            NULL);

        minLabel = XtVaCreateManagedWidget(
            MIN_LABEL_.c_str(),
            xmLabelGadgetClass, w,
            XmNtopAttachment, XmATTACH_FORM,
            XmNrightAttachment, XmATTACH_WIDGET,
            XmNrightWidget, wMin_,
            XmNleftAttachment, XmATTACH_FORM,
            XmNtopOffset, 10,
            NULL);

        maxLabel = XtVaCreateManagedWidget(
            MAX_LABEL_.c_str(),
            xmLabelGadgetClass, w,
            XmNtopWidget, minLabel,
            XmNtopAttachment, XmATTACH_WIDGET,
            XmNbottomAttachment, XmATTACH_FORM,
            XmNrightAttachment, XmATTACH_WIDGET,
            XmNrightWidget, wMax_,
            XmNleftAttachment, XmATTACH_FORM,
            XmNtopOffset, 10,
            NULL);

        XtManageChild(w);
               
        return w;
    }

    Widget createDisplayWidgetFormatText_(Widget parent, Symbol *parm) {

        Widget w, minLabel, maxLabel;
        XmString str;
        string value, minValue, maxValue;
        string::size_type index;
        char *label = "";
        const string SEPARATER_KEY = "\",\"";
        const string SEPARATER_KEY2 = "\", \"";

        if(parm->name)
        {
            if(parm->name[0] == '%')
            {
                label = &parm->name[1];
            }
            else
            {
                label = parm->name;
            }
        }

        // Get the value
        if(parm->input_generator && parm->input_generator->name)
        {
            value = parm->input_generator->name;
        }
        else
        {
            value = "\"\",\"\"";
        }

        index = value.find(SEPARATER_KEY);

        if (index == string::npos)
        {
            // Try an alternative key.
            index = value.find(SEPARATER_KEY2);

            if (index == string::npos)
            {
                fprintf(stderr, "Error (MinMax.hpp): No separator found.\n");
                return NULL;
            }
        }

        // Extract the values.
        minValue = value.substr(0, index);
        maxValue = value.substr(index+SEPARATER_KEY.size(), string::npos);

        // Remove '"'.
        minValue = removeCharInString(minValue, '"');
        maxValue = removeCharInString(maxValue, '"');

        // Make the title
        str = XmStringCreateLocalized((char *)(description_.c_str()));
        XtVaCreateManagedWidget(
            "",
            xmLabelGadgetClass, parent,
            XmNlabelString, str,
            NULL);
        XmStringFree(str);

        // Create the frame widget
        w = XtVaCreateWidget(
            "",
            xmFormWidgetClass, parent,
            XmNwidth, TYPEINFO_DEFAULT_WIDTH,
            XmNleftAttachment,	XmATTACH_FORM,
            XmNrightAttachment,	XmATTACH_FORM,
            NULL);
               
        // Create the Min text widget
        wMin_ = XtVaCreateManagedWidget(
            "", xmTextWidgetClass, w,
            XmNvalue, minValue.c_str(),
            XmNrightAttachment, XmATTACH_FORM,
            XmNtopAttachment, XmATTACH_FORM,
            XmNwidth, TYPEINFO_DEFAULT_WIDTH*3/4,
            NULL);

        // Create the Max text widget
        wMax_ = XtVaCreateManagedWidget(
            "", xmTextWidgetClass, w,
            XmNvalue,   maxValue.c_str(),
            XmNtopAttachment, XmATTACH_WIDGET,
            XmNtopWidget, wMin_,
            XmNrightAttachment, XmATTACH_FORM,
            XmNbottomAttachment, XmATTACH_FORM,
            XmNwidth, TYPEINFO_DEFAULT_WIDTH*3/4,
            NULL);

        minLabel = XtVaCreateManagedWidget(
            MIN_LABEL_.c_str(),
            xmLabelGadgetClass, w,
            XmNtopAttachment, XmATTACH_FORM,
            XmNrightAttachment, XmATTACH_WIDGET,
            XmNrightWidget, wMin_,
            XmNleftAttachment, XmATTACH_FORM,
            XmNtopOffset, 10,
            NULL);

        maxLabel = XtVaCreateManagedWidget(
            MAX_LABEL_.c_str(),
            xmLabelGadgetClass, w,
            XmNtopWidget, minLabel,
            XmNtopAttachment, XmATTACH_WIDGET,
            XmNbottomAttachment, XmATTACH_FORM,
            XmNrightAttachment, XmATTACH_WIDGET,
            XmNrightWidget, wMax_,
            XmNleftAttachment, XmATTACH_FORM,
            XmNtopOffset, 10,
            NULL);

        XtManageChild(w);
               
        return w;
    }

    void updateFromDisplayWidgetFormatNumber_(Widget w, char **value) {

        char buf[2048];
        string minValue;
        string maxValue;
        char *minVal = NULL;
        char *maxVal = NULL;

        buf[0] = '\0';

        XtVaGetValues(
            wMin_,
            XmNvalue, &minVal,
            NULL);

        XtVaGetValues(
            wMax_,
            XmNvalue, &maxVal,
            NULL);

        minValue = minVal;
        maxValue = maxVal;

        // Get rid of '\n' and white spaces.
        minValue = removeCharInString(minValue, '\n');
        minValue = removeCharInString(minValue, ' ');
        maxValue = removeCharInString(maxValue, '\n');
        maxValue = removeCharInString(maxValue, ' ');

        sprintf(buf,"%s,%s", minValue.c_str(), maxValue.c_str());
        *value = strdup(buf);
    }

    void updateFromDisplayWidgetFormatText_(Widget w, char **value) {

        char buf[2048];
        char *minValue = NULL;
        char *maxValue = NULL;

        buf[0] = '\0';

        XtVaGetValues(
            wMin_,
            XmNvalue, &minValue,
            NULL);

        XtVaGetValues(
            wMax_,
            XmNvalue, &maxValue,
            NULL);

        sprintf(buf,"\"%s\",\"%s\"", minValue, maxValue);
        *value = strdup(buf);
    }

public:

    MinMaxType(const char *TypeName, char *description, int inputFormat) : TypeInfo(TypeName) {

        wMin_ = NULL;
        wMax_ = NULL;
        description_ = description;
        inputFormat_ = inputFormat;
    }

    ~MinMaxType() {
    }

    Widget CreateDisplayWidget(Widget parent, Symbol *parm) {

        Widget w = NULL;

        switch (inputFormat_) {

        case MINMAXTYPE_INPUT_FORMAT_NUMBER:
            w = createDisplayWidgetFormatNumber_(parent, parm);
            break;

        case MINMAXTYPE_INPUT_FORMAT_TEXT:
            w = createDisplayWidgetFormatText_(parent, parm);
            break;
        }

        return w;
    }

    //void UpdateFromDisplayWidget(Widget w, char **value) {
    void UpdateFromDisplayWidget(Widget w, Symbol *parm) {

        char *value = NULL;

        if (parm == NULL)
        {
            fprintf(
                stderr,
                "Warning: MinMaxType::UpdateFromDisplayWidget(). parm is NULL.\n");
            return;
        }

        switch (inputFormat_) {

        case MINMAXTYPE_INPUT_FORMAT_NUMBER:
            updateFromDisplayWidgetFormatNumber_(w, &value);
            break;

        case MINMAXTYPE_INPUT_FORMAT_TEXT:
            updateFromDisplayWidgetFormatText_(w, &value);
            break;
        }

        if ((parm->input_generator) != NULL)
        {
            if ((parm->input_generator->name) != NULL)
            {
                free(parm->input_generator->name);
            }

            parm->input_generator->name = value;
        }
    }

    char *PrintValue(struct Symbol *parm, const char *separator) {

        char buf[2048];
        string value;

        buf[0] = '\0';

        // Get the value
        if(parm->input_generator && parm->input_generator->name) // "*" format
        {
            value = parm->input_generator->name;

            // Get rid of '\n'.
            value = removeCharInString(value, '\n');

            sprintf(buf,"%s <%s>", description_.c_str(), value.c_str());
        }

        return strdup(buf);
    }
};

const string MinMaxType::MIN_LABEL_ = "Minimum";
const string MinMaxType::MAX_LABEL_ = "Maximum";

#endif


///////////////////////////////////////////////////////////////////////
// $Log: MinMax.hpp,v $
// Revision 1.1.1.1  2008/07/14 16:44:21  endo
// MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
//
// Revision 1.2  2006/12/05 01:47:02  endo
// cdl parser now accepts {& a b} and Symbol *pair added.
//
// Revision 1.1  2006/08/29 15:14:34  endo
// MinMaxType added.
//
// Revision 1.2  2006/08/08 17:34:07  lilia
// TwoPoints width fixed.
//
// Revision 1.1.1.1  2006/07/20 17:17:49  endo
// MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
//
// Revision 1.1.1.1  2006/07/12 13:37:59  endo
// MissionLab 7.0
//
// Revision 1.1.1.1  2005/02/06 23:00:06  endo
// AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
//
// Revision 1.8  2003/04/06 12:58:48  endo
// gcc 3.1.1
//
// Revision 1.7  2002/01/13 01:24:31  endo
// list ->GTList
//
// Revision 1.6  2000/10/16 19:28:28  endo
// Modified due to the compiler upgrade.
//
// Revision 1.5  2000/04/13 22:31:17  endo
// Checked in for Doug.
//
// Revision 1.4  1999/07/03 22:04:03  endo
// Pick-point-from-Overlay feature added.
//
// Revision 1.3  1996/10/04 21:10:59  doug
// changes to get to version 1.0c
//
// Revision 1.3  1996/10/01 13:01:32  doug
// got library writes working
//
// Revision 1.2  1996/05/14  23:17:39  doug
// added twopoints case
//
// Revision 1.1  1996/05/14  18:56:27  doug
// Initial revision
//
///////////////////////////////////////////////////////////////////////
