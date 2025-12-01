/**********************************************************************
 **                                                                  **
 **                             glyph.cc                             **
 **                                                                  **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1995 - 2005 Georgia Tech Research Corporation         **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **********************************************************************/

/* $Id: glyph.cc,v 1.1.1.1 2008/07/14 16:44:15 endo Exp $ */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/cursorfont.h>

#include <X11/Intrinsic.h>

#include <Xm/Xm.h>
#include <Xm/RowColumn.h>
#include <Xm/MainW.h>
#include <Xm/Form.h>
#include <Xm/ToggleB.h>
#include <Xm/DrawingA.h>
#include <Xm/ArrowBG.h>
#include <Xm/ArrowB.h>
#include <Xm/Label.h>
#include <Xm/LabelG.h>
#include <Xm/Text.h>
#include <Xm/TextF.h>
#include <Xm/Separator.h>
#include <Xm/PushB.h>
#include <Xm/MessageB.h>
#include <Xm/SelectioB.h>
#include <Xm/ScrolledW.h>

#include "buildbitmaps.h"
#include "buildmenu.h"
#include "list.hpp"
#include "load_cdl.h"
#include "popups.h"
#include "design.h"
#include "help.h"
#include "write_cdl.h"
#include "configuration.hpp"
#include "globals.h"
#include "screen_object.hpp"
#include "page.hpp"
#include "slot_record.hpp"
#include "glyph.hpp"
#include "fsa.hpp"
#include "transition.hpp"
#include "circle.hpp"
#include "binding.h"
#include "utilities.h"
#include "reporterror.h"
#include "toolbar.h"
#include "edit.h" 
#include "callbacks.h"
#include "so_movement.h"


//const int ARROW_WIDTH = 20;
static const int DESC_COLS = 20;

/*-----------------------------------------------------------------------*/

