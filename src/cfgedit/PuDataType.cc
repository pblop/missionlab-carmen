/**********************************************************************
 **                                                                  **
 **                 PuDataType.cc                                    **
 **                                                                  **
 **  Functions used by more than one code generator                  **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1995, 1996, 1997 Georgia Tech Research Corporation    **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: PuDataType.cc,v 1.1.1.1 2008/07/14 16:44:15 endo Exp $ */

#include <Xm/Xm.h>

#include "load_cdl.h"
#include "popups.h"
#include "PuDataType.h"

//***********************************************************************

// Do a depth first search on the children to find the data type
// for this pushed up parameter.

Symbol *PuDataType(Symbol *rec, char *parm_name)
{
    Symbol *pair = NULL;
    bool is_advanced_parm, is_pair;

    return PuDataType(rec, parm_name, &is_advanced_parm, &is_pair, &pair);
}

Symbol *PuDataType(
    Symbol *rec,
    char *parm_name,
    bool *is_advanced_parm,
    bool *is_pair,
    Symbol **pair)
{
    Symbol *p = NULL;
    void *cur = NULL;
    char *plain_name = NULL;
    bool we_use_it = false;

    *is_advanced_parm = false;
    *is_pair = false;
    *pair = NULL;

    // Protect ourselves
    if(rec == NULL || parm_name == NULL || parm_name[0] == '\0')
    {
        return NULL;
    }

    // The pushed-up name def won't have a leading percent
    plain_name = &parm_name[1];

    // Check if we define the type
    if ((cur = rec->parameter_list.first(&p)) != NULL)
    {
        do
        {
            // See if there is a name caste here.
            if (p->symbol_type == PARM_HEADER)
            {
                Symbol *lp;
                void *cur_lp;

                if ((cur_lp = p->parameter_list.first(&lp)) != NULL)
                {
                    do
                    {
                        Symbol *ig = lp->input_generator;

                        // Is this a pushed up parm?
                        if (ig != NULL &&
                            ig != (Symbol *) UP &&
                            ig->symbol_type == PU_INITIALIZER)
                        {
                            // Yes, check if it is a name cast to our parm name
                            if(ig->name && strcmp(ig->name, parm_name) == 0)
                            {
                                // This should be it.
                                if(ig->data_type)
                                {
                                    *is_advanced_parm = ig->is_advanced_parm;
                                    *pair = ig->pair;
                                    return ig->data_type;
                                }
                                else
                                {
                                    // Hmm, well try to get a type for the new parm name
                                    return PuDataType(
                                        ig,
                                        p->name,
                                        is_advanced_parm,
                                        is_pair,
                                        pair);
                                }
                            }
                        }
                    } while ((cur_lp = p->parameter_list.next(&lp, cur_lp)) != NULL);
                }
            }
            else
            {
                Symbol *ig = p->input_generator;

                // Is this a pushed up parm?
                if (ig != NULL &&
                    ig != (Symbol *) UP &&
                    ig->symbol_type == PU_INITIALIZER)
                {
                    // Yes, check if it is a name cast to our parm name
                    if(ig->name && strcmp(ig->name, parm_name) == 0)
                    {
                        // This should be it.
                        if(ig->data_type)
                        {
                            *is_advanced_parm = ig->is_advanced_parm;
                            *pair = ig->pair;

                            return ig->data_type;
                        }
                        else
                        {
                            // Hmm, well try to get a type for the new parm name
                            return PuDataType(
                                ig,
                                p->name,
                                is_advanced_parm,
                                is_pair,    
                                pair);
                        }
                    }

                    // Check if its pair is a name cast to our parm name
                    if((ig->pair) &&
                       (ig->pair->name) &&
                       (strcmp(ig->pair->name, parm_name) == 0))
                    {
                        // This should be it.
                        if(ig->data_type)
                        {
                            *is_advanced_parm = ig->is_advanced_parm;
                            *is_pair = true;
                            *pair = ig->pair;

                            return ig->data_type;
                        }
                        else
                        {
                            // Hmm, well try to get a type for the new parm name
                            return PuDataType(
                                ig,
                                p->name,
                                is_advanced_parm,
                                is_pair,
                                pair);
                        }
                    }
                }
            }

            // No name casts to our parm.  Is there a direct reference?
            if(strcmp(p->name, parm_name) == 0 ||
                strcmp(p->name, plain_name) == 0)
            {
                // Found it, see if we define the type
                if(p->data_type)
                {
                    *is_advanced_parm = p->is_advanced_parm;
                    *is_pair = p->is_pair;
                    *pair = p->pair;
                    return p->data_type;
                }
                we_use_it = true;
                break;
            }

        } while ((cur = rec->parameter_list.next(&p, cur)) != NULL);
    }

    // If we use the parm, then recurse on each of our children and sources
    if(we_use_it)
    {
        // Check each of our children
        Symbol *node;
        void *cur;
        if ((cur = rec->children.first(&node)) != NULL)
        {
            do
            {
                // Check the child
                Symbol *dt = PuDataType(
                    node,
                    parm_name,
                    is_advanced_parm,
                    is_pair,
                    pair);

                // If it found the type, return it.
                if(dt)
                {
                    return dt;
                }

            } while ((cur = rec->children.next(&node, cur)) != NULL);
        }

        // Check each of our input sources.
        Symbol *p;
        if((cur = rec->parameter_list.first(&p)) != NULL)
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
                            // If child is a node, then spit it out
                            if (lp->input_generator != NULL &&
                                lp->input_generator != (Symbol *) UP &&
                                lp->input_generator->symbol_type != INITIALIZER &&
                                lp->input_generator->symbol_type != PU_INITIALIZER)
                            {
                                // Check the source
                                Symbol *dt = PuDataType(
                                    lp->input_generator,
                                    parm_name,
                                    is_advanced_parm,
                                    is_pair,
                                    pair);

                                // If it found the type, return it.
                                if(dt)
                                {
                                    return dt;
                                }

                            }
                        } while ((cur_lp = p->parameter_list.next(&lp, cur_lp)) != NULL);
                    }
                }
                else if (p->input_generator != NULL &&
                         p->input_generator != (Symbol *) UP &&
                         p->input_generator->symbol_type != INITIALIZER &&
                         p->input_generator->symbol_type != PU_INITIALIZER)
                {
                    // Check the source
                    Symbol *dt = PuDataType(
                        p->input_generator,
                        parm_name,
                        is_advanced_parm,
                        is_pair,
                        pair);

                    // If it found the type, return it.
                    if(dt)
                    {
                        return dt;
                    }
                }
            } while ((cur = rec->parameter_list.next(&p, cur)) != NULL);
        }

        fprintf(stderr, "PuDataType.cc: Unable to find data type for parameter: %s\n", parm_name);
        warn_userf("Internal Error: Unable to find data type for parameter: %s", parm_name);
    }

    // Rules are special.  They have a gap in the parms.
    if(rec->symbol_type == RULE_NAME)
	{
        if (rec->input_generator != NULL &&
            rec->input_generator != (Symbol *) UP &&
            rec->input_generator->symbol_type != INITIALIZER &&
            rec->input_generator->symbol_type != PU_INITIALIZER)
        {
            // Check the source
            Symbol *dt = PuDataType(
                rec->input_generator,
                parm_name,
                is_advanced_parm,
                is_pair,
                pair);

            // If it found the type, return it.
            if(dt)
            {
                return dt;
            }
        }
	}
   
    // We don't use it.
    return NULL;
}

