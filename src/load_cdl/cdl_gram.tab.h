/* A Bison parser, made by GNU Bison 2.5.  */

/* Bison interface for Yacc-like parsers in C
   
      Copyright (C) 1984, 1989-1990, 2000-2011 Free Software Foundation, Inc.
   
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.
   
   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */


/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     LP = 258,
     RP = 259,
     LS = 260,
     RS = 261,
     LA = 262,
     RA = 263,
     COMMA = 264,
     SEMICOLON = 265,
     ASSIGN = 266,
     LIST = 267,
     FROM = 268,
     IF = 269,
     GOTO = 270,
     DEFARCH = 271,
     DEFROBOT = 272,
     ACTUATOR = 273,
     SENSOR = 274,
     DEFIBP = 275,
     DEFOBP = 276,
     DEFRBP = 277,
     DEFTYPE = 278,
     DEFAGENT = 279,
     DEFCOORD = 280,
     INSTCOORD = 281,
     INSTROBOT = 282,
     INSTAGENT = 283,
     INSTGROUP = 284,
     INSTBP = 285,
     INSTSENSOR = 286,
     INSTACTUATOR = 287,
     SELECT_STYLE = 288,
     MDL_STYLE = 289,
     RULE_NAME = 290,
     BINDARCH = 291,
     DEFSENSOR = 292,
     DEFACTUATOR = 293,
     BINDS = 294,
     UP = 295,
     FSA_STYLE = 296,
     RL_STYLE = 297,
     CONFIGURATION_NAME = 298,
     CONST = 299,
     PU_INITIALIZER = 300,
     START_PU_INITIALIZER = 301,
     START_PU_INITIALIZER_ADV = 302,
     START_PAIR_PU_PARM_NAMES = 303,
     RB = 304,
     DEFNAMEDVALUE = 305,
     FORDATATYPE = 306,
     NAMED_VALUE = 307,
     DISPLAYAS = 308,
     CheckBox = 309,
     RadioBox = 310,
     TwoPoints = 311,
     TwoWorldPoints = 312,
     TypeInBox = 313,
     TypeInNum = 314,
     MinMaxNum = 315,
     MinMaxText = 316,
     SLIDERBAR = 317,
     NAME = 318,
     INITIALIZER = 319,
     INLINE_NAME = 320,
     PU_PARM_NAME = 321,
     DESCRIPTION = 322,
     NUMBER = 323,
     TYPE_NAME = 324,
     ARCH_NAME = 325,
     AGENT_CLASS = 326,
     ROBOT_CLASS = 327,
     PARM_NAME = 328,
     INDEX_NAME = 329,
     COORD_CLASS = 330,
     INPUT_NAME = 331,
     PU_INPUT_NAME = 332,
     BP_CLASS = 333,
     BP_NAME = 334,
     PARM_HEADER = 335,
     COORD_NAME = 336,
     AGENT_NAME = 337,
     ROBOT_NAME = 338,
     GROUP_NAME = 339,
     SENSOR_NAME = 340,
     ACTUATOR_NAME = 341,
     SENSOR_CLASS = 342,
     ACTUATOR_CLASS = 343
   };
#endif



#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{

/* Line 2068 of yacc.c  */
#line 61 "cdl_gram.y"

   Symbol   *symbol;
   T_Agent  *agent;
   char     *string;
   SymbolList *parmlist;
   Location  location;
   double    number;



/* Line 2068 of yacc.c  */
#line 149 "cdl_gram.tab.h"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif

extern YYSTYPE yylval;


