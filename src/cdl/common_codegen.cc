/**********************************************************************
 **                                                                  **
 **                 common_codegen.cc                                **
 **                                                                  **
 **  Functions used by more than one code generator                  **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1995, Georgia Tech Research Corporation               **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: common_codegen.cc,v 1.1.1.1 2008/07/14 16:44:14 endo Exp $ */

#include <fstream>
#include <libc.h>
#include <assert.h>

#include "prototypes.h"
#include "cdl_defs.h"
#include "list.hpp"
#include "symtab.h"
#include "cdl_gram.tab.h"
#include "version.h"
#include "utilities.h"

#include "codegens.h"

SymbolList AgentStack;

bool cdl_had_error = false;

//***********************************************************************

Symbol *find_pu_data(char *parm_name, int loops)
{
    Symbol *node = NULL;
    Symbol *parm = NULL;
    Symbol *ig = NULL;
    Symbol *result = NULL;
    Symbol *best_value = NULL;
    char buf[2048];
    void *cur_parm = NULL;
    void *cur = NULL;

    if(loops > 1000)
    {
        sprintf(buf,"Loop in agent stack looking for paramter '%s' in find_pu_data", parm_name);
        error(buf);
        return NULL;
    }

    if ((cur = AgentStack.first(&node)) != NULL)
    {
        do
        {
            // Now see if the parm is defined in this node
            if ((cur_parm = node->parameter_list.first(&parm)) != NULL)
            {
                do
                {
                    if(strcmp(parm->name, parm_name) == 0 ||
                       (parm_name[0] != '%' && parm->name[0] == '%' && 
                        strcmp(&parm->name[1], parm_name) == 0))
                    {
                        ig = parm->input_generator;

                        if(ig &&
                           ig != (Symbol *)UP)
                        {
                            if(ig->symbol_type == PU_INITIALIZER)
                            {
                                // if there is a name caste and the name is
                                // changed, then use the new name
                                if(ig->name && 
                                   ig->data_type && 
                                   strcmp(ig->name, parm_name) != 0)
                                {
                                    result = find_pu_data(ig->name, loops+1);

                                    if(result)
                                    {
                                        best_value = result;
                                    }
                                }
			     
                                // otherwise, move up one more level
                                break;
                            }
                            else
                            {
                                // We found it!
                                best_value = parm;
                                break;
                            }
                        }
                        else
                        {
                            error("Empty input generator in find_pu_data");
                        }
                    }
                } while ((cur_parm = node->parameter_list.next(&parm, cur_parm)) != NULL);
            }
	  
        } while ((cur = AgentStack.next(&node, cur)) != NULL);
    }

    return best_value;
}

//********************************************************************

Symbol *real_source(const Symbol *src)
{
   if(src == NULL)
      return NULL;
 
   Symbol *p = (Symbol *)src; 
 
   // If this is a group name, skip it.  They are just containers
   if(src->symbol_type == GROUP_NAME)
   {
      if (src->children.len() != 1)
      {
         p = NULL; 
	 
         // Now see if there is only one node that isn't void output
         Symbol *node;
         void *cur;
         if ((cur = src->children.first(&node)) != NULL)
         {
            do
            {
	       if(strcmp(node->defining_rec->data_type->name,"void") != 0)
	       {
		  if(p != NULL)
		  {
                     char msg[1024];
                     sprintf(msg,"There are multiple nodes in %s with unconnected outputs",src->name);
                     error(msg); 
		     return NULL;
		  }
		  else
		  {
		     p = node;
		  }
	       }
            } while ((cur = src->children.next(&node, cur)) != NULL);
         }
      }
      else
         src->children.first(&p);

      return real_source(p);
   }

   // If this is a hardware record for a binding point, return the bp instead
   else if(src->symbol_type == AGENT_NAME && src->bound_to)
      p = p->bound_to;

   return p;
}

//***********************************************************************

void
error(const char *s)
{
   cerr << "ERROR:" << s << '\n';
   cdl_had_error = true;
}

//***********************************************************************

void
error(const char *s, const char *s2)
{
   cerr << "ERROR:" << s << ' ' << s2 << '\n';
   cdl_had_error = true;
}

//***********************************************************************

void
error(const char *s, const int i)
{
   cerr << "ERROR:" << s << '=' << i << '\n';
   cdl_had_error = true;
}

//***********************************************************************

void
indent(ofstream & s, const int level)
{
   if (level > 0)
   {
      s.width(level * 2);
      s << " ";
      s.width(0);
   }
}

///////////////////////////////////////////////////////////////////////
// $Log: common_codegen.cc,v $
// Revision 1.1.1.1  2008/07/14 16:44:14  endo
// MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
//
// Revision 1.2  2006/12/05 01:51:02  endo
// cdl parser now accepts {& a b} and Symbol *pair added.
//
// Revision 1.1.1.1  2006/07/20 17:17:45  endo
// MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
//
// Revision 1.1.1.1  2006/07/12 13:37:54  endo
// MissionLab 7.0
//
// Revision 1.1.1.1  2005/02/06 22:59:28  endo
// AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
//
// Revision 1.10  2003/04/06 13:11:30  endo
// gcc 3.1.1
//
// Revision 1.9  1996/06/17  22:58:58  doug
// *** empty log message ***
//
// Revision 1.8  1996/05/14  23:16:56  doug
// *** empty log message ***
//
// Revision 1.7  1996/05/07  19:53:55  doug
// fixing compile warnings
//
// Revision 1.6  1996/03/13  01:52:36  doug
// fixed error reporting
//
// Revision 1.5  1996/03/08  20:43:36  doug
// *** empty log message ***
//
// Revision 1.4  1996/03/08  00:46:41  doug
// *** empty log message ***
//
// Revision 1.3  1996/02/25  01:13:39  doug
// *** empty log message ***
//
// Revision 1.2  1996/02/18  22:44:13  doug
// handle pushup parms in cnl_codegen
//
// Revision 1.1  1996/02/18  21:01:16  doug
// Initial revision
//
///////////////////////////////////////////////////////////////////////
