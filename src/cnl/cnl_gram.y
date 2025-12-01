%{
/*      Douglas C. MacKenzie
        gram.y ----  yacc grammer file for Configuration Network Language.

        Copyright 1995 - 2005, Georgia Tech Research Corporation 
        Atlanta, Georgia  30332-0415
        ALL RIGHTS RESERVED, See file COPYRIGHT for details. 

	$Id: cnl_gram.y,v 1.1.1.1 2008/07/14 16:44:17 endo Exp $
*/


#include <stdlib.h>
#include "defs.h" /* include the definitions file */

int  save_parent;
%}

%union {
   struct Symbol *symbol;
   int            loc;
   char          *string;
}

/* The tokens which lex will return */
%token <loc> NAME
%token <loc> STRING
%token <loc> TYPE
%token <loc> MODULE
%token <loc> NUMBER
%token <loc> CHARACTER
%token <loc> INITIALIZER
%token <string> DESCRIPTION

%token MODULE
%token MODULENAME
%token PROCEDURE
%token NPROCEDURE
%token PUBLIC
%token WITH   
%token EXTERNAL 
%token REMOTE 
%token CODE   
%token PEND
%token NODE    
%token IS  
%token COLON  
%token NEND  
%token LA  
%token RA 
%token LB
%token RB        
%token COMMA  
%token SEMICOLON 
%token ASSIGN 
%token BODY 
%token HEADER 
%token ONCE 
%token INIT 
%token INIT_CODE   
%token IEND 
%token TYPEDEF 
%token MODDEF 
%token LIST 

%type <loc>    linkstart def_remote_node lookup_remote_node
%type <symbol> nodelist first eprocedure lprocedure ehead lheader
%type <symbol> indefs node nodestart eproc lproc lstart lonce
%type <symbol> inlinks link typed_name initend 
%type <string> Desc  


/* The grammer starts here */

%%
configuration	: initend nodelist 	
			{
			   EndNodes($1,$2); 
			}
		| initend 		
			{ 
			   if( pass==PASS_1 && !compile ) 
                              fprintf(stderr,"Error: No nodes declared\n");
			}
		;

initend		: first
			{
			   $$ = EndProcs($1); 
			}
		;

first		: typedef			{$$ = NULL;}
		| mod_name			{$$ = NULL;}
		| mod_def			{$$ = NULL;}
		| init 				{$$ = NULL;}
		| eprocedure 			{$$ = $1;}
		| lprocedure 			{$$ = $1;}
		| first typedef			{$$ = $1;}
		| first mod_name		{$$ = $1;}
		| first mod_def			{$$ = $1;}
		| first init			{$$ = $1;}
		| first eprocedure		{$$ = AddParmChain($1,$2);}
		| first lprocedure		{$$ = AddParmChain($1,$2);}
		;

typedef		: TYPEDEF LA NAME COMMA STRING RA SEMICOLON 	{
                    if( pass == PASS_1 )
			AddType(symbol_name($3),strip_quotes(symbol_name($5))); 
		    else
		        fprintf(stderr,"Interpass alignment error 1\n");
		}
		| TYPEDEF LA TYPE COMMA STRING RA SEMICOLON 	{
                   if( pass == PASS_1 )
		   {
		      char msg[256];
                      sprintf(msg,
			 "Typedef declares a name which is already a type '%s'",
			 symbol_name($3));

		      SyntaxError(msg);
		   }
		}
		;

mod_def		: MODDEF NAME SEMICOLON 	{
                    if( pass == PASS_1 )
			AddModule(symbol_name($2)); 
		    else
		        fprintf(stderr,"Interpass alignment error 2\n");
		}
		| MODDEF MODULE SEMICOLON 	{
                   if( pass == PASS_1 )
		   {
		      char msg[256];
                      sprintf(msg,
			 "Moduledef declares a name which is already a module '%s'",
			 symbol_name($2));

		      SyntaxError(msg);
		   }
		   /* Undo the lex set of the_parent when saw the module */
                   THE_PARENT = NO_PARENT;
		}
		;

