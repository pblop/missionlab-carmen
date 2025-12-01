%{
/*   Douglas C. MacKenzie
     cnl.l  ----  lex patterns for the Configuration Description Language (CNL) 

     Copyright 1995, Georgia Tech Research Corporation 
     Atlanta, Georgia  30332-0415
     ALL RIGHTS RESERVED, See file COPYRIGHT for details. 

     $Id: cnl_lex.l,v 1.1.1.1 2008/07/14 16:44:17 endo Exp $
*/

#include <stdlib.h>
#include <unistd.h>
#include "defs.h"
#include "cnl_gram.tab.h"

int lineno = 1;
int tokenpos = 0;
char linebuf[MAX_LINE_LEN];
int THE_PARENT = NO_PARENT;

#define MAX_INCLUDE_DEPTH       64
#define INITIAL 		0	//Old compilation error (INITIAL not declared)

static char *filename_stack[MAX_INCLUDE_DEPTH];
static int lineno_stack[MAX_INCLUDE_DEPTH];
static int include_stack_ptr = 0;

static YY_BUFFER_STATE  include_stack[MAX_INCLUDE_DEPTH];


char filename[MAX_LINE_LEN];


/*******************************************************************/
#define YY_NO_UNPUT
#define YY_SKIP_YYWRAP 

int
yywrap(void)    
{
   if( cur_file+1 < num_files )
   {
      set_file(cur_file+1);
      return FALSE;
   }

   return TRUE;
}

/*******************************************************************/

/* handle C preprocessor line/file change comments */
// Returns false if not a line number
static bool line_numbers(char *line)
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
	 return false;
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
      strcpy(filename,name);
   }
   return true;
}


/* handle #include commands */
// Returns false if not a #include
static bool include_file(char *line)
{
   char name[256];

   int rtn = sscanf(line+1, "include \"%[^\"]\"", name);
   if( rtn == 0 )
      rtn = sscanf(line+1, "include <%[^\"]>", name);

   if( rtn != 1 )
      return false;

   if( include_stack_ptr >= MAX_INCLUDE_DEPTH )
   {     
      fprintf(stderr,"Includes nested too deeply\n");   
      exit(1);
   }

   lineno_stack[include_stack_ptr] = lineno;
   lineno = 1;
   filename_stack[include_stack_ptr]= strdup(filename);
   include_stack[include_stack_ptr++] = YY_CURRENT_BUFFER; 

   char tmp[2048];
   strcpy(tmp,name);
   if( (yyin = fopen(tmp,"r")) == NULL )
   {     
      for(int inc=0; inc<num_include_paths; inc++) 
      {
         strcpy(tmp,include_paths[inc]);
         strcat(tmp,"/");
         strcat(tmp,name);
         if( (yyin = fopen(tmp,"r")) != NULL )
            break;
      }
   }     
   if( yyin == NULL )
   {
      fprintf(stderr,"include file '%s' not found\n",name);
      exit(1);
   }
   else
   {
      if( verbose && pass == PASS_1 )
         fprintf(stderr,"%*sincluding %s\n",include_stack_ptr*3," ",tmp);

      strcpy(filename,tmp);
   }
   yy_switch_to_buffer( yy_create_buffer(yyin, YY_BUF_SIZE) );
   BEGIN(INITIAL);

   // Dump it to the output file
   outstring(yytext);

   return true;
}


%}
ALPHA 				[a-zA-Z]
NUMERIC	 			[0-9]

/* declare states */
%x ST_CODE
%x ST_INIT
%x ST_COMMENT

/* the patterns */
%%

external 				{ 
                                          tokenpos+=yyleng;
                                          return(EXTERNAL); 
				        }

procedure	     			{ 
                                          tokenpos+=yyleng;
                                          return(PROCEDURE); 
                                        }

nprocedure 	  			{ 
                                          tokenpos+=yyleng;
                                          return(NPROCEDURE); 
                                        }

