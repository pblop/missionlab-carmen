/*
 * Douglas C. MacKenzie
 *
 * symtab.c ----  C routines used by gram.y for CNL ----
 * These are the routines which build the symbol table
 *
 *    Copyright 1995 - 2005, Georgia Tech Research Corporation
 *    Atlanta, Georgia  30332-0415
 *    ALL RIGHTS RESERVED, See file COPYRIGHT for details.
 *
 *    $Id: symtab.c,v 1.1.1.1 2008/07/14 16:44:17 endo Exp $
 */

#include <stdlib.h>
#include "defs.h"
#include "cnl_gram.tab.h"

extern int      lineno;

/* These two keep track of if are in data or text segment. */
int             InData = 0;
int             InText = 0;

/* this is used to keep track of parm offsets. */
int             ParmOffset = 0;
int             LocalOffset = 0;
int             LocalStatic = 0;

/* these are used in temparary location handling */
int             CurTemp = 1;
int             MaxLocalOffset = 0;


char            Symbols[MaxSymbols];	/* memory to store symbols */
int             NextSymbolLoc;	/* pointer to start of free in Symbols */
struct SymbolTableEntry SymbolTable[SymbolTableSize];
int             TypeTable[TypeTableSize];


/********************************************************************
*								    *
*								    *
********************************************************************/

/* New_symbol_symbol will create and initialise a new symbol record */

struct Symbol  *
New_symbol(void)
{
   struct Symbol  *Cur;

   /* get a new symbol record */
   Cur = (struct Symbol *) malloc(sizeof(*Cur));

   /* initialise the fields */
   memset(Cur, 0, sizeof(*Cur));
   Cur->Type_loc = -1;


   return Cur;
}


/********************************************************************
*								    *
*		         SyntaxError			            *
*								    *
********************************************************************/

/* SyntaxError will print the error message to stderr */

void
SyntaxError(const char *Message)
{
   fprintf(stderr, "ERROR: %s, line %d\n", filename, lineno);
   fprintf(stderr, "%s\n", Message);
   fprintf(stderr, "%s\n", linebuf);
   fprintf(stderr, "%*s\n", tokenpos + 1, "^");

   had_error = TRUE;
}

/********************************************************************
*								    *
*		         SemanticError			            *
*								    *
********************************************************************/

/* SemanticError will print the error message to stderr */

void
SemanticError(const char *Message)
{
   fprintf(stderr, "ERROR: %s, line %d\n", filename, lineno);
   fprintf(stderr, "%s\n", Message);

   had_error = TRUE;
}

/**********************************************************************
*                                                                     *
*                  print_parm_chain()                                 *
*                                                                     *
**********************************************************************/

void
print_parm_chain(struct Symbol * cur)
{
   char           *name;

   while (cur)
   {
      name = &Symbols[SymbolTable[cur->SymbolLoc].SymbolLocation];

      printf("parm '%s':\n", name);
      printf("  Level=%d\n", cur->Level);
      printf("  Class=%d\n", cur->Class);
      printf("  Type=%d\n", cur->Type);
      printf("  Offset=%d\n", cur->Offset);
      printf("  Type='%s'\n",
			&Symbols[SymbolTable[cur->Type_loc].SymbolLocation]);
      printf("  IsList=%s\n", cur->IsList ? "Yes":"No");
      printf("  node_type=%s\n", cur->node_type & NODE_TYPE_PUBLIC ? "Public":
		cur->node_type & NODE_TYPE_REMOTE ? "Remote" : "Local");
      printf("\n");

      cur = cur->NextDef;
   }
}

/**********************************************************************
*                                                                     *
*                  print_proc_chain()                                 *
*                                                                     *
**********************************************************************/

void
print_proc_chain(struct Symbol * cur)
{
   char           *name;

   while (cur)
   {
      name = &Symbols[SymbolTable[cur->SymbolLoc].SymbolLocation];

      printf("proc '%s':\n", name);
      printf("  Type: %s\n", cur->Type == EXTERN ? "EXTERN" :
	      cur->Type == LOCAL ? "LOCAL" : "Unknown");

      printf("  Output type:		%s\n",
              &Symbols[SymbolTable[cur->Type_loc].SymbolLocation]);

      printf("  is output void? 	%s\n",
	      cur->Type_loc == void_type ? "Yes" : "No");

      printf("  Parms:\n");

      print_parm_chain(cur->Parms);

      printf("\n");

      cur = cur->NextDef;
   }
}

/********************************************************************
*								    *
*		         Attach_links			            *
*								    *
********************************************************************/