glyph::glyph(Symbol * sym, bool detail, Location * location)
{
    assert(this);
    assert(sym->symbol_type == AGENT_NAME ||
           sym->symbol_type == ROBOT_NAME ||
           sym->symbol_type == BP_NAME ||
           sym->symbol_type == ACTUATOR_NAME ||
           sym->symbol_type == SENSOR_NAME ||
           sym->symbol_type == COORD_NAME ||
           sym->symbol_type == GROUP_NAME);

    body_widget_ = NULL;
    is_detailed = detail;
    has_down_button_ = false;
    down_button_ = NULL;

    location_ = location;

    sym_rec = sym;
    sym->this_screen_object = (void *) this;

    int i;
    Widget last_arrow;
    XmString str;
    String translations =
        "Shift <Btn1Down>:   da_mouse_cb(shift_click)\n\
    <Btn1Down>:         da_mouse_cb(1down)\n\
    <Btn2Down>:         da_mouse_cb(2down)\n\
    <Btn1Up>:	  	da_mouse_cb(up)\n\
    <Btn1Motion>:       da_mouse_cb(btnmove)\n\
    <Btn3Down>:   	da_mouse_cb(3down)";

    // Level 0 is no inputs
    // level 1 is an arrow
    // level 2 is a pu name
    // level 3 is a constant
    int level = 0;

    // Create a slot_info array and load the level parm.
    slot_info = build_slot_info(sym, detail, level, num_slots, num_cols);

    num_translations_ = num_cols + 3;
    default_translations_ = new XtTranslations[num_translations_];

    bool is_sel = is_selected(sym);

    //***************************** create the base glyph *********************

          _glyph_widget = XtVaCreateWidget(
              "Glyph", 
              xmFormWidgetClass, drawing_area,
              XmNx, 10,
              XmNy, 10,
              XmNforeground, gCfgEditPixels.glyphFg,
              XmNbackground, is_sel ? gCfgEditPixels.select : gCfgEditPixels.glyphBg,
              NULL);
    // Save these augmentations as part of the default list
    XtOverrideTranslations(_glyph_widget, XtParseTranslationTable(translations));

    // Get the default translations so we can restore.
    XtVaGetValues(_glyph_widget,
                  XmNtranslations, &default_translations_[0],
                  NULL);

    Widget link = NULL;

    /**********************************************************/
        // IF the glyph is linked, then show the linked label
        if( sym->users.len() > 1 )
        {
            link = XtVaCreateManagedWidget("GlyphLabel", 
                                           xmLabelGadgetClass, _glyph_widget,
                                           XmNtopAttachment, XmATTACH_FORM,
                                           XmNleftAttachment, XmATTACH_FORM,
                                           XmNlabelType, XmPIXMAP,
                                           XmNlabelPixmap, pm_linked,
                                           XmNborderWidth, 0,
                                           NULL);

            for(int j=0; j<3; j++)
            {
                link = XtVaCreateManagedWidget("GlyphLabel", 
                                               xmLabelGadgetClass, _glyph_widget,
                                               XmNtopAttachment, XmATTACH_FORM,
                                               XmNleftAttachment, XmATTACH_WIDGET,
                                               XmNleftWidget, link,
                                               XmNlabelType, XmPIXMAP,
                                               XmNlabelPixmap, pm_linked,
                                               XmNborderWidth, 0,
                                               NULL);
            }
        }

        /**********************************************************/
        /* Create the push buttons at the top of the glyph */

#if 0
        // Add down button
        {
            Pixmap pix;
            bool pix_valid = false;

            down_button_ = NULL;

            // Choose the pix map
            if (sym->is_fsa())
            {
                pix = pm_fsa;
                pix_valid = true;
                has_down_button_ = true;
            }
            else if ((sym->symbol_type == BP_NAME && sym->construction == CS_RBP) ||
                     sym->symbol_type == ROBOT_NAME ||
                     sym->symbol_type == GROUP_NAME ||
                     (!detail && has_detail()))
            {
                pix = pm_down;
                pix_valid = true;
                has_down_button_ = true;
            }
            else if (config->arch_is_AuRA() ||
                     config->arch_is_AuRA_urban() ||
                     config->arch_is_AuRA_naval())
            {
                pix = pm_text;
                pix_valid = true;
            }


            // top line of the glyph

            if (pix_valid)
            {
                // Add down button
                down_button_ = XtVaCreateManagedWidget("", 
                                                       xmPushButtonWidgetClass, _glyph_widget,
                                                       XmNtopAttachment, XmATTACH_FORM,
                                                       XmNleftAttachment, XmATTACH_FORM,
                                                       XmNlabelType, XmPIXMAP,
                                                       XmNlabelPixmap, pix,
                                                       XmNnavigationType, XmNONE,
                                                       //XmNtranslations, trans,
                                                       NULL);

                move_down_cbs *p = new move_down_cbs;

                p->style = STYLE_GLYPH;
                p->ptr = (XtPointer) sym;
                //	 XtAddCallback(down_button_, XmNactivateCallback, (XtCallbackProc) move_down_cb, (XtPointer) p);
            }
        }

        Widget button = down_button_;

        // Add bind button to binding points
        if (sym_rec->symbol_type == BP_NAME &&
            config->arch_is_bound())
        {
            button = XtVaCreateManagedWidget("", xmPushButtonWidgetClass,
                                             _glyph_widget,
                                             XmNtopAttachment, XmATTACH_FORM,
                                             XmNleftAttachment,
                                             down_button_ ? XmATTACH_WIDGET : XmATTACH_FORM,
                                             XmNleftWidget, down_button_,
                                             XmNlabelType, XmPIXMAP,
                                             XmNlabelPixmap, pm_bind,
                                             XmNnavigationType, XmNONE,
                                             NULL);

            XtAddCallback(button, XmNactivateCallback, (XtCallbackProc) bind_cb,
                          (XtPointer) sym_rec);
        }
        // Add UNbind button to sensors, actuators, and robots
        else if (sym_rec->defining_rec &&
                 (sym_rec->defining_rec->symbol_type == SENSOR_NAME ||
                  sym_rec->defining_rec->symbol_type == ACTUATOR_NAME ||
                  sym_rec->symbol_type == ROBOT_NAME))
        {
            button = XtVaCreateManagedWidget("", xmPushButtonWidgetClass,
                                             _glyph_widget,
                                             XmNtopAttachment, XmATTACH_FORM,
                                             XmNleftAttachment,
                                             down_button_ ? XmATTACH_WIDGET : XmATTACH_FORM,
                                             XmNleftWidget, down_button_,
                                             XmNlabelType, XmPIXMAP,
                                             XmNlabelPixmap, pm_unbind,
                                             XmNnavigationType, XmNONE,
                                             NULL);

            XtAddCallback(button, XmNactivateCallback, (XtCallbackProc) unbind_cb,
                          (XtPointer) this);
        }
        // Add group button to non-grouping objects
        else if (sym_rec->could_be_grouping_op())
        {
            button = XtVaCreateManagedWidget("", xmPushButtonWidgetClass,
                                             _glyph_widget,
                                             XmNtopAttachment, XmATTACH_FORM,
                                             XmNleftAttachment,
                                             down_button_ ? XmATTACH_WIDGET : XmATTACH_FORM,
                                             XmNleftWidget, down_button_,
                                             XmNlabelType, XmPIXMAP,
                                             XmNlabelPixmap, pm_group,
                                             XmNnavigationType, XmNONE,
                                             NULL);

            XtAddCallback(button, XmNactivateCallback, (XtCallbackProc) group_cb,
                          (XtPointer) sym);
        }

        /**********************************************************/
        /* Add the separator */

        Widget sep1 = NULL;
        if( button )
        {
            sep1 = XtVaCreateManagedWidget("", xmSeparatorWidgetClass, _glyph_widget,
                                           XmNtopAttachment, XmATTACH_WIDGET,
                                           XmNtopWidget, button,
                                           XmNleftAttachment, XmATTACH_FORM,
                                           XmNrightAttachment, XmATTACH_FORM,
                                           NULL);
        }
#endif

        /**********************************************************/
        // Second line of the glyph

        char *desc = sym->description;
        if( desc == NULL )
        {
            if( sym->name && sym->name[0] != '$' )
                desc = sym->name;
            else
                desc = "        ";
        }

        Widget desc_sb = XtVaCreateManagedWidget("", 
                                                 xmScrolledWindowWidgetClass, _glyph_widget,
                                                 XmNscrollingPolicy, XmAPPLICATION_DEFINED,  
                                                 XmNvisualPolicy, XmVARIABLE,
                                                 XmNscrollBarDisplayPolicy, XmSTATIC,
                                                 XmNshadowThickness, 0,
                                                 XmNleftAttachment, XmATTACH_FORM,
                                                 XmNrightAttachment, XmATTACH_FORM,
                                                 XmNtopAttachment, link ? XmATTACH_WIDGET : XmATTACH_FORM,
                                                 XmNtopAttachment, XmATTACH_FORM,
                                                 XmNtopWidget, link,
                                                 NULL);
        Widget bot_widget = desc_sb;


        // Is this a readonly or changeable object?
        if( sym->record_class != RC_USER )
        {
            // Readonly.
            name_widget_ = XtVaCreateManagedWidget("GlyphName", 
                                                   xmTextFieldWidgetClass, desc_sb,
                                                   XmNcolumns, DESC_COLS,
                                                   XmNvalue, desc,
                                                   XmNeditable, false,
                                                   XmNcursorPositionVisible, false,
                                                   XmNmarginWidth, 0,
                                                   XmNmarginHeight, 0,
                                                   NULL);

            XtAddCallback(name_widget_, XmNactivateCallback, (XtCallbackProc) XmProcessTraversal, (XtPointer) XmTRAVERSE_NEXT_TAB_GROUP);

        }
        else
        {
            // Changable
            name_widget_ = XtVaCreateManagedWidget("GlyphName", 
                                                   xmTextFieldWidgetClass, desc_sb,
                                                   XmNcolumns, DESC_COLS,
                                                   XmNvalue, desc,
                                                   NULL);

            XtAddCallback(name_widget_, XmNactivateCallback, (XtCallbackProc) XmProcessTraversal, (XtPointer) XmTRAVERSE_NEXT_TAB_GROUP);
            XtAddCallback(name_widget_, XmNactivateCallback, (XtCallbackProc)update_desc, (XtPointer)&sym->description);
            XtAddCallback(name_widget_, XmNlosingFocusCallback, (XtCallbackProc) tf_loosing_focus, (XtPointer)-1);
            XtAddCallback(name_widget_, XmNfocusCallback, (XtCallbackProc) tf_gaining_focus, (XtPointer)-1);
            XtAddCallback(name_widget_, XmNmodifyVerifyCallback, (XtCallbackProc) tf_verify, (XtPointer) TF_DESC);
        }

        // Get the default translations so we can restore.
        XtVaGetValues(name_widget_,
                      XmNtranslations, &default_translations_[1],
                      NULL);


        /**********************************************************/

        /* Add the definition field */
        if (sym->defining_rec && sym->defining_rec->name)
        {
            char *line = strdup("Instance of: ");

            line = strdupcat(line, sym->defining_rec->name);
            str = XmStringCreateLocalized(line);
            bot_widget = XtVaCreateManagedWidget("GlyphLabel", xmLabelGadgetClass,
                                                 _glyph_widget,
                                                 XmNtopAttachment, XmATTACH_WIDGET,
                                                 XmNtopWidget, desc_sb,
                                                 XmNleftAttachment, XmATTACH_FORM,
                                                 XmNlabelString, str,
                                                 NULL);

            free(line);
            XmStringFree(str);
        }
        else if (sym->symbol_type == GROUP_NAME && sym->name[0] != '$')
        {
            char *line = strdup("Instance of: ");
            line = strdupcat(line, sym->name);
            str = XmStringCreateLocalized(line);
            bot_widget = XtVaCreateManagedWidget("GlyphLabel", xmLabelGadgetClass,
                                                 _glyph_widget,
                                                 XmNtopAttachment, XmATTACH_WIDGET,
                                                 XmNtopWidget, desc_sb,
                                                 XmNleftAttachment, XmATTACH_FORM,
                                                 XmNlabelString, str,
                                                 NULL);

            free(line);
            XmStringFree(str);
        }
	   

        /**********************************************************/

        // DCM 02-05-00: Add source file to library components and warn read only.
        if (sym->record_class != RC_USER)
        {
            char *line = strdup("Read only: ");

            // Get rid of the leading path, it takes up to much room.
            const char *start = strrchr(sym->source_file, '/');
            if( start )
            {
                // Skip past the last slash.
                start++;  
            }
            else
            {
                // It is clean.
                start = (char *)sym->source_file;
            }
            line = strdupcat(line, start);
            str = XmStringCreateLocalized(line);
            bot_widget = XtVaCreateManagedWidget("GlyphLabel", xmLabelGadgetClass,
                                                 _glyph_widget,
                                                 XmNtopAttachment, XmATTACH_WIDGET,
                                                 XmNtopWidget, bot_widget,
                                                 XmNleftAttachment, XmATTACH_FORM,
                                                 XmNlabelString, str,
                                                 NULL);

            free(line);
            XmStringFree(str);
        }

        /**********************************************************/

        /* Add the kind of field */
        Symbol *rec = sym->defining_rec;
        bool skip_it = false, show_robot_info = false;
        char *line = strdup("Kind of: ");

        if (rec)
        {
            switch (rec->symbol_type)
            {
            case BP_CLASS:
                if (rec->construction == CS_IBP)
                    line = strdupcat(line, "Input Binding Point");
                else if (rec->construction == CS_OBP)
                    line = strdupcat(line, "Output Binding Point");
                else if (rec->construction == CS_RBP)
                    line = strdupcat(line, "Robot Binding Point");
                else
                    skip_it = true;
                break;

            case SENSOR_NAME:
                line = strdupcat(line, "Sensor");
                break;

            case ACTUATOR_NAME:
                line = strdupcat(line, "Actuator");
                break;

            case ROBOT_CLASS:
                line = strdupcat(line, "Robot");
                show_robot_info = true;
                break;

            default:
                skip_it = true;
                break;
            }
        }
        else if (sym->symbol_type == GROUP_NAME)
        {
            line = strdupcat(line, "Assemblage");
        }
        else
        {
            skip_it = true;
        }

        if (!skip_it)
        {
            str = XmStringCreateLocalized(line);
            bot_widget = XtVaCreateManagedWidget("GlyphLabel", xmLabelGadgetClass,
                                                 _glyph_widget,
                                                 XmNtopAttachment, XmATTACH_WIDGET,
                                                 XmNtopWidget, bot_widget,
                                                 XmNleftAttachment, XmATTACH_FORM,
                                                 XmNlabelString, str,
                                                 NULL);
            XmStringFree(str);
        }
        free(line);

        // Show the robot info (ID and Name) to the user if this flag is on.
        if (show_robot_info && rec)
        {
            char buf[1048];

            // Add Robot ID.
            sprintf(buf, "Robot ID: %d", sym->id);
            line = strdup(buf);
            str = XmStringCreateLocalized(line);
            bot_widget = XtVaCreateManagedWidget("GlyphLabel", xmLabelGadgetClass,
                                                 _glyph_widget,
                                                 XmNtopAttachment, XmATTACH_WIDGET,
                                                 XmNtopWidget, bot_widget,
                                                 XmNleftAttachment, XmATTACH_FORM,
                                                 XmNlabelString, str,
                                                 NULL);
            XmStringFree(str);
            free(line);

            // Add Robot Name.
            sprintf(buf, "Robot Name: %s", sym->name);
            line = strdup(buf);
            str = XmStringCreateLocalized(line);
            bot_widget = XtVaCreateManagedWidget("GlyphLabel", xmLabelGadgetClass,
                                                 _glyph_widget,
                                                 XmNtopAttachment, XmATTACH_WIDGET,
                                                 XmNtopWidget, bot_widget,
                                                 XmNleftAttachment, XmATTACH_FORM,
                                                 XmNlabelString, str,
                                                 NULL);
            XmStringFree(str);
            free(line);
        }

        /**********************************************************/
        /* Add the separator */

        Widget top_sep = XtVaCreateManagedWidget("",
                                                 xmSeparatorWidgetClass, _glyph_widget,
                                                 XmNtopAttachment, XmATTACH_WIDGET,
                                                 XmNtopWidget, bot_widget,
                                                 XmNleftAttachment, XmATTACH_FORM,
                                                 XmNrightAttachment, XmATTACH_FORM,
                                                 NULL);


        /**********************************************************/
        // For each column in the glyph, add a form to hold stuff.

        int frac_base = max(num_slots, 1);

        // Extra one for the potential output arrow
        body_widget_ = new Widget[num_cols+1];
        for (int col = 0; col < num_cols; col++)
        {
            body_widget_[col] = XtVaCreateWidget(
                "Glyph",
                xmFormWidgetClass, _glyph_widget,
                XmNtopAttachment, XmATTACH_WIDGET,
                XmNbottomAttachment, XmATTACH_FORM,
                XmNborderWidth, 0,
                XmNtopWidget, top_sep,
                XmNleftAttachment, col == 0 ? XmATTACH_FORM : XmATTACH_WIDGET,
                XmNleftWidget, body_widget_[col - 1],
                XmNfractionBase, frac_base,
                XmNforeground, gCfgEditPixels.glyphFg, 
                XmNbackground, is_sel ? gCfgEditPixels.select : gCfgEditPixels.glyphBg,
                NULL);

            // Save these augmentations as part of the default list
            XtOverrideTranslations(body_widget_[col], XtParseTranslationTable(translations));

            // Get the default translations so we can restore.
            XtVaGetValues(body_widget_[col],
                          XmNtranslations, &default_translations_[2+col],
                          NULL);

            /**********************************************************/

            // Create items in this column.
            for (i = 0; i < num_slots; i++)
            {
                if (col == 1 && 
                    slot_info[num_slots * col + i].valid == false &&
                    slot_info[num_slots * 0 + i].valid &&
                    slot_info[num_slots * 0 + i].list_header == false &&
                    slot_info[num_slots * 0 + i].in_list == false)
                {
                    char buf[NAME_COLS + 1];
                    buf[0] = '\0';

                    if (slot_info[num_slots * 0 + i].name)
                    {
                        strncpy(buf, slot_info[num_slots * 0 + i].name, NAME_COLS);
                        buf[NAME_COLS] = '\0';
                    }

                    // Clip the name label at the max number of cols
                    if (slot_info[num_slots * 0 + i].name && ((int) strlen(slot_info[num_slots * 0 + i].name) > NAME_COLS))
                    {
                        buf[NAME_COLS - 1] = '.';
                        buf[NAME_COLS - 2] = '.';
                    }

                    str = XmStringCreateLocalized(buf);

                    XtVaCreateManagedWidget("GlyphLabel", 
                                            xmLabelGadgetClass, body_widget_[col],
                                            XmNtopAttachment, XmATTACH_POSITION,
                                            XmNtopPosition, i,
                                            XmNbottomAttachment, XmATTACH_POSITION,
                                            XmNbottomPosition, i + 1,
                                            XmNleftAttachment, XmATTACH_FORM,
                                            XmNlabelString, str,
                                            XmNalignment, XmALIGNMENT_BEGINNING,
                                            XmNcolumns, NAME_COLS,
                                            NULL);
                    XmStringFree(str);
                }
                else if (slot_info[num_slots * col + i].valid)
                {
                    // Create the appropriate input arrow/constant box
                    if (slot_info[num_slots * col + i].is_const)
                    {
                        char **valp = slot_info[num_slots * col + i].value;
                        char *val;

                        if (valp == NULL || *valp == NULL)
                            val = strdup("       ");
                        else
                            val = *valp;

                        last_arrow = XtVaCreateManagedWidget(
                            "Constant",
                            xmTextFieldWidgetClass, body_widget_[col],
                            XmNtopAttachment, XmATTACH_POSITION,
                            XmNtopPosition, i,
                            XmNleftAttachment, XmATTACH_FORM,
                            XmNbottomAttachment, XmATTACH_POSITION,
                            XmNbottomPosition, i + 1,
                            XmNcolumns, INIT_COLS,
                            XmNvalue, val,
                            NULL);

                        // Add the call backs to change the focus
                        XtAddCallback(
                            last_arrow,
                            XmNactivateCallback,    
                            (XtCallbackProc) XmProcessTraversal,
                            (XtPointer) XmTRAVERSE_NEXT_TAB_GROUP);

                        tf_activated_cbs *cbs = new tf_activated_cbs;

                        cbs->value = valp;
                        cbs->widget = last_arrow;
                        cbs->symbol = sym;
                        cbs->type = TF_INITIALIZER;

                        XtAddCallback(
                            last_arrow,
                            XmNactivateCallback,
                            (XtCallbackProc)tf_activated,
                            (XtPointer)cbs);

                        XtAddCallback(
                            last_arrow,
                            XmNlosingFocusCallback,
                            (XtCallbackProc)tf_loosing_focus,
                            (XtPointer)i);

                        // Change the value when the focus is lost (i.e., by hitting Tab)
                        // as well.
                        XtAddCallback(
                            last_arrow,
                            XmNlosingFocusCallback,
                            (XtCallbackProc)tf_activated,
                            (XtPointer)cbs);

                        XtAddCallback(
                            last_arrow,
                            XmNfocusCallback,
                            (XtCallbackProc)tf_gaining_focus,
                            (XtPointer)i);

                        XtAddCallback(
                            last_arrow,
                            XmNmodifyVerifyCallback,
                            (XtCallbackProc)tf_verify,
                            (XtPointer)TF_INITIALIZER);

                        const_click *data = new const_click;

                        data->style = STYLE_GLYPH;
                        data->g = this;
                        data->slot = i;

                        XtAddCallback(
                            last_arrow,
                            XmNfocusCallback,
                            (XtCallbackProc)pushup_constant_cb,
                            (XtPointer) data);

                        slot_info[num_slots * col + i].input_widget = last_arrow;
                    }
                    else if (slot_info[num_slots * col + i].is_readonly)
                    {
                        char **valp = slot_info[num_slots * col + i].value;
                        char *val;

                        if (valp == NULL || *valp == NULL)
                            val = strdup("       ");
                        else
                            val = *valp;

                        last_arrow = XtVaCreateManagedWidget("Constant",
                                                             xmTextFieldWidgetClass, body_widget_[col],
                                                             XmNtopAttachment, XmATTACH_POSITION,
                                                             XmNtopPosition, i,
                                                             XmNleftAttachment, XmATTACH_FORM,
                                                             XmNbottomAttachment, XmATTACH_POSITION,
                                                             XmNbottomPosition, i + 1,
                                                             XmNcolumns, INIT_COLS,
                                                             XmNvalue, val,
                                                             XmNeditable, false,
                                                             XmNcursorPositionVisible, false,
                                                             XmNmarginWidth, 0,
                                                             XmNmarginHeight, 0,
                                                             NULL);

                        // So can push the parm up.
                        const_click *data = new const_click;
                        data->style = STYLE_GLYPH;
                        data->g = this;
                        data->slot = i;
                        XtAddCallback(last_arrow, XmNfocusCallback, (XtCallbackProc)pushup_constant_cb, (XtPointer) data);

                        slot_info[num_slots * col + i].input_widget = last_arrow;
                    }
                    else if (slot_info[num_slots * col + i].is_cmds_list)
                    {
                        // Add down button
                        /*Widget w =*/ XtVaCreateManagedWidget("", xmPushButtonWidgetClass, body_widget_[col],
                                                               XmNtopAttachment, XmATTACH_POSITION,
                                                               XmNtopPosition, i,
                                                               XmNleftAttachment, XmATTACH_FORM,
                                                               XmNbottomAttachment, XmATTACH_POSITION,
                                                               XmNbottomPosition, i + 1,
                                                               XmNlabelType, XmPIXMAP,
                                                               XmNlabelPixmap, pm_text,
                                                               XmNnavigationType, XmNONE,
                                                               NULL);

                        move_down_cbs *p = new move_down_cbs;

                        p->style = STYLE_COMMAND_LIST;
                        p->ptr = (XtPointer) slot_info[num_slots * col + i].value;
                        //	    XtAddCallback(w, XmNactivateCallback, (XtCallbackProc) move_down_cb, (XtPointer) p);
                    }
                    else if (slot_info[num_slots * col + i].is_pu)
                    {
                        str = XmStringCreateLocalized("[pushed up]  ");
                        last_arrow = XtVaCreateManagedWidget("GlyphLabel", xmLabelGadgetClass, body_widget_[col],
                                                             XmNtopAttachment, XmATTACH_POSITION,
                                                             XmNtopPosition, i,
                                                             XmNleftAttachment, XmATTACH_FORM,
                                                             XmNbottomAttachment, XmATTACH_POSITION,
                                                             XmNbottomPosition, i + 1,
                                                             XmNlabelString, str,
                                                             XmNalignment, XmALIGNMENT_BEGINNING,
                                                             XmNcolumns, NAME_COLS,
                                                             NULL);
                        XmStringFree(str);
                        slot_info[num_slots * col + i].input_widget = last_arrow;
                    }
                    else if (col == 0 && slot_info[num_slots * 0 + i].extra_slot)
                    {
                        // New button
                        XmString str = XmStringCreateLocalized("New");
                        Widget pb = XtVaCreateManagedWidget("new_button", 
                                                            xmPushButtonWidgetClass, body_widget_[col],
                                                            XmNtopAttachment, XmATTACH_POSITION,
                                                            XmNtopPosition, i,
                                                            XmNleftAttachment, XmATTACH_FORM,
                                                            XmNbottomAttachment, XmATTACH_POSITION,
                                                            XmNbottomPosition, i + 1,
                                                            XmNlabelType, XmSTRING,
                                                            XmNlabelString, str,
                                                            XmNnavigationType, XmNONE,
                                                            NULL);
                        XmStringFree(str);

                        XtAddCallback(pb, XmNactivateCallback, (XtCallbackProc)add_slot_cb, (void *)i);
                        slot_info[num_slots * col + i].input_widget = pb;
                    }
                    else if (!slot_info[num_slots * col + i].list_header)
                    {
                        // Input arrow
                        if (slot_info[num_slots * col + i].src_symbol == NULL)
                        {
                            last_arrow = XtVaCreateManagedWidget("Input", 
                                                                 xmArrowButtonWidgetClass, body_widget_[col],
                                                                 XmNtopAttachment, XmATTACH_POSITION,
                                                                 XmNtopPosition, i,
                                                                 XmNbottomAttachment, XmATTACH_POSITION,
                                                                 XmNbottomPosition, i + 1,
                                                                 XmNleftAttachment, XmATTACH_FORM,
                                                                 XmNarrowDirection, XmARROW_RIGHT,
                                                                 //			XmNwidth, ARROW_WIDTH,
                                                                 NULL);

                            XtAddCallback(last_arrow, XmNactivateCallback, (XtCallbackProc) button_click, (void *)i);

                        }
                        // Kill button for the input link
                        else
                        {
                            // Add the kill button for the input link
                            last_arrow = XtVaCreateManagedWidget("kill_button", 
                                                                 xmPushButtonWidgetClass, body_widget_[col],
                                                                 XmNtopAttachment, XmATTACH_POSITION,
                                                                 XmNtopPosition, i,
                                                                 XmNleftAttachment, XmATTACH_FORM,
                                                                 XmNbottomAttachment, XmATTACH_POSITION,
                                                                 XmNbottomPosition, i + 1,
                                                                 XmNlabelType, XmPIXMAP,
                                                                 XmNlabelPixmap, pm_kill_connection,
                                                                 XmNnavigationType, XmNONE,
                                                                 NULL);

                            XtAddCallback(last_arrow, XmNactivateCallback, 
                                          (XtCallbackProc) kill_connection_cb, (void *) i);

                        }
                        slot_info[num_slots * col + i].input_widget = last_arrow;
                    }
                }
            }

            // Now create any list separators in column 1
            for (i = 0; i < num_slots; i++)
            {
                if (slot_info[num_slots * col + i].list_header)
                {
#if 0
                    XtVaCreateManagedWidget("", xmSeparatorWidgetClass, body_widget_[col],
                                            XmNtopAttachment, XmATTACH_POSITION,
                                            XmNtopPosition, i,
                                            XmNtopOffset, 4,
                                            XmNleftAttachment, XmATTACH_FORM,
                                            XmNrightAttachment, XmATTACH_FORM,
                                            NULL);
#endif

                    char buf[NAME_COLS + 1];
                    buf[0] = '\0';

                    if (slot_info[num_slots * col + i].name)
                    {
                        strncpy(buf, slot_info[num_slots * col + i].name, NAME_COLS);
                        buf[NAME_COLS] = '\0';
                    }

                    // Clip the name label at the max number of cols
                    if (slot_info[num_slots * col + i].name && ((int) strlen(slot_info[num_slots * col + i].name) > NAME_COLS))
                    {
                        buf[NAME_COLS - 1] = '.';
                        buf[NAME_COLS - 2] = '.';
                    }
                    str = XmStringCreateLocalized(buf);

                    XtVaCreateManagedWidget("GlyphLabel", xmLabelGadgetClass, body_widget_[col],
                                            XmNtopAttachment, XmATTACH_POSITION,
                                            XmNtopPosition, i,
                                            XmNleftAttachment, XmATTACH_FORM,
                                            XmNbottomAttachment, XmATTACH_POSITION,
                                            XmNbottomPosition, i + 1,
                                            XmNlabelString, str,
                                            XmNalignment, XmALIGNMENT_BEGINNING,
                                            XmNcolumns, NAME_COLS,
                                            NULL);
                    XmStringFree(str);
                }
            }

            XtManageChild(body_widget_[col]);
        }

        // Add the output arrow, if needed
        Symbol *real_src = sym->real_source();

        if (real_src &&
            real_src->defining_rec &&
            real_src->defining_rec->data_type &&
            real_src->defining_rec->data_type != void_type)
        {
            body_widget_[num_cols] = XtVaCreateWidget(
                "Glyph",
                xmFormWidgetClass, _glyph_widget,
                XmNtopAttachment, XmATTACH_WIDGET,
                XmNbottomAttachment, XmATTACH_FORM,
                XmNrightAttachment, XmATTACH_FORM,
                XmNborderWidth, 0,
                XmNtopWidget, top_sep,
                XmNleftAttachment, num_cols == 0 ? XmATTACH_NONE : XmATTACH_WIDGET,
                XmNleftWidget, body_widget_[num_cols - 1],
                XmNfractionBase, frac_base,
                XmNforeground, gCfgEditPixels.glyphFg, 
                XmNbackground, is_sel ? gCfgEditPixels.select : gCfgEditPixels.glyphBg,
                NULL);

            // Save these augmentations as part of the default list
            XtOverrideTranslations(body_widget_[num_cols], XtParseTranslationTable(translations));

            // Get the default translations so we can restore.
            XtVaGetValues(body_widget_[num_cols],
                          XmNtranslations, &default_translations_[2+num_cols],
                          NULL);

            /* output */
            Widget output_arrow = XtVaCreateManagedWidget("output", 
                                                          xmArrowButtonWidgetClass, body_widget_[num_cols],
                                                          XmNtopAttachment, XmATTACH_POSITION,
                                                          XmNtopPosition, num_slots / 2,
                                                          XmNbottomAttachment, XmATTACH_POSITION,
                                                          XmNbottomPosition, num_slots / 2 + 1,
                                                          XmNrightAttachment, XmATTACH_FORM,
                                                          XmNarrowDirection, XmARROW_RIGHT,
                                                          NULL);

            XtAddCallback(output_arrow, XmNactivateCallback, (XtCallbackProc) button_click, (void *)-1);

            output_widget = output_arrow;
            XtManageChild(body_widget_[num_cols]);
        }
        else
        {
            output_widget = NULL;
        }

        XtManageChild(_glyph_widget);
        XtUnmanageChild(_glyph_widget);
        managed_ = false;
        lifted_ = false;

        // Attach the new glyph to the page
        add_obj(this);
}

