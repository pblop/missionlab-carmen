/**********************************************************************
 **                                                                  **
 **                         RadioButtonType.hpp                      **
 **                                                                  **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1995, Georgia Tech Research Corporation               **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **********************************************************************/

/* $Id: RadioButtonType.hpp,v 1.1.1.1 2008/07/14 16:44:21 endo Exp $ */


#ifndef RadioButtonType_HPP
#define RadioButtonType_HPP

#include <string>

#include <Xm/Xm.h>
#include <Xm/LabelG.h>
#include <Xm/RowColumn.h>
#include <Xm/ToggleB.h>
#include <Xm/Form.h>

#include "utilities.h"
#include "TypeInfo.hpp"
#include "string_utils.h"

using std::string;

/*-----------------------------------------------------------------------*/
// Define the RadioButtonType class
class RadioButtonType : public TypeInfo {

    struct toggle_info {
        int  our_value;
        int *value;
    };

protected:
    string description_;
    char  **values;
    int  num_values;

    static const string EMPTY_STRING_;
    static const string EMPTY_VALUE_;
    static const string INVALID_VALUE_;
    static const string DEFAULT_DESCRIPTION_;

    static void toggled(Widget w, toggle_info *info) {
        // Set the current value to our position
        *info->value = info->our_value;
    }

public:
    RadioButtonType(const char *TypeName, char *valuelist[], int value_cnt) : TypeInfo(TypeName) {

        num_values = value_cnt;
        if(num_values > 0)
        {
            values = new char *[num_values];

            for(int i=0; i<num_values; i++)
            {
                values[i] = strdup(valuelist[i]);
            }
        }
        else
        {
            values = NULL;
        }

        description_ = DEFAULT_DESCRIPTION_;
    }

    RadioButtonType(const char *TypeName, char *valuelist[], int value_cnt, char *description) : TypeInfo(TypeName) {

        num_values = value_cnt;
        if(num_values > 0)
        {
            values = new char *[num_values];
            for(int i=0; i<num_values; i++)
                values[i] = strdup(valuelist[i]);
        }
        else
        {
            values = NULL;
        }

        description_ = description;
    }

    ~RadioButtonType(void) { 

        if(values)
        {
            delete values;
            values = NULL;
        }
    }

    Widget CreateDisplayWidget(Widget parent, Symbol *parm) {

        toggle_info *info = NULL;
        Widget w, form, btn;
        XmString str;
        string btnLabel;
        char *label = NULL, *val = NULL;
        int *current_selection = NULL;
        int i;

        // Create name 
        str = XmStringCreateLocalized((char *)description_.c_str());

        w = XtVaCreateManagedWidget(
            "",
            xmLabelGadgetClass, parent,
            XmNlabelString, str,
            NULL);

        XmStringFree(str);

        label = "";
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

        current_selection = new int;

        // Create the frame widget
        form = XtVaCreateWidget(
            "",
            xmFormWidgetClass, parent,
            XmNwidth, TYPEINFO_DEFAULT_WIDTH,
            NULL);

        w = XtVaCreateWidget(
            "", xmRowColumnWidgetClass, form,
            XtVaTypedArg, XmNtitleString, XmRString, label, strlen(label)+1,
            XmNpacking,	XmPACK_COLUMN,
            XmNnumColumns,	1,
            XmNradioBehavior, true,
            XmNradioAlwaysOne, true,
            XmNuserData,  current_selection,
            XmNleftAttachment, XmATTACH_FORM,
            XmNrightAttachment, XmATTACH_FORM,
            XmNtopAttachment, XmATTACH_FORM,
            XmNbottomAttachment, XmATTACH_FORM,
            NULL);

        // Default is not selected
        *current_selection = -1;

        // Find out which item is currently selected
        if(parm->input_generator && parm->input_generator->name)
        {
            val = parm->input_generator->name;

            // Check each of our values in turn to see which is specified.
            for(i = 0; i<num_values; i++)
            {
                // Found a match
                if(strcmp(val, values[i]) == 0)
                {
                    *current_selection = i;
                    break;
                }
            }
        }

        for(i = 0; i < num_values; i++)
        {
            btnLabel = values[i];
            btnLabel = removeCharInString(btnLabel, '"');

            btn = XtVaCreateManagedWidget(
                btnLabel.c_str(),
                xmToggleButtonWidgetClass, w, 
                XmNset,	i == *current_selection,
                NULL);

            XtManageChild(btn);

            // Make a record to handle the toggling actions
            info = new toggle_info;
            info->our_value = i;
            info->value = current_selection;

            XtAddCallback(
                btn,
                XmNvalueChangedCallback, 
                (XtCallbackProc)toggled,
                (XtPointer)info);
        }

        XtManageChild(w);
        XtManageChild(form);

        return w;
    }