/* Attach_links will add to a chain of links to a node definition */

struct Symbol  *
Attach_links(struct Symbol * node, struct Symbol * links)
{
   int             used;
   int             exists;
   struct Symbol  *input;
   struct Symbol  *proc;
   struct Symbol  *cur;
   char           *link;
   char           *t;
   char           *nname;

   /* if proc wasn't defined, then can't connect links */
   if (node == NULL)
      return node;

   /* get the name */
   nname = &Symbols[SymbolTable[node->SymbolLoc].SymbolLocation];

   /* check that each input is used once and only once */
   proc = node->Proc;

   input = proc->Parms;
   while (input)
   {
      /* if it isn't a list, check that it is used once and only once */
      used = 0;
      link = &Symbols[SymbolTable[input->SymbolLoc].SymbolLocation];

      cur = links;
      while (cur)
      {
	 t = &Symbols[SymbolTable[cur->SymbolLoc].SymbolLocation];

	 /* is this the link? */
	 if (strcmp(t, link) == 0)
	 {
	    if (input->IsList)
	    {
	       cur->IsList = TRUE;
	       cur->ThisInstance = used;
	       used++;
	    }
	    else
	    {
	       if (used)
	       {
		  char            msg[256];

		  sprintf(msg, "Node '%s' assigns more than one connection to input '%s'\n", nname, link);
		  SemanticError(msg);
	       }
	       used = TRUE;
	    }

	 }

	 cur = cur->NextDef;
      }

      if (input->IsList)
      {
	 /* keep track of how much memory to allocate */
	 if (used > input->MaxInstances)
	 {
	    input->MaxInstances = used;
	 }
      }
      else if (!used)
      {
	 char            msg[256];

	 sprintf(msg, "Node '%s' does not assign a connection to input '%s'\n", nname, link);
	 SemanticError(msg);
      }
      input = input->NextDef;
   }

   /**************************************/

   /* check that each input link is really an input in the procedured */

   cur = links;
   while (cur)
   {
      t = &Symbols[SymbolTable[cur->SymbolLoc].SymbolLocation];

      /* check that the input exists */
      exists = 0;

      /*
       * search through all inputs in the procedure to see if can find the
       * named input
       */
      input = proc->Parms;
      while (input)
      {
	 link = &Symbols[SymbolTable[input->SymbolLoc].SymbolLocation];

	 /* is this the link? */
	 if (strcmp(t, link) == 0)
	 {
	    exists = TRUE;
	    break;
	 }

	 input = input->NextDef;
      }

      if (!exists)
      {
	 char            msg[256];

	 sprintf(msg, "Node '%s' assigns a connection to input '%s' which does not exist\n", nname, t);
	 SemanticError(msg);
      }

      cur = cur->NextDef;
   }

   /**************************************/

   /* attach the links */
   node->Links = links;

   return node;
}

/********************************************************************
*								    *
*		         check_links			            *
*								    *
********************************************************************/

/* Check_links will verify a chain of links to a node definition */

struct Symbol  *
Check_links(struct Symbol * node)
{
   struct Symbol  *link;

   /* if proc wasn't defined, then can't check links */
   if (node == NULL)
      return node;

   link = node->Links;
   while (link != NULL)
   {
      if (SymbolTable[link->SrcNode].SymbolType == NAME &&
	  (SymbolTable[link->SrcNode].SymbolInfo == NULL ||
	   SymbolTable[link->SrcNode].SymbolInfo->Type != Node))
      {
	 char            msg[256];
	 char           *dname = &Symbols[SymbolTable[link->SymbolLoc].SymbolLocation];
	 char           *sname = &Symbols[SymbolTable[link->SrcNode].SymbolLocation];
	 char           *nname = &Symbols[SymbolTable[node->SymbolLoc].SymbolLocation];

	 sprintf(msg, "Node '%s' input '%s': source '%s' is undefined\n",
		 nname, dname, sname);

	 SemanticError(msg);
      }

      link = link->NextDef;
   }

   return node;
}

/********************************************************************
*								    *
*		         set_link     			            *
*								    *
********************************************************************/

/* set_link will attach an input to an output */

struct Symbol  *
set_link(int parm_name, int node_name)
{
   struct Symbol  *Cur;

   /* get a new symbol record */
   Cur = New_symbol();

   /* initialise the fields that are known */
   Cur->Level = 2;
   Cur->Class = Link;
   Cur->NextDef = NULL;
   Cur->SymbolLoc = parm_name;
   Cur->SrcNode = node_name;

   SymbolTable[parm_name].SymbolInfo = Cur;

#if 0
   printf("   Link  %s <- %s   : Src Type=%d\n",
	  &Symbols[SymbolTable[parm_name].SymbolLocation],
	  &Symbols[SymbolTable[node_name].SymbolLocation],
	  SymbolTable[node_name].SymbolType);
#endif

   return Cur;
}