mod_name	: MODULENAME ASSIGN NAME SEMICOLON 	
   {
      if( pass == PASS_1 )
      {
         if( module_name != NULL )
	 {
	    char msg[256];
            sprintf(msg, "Duplicate module name specification");
            SyntaxError(msg);
	 }
	 else
	 {
            module_name = symbol_name($3); 
	 }
      }
   }
		;

init		: INIT icode IEND 
		| INIT IEND 		
		;

icode		: icode INIT_CODE
		| INIT_CODE
		;

eprocedure	: ehead PEND {$$ = EndProcedure(EXTERN,$1);}
		;

lprocedure	: lheader code PEND {$$ = EndProcedure(LOCAL,$1);}
		| lheader PEND {$$ = EndProcedure(LOCAL,$1);}
		;

code		: code CODE
		| CODE
		;

lheader		: lstart BODY 		{$$ = StartBody($1);}
		| lstart code BODY	{$$ = StartBody($1);}
		;

ehead		: eproc indefs 
   {
      $$ = End_Proc_Dec($1,$2,EXTERN); 
      End_Once_Code(EXTERN, isNewStyle($1));
   }
		| eproc 
   {
      $$ = End_Proc_Dec($1,NULL,EXTERN);
      End_Once_Code(EXTERN, isNewStyle($1));
   }
		;

lstart		: lonce code HEADER {
      End_Once_Code(LOCAL, isNewStyle($1));
   }
		| lonce HEADER {
      End_Once_Code(LOCAL, isNewStyle($1));
   }
		| lproc indefs HEADER {
      $$ = End_Proc_Dec($1,$2,LOCAL);
      End_Once_Code(LOCAL, isNewStyle($1));
   }
		| lproc HEADER {
      $$ = End_Proc_Dec($1,NULL,LOCAL);
      End_Once_Code(LOCAL, isNewStyle($1));
   }
		;

lonce		: lproc indefs ONCE 
   { 
      $$ = End_Proc_Dec($1,$2,LOCAL);
   }
		| lproc ONCE 
   { 
      $$ = End_Proc_Dec($1,NULL,LOCAL);
   }
		;

eproc		: EXTERNAL PROCEDURE typed_name WITH 
   {
      $$ = StartProcedure(EXTERN,$3,false); 
   }
		;

lproc		: PROCEDURE typed_name WITH 
   {
      $$ = StartProcedure(LOCAL,$2,false); 
   }
		| NPROCEDURE typed_name WITH 
   {
      $$ = StartProcedure(LOCAL,$2,true); 
   }
		;

nodelist	: node
		                {
		                   $$ = $1;
				}
		| nodelist node
		                {
		                   $$ = AddNodeChain($1,$2);
				}
		;

node		: nodestart inlinks NEND 
		                { 
                                   if( pass == PASS_1 )
				   {
				      $$ = Attach_links($1,$2); 
				   }
				   else
				   {
				      $$ = Check_links($1); 
				   }
				   THE_PARENT = NO_PARENT;
				}
		| nodestart NEND 
		                { 
                                   if( pass == PASS_1 )
				   {
				      $$ = Attach_links($1, NULL); 
				   }
				   else
				   {
				      $$ = Check_links($1); 
				   }
				   THE_PARENT = NO_PARENT;
				}
		| REMOTE NODE def_remote_node IS NAME
   {
      $$ = StartNode($3,$5,FALSE,NULL); 
      THE_PARENT = NO_PARENT;
   }
		| REMOTE NODE NAME COLON NAME IS NAME
   {
      char msg[256];
      sprintf(msg,"Undefined module %s",symbol_name($3));
      SyntaxError(msg);
   }
		;

def_remote_node	: MODULE COLON NAME
   {
      $$ = def_remote_node($1,$3); 
      THE_PARENT = NO_PARENT;
   }
		;

Desc            : DESCRIPTION {
   $$ = $1;
}
                | {
   $$ = NULL;
}
                ;


nodestart	: NODE NAME IS NAME Desc WITH 
				{
				   $$=StartNode($2,$4,FALSE,$5); 
				   THE_PARENT = $4;
				}
		| PUBLIC NODE NAME IS NAME Desc WITH 
				{
				   $$=StartNode($3,$5,TRUE,$6); 
				   THE_PARENT = $5;
				}
		;

