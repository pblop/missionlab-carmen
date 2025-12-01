/**********************************************************************
 **                                                                  **
 **                           TypeInBox.hpp                          **
 **                                                                  **
 **                                                                  **
 **  Written by:  Yoichiro Endo                                      **
 **                                                                  **
 **  This code is for the CfgEdit to recoginize TypeInBoxType.       **
 **  When you try to modify the state or trigger, this routine       **
 **  will bring up a widget box, so that user can type in            **
 **  text in the box.                                                **
 **                                                                  **
 **  Copyright 2000, Georgia Tech Research Corporation               **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **********************************************************************/

/* $Id: TypeInBox.hpp,v 1.1.1.1 2008/07/14 16:44:21 endo Exp $ */

#ifndef TypeInBox_HPP
#define TypeInBox_HPP

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
#include <Xm/ScrolledW.h>

#include "utilities.h"
#include "gt_std.h"
#include "TypeInfo.hpp"
#include "string_utils.h"

using std::string;

extern bool SHOW_SLIDER_VALUES;

static Widget type_in_box_widget;

typedef enum TypeInBoxTypeStyle_t {
    TYPEINBOXTYPE_INPUT_FORMAT_NUMBER,
    TYPEINBOXTYPE_INPUT_FORMAT_TEXT
};

// Define the TypeInBoxType class
class TypeInBoxType : public TypeInfo {

protected:
    Widget type_in_widget;
    char *desc; // description of the box which will be written on top of the box.
    char *head; // heading, such as "To: ", which will be added on the left of the box.
    bool scroll; // checks whether the box should be scrollable or not.

    // Showing the contents of text, such as alert message, is too much
    // for small FSA. Thus, you can hide it by setting this value to be
    // '0' (defined in cdl_code/default.AuRA.urban etc.).
    bool show_type_text_in_FSA;

    int inputFormat_;

    Widget createDisplayWidgetFormatNumber_(Widget parent, Symbol *parm) {

        XmString str;
        Widget w;
        string value;

        char *label = " ";
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
        if( parm->input_generator && parm->input_generator->name ) // "*" format
        {
            value = parm->input_generator->name;

            // Get rid of '\n' and white spaces.
            value = removeCharInString(value, '\n');
            value = removeCharInString(value, ' ');
        }      
        else
        {
            value = "0"; // Default is zero.
        }

        // Create a description of the widget
        //XmString str = XmStringCreateLocalized(desc);
        str = XmStringCreateLtoR(desc, " ");

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
            NULL);

        // Create the type-in text widget (Not scrollable)
        type_in_widget = XtVaCreateManagedWidget(
            " ",
            xmTextWidgetClass, w,
            XmNvalue, value.c_str(),
            XmNleftAttachment, XmATTACH_FORM,
            XmNrightAttachment, XmATTACH_FORM,
            XmNtopAttachment, XmATTACH_FORM,
            NULL);

        type_in_box_widget = type_in_widget;

        XtManageChild(w);
               
