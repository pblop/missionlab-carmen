%{
/*
     cdl_lex.l : lex patterns for the Configuration Description Language (CDL) 

     By: Douglas C. MacKenzie

     Copyright 1995, Georgia Tech Research Corporation
     Atlanta, Georgia  30332-0415
     ALL RIGHTS RESERVED, See file COPYRIGHT for details.
*/

/* $Id: cdl_lex.l,v 1.1.1.1 2008/07/14 16:44:22 endo Exp $ */


#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "cdl_defs.h"
#include "symtab.h"
#include "cdl_gram.tab.h"
#include "prototypes.h"
#include "utilities.h"


int lineno = 1;
int tokenpos = 0;
int tokenlen = 0;

const int MAX_LINE_LEN = 512;
char linebuf[MAX_LINE_LEN];
char *const_str;

#define MAX_INCLUDE_DEPTH       64
static char *filename_stack[MAX_INCLUDE_DEPTH];
static int lineno_stack[MAX_INCLUDE_DEPTH];
static int include_stack_ptr = 0;

static symbol_table<char> ExternalAnonymousNames;
   
/***************************************************************/

/* Get rid of "defined but not used" warnings from gcc */
#define YY_NO_UNPUT


static int preloading = true;
static const char preload_buffer[] = 
#include "preload.cdl" 
;
static const char *preload_ptr = preload_buffer;
const char *filename = "Preload Buffer";

extern int filenum;

/********************************************************************
*                                                                   *
*                        yywrap                                     *
*                                                                   *
********************************************************************/

/* yywrap will always return true, indicating that no other file follows */

#define YY_SKIP_YYWRAP

int
yywrap(void)
{
   // New table for each file
   ExternalAnonymousNames.clear();

   if( preloading )
   {
      // If had an error in the preload, then no hope for the real file.
      if( had_error )
	 return 1;

      preloading = false;
      return 1;  /* Stop after preload */
   }

   // Else: load the next file
   return next_file();
}

/********************************************************************
*                                                                   *
*                        yyinit                                     *
*                                                                   *
********************************************************************/

/* yyinit will get ready to scan a new file */

void
yyinit(bool preload)
{
   preloading = preload;

   lineno = 1;
   tokenpos = 0;
   tokenlen = 0;
}

/********************************************************************
*                                                                   *
*                        YY_INPUT                                   *
*                                                                   *
********************************************************************/

#define YY_INPUT(buf,result,max_size) \
{ \
   if( preloading ) \
   { \
      for(result=0; result<max_size && *preload_ptr; result++) \
         buf[result] = *preload_ptr++; \
   } \
   else \
   { \
      if( ((result = fread( buf, 1, max_size, yyin )) == 0) && ferror(yyin)) \
         YY_FATAL_ERROR( "input in flex scanner failed" ); \
   } \
}


/*******************************************************************/

/* handle C preprocessor line/file change comments */
static void line_numbers(char *line)
{
   int op=0, rtn, line_number;
   char name[256];

   rtn = sscanf(line+1, "%d \"%[^\"]\" %d", &line_number, name, &op);

   if( rtn < 2 )
   {
      if( strstr(line, "pragma") != NULL )
      {
	 /* skip pragmas */
      }
      else
      {
         SyntaxError("Unknown # command");
      }
   }
   else
   {
      if( rtn >= 3 )
      {
         /* op is valid */
         if( op == 1 )
         {
            /* push */
            if( include_stack_ptr >= MAX_INCLUDE_DEPTH )
            {
               fprintf(stderr,"Includes nested too deeply\n");
            }
            else
            {
               lineno_stack[include_stack_ptr] = lineno;
               filename_stack[include_stack_ptr]= strdup(filename);
               include_stack_ptr++;
            }
         }
         else if( op == 2 )
         {
            /* pop */
            if( include_stack_ptr > 0 )
            {
               include_stack_ptr--;
               free(filename_stack[include_stack_ptr]);
            }
            else
            {
               fprintf(stderr,"Underflow of includes\n");
            }
         }
      }

      /* reset line number and filename */
      lineno = line_number;
      filename = strdup(name);
   }
}

/********************************************************************/

%}

ALPHA 				[a-zA-Z]
NUMERIC	 			[0-9]

/********************************************************************/
/* declare states */
%x ST_COMMENT
%x ST_CONST


/********************************************************************/
/* the patterns */
%%

"(" 					{ 
   // Mark starting a parm list
   is_lhs = true;

   tokenlen = yyleng;
   tokenpos+=yyleng;
   return(LP); 
   }