//********************************************************************

void 
glyph::draw()
{
   if( !lifted_ )
   {
      XtManageChild(_glyph_widget);
      managed_ = true;
   }
}

//********************************************************************

void 
glyph::erase()
{
   XtUnmanageChild(_glyph_widget);
   managed_ = false;
}

/*-----------------------------------------------------------------------*/

void
glyph::size(Dimension * height, Dimension * width) const
{
   /* find out how big it is */
   XtVaGetValues(_glyph_widget,
		 XmNheight, height,
		 XmNwidth, width,
		 NULL);
}

//-----------------------------------------------------------------------
   // copy constructor 
glyph::glyph(const glyph & a)
{
   assert(this);

}

//-----------------------------------------------------------------------
   // assignment: cleanup and copy
glyph &
glyph::operator = (const glyph & a)
{
   assert(this);

   if (this != &a)
   {
      // Delete cur rec

      // copy from a
   }
   return *this;
}

//-----------------------------------------------------------------------

void
  glyph::
update()
{
   Position x, y;

   // Read X,Y locations of glyph back in case changed
   XtVaGetValues(_glyph_widget,
		 XmNx, &x,
		 XmNy, &y,
		 NULL);
   location_->x = x;
   location_->y = y;
}

//-----------------------------------------------------------------------
   // destructor