        return w;
    }

    Widget createDisplayWidgetFormatText_(Widget parent, Symbol *parm) {

        Widget w;

        char *label = " ";
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
        char *name;
        char *dummyname; // <-- it will be used to get ridt of '"' in the text.
        if( parm->input_generator && parm->input_generator->name ) // "*" format
        {
            dummyname = strdup(parm->input_generator->name);
            name = strchr(dummyname,'\"');
            if (name != NULL) // the 1st quotation mark found. get rid of it.
            {
                *name = '\0';
                name++;
            }
            else // the 1st quotation mark not found. return NULL for the name.
            {
                name = '\0';
            }

            if (name != NULL)
            {
                dummyname = strchr(name, '\"');
                if (dummyname != NULL) // the 2nd quotation mark found. get rid of it.
                {
                    *dummyname = '\0';
                    dummyname++;
                }
                else // the 2nd quotation mark not found. return NULL for the name.
                {
                    name = '\0';
                }
            }

            // In CDL, new-line '\n' is formatted as '\\n', since
            // CDL doesn't like '\n' charactor. Here, re-formatting it
            // back to be '\n'.
            char buf[2048];
            char buf2[4096];
            sprintf(buf,"%s", name);
            int j = 0;
            for (int i = 0; i < (int)sizeof(buf); i++)
            {
                if ((buf[i] == '\\') && (buf[i+1] == 'n'))
                {
                    buf2[j] = '\n';
                    i++;
                }
                else
                {
                    buf2[j] = buf[i];
                }

                j++;
            }

            name = strdup(buf2);
        }      
        else
        {
            name = '\0'; // no name loaded. return its name to be NULL.
        }

        // Create a description of the widget
        //XmString str = XmStringCreateLocalized(desc);
        XmString str = XmStringCreateLtoR(desc, " ");

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
            NULL);

        // Create the type-in text widget
        // Make it scrollable if it was specified.
        if (scroll)
        {
            Arg args[15];
            int n = 0;
            XtSetArg(args[n], XmNscrollVertical,             True);              n++;
            XtSetArg(args[n], XmNscrollHorizontal,           False);             n++;
            XtSetArg(args[n], XmNeditMode,                   XmMULTI_LINE_EDIT); n++;
            XtSetArg(args[n], XmNwordWrap,                   True);              n++;
            XtSetArg(args[n], XmNrows,                       5);                 n++;
            XtSetArg(args[n], XmNcolumns,                    40);                n++;
            XtSetArg(args[n], XmNvalue,                      name);              n++;
            XtSetArg(args[n], XmNleftAttachment,             XmATTACH_FORM);     n++;
            XtSetArg(args[n], XmNrightAttachment,            XmATTACH_FORM);     n++;
            XtSetArg(args[n], XmNtopAttachment,              XmATTACH_FORM);     n++;
            XtSetArg(args[n], XmNeditable,                   True);              n++;
            XtSetArg(args[n], XmNcursorPositionVisible,      True);              n++;
            type_in_widget= XmCreateScrolledText(w, " ", args, n);
            XtManageChild (type_in_widget);

        }
        else
        {
            type_in_widget = XtVaCreateManagedWidget(
                " ",
                xmTextWidgetClass, w,
                XmNvalue,   name,
                XmNleftAttachment, XmATTACH_FORM,
                XmNrightAttachment, XmATTACH_FORM,
                XmNtopAttachment, XmATTACH_FORM,
                NULL);
        }
        type_in_box_widget = type_in_widget;

        XtManageChild(w);
               
        return w;
    }

    void updateFromDisplayWidgetFormatNumber_(Widget w, char **value) {

        string nvalueString;
        char *nvalue = NULL;

        XtVaGetValues(
            type_in_widget,
            XmNvalue, &nvalue,
            NULL);

        nvalueString = nvalue;

        // Get rid of '\n' and white spaces.
        nvalueString = removeCharInString(nvalueString, '\n');
        nvalueString = removeCharInString(nvalueString, ' ');

        *value = strdup(nvalueString.c_str());
    }

    void updateFromDisplayWidgetFormatText_(Widget w, char **value) {

        char *nvalue = NULL;
        XtVaGetValues(type_in_widget,
                      XmNvalue, &nvalue,
                      NULL);

        // Trim any leading whitespace
        while( *nvalue && isspace(*nvalue) )
            nvalue++;

        // Trim any trailing whitespace
        // First, get a pointer to the end of the string, then trim.
        char *endp = nvalue;
        // now at the start of the string.
        while( *endp )
            endp++;
        // now pointing at the null terminator character
        endp--;
        // now pointing at the last character
        while( endp >= nvalue && isspace(*endp) )
            *endp-- = '\0';
        // now trimed the trailing whitespace.


        char buf[2048];
        sprintf(buf,"\"%s\"", nvalue);

        // CDL doesn't like new-line (\n) command.
        // Findingg '\n', and replace it with '\\n'.
        // Here, buf is copying into buf2.
        char buf2[4096];
        int j = 0;
        for (int i = 0; i < (int)sizeof(buf); i++)
        {
            if (buf[i] == '\n')
            {
                buf2[j] = '\\';
                buf2[j+1] = 'n';
                j++;
            }
            else
                buf2[j] = buf[i];

            j++;
        }
        *value = strdup(buf2);
    }

    char *printValueFormatNumber_(struct Symbol *parm, const char *separator) {

        char buf[2048];
        string value;

        buf[0] = '\0';

        // Showing the contents of text, such as alert message, is too much
        // for small FSA. Thus, you can hide it by setting this value to be
        // '0' (defined in cdl_code/default.AuRA.urban etc.).
        if (show_type_text_in_FSA)
        { 
            // Get the value
            if(parm->input_generator && parm->input_generator->name) // "*" format
            {
                value = parm->input_generator->name;

                // Get rid of '\n' and white spaces.
                value = removeCharInString(value, '\n');
                value = removeCharInString(value, ' ');
            }
            else
            {
                value = "0"; // zero is the default.
            }

            sprintf(buf,"%s <%s>", desc, value.c_str());
        }

        return strdup(buf);
    }

    char *printValueFormatText_(struct Symbol *parm, const char *separator) {

        char buf[2048];
        buf[0] = '\0';

        // Showing the contents of text, such as alert message, is too much
        // for small FSA. Thus, you can hide it by setting this value to be
        // '0' (defined in cdl_code/default.AuRA.urban etc.).
        if (show_type_text_in_FSA)
        { 

            // Get the value
            char *name;
            char *dummyname;
            if( parm->input_generator && parm->input_generator->name ) // "*" format
            {
                dummyname = strdup(parm->input_generator->name);
                name = strchr(dummyname,'\"');
                if (name != NULL) // the 1st quotation mark found. get rid of it.
                {
                    *name = '\0';
                    name++;
                }
                else // the 1st quotation mark not found. return NULL for the name.
                {
                    name = '\0';
                }

                if (name != NULL)
                {
                    dummyname = strchr(name, '\"');
                    if (dummyname != NULL) // the 2nd quotation mark found. get rid of it.
                    {
                        *dummyname = '\0';
                        dummyname++;
                    }
                    else // the 2nd quotation mark not found. return NULL for the name.
                    {
                        name = '\0';
                    }
                }
            }
            else
            {
                name = '\0'; // no name loaded. return its name to be NULL.
            }
            sprintf(buf,"%s", name);
        }

        return strdup(buf);

    }