once                 			{
	tokenpos += yyleng;
	BEGIN(ST_CODE);
        return(ONCE);
   }

modulename				{ 
                                          tokenpos+=yyleng;
                                          return(MODULENAME); 
                                        }

public 	     			        { 
                                          tokenpos+=yyleng;
                                          return(PUBLIC); 
                                        }

type	     				{ 
                                          tokenpos+=yyleng;
                                          return(TYPEDEF); 
                                        }

module	     				{ 
                                          tokenpos+=yyleng;
                                          return(MODDEF); 
                                        }

with     				{ 
                                          tokenpos+=yyleng;
                                          return(WITH); 
                                        }

list 					{ 
                                          tokenpos+=yyleng;
                                          return(LIST); 
				        }

remote 					{ 
                                          tokenpos+=yyleng;
                                          return(REMOTE); 
				        }
<<EOF>>  {
      if( include_stack_ptr == 0 )
      {
         yyterminate();
      } 
      else
      { 
         yy_switch_to_buffer( include_stack[--include_stack_ptr] );
         lineno = lineno_stack[include_stack_ptr];
         strcpy(filename,filename_stack[include_stack_ptr]);
         free(filename_stack[include_stack_ptr]);
      }
   } 


init	     				{ BEGIN(ST_INIT);
                                          tokenpos+=yyleng;

                                          if( pass == PASS_1 )
					     ResetLineNumber();

                                          return(INIT); 
                                        }
<ST_INIT>\n.* 				{ 
                                          strcpy(linebuf, yytext+1); /* save the next line */
                                          lineno++; /* count them */ 
                                          tokenpos=0; /* start of line */ 
					  if( pass == PASS_1 )
                                             outline("");
                                          yyless(1); /* give back all but the \n to rescan */
					  // Remind lex we are at the begining of a line so the caret works
					  yy_set_bol(true);
                                        }

<ST_INIT>^"#"[^\n]*\n			{ lineno++;
                                          if( line_numbers(yytext) )
                                          { 
                                            if( pass == PASS_1 )
					       ResetLineNumber();
					  }
					  else
					  {
 					    if( pass == PASS_1 )
					    {
					       outstring(yytext); 
					    }
					    return(INIT_CODE);
					  }
					}


<ST_INIT>^[\t ]*iend[\t ]*$     { 
				  BEGIN(INITIAL); 
				  tokenpos += yyleng;

                                  if( pass == PASS_1 )
				     ResetLineNumber();

				  return(IEND); 
				}


<ST_INIT>[^\n]* 			{ 
 					   if( pass == PASS_1 )
					   {
					      outstring(yytext); 
					   }
					   return(INIT_CODE);
					}

<ST_CODE>\n.* 				{ 
                                          strcpy(linebuf, yytext+1); /* save the next line */
                                          lineno++; /* count them */ 
                                          tokenpos=0; /* start of line */ 
					  if( pass == PASS_2 )
                                             outline("");
                                          yyless(1); /* give back all but the \n to rescan */
					  // Remind lex we are at the begining of a line so the caret works
					  yy_set_bol(true);
                                        }

<ST_CODE>^"#"[^\n]*\n			{ if (line_numbers(yytext) )
                                          {
                                             if( pass == PASS_1 )
					        ResetLineNumber();
					  }
					  else
					  {
 					   if( pass == PASS_2 )
					   {
					      outstring(yytext); 
					   }
					   return(CODE);
					  }
					}

<ST_CODE>[^\n]* 			{
 					   if( pass == PASS_2 )
					   {
					      outstring(yytext); 
					   }
					   return(CODE);
					}

<ST_CODE>^([ \t])*pend([ \t])*\n        { 
	tokenpos += yyleng;
	BEGIN(INITIAL); 
	return(PEND); 
   }

<ST_CODE>^([ \t])*header([ \t])*\n        { 
	tokenpos += yyleng;
	return(HEADER); 
   }

