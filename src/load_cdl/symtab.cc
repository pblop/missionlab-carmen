/**********************************************************************
 **                                                                  **
 **                       symtab.cc                                  **  
 **                                                                  **
 **  C routines used by cnl_lex.l for CDL                            **  
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1995, Georgia Tech Research Corporation               **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: symtab.cc,v 1.1.1.1 2008/07/14 16:44:22 endo Exp $ */


//#define DEBUG_SYMTAB 

#include "prototypes.h"
#include "cdl_defs.h"
#include "list.hpp"
#include "symtab.h"
#include "cdl_gram.tab.h"
#include "utilities.h"

SymbolTable *globals = NULL;

static GTList<SymbolTable *> ScopeList;
Symbol *free_arch = NULL;
const Symbol *choosen_architecture = NULL;

SymbolList defined_architectures;

/*********************************************************************
*                                                                    *
*                                                                    *
*********************************************************************/

void
init_symtab(void)
{
   // create the global namespace table.
   Symbol *global_namespace = new Symbol(ARCH_NAME);
   global_namespace->name = "globals";
   global_namespace->table.name = "GLOBALS";
   globals = &global_namespace->table;

   // Add the global namespace to the scope list
   ScopeList.insert(globals);

   // want the free_arch symbol table to also be the global namespace table.
   free_arch = new Symbol(ARCH_NAME);
   free_arch->name = "free";
   free_arch->table.name = "FREE";

   // Add the free architecture to the global name space
   globals->put(free_arch->name, free_arch);

   // Default is to be in the free architecture
   choosen_architecture = free_arch;
}

/*********************************************************************
*                                                                    *
*                                                                    *
*********************************************************************/

/*
 * Dump the symbol scope list
 */

void
DumpScopeList(void)
{
   SymbolTable *table;
   void *cur;

   cerr << "Current Scope list:\n";
   if ((cur = ScopeList.first(&table)) == NULL)
   {
      cerr << "Scope list is empty\n";
      return;
   }

   do
   {
      cerr << "   " << table->name << "\n";

      cerr << "      SYSTEM names:\n";
      Symbol *data = (Symbol *)table->first();
      while( data )
      {
	 if( data->record_class == RC_SYSTEM )
	    cerr << "         " << data->name << "\n";
         data = (Symbol *)table->next();
      }

      cerr << "      LIBRARY names:\n";
      data = (Symbol *)table->first();
      while( data )
      {
	 if( data->record_class == RC_LIBRARY )
	    cerr << "         " << data->name << "\n";
         data = (Symbol *)table->next();
      }

      cerr << "      USER names:\n";
      data = (Symbol *)table->first();
      while( data )
      {
	 if( data->record_class != RC_SYSTEM &&
	     data->record_class != RC_LIBRARY )
	 {
	    cerr << "         " << data->name << "\n";
	 }
         data = (Symbol *)table->next();
      }
      cerr << "\n";
   } while ((cur = ScopeList.next(&table, cur)) != NULL);
}

/************************************************************************
*                                                                       *
*                                                                       *
************************************************************************/

// Enter a new sub scope
void
EnterScope(Symbol * sym)
{
   // If are an instance of a defining record (normal case)
   // then need to get it on the stack below the new rec
   // so we can find parameter names.
   Symbol *defrec = sym->defining_rec;
   if( defrec && defrec->defining_rec &&
      (defrec->symbol_type == ACTUATOR_NAME ||
       defrec->symbol_type == SENSOR_NAME ||
       defrec->symbol_type == ROBOT_NAME))
   {
#if defined(DEBUG_SYMTAB)
      if( defrec->defining_rec->table.name == NULL )
         defrec->defining_rec->table.name = defrec->defining_rec->name;
#endif
      ScopeList.insert(&defrec->defining_rec->table);
   }
   else if (defrec)
   {
#if defined(DEBUG_SYMTAB)
      if( sym->defining_rec->table.name == NULL )
         sym->defining_rec->table.name = sym->defining_rec->name;
#endif
      ScopeList.insert(&sym->defining_rec->table);
   }
#if defined(DEBUG_SYMTAB)
   if( sym->table.name == NULL )
      sym->table.name = sym->name;
#endif
   ScopeList.insert((SymbolTable *)&sym->table);

#if defined(DEBUG_SYMTAB)
   cerr << "Entered scope of " << sym->name << "\n";
   DumpScopeList();
#endif
}