/********************************************************************
*								    *
*		         create_typed_name		            *
*								    *
********************************************************************/

struct Symbol  *
create_typed_name(int type_loc, int name_loc)
{
   struct Symbol  *cur;

   /* check for duplicate definitions */
   cur = SymbolTable[name_loc].SymbolInfo;
   if (cur == NULL)
   {
      /* Allocate a symbol record for the name */
      cur = New_symbol();

      /* initialise the fields that are known */
      cur->SymbolLoc = name_loc;
      cur->Type = TYPETEXT;
      cur->Level = -1;		/* None */
      SymbolTable[name_loc].SymbolInfo = cur;
   }
   else if (cur->Type_loc != -1 && cur->Type_loc != type_loc)
   {
      char            Text[80];

      sprintf(Text, "symtab.c:create_typed_name - Duplicate definition of %s",
	      &Symbols[SymbolTable[name_loc].SymbolLocation]);
      SyntaxError(Text);
      return NULL;
   }

   cur->Type_loc = type_loc;

   if (cnl_debug)
      print_parm_chain(cur);

   return cur;
}



/********************************************************************
*								    *
*		         StartParmChain			            *
*								    *
********************************************************************/

/* StartParmChain will start a chain of parameter definitions */

struct Symbol  *
StartParmChain(int IdType, struct Symbol * name, int islist)
{
   if( name == NULL )
      return NULL;

   char            Text[80];
   struct Symbol  *proc;

   /* check for duplicate definitions */
   if( name->Level == 1 )
   {
      /* check if is an overridden external def */
      proc = SymbolTable[THE_PARENT].SymbolInfo;
      if (proc->Type != OVERRIDE)
      {
	 char *PName = &Symbols[SymbolTable[name->SymbolLoc].SymbolLocation];

	 sprintf(Text, "symtab.c:StartParmChain - Duplicate definition of %s",
		 PName);
	 SyntaxError(Text);
	 return NULL;
      }
   }

   /* initialise the fields that are known */
   name->Level = 1;
   name->Class = Param;
   name->Type = IdType;
   name->Offset = ParmOffset++;

   name->NextDef = NULL;
   name->IsList = islist;

   if (cnl_debug)
      print_parm_chain(name);

   return name;
}


/********************************************************************
*								    *
*		         AddParmChain			            *
*								    *
********************************************************************/

/* AddParmChain will add to a chain of parameter definitions */

struct Symbol  *
AddParmChain(struct Symbol * chain, struct Symbol * rec)
{
   if (rec == NULL || rec->Type == OVERRIDE)
      return chain;

   rec->NextDef = chain;

   return rec;
}


/********************************************************************
*								    *
*		         StartProcedure			            *
*								    *
********************************************************************/

/* StartProcedure will define the function type */

struct Symbol  *
StartProcedure(int IdType, struct Symbol * name, bool newstyle)
{
   if( name == NULL )
      return NULL;

   char  *PName = &Symbols[SymbolTable[name->SymbolLoc].SymbolLocation];

   if (cnl_debug)
      fprintf(stderr, "Procedure %s\n", PName);

   if (pass == PASS_1)
   {
      /* check for duplicate definitions */
      if (name->Level == 0)
      {
	 if (name->Type == EXTERN && IdType == LOCAL)
	 {
	    /* then saw an extern and now the real proc so override def */
	    IdType = OVERRIDE;
	 }
	 else
	 {
	    char            Text[256];

	    sprintf(Text, "Duplicate definition of procedure %s", PName);
	    SyntaxError(Text);
	    return NULL;
	 }
      }
      else
      {
         /* initialise the fields */
	 name->NextDef = NULL;
         name->Level = 0;
      }

      THE_PARENT = name->SymbolLoc;
      name->Type = IdType;	/* extern or local */
      if( newstyle )
         name->node_type |= NODE_TYPE_NEW_STYLE_PROC;
   }
   else
   {
      THE_PARENT = name->SymbolLoc;
   }

   return name;
}

/**********************************************************************
*                                                                     *
*                  isNewStyle()                                       *
*                                                                     *
**********************************************************************/

// Returns true if the procedure is a new style nprocedure.
bool isNewStyle(struct Symbol *name)
{
   return (name->node_type & NODE_TYPE_NEW_STYLE_PROC) != 0;
}

