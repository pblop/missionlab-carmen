/**********************************************************************
 **                                                                  **
 **                       print_symbols.cc                           **  
 **                                                                  **
 **  debug printing functions                                        **  
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1995, Georgia Tech Research Corporation               **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: print_symbols.cc,v 1.1.1.1 2008/07/14 16:44:22 endo Exp $ */


#include "symtab.h"
#include "cdl_gram.tab.h"

static void 
indent(ostream & s, const Symbol * d)
{
   if (d->level > 0)
   {
      s.width(d->level * 2);
      s << " ";
      s.width(0);
   }
}

ostream & operator << (ostream & s, Symbol * d)
{
   char *text;

   indent(s, d);
   switch (d->symbol_type)
    {
    case COORD_CLASS:
       s << "COORD Class: ";
       break;
    case AGENT_CLASS:
       s << "Agent Class: ";
       break;
    case ARCH_NAME:
       s << "ARCH: ";
       break;
    case TYPE_NAME:
       s << "TYPE: ";
       break;
    case COORD_NAME:
       s << "COORD: ";
       break;
    case AGENT_NAME:
       s << "AGENT: ";
       break;
    case PARM_NAME:
       s << "PARM: ";
       break;
    case COORD_AGENT_NAME:
       s << "COORD_AGENT: ";
       break;
    case NAME:
       s << "NAME: ";
       break;
    default:
       s << "(" << d->symbol_type << "): ";
       break;
    }

   s << d->name << '\n';
   indent(s, d);

   text = (char *)(d->arch == NULL ? "NULL" : d->arch->name);
   s << "  arch: " << text << '\n';
   indent(s, d);

   text = (char *)(d->data_type == NULL ? "NULL" : d->data_type->name);
   s << "  data_type: " << text << '\n';
   indent(s, d);

   if (!d->parameter_list.isempty())
   {
      s << "  parameter_list:\n";
      indent(s, d);

      Symbol *p;
      void *cur;

      if ((cur = d->parameter_list.first(&p)) != NULL)
      {
	 do
	 {
	    s << "    ";
	    if (p->is_list)
	       s << "list ";
	    text = (char *)(p->data_type == NULL ? "NoType" : p->data_type->name);
	    s << text << ' ';

	    text = (char *)(p->name == NULL ? "NoName" : p->name);
	    s << text;

	    if (p->input_generator)
	    {
	       s << "=" << p->input_generator->name;
	    }
	    s << ";\n";
	    indent(s, d);
	 }
	 while ((cur = d->parameter_list.next(&p, cur)) != NULL);
      }
   }

   return s << '\n';
}



/**********************************************************************
 * $Log: print_symbols.cc,v $
 * Revision 1.1.1.1  2008/07/14 16:44:22  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/20 17:17:50  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:37:59  endo
 * MissionLab 7.0
 *
 * Revision 1.2  2006/05/14 07:38:31  endo
 * gcc-3.4 upgrade
 *
 * Revision 1.1.1.1  2005/02/06 23:00:07  endo
 * AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
 *
 * Revision 1.13  2000/10/16 19:28:28  endo
 * Modified due to the compiler upgrade.
 *
 * Revision 1.12  1996/05/05 21:45:27  doug
 * fixing compile warnings
 *
 * Revision 1.11  1995/06/29  14:48:40  jmc
 * Added RCS log string.
 **********************************************************************/
