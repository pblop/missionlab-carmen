/**********************************************************************
 **                                                                  **
 **                         CheckBoxType.hpp                         **
 **                                                                  **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1995, Georgia Tech Research Corporation               **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **********************************************************************/

/* $Id: CheckBoxType.hpp,v 1.1.1.1 2008/07/14 16:44:21 endo Exp $ */


#ifndef CheckBoxType_HPP
#define CheckBoxType_HPP

#include <Xm/Xm.h>
#include <Xm/LabelG.h>
#include <Xm/RowColumn.h>
#include <Xm/ToggleB.h>
#include <Xm/Form.h>

#include "utilities.h"
#include "TypeInfo.hpp"

/*-----------------------------------------------------------------------*/
// Define the CheckBoxType class
class CheckBoxType : public TypeInfo
{
    char  **values;
    int    num_values;

    static void
    toggled(Widget w, bool *included)
        {
            // Toggle the boolean marking if this item is included 
            *included ^= true;
        }

public:
    CheckBoxType(const char *TypeName, char *valuelist[], int value_cnt) : 

        TypeInfo(TypeName)
        {
            num_values = value_cnt;
            if ( num_values > 0)
            {
                values = new char *[num_values];
                for(int i=0; i<num_values; i++)
                    values[i] = strdup(valuelist[i]);
            }
            else
            {
                values = NULL;
            }
        }

    ~CheckBoxType(void) {

        if ( values)
        {
            delete values;
            values = NULL;
        }
    }

    Widget CreateDisplayWidget(Widget parent, Symbol *parm) {

        Widget w, form;

        // Create name 
        XmString str = XmStringCreateLocalized("Select any of the following");

        w = XtVaCreateManagedWidget(
            "",
            xmLabelGadgetClass, parent,
            XmNlabelString, str,
            NULL);
        XmStringFree(str);

        char *label = "";
        if (parm->name)
        {
            if (parm->name[0] == '%')
            {
                label = &parm->name[1];
            }
            else
            {
                label = parm->name;
            }
        }

        // Make a list of booleans to keep track of the selections
        // and hang it off of the widget
        bool *current_state = new bool[num_values];

        // Create the frame widget
        form = XtVaCreateWidget(
            "",
            xmFormWidgetClass, parent,
            XmNwidth, TYPEINFO_DEFAULT_WIDTH,
            NULL);

        XtManageChild(form);

        w = XtVaCreateWidget(
            "", xmRowColumnWidgetClass, form,
            XtVaTypedArg, XmNtitleString, XmRString, label, strlen(label)+1,
            XmNpacking,	XmPACK_COLUMN,
            XmNnumColumns,	1,
            XmNradioBehavior, false,
            XmNradioAlwaysOne, false,
            XmNuserData, current_state,
            XmNleftAttachment, XmATTACH_FORM,
            XmNrightAttachment, XmATTACH_FORM,
            XmNtopAttachment, XmATTACH_FORM,
            XmNbottomAttachment, XmATTACH_FORM,
            NULL);

        // Default is not selected
        for(int i=0; i<num_values; i++)
        {
            current_state[i] = false;
        }

        // Get the list of currently selected items
        if (parm->input_generator && parm->input_generator->name)
        {
            char *val = parm->input_generator->name;

            // Check each of our values in turn to see if they are specified.
            for(int i=0; i<num_values; i++)
            {
                char *p = strstr(val, values[i]);
                if (p)
                {
                    // Found a match, make sure we aren't matching a prefix.
                    int len = strlen(values[i]);
                    if (p[len] == '\0' || p[len] == '|' || isspace(p[len]))
                    {
                        current_state[i] = true;
                    }
                }
            }
        }
        else
        {
            for(int i=0; i<num_values; i++)
            {
                current_state[i] = false;
            }
        }


        for(int i=0; i<num_values; i++)
        {
            Widget btn = XtVaCreateManagedWidget(
                values[i],
                xmToggleButtonWidgetClass, w, 
                XmNset,		current_state[i],
                NULL);

            XtAddCallback(
                btn, XmNvalueChangedCallback, 
                (XtCallbackProc)toggled, (XtPointer)&current_state[i]);
        }
        XtManageChild(w);
        return w;
    }

    //void UpdateFromDisplayWidget(Widget w, char **value) {
    void UpdateFromDisplayWidget(Widget w, Symbol *parm) {

        bool *current_state;

        if (parm == NULL)
        {
            fprintf(
                stderr,
                "Warning: CheckBoxType::UpdateFromDisplayWidget(). parm is NULL.\n");
            return;
        }

        XtVaGetValues(
            w, 
            XmNuserData, (int *)&current_state,
            NULL);

        char *val = NULL;

        // Add each specified value to the string (bitwise or'd)
        for(int i=0; i<num_values; i++)
        {
            if (current_state[i])
            {
                if (val)
                {
                    val = strdupcat(val," | ", values[i]);
                }
                else
                {
                    val = strdup(values[i]);
                }

            }
        }

        // An empty list is "" and not a NULL pointer
        if (val == NULL)
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

        char buf[2048];
        buf[0] = '\0';  

        // Get the list of currently selected items
        if (parm && parm->input_generator && parm->input_generator->name)
        {
            char *val = parm->input_generator->name;
            char *tok_end = val;
            char *tok_start = NULL;

            while (tok_start != tok_end)
            {
                // Find the start of the next token.
                tok_start = tok_end;
                while (*tok_start && !isalnum(*tok_start) && *tok_start != '_')
                    tok_start++;

                // Find the end of the token
                tok_end = tok_start;
                while (isalnum(*tok_end) || *tok_end == '_')
                    tok_end++;

                if (tok_start != tok_end)
                {
                    // Mark end of token so strcat will work.
                    char tmp = *tok_end;
                    *tok_end = '\0';
   
                    // If this isn't the first value, then separate them
                    if (buf[0])
                        strcat(buf, separator);

                    // Add this value
                    strcat(buf, tok_start);

                    // Repair
                    *tok_end = tmp;
                }
            }

            // If empty, then return "None" instead of leaving it blank
            if (buf[0] == '\0')
            {
                strcpy(buf, "None");
            }
        }
        else
        {
            // Didn't get a valid parameter list
            strcpy(buf,"???");
        }

        return strdup(buf);
    }
};

#endif


///////////////////////////////////////////////////////////////////////
// $Log: CheckBoxType.hpp,v $
// Revision 1.1.1.1  2008/07/14 16:44:21  endo
// MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
//
// Revision 1.3  2006/12/05 01:47:01  endo
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
// Revision 1.5  2003/04/06 12:58:48  endo
// gcc 3.1.1
//
// Revision 1.4  1996/05/05  21:45:27  doug
// fixing compile warnings
//
// Revision 1.4  1996/05/05  21:45:27  doug
// fixing compile warnings
//
// Revision 1.3  1996/04/11  04:07:16  doug
// *** empty log message ***
//
// Revision 1.2  1996/04/06  23:54:20  doug
// *** empty log message ***
//
// Revision 1.1  1996/04/06  20:34:08  doug
// Initial revision
//
///////////////////////////////////////////////////////////////////////
