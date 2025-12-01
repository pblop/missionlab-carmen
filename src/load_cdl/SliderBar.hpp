/**********************************************************************
 **                                                                  **
 **                         SliderBar.hpp                            **
 **                                                                  **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1995, Georgia Tech Research Corporation               **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **********************************************************************/

/* $Id: SliderBar.hpp,v 1.1.1.1 2008/07/14 16:44:21 endo Exp $ */


#ifndef SliderBar_HPP
#define SliderBar_HPP

#include <cmath>
#include <algorithm>
#include <Xm/Xm.h>
#include <Xm/LabelG.h>
#include <Xm/Scale.h>
#include <Xm/Form.h>
#include <Xm/PanedW.h>
#include <Xm/LabelG.h>

#include "utilities.h"
#include "gt_std.h"
#include "TypeInfo.hpp"

using std::max;
using std::min;

extern bool SHOW_SLIDER_VALUES;

/*-----------------------------------------------------------------------*/
// Define the SliderBar class
class SliderBar : public TypeInfo
{
    char  **values;
    int    num_values;
    double min_val;
    double max_val;
    char  *units;
    double scale;


public:
    SliderBar(const char *TypeName, 
              double min_value, double max_value, char *val_units,
              char *valuelist[], int value_cnt) : TypeInfo(TypeName) {

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
        min_val = min_value;
        max_val = max_value;
        scale   = 100.0 / (max_val - min_val);
        units = strdup(val_units);
    }

    ~SliderBar() {

        if(values)
        {
            delete values;
            values = NULL;
        }
        if(units)
        {
            delete units;
            units = NULL;
        }
    }

    Widget CreateDisplayWidget(Widget parent, Symbol *parm) {

        Widget w, slider;
        Widget label_name, label_min_val, label_max_val;

        char *label = "";
        if(parm->name)
        {
            if(parm->name[0] == '%')
                label = &parm->name[1];
            else
                label = parm->name;
        }

        // Get the value
        double dval;
        if(parm->input_generator && parm->input_generator->name)
            dval = atof(parm->input_generator->name);
        else
            dval = (max_val + min_val) / 2;
               
        // Bound it to the range
        dval = max(dval, min_val);
        dval = min(dval, max_val);

        // Make the integer values for the slider

        // A strange bug caused specific values like 1.20 (*) to
        // be decreased by 0.01 each time (**) the "parameter edit"
        // window was opened and the (not necessarily changed)
        // values saved at exit by pressing the "OK" button;
        // the problem might be a compiler bug (tested with gcc
        // versions 1.1.2, 2.96, 3.1.1):
        //      int ival = (int)(dval * 100); // original version
        // should do the same as
        //      double dival = dval * 100;
        //      int ival = (int)dival;
        // since dval is a double, but the result is still wrong,
        // now for different values. One might think the problem
        // might also be solved in one of the following ways:
        //      int ival = (int)(dval * 100.0);
        //      int ival = (int)((double)dval * (double)100.0);
        //      int ival = (int)((double)(dval * 100.0));
        // But all of these result in wrong values...
        //
        // A clean solution to the problem is to use rint()
        // to round to the nearest integer value before
        // typecasting
        //
        // (*) the values affected are values that cannot be
        //     represented exactly as floating points. But even
        //     if the result is not as expected because of that,
        //     the compiler should at least handle the different
        //     statements above equally...
        // (**) the decreasing stops once a stable value, like
        //      1.12 in the original version is reached

        // int ival = (int)(dval * 100); // doesn't work correctly
        int ival = (int)rint(dval * 100);
       
        int imax = (int)(max_val * 100);
        int imin = (int)(min_val * 100);
            
        // Create the frame widget
        w = XtVaCreateWidget(
            "",
            xmFormWidgetClass, parent,
            XmNwidth, TYPEINFO_DEFAULT_WIDTH,
            NULL);

        // Label it
        if(num_values >= 2)
        {
            label_min_val = XtVaCreateManagedWidget(
                values[0], xmLabelGadgetClass, w,
                XmNtopAttachment,         XmATTACH_FORM,
                XmNleftAttachment,        XmATTACH_FORM,
                NULL);

            label_max_val = XtVaCreateManagedWidget(
                values[num_values-1], xmLabelGadgetClass, w,
                XmNtopAttachment,         XmATTACH_FORM,
                XmNrightAttachment,       XmATTACH_FORM,
                NULL);

            label_name = XtVaCreateManagedWidget(
                label, xmLabelGadgetClass, w,
                XmNtopAttachment,         XmATTACH_FORM,
                XmNleftAttachment,        XmATTACH_FORM,
                XmNrightAttachment,       XmATTACH_FORM,
                NULL);
        }
        else
        {
            label_name = XtVaCreateManagedWidget(
                label, xmLabelGadgetClass, w,
                XmNtopAttachment,         XmATTACH_FORM,
                XmNleftAttachment,        XmATTACH_FORM,
                XmNrightAttachment,       XmATTACH_FORM,
                NULL);
        }

        slider = XtVaCreateManagedWidget(
            "",
            xmScaleWidgetClass, w,
            XmNmaximum, imax,
            XmNminimum, imin,
            XmNvalue,   ival,
            XmNorientation, XmHORIZONTAL,
            XmNprocessingDirection, XmMAX_ON_RIGHT,
            XmNshowValue, SHOW_SLIDER_VALUES,
            XmNdecimalPoints, 2,
            XmNtopAttachment,         XmATTACH_WIDGET,
            XmNtopWidget,             label_name,
            XmNleftAttachment,        XmATTACH_FORM,
            XmNrightAttachment,       XmATTACH_FORM,
            NULL);
               
        XtManageChild(w);

        return slider;
    }