/************************************************************************
*                                                                       *
*                                                                       *
************************************************************************/

// Leave a sub scope
void
LeaveScope(void)
{
   SymbolTable *left = ScopeList.get();
   if (left == NULL)
   {
      cerr << "Internal Error: Popped empty scope list\n";
      abort();
      exit(1);
   }

#if defined(DEBUG_SYMTAB)
   const char *name = left->name ? left->name : "NULL";
   cerr << "Left scope of " << name << "\n";
   DumpScopeList();
#endif
}

//***********************************************************************

// Leave a sub scope
void
LeaveScope(Symbol * sym)
{
   LeaveScope();

   if( sym )
   {
      Symbol *defrec = sym->defining_rec;
      if( defrec && defrec->defining_rec &&
         (defrec->symbol_type == ACTUATOR_NAME ||
          defrec->symbol_type == SENSOR_NAME ||
          defrec->symbol_type == ROBOT_NAME) )
      {
         LeaveScope();
      }
      else if (sym->defining_rec)
      {
         LeaveScope();
      }
   }
}

/************************************************************************
*                                                                       *
*                                                                       *
************************************************************************/

/*
 * Returns pointer to a symbol record or NULL if it is not found 
 */

Symbol *
LookupName(SymbolTable *table, const char *name)
{
   Symbol *rec = (Symbol *) table->get(name);

   if (rec && (is_lhs || (rec->symbol_type != PARM_NAME &&
  	                  rec->symbol_type != PARM_HEADER)))
   {
      return rec;
   }

   return NULL;
}

/************************************************************************
*                                                                       *
*                                                                       *
************************************************************************/

/*
 * Returns pointer to a symbol record or NULL if it is not found 
 */

Symbol *
LookupName(const char *name)
{
   SymbolTable *table;
   void *cur;

#if defined(DEBUG_SYMTAB)
   cerr << "Going to look up name " << name << "\n";
   DumpScopeList();
#endif

   if ((cur = ScopeList.first(&table)) == NULL)
   {
      cerr << "Internal Error: LookupName with empty scope list\n";
      abort();
      exit(1);
   }

   do
   {
      Symbol *rec = LookupName(table, name);
      if( rec )
      {
#if defined(DEBUG_SYMTAB)
         cerr << "Found it in " << table->name << "\n\n";
#endif
	 return rec;
      }
   }
   while ((cur = ScopeList.next(&table, cur)) != NULL);

#if defined(DEBUG_SYMTAB)
   cerr << "Didn't find it\n\n";
#endif

   return NULL;
}

/************************************************************************
*                                                                       *
*                                                                       *
************************************************************************/

/*
 * Returns pointer to a symbol record or NULL if it is not found 
 */

Symbol *
LookupLocalName(const char *name)
{
   SymbolTable *table;
   void *cur;

   if ((cur = ScopeList.first(&table)) == NULL)
   {
      cerr << "Internal Error: LookupLocalName with empty scope list\n";
      abort();
      exit(1);
   }

   return LookupName(table, name);
}

/************************************************************************
*                                                                       *
* NameIsDefined              						                           *
* Returns the same record as DefineName if it would fail.               *
* NULL if DefineName would succeed.                                     *
*                                                                       *
************************************************************************/

Symbol *
NameIsDefined(const Symbol * sym)
{
   SymbolTable *table;

   if (!ScopeList.first(&table))
   {
      cerr << "Internal Error: NameIsDefined with empty scope list\n";
      abort();
      exit(1);
   }

   // Return true if it is defined (DefineName would fail), false otherwise.
   return (Symbol *)table->get(sym->name);
}

/************************************************************************
*                                                                       *
* DefineName              						*
*                                                                       *
************************************************************************/

