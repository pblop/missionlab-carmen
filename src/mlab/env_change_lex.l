%option  noyywrap 

%{

/**********************************************************************
 **                                                                  **
 **                       env_change_lex.l                           **
 **                                                                  **
 **  A lexer for an environment change descriptor file.              **
 **                                                                  **
 **                                                                  **
 **  Written by:  J. Brian Lee                                       **
 **                                                                  **
 **  Copyright 1995, 1996, 1997 Georgia Tech Research Corporation    **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **********************************************************************/


#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include "env_change_defs.h"
#include "env_change_gram.h"

#define YY_NO_UNPUT // ENDO - gcc 3.4

int g_iLineNum = 1;
bool g_bHadEnvChangeParseError = false;

void env_change_error( char* szMessage )
{
    fprintf( stderr, "%d: %s at %s\n", g_iLineNum, szMessage, env_change_text );
    g_bHadEnvChangeParseError = true;
}

void env_change_init()
{
//    YY_FLUSH_BUFFER;
    g_iLineNum = 1;
    g_bHadEnvChangeParseError = false;
    yy_init = 1;
    yyrestart( env_change_in );
}

%}

NUMERIC	 			[0-9]

/********************************************************************/
/* declare states */
%x ST_COMMENT

/********************************************************************/

%%

","        { return COMMA;      }
STEP_COUNT { return STEP_COUNT; }
ADD        { return ADD;        }
REMOVE     { return REMOVE;     }
OBSTACLE   { return OBSTACLE;   }
REGION     { return REGION;     }

"-"?{NUMERIC}+("."{NUMERIC}*)? {
    env_change_lval.dNumber = atof( env_change_text );
    return NUMBER;
}

"/*" { BEGIN(ST_COMMENT); }

<ST_COMMENT>"*/" { BEGIN(INITIAL); }
<ST_COMMENT>"\n" { g_iLineNum++; }
<ST_COMMENT>.    { /* ignore */ }

"//"[^\n]* { /* handle C++ comments */ }

[ \r\t]+ { /* eat up whitespace */ }

"\n" { g_iLineNum++; }

. {
    char szError[ 30 ];
    if ( isprint( env_change_text[ 0 ] ) )
    {
        snprintf( szError, 30, "Illegal character (%c)", env_change_text[ 0 ] );
    }
    else
    {
        snprintf( szError, 30, "Illegal character (\\%d)", env_change_text[ 0 ] );
    }

    env_change_error( szError );
}

%%