")" 					{
   tokenlen = yyleng;
   tokenpos+=yyleng;
   return(RP); 
   }

"[" 					{ 
   // Mark list of agents
   is_lhs = false;

   tokenlen = yyleng;
   tokenpos+=yyleng;
   return(LS); 
   }

"]" 					{ 
   tokenlen = yyleng;
   tokenpos+=yyleng;
   return(RS); 
   }

"<" 					{ 
   tokenlen = yyleng;
   tokenpos+=yyleng;
   return(LA); 
   }

">" 					{ 
   tokenlen = yyleng;
   tokenpos+=yyleng;
   return(RA); 
   }

"," 					{ 
   tokenlen = yyleng;
   tokenpos+=yyleng;
   return(COMMA); 
   }

";" 					{ 
   tokenlen = yyleng;
   tokenpos+=yyleng;
   return(SEMICOLON); 
   }

"=" 					{ 
   tokenlen = yyleng;
   tokenpos+=yyleng;
   return(ASSIGN); 
   }

"^" 					{
   tokenlen = yyleng;
   tokenpos+=yyleng;
   return(UP); 
   }

from 					{ 
   tokenlen = yyleng;
   tokenpos+=yyleng;
   return(FROM); 
   }

list 					{ 
   tokenlen = yyleng;
   tokenpos+=yyleng;
   return(LIST); 
   }

if 					{ 
   tokenlen = yyleng;
   tokenpos+=yyleng;
   return(IF); 
   }

goto 					{ 
   tokenlen = yyleng;
   tokenpos+=yyleng;
   return(GOTO); 
   }

FSAstyle 				{ 
   tokenlen = yyleng;
   tokenpos+=yyleng;
   return(FSA_STYLE); 
   }

RLstyle 				{ 
	/* DCM 01-13-00: Added to support Reinforcement Learner */
   tokenlen = yyleng;
   tokenpos+=yyleng;
   return(RL_STYLE); 
   }

SELECTstyle 				{ 
   tokenlen = yyleng;
   tokenpos+=yyleng;
   return(SELECT_STYLE); 
   }

MDLstyle 				{ 
   tokenlen = yyleng;
   tokenpos+=yyleng;
   return(MDL_STYLE); 
   }

sensor 					{ 
   tokenlen = yyleng;
   tokenpos+=yyleng;
   return(SENSOR); 
   }

binds 					{ 
   tokenlen = yyleng;
   tokenpos+=yyleng;
   return(BINDS); 
   }

const 					{ 
   tokenlen = yyleng;
   tokenpos+=yyleng;
   return(CONST); 
   }

actuator 				{ 
   tokenlen = yyleng;
   tokenpos+=yyleng;
   return(ACTUATOR); 
   }

defNamedValue 				{ 
   tokenlen = yyleng;
   tokenpos+=yyleng;
   return(DEFNAMEDVALUE); 
   }

DisplayAs 				{ 
   tokenlen = yyleng;
   tokenpos+=yyleng;
   return(DISPLAYAS); 
   }

CheckBox 				{ 
   tokenlen = yyleng;
   tokenpos+=yyleng;
   return(CheckBox); 
   }

RadioBox 				{ 
   tokenlen = yyleng;
   tokenpos+=yyleng;
   return(RadioBox); 
   }

SliderBar 				{ 
   tokenlen = yyleng;
   tokenpos+=yyleng;
   return(SLIDERBAR); 
   }

TwoPoints 				{ 
   tokenlen = yyleng;
   tokenpos+=yyleng;
   return(TwoPoints); 
   }

TwoWorldPoints 				{ 
   tokenlen = yyleng;
   tokenpos+=yyleng;
   return(TwoWorldPoints); 
   }

TypeInBox 				{
   tokenlen = yyleng;
   tokenpos+=yyleng;
   return(TypeInBox); 
   }

TypeInNum 				{
   tokenlen = yyleng;
   tokenpos+=yyleng;
   return(TypeInNum); 
   }

MinMaxNum 				{
   tokenlen = yyleng;
   tokenpos+=yyleng;
   return(MinMaxNum); 
   }

MinMaxText 				{
   tokenlen = yyleng;
   tokenpos+=yyleng;
   return(MinMaxText); 
   }

ForDataType 				{ 
   tokenlen = yyleng;
   tokenpos+=yyleng;
   return(FORDATATYPE); 
   }