public:

    TypeInBoxType(
        const char *TypeName,
        bool scrollbox,
        bool showtext,
        char *description) :
        TypeInfo(TypeName) {

        desc = description;
        scroll = (int)scrollbox;
        show_type_text_in_FSA = (int)showtext;
        type_in_widget = NULL;
        inputFormat_ = TYPEINBOXTYPE_INPUT_FORMAT_TEXT;
    }

    TypeInBoxType(
        const char *TypeName,
        bool scrollbox,
        bool showtext,
        char *description,
        int inputFormat) :
        TypeInfo(TypeName) {

        desc = description;
        scroll = (int)scrollbox;
        show_type_text_in_FSA = (int)showtext;
        type_in_widget = NULL;
        inputFormat_ = inputFormat;
    }

    ~TypeInBoxType() {
    }

    Widget CreateDisplayWidget(Widget parent, Symbol *parm) {

        Widget w = NULL;

        switch (inputFormat_) {

        case TYPEINBOXTYPE_INPUT_FORMAT_NUMBER:
            w = createDisplayWidgetFormatNumber_(parent, parm);
            break;

        case TYPEINBOXTYPE_INPUT_FORMAT_TEXT:
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
                "Warning: TypeInBoxType::UpdateFromDisplayWidget(). parm is NULL.\n");
            return;
        }

        switch (inputFormat_) {

        case TYPEINBOXTYPE_INPUT_FORMAT_NUMBER:
            updateFromDisplayWidgetFormatNumber_(w, &value);
            break;

        case TYPEINBOXTYPE_INPUT_FORMAT_TEXT:
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

        char *value = NULL;

        switch (inputFormat_) {

        case TYPEINBOXTYPE_INPUT_FORMAT_NUMBER:
            value = printValueFormatNumber_(parm, separator);
            break;

        case TYPEINBOXTYPE_INPUT_FORMAT_TEXT:
            value = printValueFormatText_(parm, separator);
            break;
        }

        return value;
    }
};

#endif


///////////////////////////////////////////////////////////////////////
    // $Log: TypeInBox.hpp,v $
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
    // Revision 1.1.1.1  2005/02/06 23:00:07  endo
    // AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
    //
    // Revision 1.13  2003/04/06 12:58:48  endo
    // gcc 3.1.1
    //
    // Revision 1.12  2000/04/16 16:21:13  endo
    // Changed, so that whether to show or not to show the contents
    // of the typed message can be specified in default.AuRA.urban,
    // etc.
    //
    // Revision 1.11  2000/04/13 22:31:46  endo
    // Cheked in for Doug.
    //
    // Revision 1.10  2000/02/18 02:42:44  endo
    // When you try to modify the state or trigger, this routine
    // will bring up a widget box, so that user can type in
    // text in the box.
    //
    //
    //
    ///////////////////////////////////////////////////////////////////////
