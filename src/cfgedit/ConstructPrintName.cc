/**********************************************************************
 **                                                                  **
 **                           ConstructPrintName                     **
 **                                                                  **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1995, 1996, 1997 Georgia Tech Research Corporation    **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **********************************************************************/

/* $Id: ConstructPrintName.cc,v 1.1.1.1 2008/07/14 16:44:15 endo Exp $ */


#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#include <Xm/Xm.h>

#include "load_cdl.h"
#include "globals.h"
#include "popups.h"
#include "PuDataType.h"
//#define WANT_OBJECT_DEFS
//#include "cfgedit_common.h"

#include "TypeInfo.hpp"

#include "ConstructPrintName.h"

/*-----------------------------------------------------------------------*/
bool ParmExtremes(Symbol *rec, Symbol *parm, char **left, char **right)  
{
    Symbol *data_type = NULL;
    Symbol *n = NULL;
    void *cur = NULL;

    if((parm == NULL) || (parm->input_generator == NULL))
    {
        return false;
    }

    data_type = PuDataType(rec, parm->name);

    if(data_type == NULL || data_type->name == NULL)
    {
        return false;
    }

    // Get the left label
    cur = data_type->parameter_list.first(&n);
    if(cur == NULL)
    {
        return false;
    }

    *left = n->name;

    while(cur)
    {
        *right = n->name;
        cur = data_type->parameter_list.next(&n,cur);
    }

    return true;
}

/*-----------------------------------------------------------------------*/
static char *ParmValue(Symbol *rec, Symbol *parm, bool *is_advanced_parm)  
{
    Symbol *data_type = NULL, *pair = NULL;
    bool isPair = false;

    if((parm == NULL) || (parm->input_generator == NULL))
    {
        return strdup("???");
    }

    data_type = PuDataType(rec, parm->name, is_advanced_parm, &isPair, &pair);

    if(data_type && data_type->TypeRecord)
    {
        return data_type->TypeRecord->PrintValue(parm, "\n  ");
    } 
    else
    {
        return strdup("???");
    }
}

/*-----------------------------------------------------------------------*/

char *
ObjectName(Symbol *rec)
{
   if(rec == NULL)
   {
      warn_user("Internal Error: Null pointer to ConstructPrintName");
      return strdup("????");
   }

   if(rec->symbol_type != GROUP_NAME ||
       rec->children.isempty())
   {  
      warn_user("Internal Error: Empty container in ConstructPrintName");
      return strdup("????");
   }

   Symbol *child = NULL;
   rec->children.first(&child);
      
   if(child && child->name && child->name[0] != '$')
      return strdup(child->name);

   return strdup("?no name?");
}

/*-----------------------------------------------------------------------*/