defibp|defIBP 				{ 
   tokenlen = yyleng;
   tokenpos+=yyleng;
   return(DEFIBP); 
   }

defobp|defOBP 				{ 
   tokenlen = yyleng;
   tokenpos+=yyleng;
   return(DEFOBP); 
   }

defrbp|defRBP 				{ 
   tokenlen = yyleng;
   tokenpos+=yyleng;
   return(DEFRBP); 
   }

defSensor|defsensor 			{ 
   tokenlen = yyleng;
   tokenpos+=yyleng;
   return(DEFSENSOR); 
   }

defActuator|defactuator 		{ 
   tokenlen = yyleng;
   tokenpos+=yyleng;
   return(DEFACTUATOR); 
   }

defrobot|defRobot 			{ 
   tokenlen = yyleng;
   tokenpos+=yyleng;
   return(DEFROBOT); 
   }

defarch|defArch				{ 
   tokenlen = yyleng;
   tokenpos+=yyleng;
   return(DEFARCH); 
   }

bindarch|bindArch 			{ 
   tokenlen = yyleng;
   tokenpos+=yyleng;
   return(BINDARCH); 
   }

deftype|defType				{ 
   tokenlen = yyleng;
   tokenpos+=yyleng;
   return(DEFTYPE); 
   }

defop|defOp				{ 
   tokenlen = yyleng;
   tokenpos+=yyleng;
   return(DEFCOORD); 
   }

defagent|defAgent 			{ 
   tokenlen = yyleng;
   tokenpos+=yyleng;
   return(DEFAGENT); 
   }

instagent|instAgent			{ 
   tokenlen = yyleng;
   tokenpos+=yyleng;
   return(INSTAGENT); 
   }

instgroup|instGroup			{ 
   tokenlen = yyleng;
   tokenpos+=yyleng;
   return(INSTGROUP); 
   }

instrobot|instRobot			{ 
   tokenlen = yyleng;
   tokenpos+=yyleng;
   return(INSTROBOT); 
   }

instsensor|instSensor			{ 
   tokenlen = yyleng;
   tokenpos+=yyleng;
   return(INSTSENSOR); 
   }

instactuator|instActuator			{ 
   tokenlen = yyleng;
   tokenpos+=yyleng;
   return(INSTACTUATOR); 
   }

instbp|instBP				{ 
   tokenlen = yyleng;
   tokenpos+=yyleng;
   return(INSTBP); 
   }

instop|instOp				{ 
   tokenlen = yyleng;
   tokenpos+=yyleng;
   return(INSTCOORD); 
   }

"|"[^|\n]*"|"	{  // A description
   tokenlen = yyleng;
   tokenpos+=yyleng;
   yylval.string = strdup(yytext+1); 	// NOTE: Trims the bracketing bars
   yylval.string[strlen(yylval.string)-1] = '\0';
   return DESCRIPTION;
   }

"%"{ALPHA}("_"|"-"|{ALPHA}|{NUMERIC})*	{  // A pushed-up parm name
   tokenlen = yyleng;
   tokenpos+=yyleng;
   yylval.string = strdup(yytext); 	// NOTE: Leaves the leading '%'
   return PU_PARM_NAME;
   }

"$"{ALPHA}("_"|"-"|{ALPHA}|{NUMERIC}|"*")*":"	{  // An anonymous inline name
      tokenlen = yyleng;
      tokenpos += yyleng;

      // Need to lookup the name without the colon
      char *p = strdup(yytext);
      p[strlen(p)-1] = '\0'; // Trim the colon

      const char *internal_name = ExternalAnonymousNames.get(p);
      if( internal_name == NULL )
      {
         internal_name = AnonymousName();
         ExternalAnonymousNames.put(p,internal_name);
      }

      // free the string copy we made
      free(p);

      yylval.string = strdup(internal_name);
      return INLINE_NAME;
   }

{ALPHA}("_"|"-"|{ALPHA}|{NUMERIC}|"*")*":"	{  // An inline name
   tokenlen = yyleng;
   tokenpos+=yyleng;
   yylval.string = strdup(yytext);
   yylval.string[strlen(yylval.string)-1] = '\0'; // Trim the colon
   return INLINE_NAME;
   }

