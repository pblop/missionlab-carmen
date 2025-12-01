#ifndef PROTOTYPES_H
#define PROTOTYPES_H

/**********************************************************************
 **                                                                  **
 **                            prototypes.h                          **
 **                                                                  **
 **  Prototype include file parser support functions                 **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1995, Georgia Tech Research Corporation               **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/
 
/* $Id: prototypes.h,v 1.1.1.1 2008/07/14 16:44:22 endo Exp $ */

#ifndef USE
// Trick compiler to not generate warnings that a variable wasn't used.
#define USE(var) static void * use_##var = (void *)&var
#endif


#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "symtab.h"

extern FILE *outfile;

extern int yydebug;	// set to TRUE to get yacc debug info


void SyntaxError(const char *Message);
void SemanticError(const char *Message);
char *AnonymousName(void);
char *Anonymous2External(const char *anonymous_name);
Symbol *find_index(Symbol *parm, Symbol *index);
Symbol *find_name(Symbol *rec, char *name);
void AddUser(SymbolList *parmlist, Symbol *agent);

// Add this user to the "users" list for each agent in the children list
void UseChildren(Symbol *user);

void fill_parm_list(Symbol *defining_rec, SymbolList **list);


int next_file();
int load_first_file(void);

extern int             cur_file;
extern int             num_files;
extern FILE *yyin;



/**********************************************************************
 * $Log: prototypes.h,v $
 * Revision 1.1.1.1  2008/07/14 16:44:22  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/20 17:17:50  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:37:59  endo
 * MissionLab 7.0
 *
 * Revision 1.1.1.1  2005/02/06 23:00:07  endo
 * AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
 *
 * Revision 1.16  1996/05/05  21:45:27  doug
 * fixing compile warnings
 *
 * Revision 1.16  1996/05/05  21:45:27  doug
 * fixing compile warnings
 *
 * Revision 1.15  1995/10/30  21:38:25  doug
 * *** empty log message ***
 *
 * Revision 1.14  1995/09/19  15:31:53  doug
 * change so all parms defined in the def record exist in any instances
 *
 * Revision 1.13  1995/06/29  14:39:33  jmc
 * Added RCS log string.
 **********************************************************************/

#endif