Symbol *
DefineName(const Symbol * sym)
{
   SymbolTable *table;

#if defined(DEBUG_SYMTAB)
   cerr << "DefineName " << sym->name << " ... ";
#endif

   if (!ScopeList.first(&table))
   {
      cerr << "Internal Error: DefineName with empty scope list\n";
      abort();
      exit(1);
   }

   // Check if already exists, if so, return the existing record
   Symbol *rec = (Symbol *) table->get(sym->name);

   if (rec)
   {
#if defined(DEBUG_SYMTAB)
      cerr << sym->name << " already is defined\n\n";
      DumpScopeList();
      cerr << "\n";
#endif
      return rec;		// Symbol already exists
   }

   // Add the new record
   table->put(sym->name, sym);

#if defined(DEBUG_SYMTAB)
   cerr << "Done\n";
   DumpScopeList();
   cerr << "\n";
#endif

   // No collisions
   return NULL;
}

/************************************************************************
*                                                                       *
* RemoveName              						*
*                                                                       *
* returns true if name existed                                          *
************************************************************************/

bool
RemoveName(const Symbol * sym, bool debug)
{
   if( sym->name == NULL )
      return false;

   SymbolTable *table;

   if (!ScopeList.first(&table))
   {
      cerr << "Internal Error: RemoveName with empty scope list\n";
      abort();
      exit(1);
   }

   // Try to delete the name and return true if it existed
   bool existed = table->remove(sym->name);

   if( debug )
   {
      if( !existed )
      {
         cerr << "Name " << sym->name << " didn't exist in the symbol table\n";
         table->dump();
      }
      else
         cerr << "Removed " << sym->name << " from symbol table\n";
   }

   return existed;
}

/************************************************************************
*                                                                       *
* DefineRootName              						*
*                                                                       *
************************************************************************/

// Define a name in the global address space, no matter what our current scope.

Symbol *
DefineRootName(const Symbol * sym)
{
   Symbol *rec;
   if( (rec=LookupName(sym->name)) != NULL)
   {
      char buf[256];
      sprintf(buf, "Attempt to redefine root name '%s'", sym->name);
      SyntaxError(buf);
      return rec;
   }

   // Add the new record
   globals->put(sym->name, sym);

   // No collisions
   return NULL;
}

/************************************************************************
*                                                                       *
* AddArch              						*
*                                                                       *
************************************************************************/

Symbol *
AddArch(const char *arch_name)
{
   Symbol *p = new Symbol(ARCH_NAME);

   p->name = strdup(arch_name);

   // Check if already exists, if so, return the existing record
   Symbol *dup = DefineRootName(p);

   if (dup)
   {
      char buf[256];

      sprintf(buf, "Attempt to redefine existing architecture type '%s'",
	      arch_name);
      SyntaxError(buf);
      return dup;
   }

   // The architecture defines a name space (symbol table) where architecture 
   // specific records will be defined.

   p->table.name = "ARCH";

   defined_architectures.append(p);

   return p;
}

//***********************************************************************

// Pass in the symbol record for the new architecture.
// Use NULL to unbind.
void
UseArch(Symbol * arch)
{
   if( ScopeList.len() > 2 )
   {
      SyntaxError("Invalid scoping of architecture binding.\nMust be at top level.");
      return;
   }

   if( ScopeList.len() == 1 )
   {
      SyntaxError("No current architecture record in the scope list.");
      return;
   }

   // Figure out the current architecture
   SymbolTable *table;
   void *cur;
   if ((cur = ScopeList.first(&table)) == NULL)
   {
      cerr << "Internal Error: UseArch with empty scope list\n";
      abort();
      exit(1);
   }

#if 0
   // This is pretty much set up to use multiple code generators 
   // from a single source file.  However, I don't need that 
   // functionality for my research, and in fact, don't even know
   // how I would test it, so I'll preclude it here.  DCM.

   if( arch && (table->name == NULL || strcmp(table->name, "FREE") != 0) )
   {
      SyntaxError("The use of multiple architectures is not supported yet");
      return;
   }
#endif

   // pop the current architecture name table.
   LeaveScope();

   // Enter the desired architecture
   if( arch == NULL )
   {
      EnterScope(free_arch);
      choosen_architecture = free_arch;
   }
   else
   {
      EnterScope(arch);
      choosen_architecture = arch;
   }
}