/**********************************************************************
*                                                                     *
*                  StartBody()                                        *
*                                                                     *
**********************************************************************/

/* StartBody emits the top of the loop */

struct Symbol  *
StartBody(struct Symbol * item)
{
   if (pass == PASS_2)
      start_body(item);

   return item;
}

/**********************************************************************
*                                                                     *
*                  End_Proc_Dec()                                     *
*                                                                     *
**********************************************************************/

/* End_Header will close the function declarations */

struct Symbol  *
End_Proc_Dec(struct Symbol * item, struct Symbol * parms, int type)
{
   if( item == NULL )
      return NULL;

   if (pass == PASS_1)
   {
      if (item->Type != OVERRIDE)
      {
	 /* keep a pointer to the inputs */
	 item->Parms = parms;
      }
   }
   else
   {
      print_proc(item, type);
   }

   if (cnl_debug)
   {
      fprintf(stderr, "End_header parm chain:\n");
      print_parm_chain(item->Parms);
   }

   return item;
}

/**********************************************************************
*                                                                     *
*                  End_Once_Code()                                    *
*                                                                     *
**********************************************************************/
// End_Once_Code will close the once code and start the header

void End_Once_Code(int type, bool new_style)
{
    if (pass == PASS_2)
    {
        close_once_code(type, new_style);
    }
}

/********************************************************************
*								    *
*		         EndProcedure			            *
*								    *
********************************************************************/

/* EndProcedure */

struct Symbol  *
EndProcedure(int type, struct Symbol * item)
{
   /* generate the code for the end of the function */
   if (pass == PASS_2)
      end_procedure(type, item);

   /* reset counters */
   ParmOffset = 0;
   LocalOffset = 0;
   MaxLocalOffset = 0;

   THE_PARENT = NO_PARENT;

   return item;
}


/********************************************************************
*								    *
*		         AddNodeChain			            *
*								    *
********************************************************************/

/* AddNodeChain will add to a chain of node definitions */

struct Symbol  *
AddNodeChain(struct Symbol * NextRec, struct Symbol * Cur)
{
   struct Symbol  *RtnVal;
   struct Symbol  *p;

   if (Cur != NULL)
   {
      Cur->NextDef = NextRec;
      RtnVal = Cur;
   }
   else
   {
      RtnVal = NextRec;
   }

   if (cnl_debug)
   {
      fprintf(stderr, "node chain: ");

      p = RtnVal;
      while (p)
      {
	 fprintf(stderr, "%s ", &Symbols[SymbolTable[p->SymbolLoc].SymbolLocation]);
	 p = p->NextDef;
      }
      printf("\n");
   }

   return RtnVal;
}


/********************************************************************
*								    *
********************************************************************/

int
def_remote_node(int module_ptr, int node_ptr)
{
   struct Symbol  *Cur = NULL;
   char           *node_name;
   char           *module_name;
   char            Text[80];
   int             Ok;

   Ok = 1;
   node_name = &Symbols[SymbolTable[node_ptr].SymbolLocation];
   module_name = &Symbols[SymbolTable[module_ptr].SymbolLocation];

   if (pass == PASS_1)
   {
      if (SymbolTable[node_ptr].SymbolInfo)
      {
	 sprintf(Text, "Node %s is already defined in module %s",
		 node_name, module_name);
	 SyntaxError(Text);
	 return node_ptr;
      }

      /* get a new symbol record */
      Cur = New_symbol();

      /* initialise the fields that are known */
      Cur->Level = 1;
      Cur->Type = Node;
      Cur->NextDef = NULL;
      Cur->SymbolLoc = node_ptr;
      Cur->node_type |= NODE_TYPE_REMOTE;
      Cur->process_loc = module_ptr;

      SymbolTable[node_ptr].SymbolInfo = Cur;
   }
   else
   {
      Cur = SymbolTable[node_ptr].SymbolInfo;

      if (Cur == NULL)
      {
	 fprintf(stderr, "phase error: node %s is not defined", node_name);
	 exit(1);
      }

   }

   return node_ptr;
}


/********************************************************************
*								    *
*		         StartNode			            *
*								    *
********************************************************************/