glyph::~glyph()
{
   assert(this);

   // No longer a valid screen object
   remove_obj(this);

   // Delete any allocated mem.
   if (num_slots > 0)
      delete [] slot_info;

   if (body_widget_)
      delete [] body_widget_;

   // Delete the widgets
   XtDestroyWidget(_glyph_widget);

   if (sym_rec != NULL)
      sym_rec->this_screen_object = NULL;
}

//-----------------------------------------------------------------------

/* draw links between arrows */
static void
draw_link(Widget src, Widget des, int erase)
{
   assert(src);
   assert(des);

   const int LINK_STRAIGHT = 20;

   Position x1, y1, x2, y2;
   Position base_x1, base_y1, base_x2, base_y2;
   Position form_x1, form_y1, form_x2, form_y2;
   Position width_1, height_1, width_2, height_2;
   Position border_1, border_2;
   XPoint points[4];

   //********************* src ***********************
   XtVaGetValues(src,
		 XmNx, &x1,
		 XmNy, &y1,
		 XmNwidth, &width_1,
		 XmNheight, &height_1,
		 XmNborderWidth, &border_1,
		 NULL);

   XtVaGetValues(XtParent(src),
		 XmNx, &form_x1,
		 XmNy, &form_y1,
		 NULL);

   XtVaGetValues(XtParent(XtParent(src)),
		 XmNx, &base_x1,
		 XmNy, &base_y1,
		 NULL);

   //********************* des ***********************
   XtVaGetValues(des,
		 XmNx, &x2,
		 XmNy, &y2,
		 XmNwidth, &width_2,
		 XmNheight, &height_2,
		 XmNborderWidth, &border_2,
		 NULL);

   XtVaGetValues(XtParent(des),
		 XmNx, &form_x2,
		 XmNy, &form_y2,
		 NULL);

   XtVaGetValues(XtParent(XtParent(des)),
		 XmNx, &base_x2,
		 XmNy, &base_y2,
		 NULL);

   x1 += form_x1 + base_x1 + width_1;
   y1 += form_y1 + base_y1 + height_1 / 2 - border_1;
   x2 += form_x2 + base_x2;
   y2 += form_y2 + base_y2 + height_2 / 2 - border_2;

   points[0].x = x1;
   points[0].y = y1;

   points[1].x = x1 + LINK_STRAIGHT;
   points[1].y = y1;

   points[2].x = x2 - LINK_STRAIGHT;
   points[2].y = y2;

   points[3].x = x2;
   points[3].y = y2;

   if (erase)
   {
       XDrawLines(
           XtDisplay(drawing_area),
           XtWindow(drawing_area),
           gCfgEditGCs.erase,
           points,
           4,
           CoordModeOrigin);
   }
   else
   {
       XDrawLines(
           XtDisplay(drawing_area),
           XtWindow(drawing_area),
           gCfgEditGCs.black,
           points,
           4,
           CoordModeOrigin);
   }
}