/************************************************************************
*                                                                       *
* AddType              						        *
*                                                                       *
************************************************************************/

Symbol *
AddType(Symbol *arch, const char *type_name)
{
   Symbol *p = new Symbol(TYPE_NAME);

   p->name = strdup(type_name);
   p->arch = arch;

   // Check if already exists, if so, return the existing record
   Symbol *dup = DefineName(p);

   if (dup)
   {
      char buf[256];

      sprintf(buf, "Attempt to redefine existing data type '%s'",
	      type_name);
      SyntaxError(buf);
      return dup;
   }
   return p;
}

/************************************************************************
*                                                                       *
* DumpSymbolTable              						*
*                                                                       *
************************************************************************/
void
DumpTable(SymbolTable * base)
{
   Symbol *p = (Symbol *) base->first();

   while (p)
   {
#if 0
      cerr << p;
#endif

      DumpTable(&p->table);

      p = (Symbol *) base->next();
   };
}

void
DumpSymbolTable(void)
{
   cerr << "\n -- DumpSymbolTable -- \n";
   DumpTable(globals);
   cerr << "\n ===================== \n";
}

/*********************************************************************
*                                                                    *
*                                                                    *
*********************************************************************/

// Count the number of architectures (includes free).

int
CountArches()
{
   int cnt = 0;
   Symbol *data;

   if ((data = (Symbol *)globals->first()) != NULL)
   {
      do
      {
         if( data->symbol_type == ARCH_NAME )
	    cnt++;
      } while( (data = (Symbol *)globals->next()) != NULL);
   }
   return cnt;
}

/*********************************************************************
*                                                                    *
*                                                                    *
*********************************************************************/

// Check if the named symbol exists in the named architecture
bool
is_defined_in(char *sym_name, char *arch_name)
{
   Symbol *arch = LookupName(arch_name);
   if( arch == NULL )
      return false;

   Symbol *sym = LookupName(&arch->table, sym_name);

   return sym != NULL; 
}

/*********************************************************************
*                                                                    *
*                                                                    *
*********************************************************************/

// Check if the named symbol exists in all the named architectures
static bool
passes_restrictions(char *sym_name, char *restrictions[])
{
   if( restrictions == NULL )
      return true;

   int index = 0;
   while( restrictions[index] )
   {
      if( !is_defined_in(sym_name, restrictions[index]) )
	 return false;
      index++;
   }

   return true;
}