"$"{ALPHA}("_"|"-"|{ALPHA}|{NUMERIC}|"*")*		{
      tokenlen = yyleng;
      tokenpos+=yyleng;

      const char *internal_name = ExternalAnonymousNames.get(yytext);
      if( internal_name == NULL )
      {
         internal_name = AnonymousName();
         ExternalAnonymousNames.put(yytext,internal_name);
      }
      yylval.symbol = LookupName(internal_name);

      if( yylval.symbol == NULL )
      {
         yylval.string = strdup(internal_name);
         return NAME;
      }
      return yylval.symbol->symbol_type; 
   }

{ALPHA}("_"|"-"|"."|{ALPHA}|{NUMERIC}|"*")*		{
   tokenlen = yyleng;
   tokenpos+=yyleng;
   yylval.symbol = LookupName(yytext);

   if( yylval.symbol == NULL )
   {
      yylval.string = strdup(yytext);
      return NAME;
   }
   return yylval.symbol->symbol_type; 
   }

"-"?{NUMERIC}+("."{NUMERIC}*)?	{ 
   tokenlen = yyleng;
   tokenpos+=yyleng;
   yylval.number = atof(yytext);

   return NUMBER;
   }

"{^"					{ // start of pushed up initializer
   tokenlen = yyleng;
   tokenpos+=yyleng;

   return START_PU_INITIALIZER;
   }

"{@^"					{ // start of pushed up initializer for an advanced parameter
   tokenlen = yyleng;
   tokenpos+=yyleng;

   return START_PU_INITIALIZER_ADV;
   }

"}"					{ // right bracket (likely end of PUI
   tokenlen = yyleng;
   tokenpos+=yyleng;

   return RB;
   }

"{&"					{
   tokenlen = yyleng;
   tokenpos+=yyleng;

   return START_PAIR_PU_PARM_NAMES;
}

"{"                 			{
  BEGIN(ST_CONST);
  const_str = NULL;
  tokenlen = yyleng;
  tokenpos+=yyleng;
 }

<ST_CONST>[^}\n]*                     { 
   tokenlen = yyleng;
   tokenpos+=yyleng;
   const_str = strdupcat(const_str,yytext);
 }

<ST_CONST>"\n"                        {
   if( yyleng >= MAX_LINE_LEN )
   {
      SyntaxError("Length of line too long: Skipping buffering\n");
      memcpy(linebuf, yytext+1, MAX_LINE_LEN-1);
      linebuf[MAX_LINE_LEN-1] = '\0';
   }
   else
   {
      strcpy(linebuf, yytext+1); // save the next line
   }
   lineno++; /* count them */    
   tokenpos=0; /* start of line */
   yyless(1); /* give back all but the \n to rescan */ 

   // Remind lex we are at the begining of a line so the caret works
   yy_set_bol(true);

   const_str = strdupcat(const_str,"\n");
 }

<ST_CONST>"}"     		{
  tokenlen = yyleng;
  tokenpos+=yyleng;
  BEGIN(INITIAL);
  yylval.string = const_str;
  return INITIALIZER;
 }


^"#"[^\n]*\n  { 
   line_numbers(yytext);
   }


"\t"					{ 
   /* figure out where next tab stop is located */ 
   int tabstop = 8 * ((tokenpos + 8) / 8);
   tokenlen = tabstop - tokenpos;
   tokenpos = tabstop;
   }

[ \r]+        				{ 
   tokenlen = yyleng;
   tokenpos+=yyleng;
   /* eat up whitespace */ 
   }

"/*"                 			{
  BEGIN(ST_COMMENT);
  tokenlen = yyleng;
  tokenpos+=yyleng;
 }

<ST_COMMENT>[^*\n]*                     { 
   tokenlen = yyleng;
   tokenpos+=yyleng;
   /* eat anything that's not a '*' */ 
 }

<ST_COMMENT>"\n"                        {
   if( yyleng >= MAX_LINE_LEN )
   {
      SyntaxError("Length of line too long: Skipping buffering\n");
      memcpy(linebuf, yytext+1, MAX_LINE_LEN-1);
      linebuf[MAX_LINE_LEN-1] = '\0';
   }
   else
   {
      strcpy(linebuf, yytext+1); // save the next line
   }
   lineno++; /* count them */    
   tokenpos=0; /* start of line */
   yyless(1); /* give back all but the \n to rescan */ 

   // Remind lex we are at the begining of a line so the caret works
   yy_set_bol(true);

 }

<ST_COMMENT>"*"+[^*/\n]*                  { 
	/* DCM 01-13-00: Added CR exclusion to above rule so counts comment lines correctly */
   tokenlen = yyleng;
   tokenpos+=yyleng;
   /* eat up '*'s not followed by '/'s */
 } 
 