/*-----------------------------------------------------------------------*/

// draw_links: called to draw or erase links into and out of a glyph
void
glyph::draw_links(bool erase)
{
   assert(this);

   // Don't draw links if lifted
   if( !erase && is_lifted() )
      return;

   int i;
   Widget src, des;

   /* draw inputs */
   for (i = 0; i < num_slots; i++)
   {
      // find the input
      glyph *g = src_glyph(i);

      if (g != NULL)
      {
         if( erase || !g->is_lifted() )
	 {
	    src = g->output_widget;
	    des = slot_info[num_slots * 0 + i].input_widget;
   
	    draw_link(src, des, erase);
	 }
      }
   }

   // draw our outputs
   Symbol *p;
   void *cur;

   // Check each user to see if they are on this page.
   if ((cur = sym_rec->users.first(&p)) != NULL)
   {
      do
      {
	 glyph *g = (glyph *) p->this_screen_object;

	 if (g != NULL)
	 {
            if( erase || !g->is_lifted() )
	    {
	       Widget src = output_widget;
	       Widget des = g->find_dest(this);
   
	       if (des != NULL)
	          draw_link(src, des, erase);
	    }
	 }
      }
      while ((cur = sym_rec->users.next(&p, cur)) != NULL);
   }
}

//-----------------------------------------------------------------------

// Try to find the widget for the glyph
Widget
glyph::find_dest(glyph * src)
{
   assert(this);

   int i;

   /* check each input to see if it is connected from src */
   for (i = 0; i < num_slots; i++)
   {
      /* connect the input to the output */
      glyph *sglyph = src_glyph(i);

      if (sglyph != NULL && sglyph == src)
      {
	 return slot_info[num_slots * 0 + i].input_widget;
      }
   }
   return NULL;
}

//-----------------------------------------------------------------------

// Try to find the widget for the glyph
int
glyph::find_input_slot(glyph * src)
{
   /* check each input to see if it is connected from src */
   for (int i = 0; i < num_slots; i++)
   {
      glyph *sglyph = src_glyph(i);

      if (sglyph != NULL && sglyph == src)
      {
	 return i;
      }
   }
   return -1;
}

//*************************************************************************

glyph *
add_new_prim(Symbol * def, bool detail)
{
   // Create a new instance with an anonymous name
   Symbol *p = def->inst_of();

   // create the new glyph.
   glyph *g = new glyph(p, detail, &p->location);

   // If we just created an FSA then lets add a START state.
   if( p->is_fsa() )
   {
      fsa *f = new fsa(p);
      f->add_state();
      delete f;
   }

   return g;
}

//*************************************************************************
// Returns -1 if failure
int
  glyph::
arrow2slot(Widget src)
{
   for (int i = 0; i < num_slots; i++)
   {
      if (slot_info[num_slots * 0 + i].input_widget == src)
	 return i;
   }
   return -1;
}

//*************************************************************************
// Returns -1 if failure
Symbol *
  glyph::
input_parm(int slot)
     const
     {
	return slot_info[num_slots * 0 + slot].parm_rec;
     }

//*************************************************************************

// This is called on the source object to add an output connection between
// it and input number "slot_num" in object "dest".
//         false if slot wasn't able to be added

