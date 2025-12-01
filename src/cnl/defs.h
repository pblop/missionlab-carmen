/*      Douglas C. MacKenzie

        defs.h ----  definition include file for CNL.

        Copyright 1995 - 2005, Georgia Tech Research Corporation
        Atlanta, Georgia  30332-0415
        ALL RIGHTS RESERVED, See file COPYRIGHT for details.

	$Id: defs.h,v 1.1.1.1 2008/07/14 16:44:17 endo Exp $
*/

#include <stdio.h>
#include <ctype.h>
#include <string.h>

#define  FALSE	0
#define  TRUE   (!FALSE)

#define NO_PARENT            -1    /* used in hash routines */
#define EMPTY                 0    /* used in hash routines */
#define SymbolTableSize    2048    /* Size for symbol table (Closed hashing) */
#define TypeTableSize       256
#define MaxSymbols   (1024*1024)   /* Size for character array to store syms */
#define MAX_NAME_LEN	    256    /* max length of an identifier */
#define MAX_TYPESTRING	    256    /* max length of a type declaration */
#define MAX_LINE_LEN	    512    /* max length of an input line */

/* define extra types */
#define None		       0

/* defines for class types */
#define EXTERN		       1
#define LOCAL		       2
#define OVERRIDE	       3
#define INPUTS		       4
#define Param		       5
#define Constant	       6
#define Node	       	       7
#define Link	       	       8
#define TYPETEXT	       9
#define Variable	      10



#define LOCAL_PROCESS		-1	/* Used in startnode to say not remote*/

struct Symbol
         {
         int   Level;
         int   Class;
         int   Type;
	 int   Type_loc;
         int   Offset;
         struct Symbol *NextDef;
         struct Symbol *Parms;
         struct Symbol *Proc;
         struct Symbol *Links;
         int   SrcNode;
         int   SymbolLoc;

         int   IsList;
         int   ThisInstance;
         int   MaxInstances;
	 int   node_type;	/* Bit field */
				/*    Bit 0: Is this node public? */
				/*    Bit 1: Is this (src) node remote? */
				/*    Bit 2: Is this a new style procedure? */
         int   process_loc;	/*if node_type is remote, then is process name*/
	 const char *type_format_string;
	 char *description;     /* NULL if none. */
         };

/* bit masks for the node_type field */
#define NODE_TYPE_PUBLIC 	 (1<<0)
#define NODE_TYPE_REMOTE 	 (1<<1)
#define NODE_TYPE_NEW_STYLE_PROC (1<<2)

struct SymbolTableEntry
          {
          int  SymbolType;
          int  SymbolLocation;
          int  Parent;
          struct Symbol * SymbolInfo;
          };

extern char Symbols[MaxSymbols];           /* memory to store symbols */
extern int  NextSymbolLoc;                 /* pointer to start of free in Symbols */
extern struct SymbolTableEntry SymbolTable[SymbolTableSize];
extern int     TypeTable[TypeTableSize];
extern int     THE_PARENT;

extern FILE *outfile;
extern char outname[256];
extern char *inname;

extern int yydebug;		/* set to TRUE to get yacc debug info */
extern int cnl_debug;		/* set to TRUE to get compiler debug info */
extern int compile;		/* set to TRUE to compile procedures only */
extern int void_type;		/* so can check if void */

extern int pass;
#define PASS_1		1
#define PASS_2		2


/* Used in the generated code to determine the type of input link */
#define SRC_TYPE_LOCAL_NODE 	0
#define SRC_TYPE_REMOTE 	1
#define SRC_TYPE_CONSTANT 	2


/* external procs */
char *symbol_name(int loc);
int AddString(const char *Str, int TokenType, int the_parent);
int AddNumber(char *Str,int parent);
int AddChar(char *Str,int parent);
int AddInitializer(char *Str,int parent);
int AddName(char *Str,int parent);
int symbol_type(int loc);
int LookupName(char *Str,int parent);
int AddType(const char *Str, const char *format);
int AddModule(char *Str);
char *strip_quotes(char *in);


extern char 	filename[MAX_LINE_LEN];
extern int 	lineno;
extern int 	tokenpos;
extern char 	linebuf[MAX_LINE_LEN];
extern int      had_error;


void file_reset(void);
void SyntaxError(const char *Message);
void SemanticError(const char *Message);
void yyerror(char *s);
int  yylex(void);
int yyparse (void);

void EndNodes(struct Symbol *procs, struct Symbol *nodes);
struct Symbol *EndProcs(struct Symbol *procs);
struct Symbol *AddParmChain(struct Symbol * chain, struct Symbol * rec);
struct Symbol *EndProcedure(int type, struct Symbol *item);
struct Symbol *StartBody(struct Symbol *item);
struct Symbol *End_Proc_Dec(struct Symbol *item,struct Symbol *parms,int type);
void End_Once_Code(int type, bool new_style);
struct Symbol *StartProcedure(int IdType, struct Symbol *name, bool newstyle);

struct Symbol *AddNodeChain(struct Symbol * NextRec, struct Symbol * Cur);
struct Symbol *Check_links(struct Symbol * node);
struct Symbol *StartNode(int node_ptr,int proc_ptr,int pub, char *desc);
struct Symbol *StartParmChain(int IdType, struct Symbol *name, int islist);

struct Symbol *set_link(int parm_name, int node_name);
struct Symbol *Attach_links(struct Symbol * node, struct Symbol * links);
struct Symbol *New_symbol(void);
struct Symbol *create_typed_name(int type_loc, int name_loc);
int def_remote_node(int module_ptr, int node_ptr);

// Returns true if the procedure is a new style nprocedure.
bool isNewStyle(struct Symbol *name);



void print_proc(struct Symbol *item, int type);
void outstring(const char *str);
void outline(const char *str);
void ResetLineNumber(void);
void output_header(int argc, char **argv);
void start_body(struct Symbol *item);
void end_procedure(int type, struct Symbol * Item);
void end_procs(void);
void emit_defs(struct Symbol * nodes);
int  emit_nodes(struct Symbol *procs, struct Symbol *nodes);
int end_nodes(int num_threads);

void  close_once_code (int type, bool new_style);




extern struct Symbol *Type_Chain;
extern struct Symbol *Module_Chain;
extern int num_modules;
extern char *module_name;

extern FILE *yyin;

void set_file(int num);
extern int num_files;
extern int cur_file;
extern int generate_line_defines;

extern char include_paths[8][256];
extern int  num_include_paths;
extern int run_preprocessor;
extern int verbose;
extern bool no_builtin_bool;


/**********************************************************************
 * $Log: defs.h,v $
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
 * Revision 1.29  1996/04/17  15:47:46  doug
 * *** empty log message ***
 *
 * Revision 1.28  1996/02/14  17:12:53  doug
 * added support for descriptions
 * ./
 *
 * Revision 1.27  1996/02/09  01:49:36  doug
 * *** empty log message ***
 *
 * Revision 1.26  1996/02/08  09:18:53  doug
 * added support to skim include files without expanding them if
 * didn't run the preprocessor.
 *
 * Revision 1.25  1995/11/01  23:00:28  doug
 * added the tcb class
 *
 * Revision 1.24  1995/10/18  13:59:36  doug
 * added support for new style procs
 *
 * Revision 1.23  1995/06/15  21:59:00  jmc
 * Added RCS ID and Log strings.
 **********************************************************************/