header                 			{
	tokenpos += yyleng;
	BEGIN(ST_CODE);
        return(HEADER);
   }

<ST_CODE>^([ \t])*body([ \t])*\n        { 
	tokenpos += yyleng;
	return(BODY); 
   }

^([ \t])*pend([ \t])*\n                 { 
	tokenpos += yyleng;
	return(PEND); 
   }

node 					{ tokenpos += yyleng;return(NODE); }
is 					{ tokenpos += yyleng;return(IS); }
nend 					{ tokenpos += yyleng;return(NEND); }

"$"?{ALPHA}("_"|"-"|{ALPHA}|{NUMERIC})*	{
  	tokenpos += yyleng;

	yylval.loc = LookupName(yytext,NO_PARENT);
	if( yylval.loc >= 0 && symbol_type(yylval.loc) == TYPE )
	{
	   /* is a type */
	   return TYPE;
	}
	else if( yylval.loc >= 0 && symbol_type(yylval.loc) == MODULE )
	{
	   /* is a module */
	   THE_PARENT = yylval.loc;
	   return MODULE;
	}

	yylval.loc = LookupName(yytext,THE_PARENT);
	if( yylval.loc >= 0 )
	{
	   /* return pointer to the existing record */
  	   return NAME; 
	}
	else
	{
  	   if( pass == PASS_2 )
	   {
	      fprintf(stderr,"Alignment error: name '%s' undefined in pass 2\n",
			     yytext);
	   }

           /* new name: add it */
  	   yylval.loc =AddName(yytext,THE_PARENT); 
  	   return NAME; 
	}
   }

"-"?{NUMERIC}+("."{NUMERIC}*)? {  // numbers with a digit before the period
	yylval.loc =AddNumber(yytext,THE_PARENT); 
	tokenpos += yyleng;
	return(NUMBER); 
   }

"(""-"?{NUMERIC}+("."{NUMERIC}*)?")" { // numbers with a digit before the period
	yylval.loc =AddNumber(yytext,THE_PARENT); 
	tokenpos += yyleng;
	return(NUMBER); 
   }

"(""-"?{NUMERIC}*"."{NUMERIC}+")" {  // numbers with a digit after the period
	yylval.loc =AddNumber(yytext,THE_PARENT); 
	tokenpos += yyleng;
	return(NUMBER); 
   }

"'"."'" { 
	yylval.loc =AddChar(yytext,THE_PARENT); 
	tokenpos += yyleng;
	return(CHARACTER); 
   }

"{".+"}" { 
	yylval.loc =AddInitializer(yytext,THE_PARENT);
	tokenpos += yyleng; 
	return(INITIALIZER); 
   }

\".*\" { 
	yylval.loc =AddInitializer(yytext,THE_PARENT);
	tokenpos += yyleng; 
	return(STRING); 
   }

"[" 			{ 
  	                   tokenpos += yyleng;
			   return(LB); 
			}

"]" 			{ 
  	                   tokenpos += yyleng;
			   return(RB); 
			}

"<" 			{ 
  	                   tokenpos += yyleng;
			   return(LA); 
			}

">" 			{ 
  	                   tokenpos += yyleng;
			   return(RA); 
			}

"," 			{ 
  	                   tokenpos += yyleng;
			   return(COMMA); 
			}

"=" 			{ 
  	                   tokenpos += yyleng;
			   return(ASSIGN); 
			}

";" 			{ 
  	                   tokenpos += yyleng;
			   return(SEMICOLON); 
			}

":"	     		{ 
                           tokenpos+=yyleng;
                           return(COLON); 
                        }

"|"[^|\n]*"|"   {  /* A description */
   tokenpos+=yyleng;
   yylval.string = strdup(yytext+1);    /* NOTE: Trims the bracketing bars */
   yylval.string[strlen(yylval.string)-1] = '\0';
   return DESCRIPTION;
   } 


