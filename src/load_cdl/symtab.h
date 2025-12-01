#ifndef SYMTAB_H
#define SYMTAB_H
/**********************************************************************
 **                                                                  **
 **                            symtab.h                              **
 **                                                                  **
 **  Prototype include file for parser symbol table.                 **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1995, Georgia Tech Research Corporation               **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/
 
/* $Id: symtab.h,v 1.1.1.1 2008/07/14 16:44:22 endo Exp $ */

#include <iostream>
#include "symbol_table.hpp"
#include "list.hpp"
#include "symbol.hpp"

using std::ostream;

// Defines the various types of symbols
const int PG_PRIMS = 0;
const int PG_PRIM_TRIGS = 1;
const int PG_OPS = 2;
const int PG_ACTUATOR = 3;
const int PG_SENSOR = 4;
const int PG_RBP = 5;
const int PG_IBP = 6;
const int PG_OBP = 7;
const int PG_AGENTS = 8;
const int PG_TRIGS = 9;
const int NUM_GROUPS = 10;

/* Agent class */
typedef struct T_Agent
{
   int type;
}
T_Agent;

void DumpScopeList(void);
void EnterScope(Symbol * sym);
void LeaveScope(Symbol * sym);
void LeaveScope(void);
Symbol *LookupName(const char *name);
Symbol *LookupLocalName(const char *name);
Symbol *LookupName(SymbolTable *table, const char *name);
Symbol *DefineName(const Symbol * sym);
Symbol *NameIsDefined(const Symbol * sym);
bool RemoveName(const Symbol * sym, bool debug=false);
Symbol *AddArch(const char *arch_name);
Symbol *AddType(Symbol *arch, const char *type_name);
void DumpSymbolTable(void);
void DumpTable(SymbolTable * base);
void init_symtab(void);
void UseArch(Symbol * arch);

typedef enum e_fields {f_symbol_type, f_binds_to, 
		       f_construction, f_none} e_fields;
typedef enum e_cmds   {c_equal, c_none} e_cmds;

SymbolList *
GetRecords(const e_fields field1=f_none,const e_cmds cmd1=c_none,void *data1=0,
           const e_fields field2=f_none,const e_cmds cmd2=c_none,void *data2=0);

SymbolList *
GetRecords(SymbolTable *table,
	   const e_fields field1=f_none,const e_cmds cmd1=c_none,void *data1=0,
           const e_fields field2=f_none,const e_cmds cmd2=c_none,void *data2=0);

void AllNames(GTList<char *> agentlist[], char *restrictions[]);

void ArchNames(GTList<char *> *archnames);

extern Symbol *top_agent;

extern Symbol *free_arch;

// special architectures
extern Symbol *AuRA_arch;
extern Symbol *AuRA_urban_arch;
extern Symbol *AuRA_naval_arch;
extern Symbol *UGV_arch;

// Special types
extern Symbol *expression_type;
extern Symbol *member_type;
extern Symbol *void_type;
extern Symbol *UTM_type;
extern Symbol *boolean_type;
extern Symbol *string_type;
extern Symbol *commands_type;

extern const Symbol *choosen_architecture;
extern RECORD_CLASS current_class;
extern const char *current_source_file;
extern SymbolList defined_architectures;


// Get list of records defined in the specified architecture which
// were defined from the specified library file
SymbolList *GetLibraryRecords(Symbol * arch, const char *source_file);

// Add a record to the specified library
Symbol *DefineNameInArch(Symbol * arch, const Symbol * sym);

char **extract_options(const char *name, const char *force = NULL);

bool isanonymous(const char *name);
bool is_defined_in(char *sym_name, char *arch_name);

bool RemoveFromLibrary(Symbol *sym, Symbol * arch);


/**********************************************************************
 * $Log: symtab.h,v $
 * Revision 1.1.1.1  2008/07/14 16:44:22  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/20 17:17:50  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:37:59  endo
 * MissionLab 7.0
 *
 * Revision 1.2  2005/05/18 21:19:07  endo
 * AuRA.naval added.
 *
 * Revision 1.1.1.1  2005/02/06 23:00:07  endo
 * AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
 *
 * Revision 1.58  2003/04/06 12:58:48  endo
 * gcc 3.1.1
 *
 * Revision 1.57  2002/01/13 01:24:31  endo
 * list ->GTList
 *
 * Revision 1.56  1999/12/16 22:25:27  endo
 * rh-6.0 porting.
 *
 * Revision 1.55  1999/11/09 17:52:10  endo
 * The modification made by Doug MacKenzie.
 * He fixed the problem of CfgEdit not being able to
 * load more than 15 states mission by adding some
 * checking procedure.
 *
 * Revision 1.54  1999/09/03 20:09:42  endo
 * *AuRA_urban_arch added.
 *
 * Revision 1.53  1996/10/04 21:10:59  doug
 * changes to get to version 1.0c
 *
 * Revision 1.53  1996/10/01 13:01:32  doug
 * got library writes working
 *
 * Revision 1.52  1996/05/02 22:58:45  doug
 * *** empty log message ***
 *
 * Revision 1.51  1996/02/22  00:56:01  doug
 * *** empty log message ***
 *
 * Revision 1.50  1996/02/19  21:57:42  doug
 * library components and permissions now work
 *
 * Revision 1.49  1996/02/18  00:03:53  doug
 * *** empty log message ***
 *
 * Revision 1.48  1995/12/05  17:25:55  doug
 * *** empty log message ***
 *
 * Revision 1.47  1995/12/01  21:16:50  doug
 * *** empty log message ***
 *
 * Revision 1.46  1995/11/30  23:30:28  doug
 * *** empty log message ***
 *
 * Revision 1.45  1995/11/29  23:12:30  doug
 * *** empty log message ***
 *
 * Revision 1.44  1995/11/07  14:29:58  doug
 * *** empty log message ***
 *
 * Revision 1.43  1995/10/31  19:23:06  doug
 * moved find_file to utilities dir
 *
 * Revision 1.42  1995/10/30  21:38:25  doug
 * *** empty log message ***
 *
 * Revision 1.41  1995/10/11  22:05:07  doug
 * *** empty log message ***
 *
 * Revision 1.40  1995/09/15  15:37:40  doug
 * add commands_type
 *
 * Revision 1.39  1995/09/07  14:23:14  doug
 * works
 *
 * Revision 1.38  1995/06/29  14:40:41  jmc
 * Added RCS log string.
 **********************************************************************/

#endif