    //void UpdateFromDisplayWidget(Widget w, char **value) {
    void UpdateFromDisplayWidget(Widget w, Symbol *parm) {

        char buf[256];
        int i, val = 0;

        if (parm == NULL)
        {
            fprintf(
                stderr,
                "Warning: SliderBar::UpdateFromDisplayWidget(). parm is NULL.\n");
            return;
        }

        XtVaGetValues(
            w,
            XmNvalue, &val,
            NULL);

        sprintf(buf,"%f", (double)val / 100.0);

        // If there is a decimal point, then trim any trailing zeros, 
        if(strchr(buf,'.'))
        {
            i = strlen(buf) - 1;
            while(i > 0 && buf[i] == '0' && buf[i-1] != '.')
            {
                buf[i--] = '\0';
            }
        }

        // Return to sender
        //free(*value);
        //*value = strdup(buf);

        if ((parm->input_generator) != NULL)
        {
            if ((parm->input_generator->name) != NULL)
            {
                free(parm->input_generator->name);
            }

            parm->input_generator->name = strdup(buf);
        }
    }

    char *PrintValue(struct Symbol *parm, const char *separator) {

        char *val = "Unknown";
        if(SHOW_SLIDER_VALUES)
        {
            if(parm->input_generator && parm->input_generator->name)
                val = parm->input_generator->name;
            else
                val = "???";
        }
        else if(num_values > 0)
        { 
            // Get the value
            double dval = 0;
            if(parm->input_generator && parm->input_generator->name)
                dval = atof(parm->input_generator->name);

            // Scale it to 0..1
            dval -= min_val;
            double range = max_val - min_val;
            if(range > 0)
                dval /= (max_val - min_val);
            else
                dval = 0;
   
            // Bound it to 0..1
            dval = max(dval, 0.0);
            dval = min(dval, 1.0);

            // Determine which of the names is the closest.
            // Tricky math: floor(dval * num_names) gives correct index 0..n-1
            int index = (int)(dval * num_values);

            // If value is exactly max, can get a +1 problem that this fixes.
            if(index >= num_values)
                index = num_values - 1;

            // Get the right label
            val = values[index];
        } 

        char buf[2048];
        if(parm->name[0] == '%')
            sprintf(buf,"%s %s", val, &parm->name[1]);
        else
            sprintf(buf,"%s %s", val, parm->name);
  
        return strdup(buf);
    }
};

#endif


///////////////////////////////////////////////////////////////////////
// $Log: SliderBar.hpp,v $
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
// Revision 1.3  2006/05/14 07:38:31  endo
// gcc-3.4 upgrade
//
// Revision 1.2  2005/02/07 23:56:26  endo
// Mods from usability-2004
//
// Revision 1.1.1.1  2005/02/06 23:00:06  endo
// AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
//
// Revision 1.8  2003/04/06 12:58:48  endo
// gcc 3.1.1
//
// Revision 1.7  2002/11/03 00:48:33  kaess
// strange bug fixed, which caused some parameters
// to decrease by 0.01 each time the parameter edit
// window was closed
//
// Revision 1.6  2000/05/26 19:49:25  endo
// *** empty log message ***
//
// Revision 1.5  2000/05/26 19:12:04  endo
// The GUI problem, which the slider bars show up unevenly, was
// fixed.
//
// Revision 1.4  1996/06/01 21:53:40  doug
// *** empty log message ***
//
// Revision 1.3  1996/05/05  21:45:27  doug
// fixing compile warnings
//
// Revision 1.2  1996/04/11  15:11:32  doug
// Fixed scaling problems on display
//
// Revision 1.1  1996/04/11  04:07:24  doug
// Initial revision
//
//
///////////////////////////////////////////////////////////////////////