struct Symbol  *
StartNode(int node_ptr, int proc_ptr, int pub, char *desc)
{
   struct Symbol  *Cur = NULL;
   struct Symbol  *proc;
   char           *node_name;
   char           *proc_name;
   char            Text[80];

   node_name = &Symbols[SymbolTable[node_ptr].SymbolLocation];
   proc_name = &Symbols[SymbolTable[proc_ptr].SymbolLocation];

   if (cnl_debug)
   {
      fprintf(stderr, "Node %s using proc %s\n", node_name, proc_name);
   }

   if (pass == PASS_1)
   {
      /* check for duplicate definitions */
      Cur = SymbolTable[node_ptr].SymbolInfo;

      if (Cur != NULL)
      {
	 if (Cur->node_type & NODE_TYPE_REMOTE)
	 {
	    char *module_name = &Symbols[SymbolTable[SymbolTable[node_ptr].SymbolInfo->process_loc].SymbolLocation];

	    if (pub)
	    {
	       sprintf(Text, "Node %s:%s is defined both public and remote",
		       module_name, node_name);
	       SyntaxError(Text);
	       return NULL;
	    }

	    if (Cur->Proc != NULL)
	    {
	       sprintf(Text, "Duplicate definition of remote node %s:%s",
		       module_name, node_name);
	       SyntaxError(Text);
	       return NULL;
	    }
	 }
	 else if (Cur->Level != 0)
	 {
	    fprintf(stderr, "symtab.c:StartNode - Level=%d for node %s",
		    Cur->Level, node_name);
	    SyntaxError("Internal error");
	    return NULL;
	 }
	 else
	 {
	    sprintf(Text, "symtab.c:StartNode - Duplicate definition of node %s", node_name);
	    SyntaxError(Text);
	    return NULL;
	 }
      }

      if ((proc = SymbolTable[proc_ptr].SymbolInfo) == NULL)
      {
	 sprintf(Text, "Procedure %s referenced but not defined", proc_name);
	 SyntaxError(Text);
	 return NULL;
      }

      if (Cur == NULL)
      {
	 /* get a new symbol record */
	 Cur = New_symbol();
	 SymbolTable[node_ptr].SymbolInfo = Cur;

	 Cur->Level = 0;
      }

      /* initialise the fields */
      Cur->Type = Node;
      Cur->NextDef = NULL;
      Cur->SymbolLoc = node_ptr;
      Cur->Proc = proc;
      Cur->description = desc;
      if (pub)
	 Cur->node_type |= NODE_TYPE_PUBLIC;
   }
   else
   {
      Cur = SymbolTable[node_ptr].SymbolInfo;

      if (Cur == NULL)
      {
	 fprintf(stderr, "phase error: node %s is not defined", node_name);
	 exit(1);
      }

   }

   return Cur;
}


/********************************************************************
*								    *
*		         EndProcs   			            *
*								    *
********************************************************************/

/* EndProcs will end the procedure section and start the node section */

struct Symbol  *
EndProcs(struct Symbol * procs)
{
   if (pass == PASS_1)
   {
      if (cnl_debug)
      {
	 printf("\n============Procedures defined in pass 1==========\n");
	 print_proc_chain(procs);

	 printf("\n============Modules defined ==========\n");
	 struct Symbol *cur = Module_Chain;
         while (cur)
         {
            printf("	%s\n",&Symbols[SymbolTable[cur->SymbolLoc].SymbolLocation]);
            cur = cur->NextDef;
         }
	 printf("\n");
      }

      /* print the structure definitions for the procedures */
      emit_defs(procs);
   }
   else
   {
      end_procs();
   }

   THE_PARENT = NO_PARENT;

   return procs;
}


/********************************************************************
*								    *
*		         EndNodes   			            *
*								    *
********************************************************************/

/* EndNodes will end the procedure section and start the node section */

void
EndNodes(struct Symbol * procs, struct Symbol * nodes)
{
   int             num_threads;

   if (pass == PASS_2)
   {
      num_threads = emit_nodes(procs, nodes);

      end_nodes(num_threads);
   }
}



/**********************************************************************
 * $Log: symtab.c,v $
 * Revision 1.1.1.1  2008/07/14 16:44:17  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/20 17:17:47  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:37:55  endo
 * MissionLab 7.0
 *
 * Revision 1.2  2005/02/07 23:45:47  endo
 * Mods from usability-2004
 *
 * Revision 1.1.1.1  2005/02/06 22:59:38  endo
 * AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
 *
 * Revision 1.28  1996/04/17  15:47:46  doug
 * *** empty log message ***
 *
 * Revision 1.27  1996/02/14  17:12:53  doug
 * added support for descriptions
 * ./
 *
 * Revision 1.26  1995/11/01  23:00:28  doug
 * added the tcb class
 *
 * Revision 1.25  1995/10/18  13:59:36  doug
 * added support for new style procs
 *
 * Revision 1.24  1995/06/15  22:01:13  jmc
 * Added RCS ID and Log strings.
 **********************************************************************/