[ \t\r]+        			{ /* eat up whitespace */ 
  	                                  tokenpos += yyleng;
					}


"/*"                 BEGIN(ST_COMMENT);
<ST_COMMENT>[^\*\n]*                    { /* eat anything that's not a '*' */ }

<ST_COMMENT>"\n"                        {
                                          strcpy(linebuf, yytext+1); /* save the
 next line */
                                          lineno++; /* count them */
                                          tokenpos=0; /* start of line */
                                          yyless(1); /* give back all but the \n to rescan */

					  // Remind lex we are at the begining of a line so the caret works
					  yy_set_bol(true);

                                        }

<ST_COMMENT>"*"+[^\*/]*                 { /* eat up '*'s not followed by '/'s */
 }

<ST_COMMENT>"*"+"/"     BEGIN(INITIAL);

"//"[^\n]*                  		{ /* handle C++ comments */ }

\n.*        				{ 
                                          strcpy(linebuf, yytext+1); /* save the next line */
                                          lineno++; /* count them */ 
                                          tokenpos=0; /* start of line */ 
                                          yyless(1); /* give back all but the \n to rescan */
					  // Remind lex we are at the begining of a line so the caret works
					  yy_set_bol(true);
                                        }

^"#"[^\n]*\n	{ lineno++;
                  if( include_file(yytext) )
                  {
                     if( pass == PASS_1 )
		        ResetLineNumber();
		  }
                  else if( line_numbers(yytext) )
                  {
                     if( pass == PASS_1 )
		        ResetLineNumber();
		  }
		  else if (run_preprocessor)
	             SyntaxError("Unknown # command");
		}



.		{ 
                   char buf[256];

                   if (isprint(yytext[0]))
	              sprintf(buf, "Illegal character (%c)\n", yytext[0]);
	           else
		      sprintf(buf, "Illegal character (\\%d)\n", yytext[0]);
                
		   SyntaxError(buf);
  	           tokenpos += yyleng;
		}

%%


/************************************************************************
*                                                                       *
*                     void yyerror()                                    *
*                                                                       *
************************************************************************/

/* yyerror is called by yacc when it finds an error. */

void yyerror(char *s)
{
   SyntaxError(s);
}

/************************************************************************
*                                                                       *
*                     void file_reset()                                 *
*                                                                       *
************************************************************************/

/* called between passes 1 & 2 to reset yacc */

void file_reset(void)
{
   lineno = 1;
   tokenpos = 0;

   YY_NEW_FILE;
}




/**********************************************************************
 * $Log: cnl_lex.l,v $
 * Revision 1.1.1.1  2008/07/14 16:44:17  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/20 17:17:47  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:37:55  endo
 * MissionLab 7.0
 *
 * Revision 1.1.1.1  2005/02/06 22:59:38  endo
 * AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
 *
 * Revision 1.37  2002/01/13 01:33:53  endo
 * A filename can now include "-".
 *
 * Revision 1.36  1996/10/08  19:42:15  doug
 * with new version of flex can remove kludges
 *
 * Revision 1.35  1996/10/07  01:55:41  doug
 * Flex is new enough I can get rid of the kludge
 *
 * Revision 1.34  1996/08/22  15:09:34  doug
 * Added linux compatibility code
 *
 * Revision 1.33  1996/04/17 15:47:46  doug
 * *** empty log message ***
 *
 * Revision 1.32  1996/02/25  16:56:17  doug
 * fixed line number counting so counted #include lines
 *
 * Revision 1.31  1996/02/14  17:12:53  doug
 * added support for descriptions
 * ./
 *
 * Revision 1.30  1996/02/08  09:18:53  doug
 * added support to skim include files without expanding them if
 * didn't run the preprocessor.
 *
 * Revision 1.29  1995/10/18  13:59:36  doug
 * added support for new style procs
 *
 * Revision 1.28  1995/06/15  22:04:07  jmc
 * Added RCS ID and Log strings.
 **********************************************************************/