//-----------------------------------------------------------------------
// List behavior names in the current architecture
//-----------------------------------------------------------------------
void AllNames(GTList<char *> agentlist[], char *restrictions[])
{
    SymbolTable *table = NULL;
    Symbol *data = NULL;
    char linebuf[2048];
    int i;

    for(i = 0; i < NUM_GROUPS; i++)
    {
        agentlist[i].clear();
    }

    table = (SymbolTable *)&choosen_architecture->table;

    if ((data = (Symbol *)table->first()) != NULL)
    {
        do
        {
            if( data->data_type == boolean_type &&
                data->name[0] != '$' &&
                passes_restrictions(data->name, restrictions) )
            {
                sprintf(
                    linebuf,
                    //"%-24s| %s",
                    "%-30s | %s",
                    data->name, 
                    data->description ? data->description : "");
            
                agentlist[PG_PRIM_TRIGS].append(strdup(linebuf));
            }

            if((data->symbol_type == ROBOT_CLASS ||
                data->symbol_type == AGENT_CLASS) &&
               data->name[0] != '$' &&
               passes_restrictions(data->name, restrictions) )
            {
                sprintf(
                    linebuf,
                    //"%-24s| %s",
                    "%-30s | %s",
                    data->name, 
                    data->description ? data->description : "");

                agentlist[PG_PRIMS].append(strdup(linebuf));
            }
            else if( (data->symbol_type == COORD_CLASS ||
                      data->symbol_type == COORD_NAME ) &&
                     data->name[0] != '$' &&
                     passes_restrictions(data->name, restrictions) )
            {
                sprintf(
                    linebuf,
                    //"%-24s| %s",
                    "%-30s | %s",
                    data->name, 
                    data->description ? data->description : "");

                agentlist[PG_OPS].append(strdup(linebuf));
            }
            else if( data->symbol_type == SENSOR_NAME &&
                     data->name[0] != '$' &&
                     passes_restrictions(data->name, restrictions) )
            {
                sprintf(
                    linebuf,
                    //"%-24s| %s",
                    "%-30s | %s",
                    data->name, 
                    data->description ? data->description : "");

                agentlist[PG_SENSOR].append(strdup(linebuf));
            }
            else if( data->symbol_type == ACTUATOR_NAME &&
                     data->name[0] != '$' &&
                     passes_restrictions(data->name, restrictions) )
            {
                sprintf(
                    linebuf,
                    //"%-24s| %s",
                    "%-30s | %s",
                    data->name, 
                    data->description ? data->description : "");

                agentlist[PG_ACTUATOR].append( strdup(linebuf) );
            }
        } while((data = (Symbol *)table->next()) != NULL);
    }

    // Binding points are not architecture specific
    table = &free_arch->table;
    if ((data = (Symbol *)table->first()) != NULL)
    {
        do
        {
            if(data->symbol_type == BP_CLASS &&
               passes_restrictions(data->name, restrictions))
            {
                if(data->construction == CS_RBP)
                {
                    agentlist[PG_RBP].append(data->name);
                }
                else if( data->construction == CS_IBP )
                {
                    agentlist[PG_IBP].append(data->name);
                }
                else if( data->construction == CS_OBP )
                {
                    agentlist[PG_OBP].append(data->name);
                }
            }

            if(data->record_class == RC_LIBRARY && data->symbol_type == GROUP_NAME)
            {
                if(data->name[0] != '$' &&
                   passes_restrictions(data->name, restrictions))
                {
                    sprintf(
                        linebuf,
                        //"%-24s| %s",
                        "%-30s | %s",
                        data->name, 
                        data->description ? data->description : "");

                    Symbol *src = data->real_source();

                    if(src && 
                       src->defining_rec && 
                       src->defining_rec->data_type == boolean_type)
                    {
                        agentlist[PG_TRIGS].merge(strdup(linebuf));
                    }
                    else if(src)
                    {
                        agentlist[PG_AGENTS].merge(strdup(linebuf));
                    }
                }
            }

        } while((data = (Symbol *)table->next()) != NULL);
    }

    // Sort the lists
    for(i = 0; i < NUM_GROUPS; i++)
    {
        agentlist[i].alphabetize();
    }
}

/*********************************************************************
*                                                                    *
*                                                                    *
*********************************************************************/

/*
 * List the architecture names 
 */

void
ArchNames(GTList<char *> *archlist)
{
   Symbol *data;

   if ((data = (Symbol *)globals->first()) != NULL)
   {
      do
      {
         if( data->symbol_type == ARCH_NAME && data != free_arch )
         {
	    archlist->append(data->name);
         }
      } while( (data = (Symbol *)globals->next()) != NULL);
   }
}

/*********************************************************************
*                                                                    *
*                                                                    *
*********************************************************************/

//Get list of records in the current architecture, with the desired symbol_type 