    //void UpdateFromDisplayWidget(Widget w, char **value) {
    void UpdateFromDisplayWidget(Widget w, Symbol *parm) {

        int *current_selection = NULL;
        char *val = NULL;

        if (parm == NULL)
        {
            fprintf(
                stderr,
                "Warning: RadioBoxType::UpdateFromDisplayWidget(). parm is NULL.\n");
            return;
        }

        XtVaGetValues(
            w, 
            XmNuserData, (int *)&current_selection,
            NULL);

        // Add the specified value to the string 
        if(*current_selection >= 0 && *current_selection < num_values)
        {
            val = strdup(values[*current_selection]);
        }
        else
        {
            val = strdup("");
        }

        // Return to sender
        //free(*value);
        //*value = val;

        if ((parm->input_generator) != NULL)
        {
            if ((parm->input_generator->name) != NULL)
            {
                free(parm->input_generator->name);
            }

            parm->input_generator->name = val;
        }
    }


    char *PrintValue(struct Symbol *parm, const char *separator) {

        string value = EMPTY_STRING_;

        // Get the currently selected item
        if(parm && parm->input_generator && parm->input_generator->name)
        {
            value = parm->input_generator->name;

            // Remove '"'.
            value = removeCharInString(value, '"');

            if (value == EMPTY_STRING_)
            {
                value = EMPTY_VALUE_;
            }

        }
        else
        {
            // Didn't get a valid parameter list
            value = INVALID_VALUE_;
        }

        return strdup(value.c_str());
    }
};

const string RadioButtonType::EMPTY_STRING_ = "";
const string RadioButtonType::EMPTY_VALUE_ = "None";
const string RadioButtonType::INVALID_VALUE_ = "???";
const string RadioButtonType::DEFAULT_DESCRIPTION_ = "Select one of the following";

#endif


///////////////////////////////////////////////////////////////////////
// $Log: RadioButtonType.hpp,v $
// Revision 1.1.1.1  2008/07/14 16:44:21  endo
// MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
//
// Revision 1.3  2006/12/05 01:47:02  endo
// cdl parser now accepts {& a b} and Symbol *pair added.
//
// Revision 1.2  2006/08/29 15:13:57  endo
// Advanced parameter flag added to symbol.
//
// Revision 1.1.1.1  2006/07/20 17:17:49  endo
// MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
//
// Revision 1.1.1.1  2006/07/12 13:37:59  endo
// MissionLab 7.0
//
// Revision 1.2  2006/05/14 07:38:31  endo
// gcc-3.4 upgrade
//
// Revision 1.1.1.1  2005/02/06 23:00:06  endo
// AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
//
// Revision 1.3  2003/04/06 12:58:48  endo
// gcc 3.1.1
//
// Revision 1.2  1996/05/05  21:45:27  doug
// fixing compile warnings
//
// Revision 1.2  1996/05/05  21:45:27  doug
// fixing compile warnings
//
// Revision 1.1  1996/04/08  16:04:06  doug
// Initial revision
//
//
///////////////////////////////////////////////////////////////////////