<ST_COMMENT>"*"+"/"     		{
  tokenlen = yyleng;
  tokenpos+=yyleng;
  BEGIN(INITIAL);
 }

"//"[^\n]*                              { 
   tokenlen = yyleng;
   tokenpos+=yyleng;
   /* handle C++ comments */ 
   }  



\n.*        				{ 
   if( yyleng >= MAX_LINE_LEN )
   {
      SyntaxError("Length of line too long: Skipping buffering\n");
      memcpy(linebuf, yytext+1, MAX_LINE_LEN-1);
      linebuf[MAX_LINE_LEN-1] = '\0';
   }
   else
   {
      strcpy(linebuf, yytext+1); 	// save the next line 
   }
   lineno++; 			// count them 
   tokenpos = 0; 		// Start of line
   tokenlen = 0;

   yyless(1); 			// give back all but the \n to rescan

   // Remind lex we are at the begining of a line so the caret works
   yy_set_bol(true);

   }

.					{ 
   tokenlen = yyleng;
   tokenpos+=yyleng;

   char buf[256];
   if (isprint(yytext[0]))
      sprintf(buf, "Illegal character (%c)", yytext[0]);
   else
      sprintf(buf, "Illegal character (\\%d)", yytext[0]);

   SyntaxError(buf);
   }

%%


/**********************************************************************
 * $Log: cdl_lex.l,v $
 * Revision 1.1.1.1  2008/07/14 16:44:22  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.4  2006/12/05 01:47:02  endo
 * cdl parser now accepts {& a b} and Symbol *pair added.
 *
 * Revision 1.3  2006/11/28 04:13:29  ebeowulf
 * An extension to the TwoPoints selection system, only applied to
 * selecting latitude and longitude.
 *
 * Revision 1.2  2006/08/29 15:13:57  endo
 * Advanced parameter flag added to symbol.
 *
 * Revision 1.1.1.1  2006/07/20 17:17:49  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:37:59  endo
 * MissionLab 7.0
 *
 * Revision 1.1.1.1  2005/02/06 23:00:06  endo
 * AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
 *
 * Revision 1.50  2002/01/13 01:24:31  endo
 * A filename can now include "-".
 *
 * Revision 1.49  2001/09/24 21:18:20  ebeowulf
 * Added RLstyle
 *
 * Revision 1.48  2000/04/16 15:44:56  endo
 * states can now accept "*" inside its name.
 *
 * Revision 1.47  2000/02/18 02:44:32  endo
 * TypeInBox.hpp added.
 *
 * Revision 1.46  1999/10/25 18:12:00  endo
 * rolled back to this version.
 *
 * Revision 1.44  1999/09/03 20:07:20  endo
 * rule modified, so that it accepts "." inside
 * a name, such as "agents.AuRA.urban".
 *
 * Revision 1.43  1996/10/09 15:40:49  doug
 * new version of flex allows getting rid of kludge
 *
 * Revision 1.42  1996/10/04  21:10:59  doug
 * changes to get to version 1.0c
 *
 * Revision 1.42  1996/10/01 13:01:32  doug
 * got library writes working
 *
 * Revision 1.41  1996/05/14 23:17:39  doug
 * added twopoints case
 *
 * Revision 1.40  1996/04/11  15:11:32  doug
 * Fixed sliders
 *
 * Revision 1.39  1996/04/11  04:07:16  doug
 * *** empty log message ***
 *
 * Revision 1.38  1996/04/06  23:54:20  doug
 * *** empty log message ***
 *
 * Revision 1.37  1996/02/25  01:13:50  doug
 * *** empty log message ***
 *
 * Revision 1.36  1996/02/18  00:03:53  doug
 * *** empty log message ***
 *
 * Revision 1.35  1995/12/14  21:54:03  doug
 * *** empty log message ***
 *
 * Revision 1.34  1995/10/30  21:38:25  doug
 * *** empty log message ***
 *
 * Revision 1.33  1995/10/27  20:29:18  doug
 * *** empty log message ***
 *
 * Revision 1.32  1995/09/15  22:01:29  doug
 * fixed so handles multiline initializers
 *
 * Revision 1.31  1995/09/07  14:23:14  doug
 * works
 *
 * Revision 1.30  1995/07/10  19:53:45  doug
 * *** empty log message ***
 *
 * Revision 1.29  1995/06/29  14:12:49  jmc
 * Added copyright notice and RCS id and log strings.
 **********************************************************************/