SymbolList *
GetRecords(SymbolTable *table,
           const e_fields field1,const e_cmds cmd1,void *data1, 
           const e_fields field2,const e_cmds cmd2,void *data2)
{
   SymbolList *recs = new SymbolList;
   
   // Copy parms to arrays to simplfy use
   const int MAX_NUM = 2;
   enum e_fields fields[MAX_NUM] = {field1, field2};
   enum e_cmds   cmds[MAX_NUM]   = {cmd1, cmd2};
   void *datums[MAX_NUM]         = {data1, data2};
   int num;

   // Decide number of entries passed
   if( field1 == f_none )
      num = 0;
   else if( field2 == f_none )
   {
      num = 1;
   }
   else 
   {
      num = 2;
   }

   // Find the records
   Symbol *data;
   if ((data = (Symbol *)table->first()) != NULL)
   {
      do
      {
	 bool useit=true;
	 for(int i=0; useit && i<num; i++)
	 {
	    switch( fields[i] )
	    {
	       case f_symbol_type:
		  switch( cmds[i] )
		  {
		     case c_equal:
                        if( data->symbol_type != (int)datums[i] )
			   useit = false;
			break;
		     default:
			break;
		  }
		  break;

	       case f_construction:
		  switch( cmds[i] )
		  {
		     case c_equal:
                        if( data->construction != (int)datums[i] )
			   useit = false;
			break;
		     default:
			break;
		  }
		  break;

	       case f_binds_to:
		  switch( cmds[i] )
		  {
		     case c_equal:
                        if( data->binds_to != (Symbol *)datums[i] )
			   useit = false;
			break;
		     default:
			break;
		  }
		  break;

	       default:
		  break;
	    }
         }
		  
         if( useit )
	    recs->append(data);
      } while( (data = (Symbol *)table->next()) != NULL);
   }

   return recs;
}

/*********************************************************************
*                                                                    *
*                                                                    *
*********************************************************************/

//Get list of records in the current architecture, with the desired symbol_type 

SymbolList *
GetRecords(const e_fields field1,const e_cmds cmd1,void *data1, 
           const e_fields field2,const e_cmds cmd2,void *data2)
{
   // Pick the correct table to use
   SymbolTable *table;
   table = (SymbolTable *)&choosen_architecture->table;

   return GetRecords(table,field1,cmd1,data1,field2,cmd2,data2);
}


/*-----------------------------------------------------------------------*/
/* extract the list of directories from the environment variable */
char **
extract_options(const char *name, const char *force)
{
   char **list = NULL;
   char *string = getenv(name);

   if( force != NULL )
   {
      if( string == NULL || strlen(string) < 1 )
      {
         string = (char *)force;
      }
      else
      {
         string = strdupcat(strdup(string)," ");
         string = strdupcat(string,(char *)force);
      }
   }

   int num = 1;

   if (string != NULL)
   {
      // Count the occurances so can alloc the array
      char *pos = strtok(string, " \n\r\t");

      while (pos)
      {
	 num++;
	 pos = strtok(NULL, " \n\r\t");
      }
   }

   if ((list = (char **) malloc(sizeof(char *) * num)) == NULL)
   {
      fprintf(stderr, "Out of memory allocating list\n");
      exit(1);
   }

   int offset = 0;

   int out_pos = 0;
   for (int pos = 0; pos < num - 1; pos++)
   {
      // Only add one copy of each library
      bool is_dup = false;
      for(int i=0; i<out_pos; i++)
      {
	 if( strcmp(list[i],&string[offset]) == 0 )
	 {
	    is_dup = true;
	    break;
	 }
      }

      if( !is_dup )
      {
         list[out_pos] = strdup(&string[offset]);
         offset += strlen(list[out_pos]) + 1;
         out_pos++;
      }
   }
   list[out_pos] = NULL;

   return list;
}

/*********************************************************************
*                                                                    *
*                                                                    *
*********************************************************************/

// Get list of records defined in the specified architecture which
// were defined from the specified library file

SymbolList *
GetLibraryRecords(Symbol * arch, const char *source_file)
{
   // Pick the correct table to use
   SymbolTable *table = (SymbolTable *)&arch->table;

   SymbolList *recs = new SymbolList;
   
   // Find the records
   Symbol *data;
   if ((data = (Symbol *)table->first()) != NULL)
   {
      do
      {
         if( data->record_class == RC_LIBRARY &&
	     strcmp(data->source_file, source_file) == 0)
	 {
	       recs->append(data);
	 }
      } while( (data = (Symbol *)table->next()) != NULL);
   }

   return recs;
}

/*********************************************************************
*                                                                    *
*                                                                    *
*********************************************************************/