//-----------------------------------------------------------------------
// Do a depth first search on the children to find the is_advanced_parm value
// for this pushed up parameter.
//-----------------------------------------------------------------------
bool PuIsAdvancedParmValue(Symbol *rec, char *parm_name)
{
    // Protect ourselves
    if(rec == NULL || parm_name == NULL || parm_name[0] == '\0')
    {
        return false;
    }

    // The pushed-up name def won't have a leading percent
    char *plain_name = &parm_name[1];

    // Do we even use this parameter?
    bool we_use_it = false;

    // Check if we define the type
    Symbol *p;
    void *cur;
    if ((cur = rec->parameter_list.first(&p)) != NULL)
    {
        do
        {
            // See if there is a name caste here.
            if (p->symbol_type == PARM_HEADER)
            {
                Symbol *lp;
                void *cur_lp;

                if ((cur_lp = p->parameter_list.first(&lp)) != NULL)
                {
                    do
                    {
                        Symbol *ig = lp->input_generator;

                        // Is this a pushed up parm?
                        if (ig != NULL &&
                            ig != (Symbol *) UP &&
                            ig->symbol_type == PU_INITIALIZER)
                        {
                            // Yes, check if it is a name cast to our parm name
                            if(ig->name && strcmp(ig->name, parm_name) == 0)
                            {
                                // This should be it.
                                if(ig->data_type)
                                {
                                    return (ig->is_advanced_parm);
                                }
                                else
                                {
                                    // Hmm, well try to get a type for the new parm name
                                    return PuIsAdvancedParmValue(ig,p->name);
                                }
                            }
                        }
                    } while ((cur_lp = p->parameter_list.next(&lp, cur_lp)) != NULL);
                }
            }
            else
            {
                Symbol *ig = p->input_generator;

                // Is this a pushed up parm?
                if (ig != NULL &&
                    ig != (Symbol *) UP &&
                    ig->symbol_type == PU_INITIALIZER)
                {
                    // Yes, check if it is a name cast to our parm name
                    if(ig->name && strcmp(ig->name, parm_name) == 0)
                    {
                        // This should be it.
                        if(ig->data_type)
                        {
                            return (ig->is_advanced_parm);
                        }
                        else
                        {
                            // Hmm, well try to get a type for the new parm name
                            return PuIsAdvancedParmValue(ig,p->name);
                        }
                    }
                }
            }

            // No name casts to our parm.  Is there a direct reference?
            if(strcmp(p->name, parm_name) == 0 ||
               strcmp(p->name, plain_name) == 0)
            {
                // Found it, see if we define the type
                if(p->data_type)
                {
                    return (p->is_advanced_parm);
                }

                we_use_it = true;
                break;
            }

        } while ((cur = rec->parameter_list.next(&p, cur)) != NULL);
    }

    // If we use the parm, then recurse on each of our children and sources
    if(we_use_it)
    {
        // Check each of our children
        Symbol *node;
        void *cur;
        if ((cur = rec->children.first(&node)) != NULL)
        {
            do
            {
                // Check the child
                Symbol *dt = PuDataType(node, parm_name);

                // If it found the type, return it.
                if(dt)
                {
                    return PuIsAdvancedParmValue(node, parm_name);
                }

            } while ((cur = rec->children.next(&node, cur)) != NULL);
        }

        // Check each of our input sources.
        Symbol *p;
        
        if((cur = rec->parameter_list.first(&p)) != NULL)
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
                            // If child is a node, then spit it out
                            if (lp->input_generator != NULL &&
                                lp->input_generator != (Symbol *) UP &&
                                lp->input_generator->symbol_type != INITIALIZER &&
                                lp->input_generator->symbol_type != PU_INITIALIZER)
                            {
                                // Check the source
                                Symbol *dt = PuDataType(lp->input_generator, parm_name);

                                // If it found the type, return it.
                                if(dt)
                                {
                                    return PuIsAdvancedParmValue(lp->input_generator, parm_name);
                                }
                            }
                        } while ((cur_lp = p->parameter_list.next(&lp, cur_lp)) != NULL);
                    }
                }
                else if (p->input_generator != NULL &&
                         p->input_generator != (Symbol *) UP &&
                         p->input_generator->symbol_type != INITIALIZER &&
                         p->input_generator->symbol_type != PU_INITIALIZER)
                {
                    // Check the source
                    Symbol *dt = PuDataType(p->input_generator, parm_name);

                    // If it found the type, return it.
                    if(dt)
                    {
                        return PuIsAdvancedParmValue(p->input_generator, parm_name);
                    }
                }
            } while ((cur = rec->parameter_list.next(&p, cur)) != NULL);
        }

        warn_userf("Internal Error: Unable to find data type for parameter: %s", parm_name);
    }

    // Rules are special.  They have a gap in the parms.
    if(rec->symbol_type == RULE_NAME)
	{
        if (rec->input_generator != NULL &&
            rec->input_generator != (Symbol *) UP &&
            rec->input_generator->symbol_type != INITIALIZER &&
            rec->input_generator->symbol_type != PU_INITIALIZER)
        {
            // Check the source
            Symbol *dt = PuDataType(rec->input_generator, parm_name);

            // If it found the type, return it.
            if(dt)
            {
                return PuIsAdvancedParmValue(rec->input_generator, parm_name);
            }
        }
	}
   
    // We don't use it.
    return false;
}

///////////////////////////////////////////////////////////////////////
// $Log: PuDataType.cc,v $
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
// Revision 1.1.1.1  2005/02/06 22:59:31  endo
// AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
//
// Revision 1.7  2002/01/12 23:23:16  endo
// Mission Expert functionality added.
//
// Revision 1.6  2000/04/25 07:11:38  endo
// Checked in for Doug. The parameter for the triggers
// in the sub-FSA can now pushed up to the main FSA.
//
// Revision 1.5  1997/02/14  16:53:35  zchen
// *** empty log message ***
//
// Revision 1.4  1996/05/08  15:16:39  doug
// fixing compile warnings
//
// Revision 1.3  1996/02/26  05:01:33  doug
// *** empty log message ***
//
// Revision 1.2  1996/02/25  01:13:14  doug
// *** empty log message ***
//
// Revision 1.1  1996/02/24  17:07:51  doug
// Initial revision
//
///////////////////////////////////////////////////////////////////////