typed_name	: TYPE NAME 
   {
      if( pass == PASS_1 )
         $$ = create_typed_name($1,$2);
      else
	 $$ = SymbolTable[$2].SymbolInfo;

   }
		| NAME NAME 
   {
      char msg[256];
      sprintf(msg,"Unknown type %s",symbol_name($1));
      SyntaxError(msg);
      $$ = SymbolTable[$2].SymbolInfo;
   }
		;

indefs		: indefs typed_name SEMICOLON
		                {
                                   if( pass == PASS_1 )
		                      $$ = AddParmChain($1,StartParmChain(INPUTS,$2,FALSE));
				}
		| indefs LIST typed_name SEMICOLON
		                {
                                   if( pass == PASS_1 )
		                      $$ = AddParmChain($1,StartParmChain(INPUTS,$3,TRUE));
				}
		| typed_name SEMICOLON	
				{
                                   if( pass == PASS_1 )
		                      $$ = StartParmChain(INPUTS,$1,FALSE);
				}
		| LIST typed_name SEMICOLON	
				{
                                   if( pass == PASS_1 )
		                      $$ = StartParmChain(INPUTS,$2,TRUE);
				}
		;

inlinks		: inlinks link		
				{
                                   if( pass == PASS_1 )
				      $$ = AddParmChain($1,$2);
				}
		| link                  
				{$$ = $1;}
		;

lookup_remote_node : MODULE COLON NAME
   {
      $$ = $3; 
      THE_PARENT = NO_PARENT;
   }
		;

link		: linkstart NAME SEMICOLON	
				{ 
                                   THE_PARENT = save_parent;
                                   if( pass == PASS_1 )
				     $$ = set_link($1,$2); 
				}
		| linkstart lookup_remote_node SEMICOLON	
				{ 
                                   THE_PARENT = save_parent;
                                   if( pass == PASS_1 )
				     $$ = set_link($1,$2); 
				}
		| linkstart NAME COLON NAME SEMICOLON	
   {
      char msg[256];
      sprintf(msg,"Unknown module %s",symbol_name($2));
      SyntaxError(msg);
      $$ = NULL;
   }
 		| linkstart NUMBER SEMICOLON	
				{ 
                                   THE_PARENT = save_parent;
                                  if( pass == PASS_1 )
				     $$ = set_link($1,$2); 
				}
 		| linkstart CHARACTER SEMICOLON	
				{ 
                                   THE_PARENT = save_parent;
                                  if( pass == PASS_1 )
				     $$ = set_link($1,$2); 
				}
 		| linkstart INITIALIZER SEMICOLON	
				{ 
                                   THE_PARENT = save_parent;
                                  if( pass == PASS_1 )
				     $$ = set_link($1,$2); 
				}
 		| linkstart STRING SEMICOLON	
				{ 
                                   THE_PARENT = save_parent;
                                  if( pass == PASS_1 )
				     $$ = set_link($1,$2); 
				}
		;

linkstart	: NAME ASSIGN 	
				{
				   $$ = $1;

				   /* need to look for the proc name outside 
				      of the parent's context 
				   */
				   save_parent = THE_PARENT;
                                   THE_PARENT = NO_PARENT;
				}
		;
%%

/**********************************************************************
 * $Log: cnl_gram.y,v $
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
 * Revision 1.29  2003/04/06 11:42:44  endo
 * gcc 3.1.1
 *
 * Revision 1.28  2002/11/08 16:20:09  endo
 * Rolled back to cancel ARBITRATE.
 *
 * Revision 1.27  2002/11/07 22:09:11  blee
 * added an eproc rule to accept external nprocedure declarations.
 *
 * Revision 1.26  2000/10/16 19:14:48  endo
 * Modification due to the compiler upgrade.
 *
 * Revision 1.25  1996/04/17 15:47:46  doug
 * *** empty log message ***
 *
 * Revision 1.24  1996/02/14  17:12:53  doug
 * added support for descriptions
 * ./
 *
 * Revision 1.23  1995/10/18  13:59:36  doug
 * added support for new style procs
 *
 * Revision 1.22  1995/06/28  19:11:18  jmc
 * Moved header info into %{ %} so that it would appear in the
 * generated file.
 *
 * Revision 1.21  1995/06/15  21:55:55  jmc
 * Added RCS ID and Log strings.
 **********************************************************************/