// Remove the record from the library
// Returns true if the record was found and removed.
bool
RemoveFromLibrary(Symbol *sym, Symbol * arch)
{
   // Pick the correct table to use
   SymbolTable *table = (SymbolTable *)&arch->table;

   return table->remove(sym->name);
}

/************************************************************************
*                                                                       *
* DefineNameInArch            						*
*                                                                       *
************************************************************************/

Symbol *
DefineNameInArch(Symbol * arch, const Symbol * sym)
{
   // Pick the correct table to use
   SymbolTable *table = (SymbolTable *)&arch->table;

   // Check if already exists, if so, return the existing record
   Symbol *rec = (Symbol *) table->get(sym->name);

   if (rec)
      return rec;		// Symbol already exists

   // Add the new record
   table->put(sym->name, sym);

   // No collisions
   return NULL;
}


/**********************************************************************
 * $Log: symtab.cc,v $
 * Revision 1.1.1.1  2008/07/14 16:44:22  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/20 17:17:50  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:37:59  endo
 * MissionLab 7.0
 *
 * Revision 1.2  2006/01/10 06:08:16  endo
 * AO-FNC Type-I check-in.
 *
 * Revision 1.1.1.1  2005/02/06 23:00:07  endo
 * AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
 *
 * Revision 1.58  2002/01/13 01:26:56  endo
 * list -> GTList
 *
 * Revision 1.57  1999/11/09 17:49:27  endo
 * The modification made by Doug MacKenzie.
 * He fixed the problem of CfgEdit not being able to
 * load more than 15 states mission by adding some
 * checking procedure.
 *
 * Revision 1.56  1996/10/04  21:10:59  doug
 * changes to get to version 1.0c
 *
 * Revision 1.56  1996/10/01 13:01:32  doug
 * got library writes working
 *
 * Revision 1.55  1996/05/05  21:45:27  doug
 * fixing compile warnings
 *
 * Revision 1.55  1996/05/05  21:45:27  doug
 * fixing compile warnings
 *
 * Revision 1.54  1996/05/02  22:58:45  doug
 * *** empty log message ***
 *
 * Revision 1.53  1996/02/22  00:56:01  doug
 * *** empty log message ***
 *
 * Revision 1.52  1996/02/19  21:57:42  doug
 * library components and permissions now work
 *
 * Revision 1.51  1996/02/18  00:03:53  doug
 * *** empty log message ***
 *
 * Revision 1.50  1996/02/16  00:07:59  doug
 * *** empty log message ***
 *
 * Revision 1.49  1996/02/15  19:30:32  doug
 * fixed pushed up parms in groups
 *
 * Revision 1.48  1996/01/17  18:46:06  doug
 * *** empty log message ***
 *
 * Revision 1.47  1995/12/05  17:25:55  doug
 * *** empty log message ***
 *
 * Revision 1.46  1995/12/01  21:16:50  doug
 * *** empty log message ***
 *
 * Revision 1.45  1995/11/30  23:30:28  doug
 * *** empty log message ***
 *
 * Revision 1.44  1995/11/29  23:17:06  doug
 * *** empty log message ***
 *
 * Revision 1.43  1995/11/29  23:12:30  doug
 * *** empty log message ***
 *
 * Revision 1.42  1995/11/07  14:29:58  doug
 * *** empty log message ***
 *
 * Revision 1.41  1995/10/31  19:23:06  doug
 * moved find_file to utilities dir
 *
 * Revision 1.40  1995/10/30  23:05:55  doug
 * *** empty log message ***
 *
 * Revision 1.39  1995/10/30  21:38:25  doug
 * *** empty log message ***
 *
 * Revision 1.38  1995/10/27  20:29:18  doug
 * *** empty log message ***
 *
 * Revision 1.37  1995/09/15  22:01:29  doug
 * fixed so handles multiline initializers
 *
 * Revision 1.36  1995/09/07  14:23:14  doug
 * works
 *
 * Revision 1.35  1995/06/29  14:52:08  jmc
 * Added RCS log string.
 **********************************************************************/