bool
glyph::add_input_slot(int slot_num)
{
   // make sure the slot is the extra one
   if ( slot_info[num_slots * 0 + slot_num].extra_slot == false )
   {
      return false;
   }

   // Need to find the header slot for the list
   bool found_it = false;
   int  hdr_slot;
   for (hdr_slot = slot_num; hdr_slot >= 0; hdr_slot--)
   {
      if (slot_info[num_slots * 0 + hdr_slot].list_header)
      {
	 found_it = true;
         break;
      }
   }
   // make sure found the header
   if ( !found_it )
   {
      return false;
   }

   // Define the new index value
   Symbol *c = new Symbol(INDEX_NAME);

   c->name = AnonymousName();
   Symbol *hdr0 = slot_info[num_slots * 0 + hdr_slot].src_symbol;
   c->index_value = hdr0->index_value;
   DefineName(c);

   for(int col=0; col<num_cols; col++)
   {
      if( slot_info[num_slots * col + hdr_slot].valid )
      {
         Symbol *hdr = slot_info[num_slots * col + hdr_slot].src_symbol;

	 // Added a new index rec
         hdr->index_value++;

         // Add this index to the parm_header's symbol table
         hdr->table.put(c->name, c);

         // Build a data record for this instance of the parameter reference
         // And attach the index record
         Symbol *inst = new Symbol;

         *inst = *hdr;
         inst->symbol_type = INPUT_NAME;
         inst->list_index = c;
	 inst->input_generator = NULL;

         if( hdr->constant )
	 {
            // Create an empty value record and hook it up.
	    Symbol *value = new Symbol(INITIALIZER);
	    value->name = strdup("");
	    inst->input_generator = value;
	 }

         // Add it to the list in the header record
         hdr->parameter_list.append(inst);
      }
   }

   return true;
}

//*************************************************************************

// This is called on the source object to add an output connection between
// it and input number "slot_num" in object "dest".
// returns true if successful

bool
glyph::add_connection(screen_object *dest, int slot_num)
{
   // Take care that if the input rec is a bound input_binding point, that
   // we connect to the binding point and not the hardware record.
   Symbol *source = sym_rec;

   if (source->symbol_type == AGENT_NAME && source->bound_to)
      source = source->bound_to;

   // Add the consumer to the source
   source->users.append(dest->get_src_sym());

   // Connect the link to the sink.
   slot_record slot_info = dest->get_slot_info(slot_num);
   slot_info.src_symbol = source;
   dest->set_slot_info(slot_num,slot_info);

   Symbol **ig = slot_info.input_generator_;
   if (ig)
      *ig = source;

   // Copy the location over so we don't jump the glyph
   Symbol *p = slot_info.parm_rec;
   if( p )
   {
      if( slot_info.in_list )
      {
         if( p->list_index )
            p->list_index->location =  sym_rec->location;
      }
      else 
      {
         p->location =  sym_rec->location;
      }
   }


   // Figure out the node defining this page
   Symbol *parent = config->this_page()->this_agent();

   // Delete us as an unconnected child, since are now a child of the 
   // sink node and remove the group as a user of this record
   parent->children.remove(source);
   source->users.remove(parent);

   // Mark drawing as changed
   config->made_change();
   draw_links();

   return true;
}

//*************************************************************************

void
glyph::remove_input_connection(int slot_num)
{
   /* erase the line */
   glyph *sglyph = src_glyph(slot_num);

   if (sglyph != NULL)
   {
      Widget src = sglyph->output_widget;
      Widget des = slot_info[num_slots * 0 + slot_num].input_widget;

      draw_link(src, des, true);
   }

   // Remove the link.
   slot_info[num_slots * 0 + slot_num].src_symbol = NULL;
   Symbol **ig = slot_info[num_slots * 0 + slot_num].input_generator_;

   if (ig && *ig)
   {
      Symbol *src = *ig;

      // Copy the location over so we don't jump the glyph
      Symbol *p = slot_info[num_slots * 0 + slot_num].parm_rec;
      if( p )
      {
         if( slot_info[num_slots * 0 + slot_num].in_list )
         {
            if( p->list_index )
               src->location = p->list_index->location;
         }
         else 
         {
            src->location = p->location;
         }
      }

      // forget who our source was.
      *ig = NULL;

      // remove the consumer from the list
      if (src->users.remove(sym_rec) == 0)
	 warn_user("Internal Error: Unable to remove user of glyph");

      // Figure out the node defining this page
      Symbol *parent = config->this_page()->this_agent();

      // Check if this was the last consumer of this node on this page
      bool was_last = true;
      void *cur;

      if ((cur = src->users.first(&p)) != NULL)
      {
	 do
	 {
	    if (config->this_page()->is_on_this_page(p))
	    {
	       was_last = false;
	       break;
	    }
	 }
	 while ((cur = src->users.next(&p, cur)) != NULL);
      }

      // need add it as an unconnected child
      if (was_last)
      {
	 // Parent node of this page uses the record
	 parent->children.append(src);
	 // Add the group as a user of this record
	 src->users.append(parent);
      }
   }

   // Mark drawing as changed
   config->made_change();
}

//*************************************************************************

void
  glyph::
replace_input_connection(int slot_num, Symbol * new_sym)
{
   // Replace the link connection
   slot_info[num_slots * 0 + slot_num].src_symbol = new_sym;
   Symbol *old_sym = slot_info[num_slots * 0 + slot_num].src_symbol;

   // remove the us as a consumer of the old list
   old_sym->users.remove(sym_rec);

   // add the new consumer to the list
   new_sym->users.append(sym_rec);

   Symbol **ig = slot_info[num_slots * 0 + slot_num].input_generator_;

   if (ig)
      *ig = new_sym;

   // Mark drawing as changed
   config->made_change();
}

//*************************************************************************
// Convert an input arrow to a constant
void
glyph::convert_link_to_constant(int slot_num)
{
   if (slot_info[num_slots * 0 + slot_num].in_list &&
       slot_info[num_slots * 0 + slot_num].extra_slot)
   {
      // Need to find the header slot for this list
      int i;
      for (i = slot_num; i > 0; i--)
         if (slot_info[num_slots * 0 + i].list_header)
            break;
      Symbol *hdr = slot_info[num_slots * 0 + i].src_symbol;

      // Define the new state
      Symbol *c = new Symbol(INDEX_NAME);

      c->name = AnonymousName();
      c->index_value = hdr->index_value;
      hdr->index_value++;
      DefineName(c);

      // Add this index to the parm_header's symbol table
      hdr->table.put(c->name, c);

      // Build a data record for this instance of the parameter reference
      // And attach the index record
      Symbol *inst = new Symbol;

      *inst = *hdr;
      inst->symbol_type = INPUT_NAME;
      inst->list_index = c;

      // Add it to the list in the header record
      hdr->parameter_list.append(inst);

      // Hook it up so the code below will find it.
      slot_info[num_slots * 0 + slot_num].input_generator_ = &inst->input_generator;
   }

   Symbol *inst = new Symbol(INITIALIZER);

   inst->name = strdup("");

   Symbol **ptr = slot_info[num_slots * 0 + slot_num].input_generator_;

   if (ptr)
      *ptr = inst;

   // Zero out the arrow's widget pointer since will now think it is a text one.
   slot_info[num_slots * 0 + slot_num].input_widget = NULL;
   slot_info[num_slots * 0 + slot_num].is_const = true;
   slot_info[num_slots * 0 + slot_num].value = &inst->name;
}

//*************************************************************************
// Convert a constant to an input arrow
// DCM 12-5-99: Support converting a constant back to a link
void
glyph::convert_constant_to_link(int slot_num)
{
   //printf("Trying to convert_constant_to_link slot=%d\n", slot_num);

   // Delete the record holding the constant's value.
   Symbol **ptr = slot_info[num_slots * 0 + slot_num].input_generator_;
   if (ptr)
	{
      delete *ptr;
		*ptr = NULL;
	}

   // We don't have a constant input any more.
   slot_info[num_slots * 0 + slot_num].input_generator_ = NULL;
   slot_info[num_slots * 0 + slot_num].is_const = false;
   slot_info[num_slots * 0 + slot_num].value = NULL;

   // Zero out the arrow's widget pointer since will now think it is a text one.
//   slot_info[num_slots * 0 + slot_num].input_widget = NULL;
}

/*-----------------------------------------------------------------------*/

// Unhook those direct children which appear on the current page.