char *
ConstructPrintName(Symbol *rec)
{
    const int NAME_SIZE = 2048;
    bool is_advanced_parm = false;

    if(rec == NULL)
    {
        return strdup("????");
    }

    if(rec->symbol_type != GROUP_NAME ||
        rec->children.isempty())
    {  
        return strdup("????");
    }

    Symbol *child = NULL;
    rec->children.first(&child);
      
    char name[NAME_SIZE];
    if(child && child->name && child->name[0] != '$')
        strcpy(name,child->name);
    else if(child && child->description)
        strcpy(name,child->description);
    else 
        strcpy(name,"?no name?");

    void *cur;
    Symbol *p;
    if((cur = rec->parameter_list.first(&p)) != NULL)
    {
        do
        {
            if(p->name && p->name[0] == '%')
            {
                if(p->symbol_type == PARM_HEADER)
                {
                    Symbol *lp;
                    void *cur_lp;

                    if ((cur_lp = p->parameter_list.first(&lp)) != NULL)
                    {
                        do
                        {
                            // If is a constant, then we are interested
                            if (lp->input_generator &&
                                lp->input_generator != (Symbol *) UP)
                            {
                                if(lp->input_generator->symbol_type == PU_INITIALIZER)
                                {
                                    strcat(name, "\n  [");
                                    if(lp->input_generator->name)
                                        strcat(name, lp->input_generator->name);
                                    else
                                        strcat(name, p->name);
                                    strcat(name, "]");
                                }
                                else if(lp->input_generator->symbol_type == INITIALIZER)
                                {
                                    strcat(name, "\n  ");
                                    char *pv = ParmValue(rec, lp, &is_advanced_parm);
                                    strcat(name, pv);
                                    free(pv);
                                }
                            }
                        }
                        while ((cur_lp = p->parameter_list.next(&lp, cur_lp)) != NULL);
                    }
                }
                else
                {
                    if (p->input_generator &&
                        p->input_generator != (Symbol *) UP)
                    {
                        if(p->input_generator->symbol_type == PU_INITIALIZER)
                        {
                            strcat(name, "\n  [");
                            if(p->input_generator->name)
                                strcat(name, p->input_generator->name);
                            else
                                strcat(name, p->name);
                            strcat(name, "]");
                        }
                        else if (p->input_generator->symbol_type == INITIALIZER)
                        {
                            // If is a constant, then we are interested,
                            // unless that's a too much detail the user want
                            // to hide.
                            char *pv = ParmValue(rec, p, &is_advanced_parm);
                            if (strlen(pv) > 0) // Make sure, there is something to write.
                            {
                                if (hide_FSA_detail) // Hide some detail parameters
                                {
                                    char tmp[4096];
                                    int num_check;

                                    // Let's check if the first letter in the string
                                    // is a number or the minus sign. If it is, then,
                                    // this is a parameter for gain etc. which
                                    // we can hide.
                                    sprintf(tmp,"%s",pv);
                                    num_check = tmp[0] - '0';

                                    if (((num_check<0)||(10<num_check))&&(tmp[0]!='-'))
                                    {
                                        // Also, check for the is_advanced_parm flag.
                                        //if (!PuIsAdvancedParmValue(rec, p->name))
                                        if (!is_advanced_parm)
                                        {
                                            strcat(name, "\n  ");
                                            strcat(name, pv);
                                        }
                                    }
                                }
                                else // Show all parameters
                                {       
                                    strcat(name, "\n  ");
                                    strcat(name, pv);
                                }
                            }
                            free(pv);
                        }
                    }
                }
            }
        } while((cur = rec->parameter_list.next(&p,cur)) != NULL);
    }

    // Make sure didn't overflow the buffer
    assert((int)(strlen(name)) < NAME_SIZE);

    return strdup(name);
}


///////////////////////////////////////////////////////////////////////
// $Log: ConstructPrintName.cc,v $
// Revision 1.1.1.1  2008/07/14 16:44:15  endo
// MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
//
// Revision 1.3  2006/12/05 01:42:43  endo
// cdl parser now accepts {& a b}.
//
// Revision 1.2  2006/08/29 15:12:31  endo
// Advanced parameter flag added.
//
// Revision 1.1.1.1  2006/07/20 17:17:45  endo
// MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
//
// Revision 1.1.1.1  2006/07/12 13:37:54  endo
// MissionLab 7.0
//
// Revision 1.1.1.1  2005/02/06 22:59:30  endo
// AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
//
// Revision 1.17  2003/04/06 12:25:34  endo
// gcc 3.1.1
//
// Revision 1.16  2000/06/18 02:04:42  endo
// When it is in "Hide Parameter" mode, it will also check for
// "-" sign.
//
// Revision 1.15  2000/02/18 02:48:30  endo
// Hide/Show FSA Parameters function was a little
// modified, so that it only hides certain parameters,
// such as gains, rather than hide everything.
//
// Revision 1.14  1999/12/18 10:12:21  endo
// hide_FSA_detail capability added.
//
// Revision 1.13  1997/02/14  16:53:35  zchen
// *** empty log message ***
//
// Revision 1.12  1996/10/04  20:58:17  doug
// changes to get to version 1.0c
//
// Revision 1.13  1996/10/03 21:46:40  doug
// nested FSA's are working
//
// Revision 1.12  1996/10/02 21:45:20  doug
// working on pushup in states and transitions
//
// Revision 1.11  1996/05/07 19:53:20  doug
// fixing compile warnings
//
// Revision 1.10  1996/04/30  20:43:47  doug
// *** empty log message ***
//
// Revision 1.9  1996/04/06  23:54:05  doug
// *** empty log message ***
//
// Revision 1.8  1996/03/05  22:55:37  doug
// *** empty log message ***
//
// Revision 1.7  1996/03/04  22:52:12  doug
// *** empty log message ***
//
// Revision 1.6  1996/03/01  00:47:46  doug
// *** empty log message ***
//
// Revision 1.5  1996/02/29  01:48:49  doug
// *** empty log message ***
//
// Revision 1.4  1996/02/28  03:56:24  doug
// *** empty log message ***
//
// Revision 1.3  1996/02/27  05:01:12  doug
// *** empty log message ***
//
// Revision 1.2  1996/02/26  05:01:33  doug
// *** empty log message ***
//
// Revision 1.1  1996/02/25  01:14:20  doug
// Initial revision
//
//
///////////////////////////////////////////////////////////////////////