void
glyph::unhook_leaving_visible_children()
{
   // remove our output connections
   Symbol *p;
   void *cur;
   bool looping;

   do
   {
      looping = false;
      if ((cur = sym_rec->users.first(&p)) != NULL)
      {
	 do
	 {
	    glyph *g = (glyph *) p->this_screen_object;

	    if (g != NULL)
	    {
	       int slot = g->find_input_slot(this);

	       if (slot != -1)
	       {
		  g->remove_input_connection(slot);

		  // remove_input_connection mucks with the user list,
		  // so need to restart from beginning.
		  looping = true;
		  break;
	       }
	    }
	 }
	 while ((cur = sym_rec->users.next(&p, cur)) != NULL);
      }
   }
   while (looping);

   // Leave direct children which are showing by unconnecting them.
   for(int slot=0; slot<num_slots; slot++)
   {
      if( slot_info[num_slots * 0 + slot].valid &&
	     slot_info[num_slots * 0 + slot].src_symbol &&
	     slot_info[num_slots * 0 + slot].src_symbol->this_screen_object )
      {
	    remove_input_connection(slot);
      }
   }

   // If we are in the page group, remove us
   Symbol *parent = config->this_page()->this_agent();

   // If are a bound device then need to unhook the binding point instead.
   Symbol *base_rec = sym_rec;
   if (sym_rec->is_hdw())
      base_rec = sym_rec->bound_to;

   if (parent != base_rec)
      parent->children.remove(base_rec);
}

/*-----------------------------------------------------------------------*/

// called to delete a glyph and its subtree.
// It will only delete those direct children which do not appear 
// on the current page.

// WARNING:  Also deletes this!!!!!
void
glyph::delete_tree()
{
   // Erase the input lines, the outputs get deleted by remove_input_connection
   erase_links();

   unhook_leaving_visible_children();

   // Delete the symbol subtree
   sym_rec->delete_tree();
   sym_rec = NULL;

   // Delete this glyph
   delete this;
}

/*-----------------------------------------------------------------------*/

// verify can make a copy of the current glyph
bool
glyph::verify_can_copy() const
{
   // Refuse to copy input or output binding points
   if (sym_rec->symbol_type == AGENT_NAME &&
       (sym_rec->construction == CS_IBP || sym_rec->construction == CS_OBP))
   {
      warn_user("Input and output binding points can only be LINKed");
      return false;
   }

   return true;
}

/*-----------------------------------------------------------------------*/

// verify can delete the current glyph
bool
glyph::verify_can_delete() const
{
   return true;
}

/*-----------------------------------------------------------------------*/

// verify can link the current glyph
bool
glyph::verify_can_link() const
{
   return true;
}

/*-----------------------------------------------------------------------*/

// make a copy of the current glyph
screen_object *
glyph::dup(bool entire_tree)
{
   // Refuse to copy input or output binding points
   if (sym_rec->symbol_type == AGENT_NAME &&
       (sym_rec->construction == CS_IBP || sym_rec->construction == CS_OBP))
   {
      warn_user("Input and output binding points can only be LINKed");
      return NULL;
   }

   glyph *new_g = NULL;

   // If is an instance of a robot binding point, dup the binding
   // point and will get a new bound agent too.
   if (sym_rec->bound_to && sym_rec->symbol_type == ROBOT_NAME)
   {
      // Get a duplicate of the defining binding point
      Symbol *cur;
      if( entire_tree )
         cur = sym_rec->bound_to->dup_tree(false);
      else
         cur = sym_rec->bound_to->dup();

      // Make a new glyph from the binding point
      new_g = new glyph(cur->bound_to, false, &cur->bound_to->location);
   }
   else
   {
      // Get a duplicate of the selected symbol
      Symbol *cur;
      if( entire_tree )
         cur = sym_rec->dup_tree(false);
      else
         cur = sym_rec->dup();

      // Make a new glyph from it
      new_g = new glyph(cur, false, &cur->location);
   }

   return new_g;
}

/*-----------------------------------------------------------------------*/
// called to redraw a glyph as normal
void glyph::redraw_unselected(void)
{
    XtVaSetValues(
        _glyph_widget,
        XmNbackground, gCfgEditPixels.glyphBg, 
        NULL);
 
    for (int col = 0; col < num_cols; col++)
    {
        XtVaSetValues(
            body_widget_[col],
            XmNbackground, gCfgEditPixels.glyphBg, 
            NULL);
    }

    // If has an output arrow, then there is one more form to update
    if (has_output_arrow())
    {
        XtVaSetValues(
            body_widget_[num_cols],
            XmNbackground, gCfgEditPixels.glyphBg, 
            NULL);
    }
}

/*-----------------------------------------------------------------------*/
// called to redraw a glyph as selected
void glyph::redraw_selected(void)
{
    XtVaSetValues(
        _glyph_widget,
        XmNbackground, gCfgEditPixels.select, 
        NULL);
 
    for (int col = 0; col < num_cols; col++)
    {
        XtVaSetValues(
            body_widget_[col],
            XmNbackground, gCfgEditPixels.select, 
            NULL);
    }

    // If has an output arrow, then there is one more form to update
    if (has_output_arrow())
    {
        XtVaSetValues(
            body_widget_[num_cols],
            XmNbackground, gCfgEditPixels.select, 
            NULL);
    }
}


/*-----------------------------------------------------------------------*/

void 
glyph::xor_outline(Position x, Position y)
{
    Dimension height, width;
    size(&height, &width);

    XPoint points[5] = {{x, y}, {width, 0}, {0, height}, {-width, 0}, {0, -height}};

    XDrawLines(
        XtDisplay(drawing_area),
        XtWindow(drawing_area),
        gCfgEditGCs.XOR,
        points, 
        5, 
        CoordModePrevious);
}

//-----------------------------------------------------------------------

void
glyph::get_xy(Position *x, Position *y) const
{
   // Read X,Y locations of glyph 
   XtVaGetValues(_glyph_widget,
		 XmNx, x,
		 XmNy, y,
		 NULL);
}

//-----------------------------------------------------------------------

void
glyph::get_output_xy(Position *x_out, Position *y_out) const
{
   // Read X,Y locations of glyph 
   Position x, y;
   XtVaGetValues(_glyph_widget,
		 XmNx, &x,
		 XmNy, &y,
		 NULL);

   Dimension height, width;
   size(&height, &width);

   *x_out = x + width;
   *y_out = y + height/2;
}

//-----------------------------------------------------------------------

void
glyph::set_xy(Position x, Position y)
{
   // Set X,Y locations of glyph 
   XtVaSetValues(_glyph_widget,
		 XmNx, x,
		 XmNy, y,
		 NULL);

   if( location_ )
   {
      location_->x = x;
      location_->y = y;
   }
}

//-----------------------------------------------------------------------

// check if there are any hidden children to show
// Returns 1 if there is a subtree
//         2 if there is an implementation file
//         0 if there is nothing to show.
int glyph::has_subtree(void) const
{ 
    if (sym_rec->is_fsa())
    {
        return 1;
    }

    if ((sym_rec->symbol_type == BP_NAME && sym_rec->construction == CS_RBP) ||
        sym_rec->symbol_type == ROBOT_NAME ||
        sym_rec->symbol_type == GROUP_NAME ||
        (!is_detailed && has_detail()))
    {
        return 1;
    }

    if (config->arch_is_AuRA() ||
        config->arch_is_AuRA_urban() ||
        config->arch_is_AuRA_naval())
    {
        return 2;
    }

    return 0;
}

//********************************************************************
void
glyph::handle_events()
{
   // restore the default translations.
   XtVaSetValues(_glyph_widget, XmNtranslations, default_translations_[0],NULL);
   XtVaSetValues(name_widget_, XmNtranslations, default_translations_[1],NULL);

   for (int i = 0; i < num_cols; i++)
      XtVaSetValues(body_widget_[i], XmNtranslations, default_translations_[2+i],NULL);

   if( output_widget != NULL )
      XtVaSetValues(body_widget_[num_cols], XmNtranslations, default_translations_[2+num_cols],NULL);


   // make the "new" and "kill" buttons sensitive
   for (int i = 0; i < num_slots; i++)
   {
      if( slot_info[num_slots * 0 + i].valid &&
          slot_info[num_slots * 0 + i].extra_slot)
      {
         XtSetSensitive(slot_info[num_slots * 0 + i].input_widget,true);
      }
   }

}  
  
//********************************************************************
void
glyph::bypass_events()
{  
   // Set the translations to pass mouse events to the state machine.
   XtVaSetValues(_glyph_widget, XmNtranslations, mouse_translations, NULL);
   XtVaSetValues(name_widget_, XmNtranslations, mouse_translations, NULL);
   for (int i = 0; i < num_cols; i++)
      XtVaSetValues(body_widget_[i], XmNtranslations, mouse_translations,NULL);

   if( output_widget != NULL )
      XtVaSetValues(body_widget_[num_cols], XmNtranslations,mouse_translations,NULL);



   // make new buttons unsensitive
   for (int i = 0; i < num_slots; i++)
   {
      if( slot_info[num_slots * 0 + i].valid &&
          slot_info[num_slots * 0 + i].extra_slot)
      {
         XtSetSensitive(slot_info[num_slots * 0 + i].input_widget,false);
      }
   }
}  

//********************************************************************

bool
glyph::cursor_hit(Position x, Position y) const
{        
   Dimension height, width;
   Position  cur_x, cur_y;
   size(&height, &width);
   get_xy(&cur_x, &cur_y);

   return ( !lifted_ && 
	    x > cur_x && x < cur_x + width && 
	    y > cur_y && y < cur_y + height);
}

//********************************************************************
void 
glyph::set_subtree(Symbol *new_subtree)
{
   // Figure out the node defining this page
   Symbol *parent = config->this_page()->this_agent();

   // unconnect the old subtree from our parent.
   if( parent->children.remove(sym_rec) )
	{
      // Got him.
      sym_rec->users.remove(parent);

      // connect the new subtree to our parent.
	   parent->children.append(new_subtree);
	   new_subtree->users.append(parent);
   }
   else
   {
	   // Hmm, must be a connected child of one of the children.
      bool found_it = false; 

      // Step though each child
      Symbol *child;
      void *child_cur;
      if ((child_cur = parent->children.first(&child)) != NULL)
      {
         do
         {
            // Step through this child's input parms
	         Symbol *p;
	         void *p_cur;

	         if ((p_cur = child->parameter_list.first(&p)) != NULL)
	         {
	            do
	            {
	               if (p->symbol_type == PARM_HEADER)
	               {
		               Symbol *lp;
		               void *cur_lp;

		               if ((cur_lp = p->parameter_list.first(&lp)) != NULL)
		               {
		                  do
		                  {
                           if( lp->input_generator == sym_rec )
                           {
                              // Found him.
                              sym_rec->users.remove(child);
                              lp->input_generator = new_subtree;
                              new_subtree->users.append(child);
                              found_it = true;

										// Need to keep looking, in case it is hooked
										// to more than one user on this page.
                           }
		                  } while ((cur_lp = p->parameter_list.next(&lp, cur_lp)) != NULL);
		               }
	               }
	               else
	               {
                     if( p->input_generator == sym_rec )
                     {
                        // Found him.
                        sym_rec->users.remove(child);
                        p->input_generator = new_subtree;
                        new_subtree->users.append(child);
                        found_it = true;

								// Need to keep looking, in case it is hooked
								// to more than one user on this page.
                     }
	               }
	            } while((p_cur=child->parameter_list.next(&p,p_cur)) != NULL);
	         }
         } while((child_cur=parent->children.next(&child,child_cur)) != NULL);
      }

      if( !found_it )
      {
         warn_user("Internal Error: Unable to set glyph subtree");
      }
    
	}
   // The screen object is now invalid.
   sym_rec->this_screen_object = NULL;
   sym_rec = NULL;

   // Mark drawing as changed
   config->made_change();
}

//********************************************************************
// This massively mucks with the screen objects.
// A full page regen is necessary.
void 
glyph::import_object()
{
	// Import it.
	sym_rec->import_symbol();

   // Mark drawing as changed
   config->made_change();
}

///////////////////////////////////////////////////////////////////////
// $Log: glyph.cc,v $
// Revision 1.1.1.1  2008/07/14 16:44:15  endo
// MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
//
// Revision 1.2  2007/08/15 19:19:08  endo
// The velocity value etc. can be now changed by hitting the Tab key (i.e., without hitting the Enter key).
//
// Revision 1.1.1.1  2006/07/20 17:17:45  endo
// MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
//
// Revision 1.1.1.1  2006/07/12 13:37:54  endo
// MissionLab 7.0
//
// Revision 1.4  2006/07/10 21:47:39  endo
// GUI Clean-up.
//
// Revision 1.3  2005/05/18 21:14:43  endo
// AuRA.naval added.
//
// Revision 1.2  2005/02/07 22:25:25  endo
// Mods for usability-2004
//
// Revision 1.1.1.1  2005/02/06 22:59:33  endo
// AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
//
// Revision 1.82  2003/04/06 12:25:34  endo
// gcc 3.1.1
//
// Revision 1.81  2001/02/01 20:06:55  endo
// Modification to display the robot ID and name in the
// "Individual Robot" glyph was added.
//
// Revision 1.80  2000/04/13 21:43:53  endo
// This patch extends MissionLab to allow the user to import read-only
// library code.  When the user attempts to change something which is
// read-only, cfgedit pops up a dialog box and asks if it should import
// the object so it can be edited.  If OK'd, it imports the object.
//
// This fixes the problem with FSA's (and other assemblages) not being
// editable when they are loaded from the library.
//
// Revision 1.79  2000/02/23 18:18:01  endo
// Checked in for doug.
// Code changed, so that the library agents
// are readonly in the editor. This will
// keep users from making changes to parameters
// (which will be lost) and also make the
// istinction between user space agents and
// ibrary agents explicit.
// Readonly glyphs have a new text line added
// which states that the object is read only
// and lists the library it comes from and
// the description and any constant fields
// are not editable.
//
// Revision 1.78  2000/01/20 03:35:55  endo
// *** empty log message ***
//
// Revision 1.77  2000/01/20 03:20:48  endo
// Code checked in for doug. A feature for
// cfgedit to allow toggling constants to
// input allows was added.
//
// Revision 1.76  1999/12/16 21:33:09  endo
// rh-6.0 porting.
//
// Revision 1.75  1999/09/06 17:25:32  endo
// In glyph::has_subtree(), "else if (config->
// arch_is_AuRA_urban())" added.
//
// Revision 1.74  1999/09/03 19:48:59  endo
// else if (config->arch_is_AuRA_urban()) stuff added.
//
// Revision 1.73  1997/02/14 16:53:35  zchen
// *** empty log message ***
//
// Revision 1.72  1996/10/04  20:58:17  doug
// changes to get to version 1.0c
//
// Revision 1.73  1996/10/02 21:45:20  doug
// working on pushup in states and transitions
//
// Revision 1.72  1996/10/01 13:00:11  doug
// went to version 1.0c
//
// Revision 1.71  1996/06/02 20:27:40  doug
// Fixed pushed up parms as weights in a list
//
// Revision 1.70  1996/05/07  19:17:03  doug
// fixing compile warnings
//
// Revision 1.69  1996/03/09  01:09:23  doug
// *** empty log message ***
//
// Revision 1.68  1996/02/26  05:01:33  doug
// *** empty log message ***
//
// Revision 1.67  1996/02/25  01:13:14  doug
// *** empty log message ***
//
// Revision 1.66  1996/02/22  00:55:17  doug
// adding popups to fsa states/trans
//
// Revision 1.65  1996/02/19  21:57:05  doug
// library components and permissions now work
//
// Revision 1.64  1996/02/18  00:03:13  doug
// binding is working better
//
// Revision 1.63  1996/02/17  17:05:53  doug
// *** empty log message ***
//
// Revision 1.62  1996/02/16  00:07:18  doug
// *** empty log message ***
//
// Revision 1.61  1996/02/08  19:21:56  doug
// *** empty log message ***
//
// Revision 1.60  1996/02/06  18:16:42  doug
// *** empty log message ***
//
// Revision 1.59  1996/02/04  23:21:52  doug
// *** empty log message ***
//
// Revision 1.58  1996/02/04  17:11:48  doug
// *** empty log message ***
//
// Revision 1.57  1996/02/02  03:00:09  doug
// getting closer
//
// Revision 1.56  1996/02/01  04:04:30  doug
// *** empty log message ***
//
// Revision 1.55  1996/01/31  03:06:53  doug
// *** empty log message ***
//
// Revision 1.54  1996/01/30  01:50:19  doug
// *** empty log message ***
//
// Revision 1.53  1996/01/29  00:07:50  doug
// *** empty log message ***
//
// Revision 1.52  1996/01/27  00:09:24  doug
// added grouped lists of parameters
//
///////////////////////////////////////////////////////////////////////
