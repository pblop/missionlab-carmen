/**********************************************************************
 **                                                                  **
 **                             cdl_gram.y                           **
 **                                                                  **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  - yacc grammer for Configuration Description Language.          **
 **                                                                  **
 **  Copyright 1995 - 2006  Georgia Tech Research Corporation        **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **********************************************************************/

/* $Id: cdl_gram.y,v 1.1.1.1 2008/07/14 16:44:22 endo Exp $ */

%{
#include <assert.h>
#include <string>

#include "prototypes.h"
#include "symtab.h"
#include "cdl_defs.h"
#include "CheckBoxType.hpp"
#include "RadioButtonType.hpp"
#include "SliderBar.hpp"
#include "TwoPoints.hpp"
#include "TwoWorldPoints.hpp"
#include "TypeInBox.hpp"
#include "MinMax.hpp"

void yyerror(char *s);

bool is_lhs = true;

static bool loc_is_default(const Location loc)
{
   if((loc.x == 100 || loc.x == 0) && 
       (loc.y == 100 || loc.y == 0))
   {
      return true;
   }
   return false;
}

// Return to the default architecture
static void 
revert_arch(void)
{
   // Leave the current architecture
   LeaveScope();
   
   // Reenter the free architecture
   EnterScope((Symbol *)choosen_architecture);
}

static Symbol *parm_list_head = NULL;

%}

%union {
   Symbol   *symbol;
   T_Agent  *agent;
   char     *string;
   SymbolList *parmlist;
   Location  location;
   double    number;
}

/* Define the tokens that lex will return */
%token LP  
%token RP 
%token LS
%token RS
%token LA
%token RA
%token COMMA
%token SEMICOLON
%token ASSIGN
%token LIST
%token FROM
%token IF
%token GOTO
%token DEFARCH
%token DEFROBOT
%token ACTUATOR
%token SENSOR
%token DEFIBP
%token DEFOBP
%token DEFRBP
%token DEFTYPE
%token DEFAGENT
%token DEFCOORD
%token INSTCOORD
%token INSTROBOT
%token INSTAGENT
%token INSTGROUP
%token INSTBP
%token INSTSENSOR
%token INSTACTUATOR
%token SELECT_STYLE
%token MDL_STYLE
%token RULE_NAME
%token BINDARCH
%token DEFSENSOR
%token DEFACTUATOR
%token BINDS
%token UP                
%token FSA_STYLE          
%token RL_STYLE          
%token CONFIGURATION_NAME
%token CONST
%token PU_INITIALIZER
%token START_PU_INITIALIZER
%token START_PU_INITIALIZER_ADV
%token START_PAIR_PU_PARM_NAMES
%token RB
%token DEFNAMEDVALUE
%token FORDATATYPE
%token NAMED_VALUE
%token DISPLAYAS
%token CheckBox
%token RadioBox
%token TwoPoints
%token TwoWorldPoints
%token TypeInBox
%token TypeInNum
%token MinMaxNum
%token MinMaxText
%token SLIDERBAR

%token <string> NAME INITIALIZER INLINE_NAME PU_PARM_NAME DESCRIPTION
%token <number> NUMBER

%token <symbol> TYPE_NAME
%token <symbol> ARCH_NAME
%token <symbol> AGENT_CLASS
%token <symbol> ROBOT_CLASS
%token <symbol> PARM_NAME
%token <symbol> INDEX_NAME
%token <symbol> COORD_CLASS
%token <symbol> INPUT_NAME
%token <symbol> PU_INPUT_NAME
%token <symbol> BP_CLASS
%token <symbol> BP_NAME
%token <symbol> PARM_HEADER
%token <symbol> COORD_NAME
%token <symbol> AGENT_NAME
%token <symbol> ROBOT_NAME
%token <symbol> GROUP_NAME
%token <symbol> SENSOR_NAME
%token <symbol> ACTUATOR_NAME
%token <symbol> SENSOR_CLASS
%token <symbol> ACTUATOR_CLASS


%type <parmlist> ParmDef Glist ParmSet BPList RobotLinks RobotParms ValueList

%type <symbol> StartAgent StartCoordRef StartRobot
%type <symbol> StartClass StartBP
%type <symbol> StartRefAgent RefAgent Agent
%type <symbol> StartDefop MidDefineOp
%type <symbol> Link AParm BPparmdef
%type <symbol> LHS Rule RuleHead StartCoordInst StartRobotInst
%type <symbol> RefCoord RefGroup
%type <symbol> RefRobot StartRefRobot
%type <symbol> SetArch ReqArch
%type <symbol> StartSensor StartActuator SandA SorA SorAclass 
%type <symbol> StartSorARef RefSorA
%type <symbol> LG StartSorAInst typed_parm
%type <symbol> StartBPRef RefBP StartBPInst RobotLink
%type <parmlist> AgentList
%type <location> Loc
%type <string> MaybeName Desc

/************************** Start of CDL Grammer *****************************/
%%

Start		: Start DefineClass
		| Start DefineBP
		| Start InstAgent
		| Start InstGroup
		| Start InstSorA
		| Start DefineRobot
		| Start DefineOp
		| Start DefineArch
		| Start DefineType
		| Start InstCoord
		| Start InstBP
		| Start InstRobot
		| Start BindArch
		| Start DefineSandA
		| Start DefineMacro
		| Start Agent
		| Start error { had_error = true; }
		| DefineClass
		| DefineBP
		| InstAgent
		| InstGroup
		| InstSorA
		| DefineRobot
		| DefineOp
		| DefineArch
		| DefineType
		| InstCoord
		| InstBP
		| InstRobot 
		| BindArch
		| DefineSandA
		| DefineMacro
		| Agent {}
		| error { had_error = true; }
		| /* Empty */
		;

Agent		: AGENT_NAME  { top_agent = $1; }
		| ROBOT_NAME  { top_agent = $1; }
		| GROUP_NAME  { top_agent = $1; }
		| BP_NAME     { top_agent = $1; }
		| RefRobot    { top_agent = $1; }
		| RefGroup    { top_agent = $1; }
		| RefCoord    { top_agent = $1; }
		| RefSorA     { top_agent = $1; }
		| RefBP       { top_agent = $1; }
		| RefAgent      { top_agent = $1; }
		| COORD_NAME  { top_agent = $1; }
		| SENSOR_NAME { top_agent = $1; }
		;

DefineArch	: DEFARCH NAME SEMICOLON {
      AddArch($2);
   }
		;

Desc		: DESCRIPTION {
   $$ = $1;
}
		| {
   $$ = NULL;
}
		;

DefineMacro	: DEFNAMEDVALUE NAME FORDATATYPE TYPE_NAME ASSIGN INITIALIZER SEMICOLON {
      Symbol *p = new Symbol(NAMED_VALUE);

      p->arch = (Symbol *)choosen_architecture;
      p->data_type = $4;
      p->name = $2;

      // Build a data record for the constant
      Symbol *inst = new Symbol(INITIALIZER);
      inst->name = $6;
      inst->data_type = $4;
      p->input_generator = inst;
   
      Symbol *dup;
      if((dup = DefineName(p)) != NULL)
      {
         char buf[256];
         sprintf(buf, "Duplicate definition of named value '%s'",p->name);
         SyntaxError(buf);
      }
   }
		;

BindArch	: BINDARCH ARCH_NAME SEMICOLON {
      UseArch($2);
   }
		| BINDARCH NAME SEMICOLON {
      char msg[256];
      sprintf(msg,"Binding to undefined architecture: %s",$2);
      SyntaxError(msg);
   }
		;

SetArch		: LS ARCH_NAME RS {
      // Any definitions are local to this architecture

      // Leave the free architecture
      LeaveScope();

      // Enter the specified architecture
      EnterScope($2);

      $$ = $2;
   }
		| LS NAME RS {
      char msg[256];
      sprintf(msg,"Undefined architecture: %s",$2);
      SyntaxError(msg);
      $$ = NULL;
   }
		| { 
      // The default arch is already on the stack so just return it.
      $$ = (Symbol *)choosen_architecture;
}
		;

ValueList	: INITIALIZER {
   // Build a data record for the constant
   Symbol *inst = new Symbol(INITIALIZER);
   inst->name = $1;
   
   // Return the value
   $$ = new SymbolList(inst);
   }
		| ValueList COMMA INITIALIZER {
   // Build a data record for the constant
   Symbol *inst = new Symbol(INITIALIZER);
   inst->name = $3;
   
   // Add the value to the list
   $1->append(inst);
   $$ = $1;
   }
		;

DefineType	: DEFTYPE SetArch NAME SEMICOLON {
    if($2)
    {
        AddType($2,$3);

        // Leave architecture scope 
        revert_arch();
    }
}

| DEFTYPE SetArch TYPE_NAME SEMICOLON {
    SyntaxError("Duplicate definition for type");
    if($2)
    {
        // Leave architecture scope 
        revert_arch();
    }
}

| DEFTYPE SetArch NAME DISPLAYAS CheckBox ASSIGN ValueList SEMICOLON {
    if($2)
    {
        Symbol *TypeRec = AddType($2,$3);

        TypeRec->parameter_list = *($7);

        // Create the type info record
        const int size = $7->len();
        char **values = new char *[size];
        for(int i=0; i<size; i++)
            values[i] = ($7->get())->name;

        TypeRec->TypeRecord = new CheckBoxType($3, values, size);

        delete [] values;

        // Leave architecture scope 
        revert_arch();
    }
}

| DEFTYPE SetArch TYPE_NAME DISPLAYAS CheckBox ASSIGN ValueList SEMICOLON {
    SyntaxError("Duplicate definition for type");
    if($2)
    {
        // Leave architecture scope 
        revert_arch();
    }
}

| DEFTYPE SetArch NAME DISPLAYAS RadioBox ASSIGN ValueList SEMICOLON {
    if($2)
    {
        Symbol *TypeRec = AddType($2,$3);

        TypeRec->parameter_list = *($7);

        // Create the type info record
        const int size = $7->len();
        char **values = new char *[size];
        for(int i=0; i<size; i++)
        {
            values[i] = ($7->get())->name;
        }

        TypeRec->TypeRecord = new RadioButtonType($3, values, size);

        delete [] values;

        // Leave architecture scope 
        revert_arch();
    }
}

| DEFTYPE SetArch TYPE_NAME DISPLAYAS RadioBox ASSIGN ValueList SEMICOLON {
    SyntaxError("Duplicate definition for type");
    if($2)
    {
        // Leave architecture scope 
        revert_arch();
    }
}

| DEFTYPE SetArch NAME DISPLAYAS RadioBox ASSIGN ValueList DESCRIPTION SEMICOLON {
    if($2)
    {
        Symbol *TypeRec = AddType($2,$3);

        TypeRec->parameter_list = *($7);

        // Create the type info record
        const int size = $7->len();
        char **values = new char *[size];
        for(int i=0; i<size; i++)
        {
            values[i] = ($7->get())->name;
        }

        TypeRec->TypeRecord = new RadioButtonType($3, values, size, $8);

        delete [] values;

        // Leave architecture scope 
        revert_arch();
    }
}

| DEFTYPE SetArch TYPE_NAME DISPLAYAS RadioBox ASSIGN ValueList DESCRIPTION SEMICOLON {
    SyntaxError("Duplicate definition for type");
    if($2)
    {
        // Leave architecture scope 
        revert_arch();
    }
}

| DEFTYPE SetArch NAME DISPLAYAS TwoPoints SEMICOLON {
    if($2)
    {
        Symbol *TypeRec = AddType($2,$3);
        TypeRec->TypeRecord = new TwoPointsType($3);

        // Leave architecture scope 
        revert_arch();
    }
}

| DEFTYPE SetArch TYPE_NAME DISPLAYAS TwoPoints SEMICOLON {
    SyntaxError("Duplicate definition for type");
    if($2)
    {
        // Leave architecture scope 
        revert_arch();
    }
}

| DEFTYPE SetArch NAME DISPLAYAS TwoWorldPoints SEMICOLON {
    if($2)
    {
        Symbol *TypeRec = AddType($2,$3);
        TypeRec->TypeRecord = new TwoWorldPointsType($3);

        // Leave architecture scope 
        revert_arch();
    }
}

| DEFTYPE SetArch TYPE_NAME DISPLAYAS TwoWorldPoints SEMICOLON {
    SyntaxError("Duplicate definition for type");
    if($2)
    {
        // Leave architecture scope 
        revert_arch();
    }
}

| DEFTYPE SetArch NAME DISPLAYAS MinMaxNum DESCRIPTION SEMICOLON {
    if($2)
    {
        Symbol *TypeRec = AddType($2,$3);
        char *description = $6;

        TypeRec->TypeRecord = new MinMaxType(
            $3,
            description,
            MINMAXTYPE_INPUT_FORMAT_NUMBER);

        // Leave architecture scope 
        revert_arch();
    }
}

| DEFTYPE SetArch TYPE_NAME DISPLAYAS MinMaxNum DESCRIPTION SEMICOLON {
    SyntaxError("Duplicate definition for type");
    if($2)
    {
        // Leave architecture scope 
        revert_arch();
    }
}

| DEFTYPE SetArch NAME DISPLAYAS MinMaxText DESCRIPTION SEMICOLON {
    if($2)
    {
        Symbol *TypeRec = AddType($2,$3);
        char *description = $6;

        TypeRec->TypeRecord = new MinMaxType(
            $3,
            description,
            MINMAXTYPE_INPUT_FORMAT_TEXT);

        // Leave architecture scope 
        revert_arch();
    }
}

| DEFTYPE SetArch TYPE_NAME DISPLAYAS MinMaxText DESCRIPTION SEMICOLON {
    SyntaxError("Duplicate definition for type");
    if($2)
    {
        // Leave architecture scope 
        revert_arch();
    }
}

| DEFTYPE SetArch NAME DISPLAYAS TypeInBox NUMBER NUMBER DESCRIPTION SEMICOLON {
    if($2)
    {
        Symbol *TypeRec = AddType($2,$3);

        double scrollbox = $6;
        double showtext = $7;
        char *description = $8;

        TypeRec->TypeRecord = new TypeInBoxType($3, scrollbox, showtext, description);

        // Leave architecture scope 
        revert_arch();
    }
}

| DEFTYPE SetArch TYPE_NAME DISPLAYAS TypeInBox NUMBER NUMBER DESCRIPTION SEMICOLON {
    SyntaxError("Duplicate definition for type");
    if($2)
    {
        // Leave architecture scope 
        revert_arch();
    }
}

| DEFTYPE SetArch NAME DISPLAYAS TypeInNum DESCRIPTION SEMICOLON {
    if($2)
    {
        Symbol *TypeRec = AddType($2,$3);
        char *description = $6;

        TypeRec->TypeRecord = new TypeInBoxType(
            $3,
            false,
            true,
            description,
            TYPEINBOXTYPE_INPUT_FORMAT_NUMBER);

        // Leave architecture scope 
        revert_arch();
    }
}

| DEFTYPE SetArch TYPE_NAME DISPLAYAS TypeInNum DESCRIPTION SEMICOLON {
    SyntaxError("Duplicate definition for type");
    if($2)
    {
        // Leave architecture scope 
        revert_arch();
    }
}

| DEFTYPE SetArch NAME DISPLAYAS TypeInNum NUMBER DESCRIPTION SEMICOLON {
    if($2)
    {
        Symbol *TypeRec = AddType($2,$3);
        char *description = $7;
        double showtext = $6;

        TypeRec->TypeRecord = new TypeInBoxType(
            $3,
            false,
            (bool)showtext,
            description,
            TYPEINBOXTYPE_INPUT_FORMAT_NUMBER);

        // Leave architecture scope 
        revert_arch();
    }
}

| DEFTYPE SetArch TYPE_NAME DISPLAYAS TypeInNum NUMBER DESCRIPTION SEMICOLON {
    SyntaxError("Duplicate definition for type");
    if($2)
    {
        // Leave architecture scope 
        revert_arch();
    }
}

| DEFTYPE SetArch NAME DISPLAYAS SLIDERBAR NUMBER NUMBER DESCRIPTION ASSIGN ValueList SEMICOLON {
    if($2)
    {
        Symbol *TypeRec = AddType($2,$3);

        TypeRec->parameter_list = *($10);

        // Create the type info record
        const int size = $10->len();
        char **values = new char *[size];
        for(int i=0; i<size; i++)
            values[i] = ($10->get())->name;

        double minv = $6;
        double maxv = $7;
        char *units = $8;

        TypeRec->TypeRecord = new SliderBar($3, minv, maxv, units, values, size);

        delete [] values;

        // Leave architecture scope 
        revert_arch();
    }
}

| DEFTYPE SetArch TYPE_NAME DISPLAYAS SLIDERBAR NUMBER NUMBER DESCRIPTION ASSIGN ValueList SEMICOLON {
    SyntaxError("Duplicate definition for type");
    if($2)
    {
        // Leave architecture scope 
        revert_arch();
    }
}

| DEFTYPE SetArch NAME ASSIGN ValueList SEMICOLON {
    if($2)
    {
        Symbol *TypeRec = AddType($2,$3);

        TypeRec->parameter_list = *($5);

        // Leave architecture scope 
        revert_arch();
    }
}

| DEFTYPE SetArch TYPE_NAME ASSIGN ValueList SEMICOLON {
    SyntaxError("Duplicate definition for type");
    if($2)
    {
        // Leave architecture scope 
        revert_arch();
    }
}
;

DefineOp	: MidDefineOp LP ParmDef RP SEMICOLON {
    // Hookup the params
    if($1 && $3)
        $1->parameter_list = *($3);

    // Delete the old copies
    if($3)
        delete $3;

    // Leaving Coord scope
    if($1)
        LeaveScope($1);

    // Leave architecture scope 
    revert_arch();
}

| MidDefineOp LP RP SEMICOLON {

    // Leaving Coord scope
    if($1)
        LeaveScope($1);

    // Leave architecture scope 
    revert_arch();
}

| MidDefineOp LP error RP SEMICOLON {

    // Leaving Coord scope
    if($1)
        LeaveScope($1);

    // Leave architecture scope 
    revert_arch();

    SyntaxError("Incorrect parameter list definition");
}
;

MidDefineOp	: StartDefop FSA_STYLE {
    // Remember the operator style
    if($1)
        $1->operator_style = FSA_STYLE;

    $$ = $1;
}

| StartDefop RL_STYLE {
    // Remember the operator style
    if($1)
        $1->operator_style = RL_STYLE;

    $$ = $1;
}

| StartDefop SELECT_STYLE {
    // Remember the operator style
    if($1)
        $1->operator_style = SELECT_STYLE;

    $$ = $1;
}

| StartDefop MDL_STYLE {
    // Remember the operator style
    if($1)
        $1->operator_style = MDL_STYLE;

    $$ = $1;
}
;


StartDefop 	: DEFCOORD TYPE_NAME NAME {
    Symbol *p = new Symbol(COORD_CLASS);

    p->arch = (Symbol *)choosen_architecture;
    p->data_type = $2;
    p->name = $3;

    Symbol *dup;
    if((dup = DefineName(p)) != NULL)
    {
        char buf[256];
        sprintf(buf, "Duplicate definition of name '%s'",p->name);
        SyntaxError(buf);
    }

    // Any parm definitions are now local to this coord
    EnterScope(p);

    $$ = p;
}
		| DEFCOORD ReqArch BINDS AGENT_CLASS TYPE_NAME NAME {
      Symbol *p = new Symbol(COORD_CLASS);

      p->arch = $2;
      p->data_type = $5;
      p->binds_to = $4;
      p->name = $6;

      Symbol *dup;
      if((dup = DefineName(p)) != NULL)
      {
         char buf[256];
         sprintf(buf, "Duplicate definition of name '%s'",p->name);
         SyntaxError(buf);
      }

      // Any parm definitions are now local to this coord
      EnterScope(p);

      $$ = p;
   }

| DEFCOORD ReqArch BINDS NAME TYPE_NAME NAME {
    Symbol *bt_rec = LookupName(&free_arch->table,$4);
    if(bt_rec == NULL)
    {
        char buf[256];
        sprintf(buf, "Undefined binding source '%s'",$4);
        SyntaxError(buf);
    }
    Symbol *p = new Symbol(COORD_CLASS);

    p->arch = $2;
    p->data_type = $5;
    p->binds_to = bt_rec;
    p->name = $6;

    Symbol *dup;
    if((dup = DefineName(p)) != NULL)
    {
        char buf[256];
        sprintf(buf, "Duplicate definition of name '%s'",p->name);
        SyntaxError(buf);
    }

    // Any parm definitions are now local to this coord
    EnterScope(p);

    $$ = p;
}


| DEFCOORD TYPE_NAME COORD_CLASS {
    char buf[256];
    sprintf(buf,"Duplicate definition of coordination operator: %s",$3->name);
    SyntaxError(buf);
    $$ = NULL;
}

| DEFCOORD ReqArch TYPE_NAME NAME {
    char buf[256];
    sprintf(buf, "Missing binding clause in definition of agent %s",$4);
    SyntaxError(buf);
    $$ = NULL;
}

| DEFCOORD ReqArch BINDS AGENT_CLASS TYPE_NAME AGENT_CLASS {
    char buf[256];
    sprintf(buf, "Duplicate definition of agent: %s",$6->name);
    SyntaxError(buf);
    $$ = NULL;
}
;

DefineRobot	: StartRobot LP BPList RP SEMICOLON {
    // Hookup the params
    $1->parameter_list = *($3);

    // Delete the old copies
    delete $3;

    // Leaving class scope
    LeaveScope($1);

    // Leave architecture scope 
    revert_arch();
}
		| StartRobot LP RP SEMICOLON {
      // Leaving class scope
      LeaveScope($1);

      // Leave architecture scope 
      revert_arch();
   }
		| StartRobot LP error RP SEMICOLON {
      SyntaxError("Incorrect parameter list definition");

      // Leaving class scope
      LeaveScope($1);

      // Leave architecture scope 
      revert_arch();
   }
;

StartRobot	: DEFROBOT SetArch BINDS BP_CLASS NAME {
    Symbol *p = new Symbol(ROBOT_CLASS);

    p->arch = $2;
    p->binds_to = $4;
    p->name = $5;

    Symbol *dup;
    if((dup = DefineName(p)) != NULL)
    {
        char buf[256];
        sprintf(buf, "Duplicate definition of name '%s'",p->name);
        SyntaxError(buf);
    }

    // Any parm definitions are now local to this class
    EnterScope(p);

    $$ = p;
}
;

BPList		: BPList COMMA BPparmdef {
    $$ = $1;
    $$->append($3);
}

| BPparmdef {
    // return this parameter
    $$ = new SymbolList($1);
}
;

BPparmdef 	: SENSOR INLINE_NAME SENSOR_CLASS {
    Symbol *p = $3->inst_of($2);

    Symbol *dup;
    if((dup = DefineName(p)) != NULL)
    {
        char buf[256];
        sprintf(buf, "Duplicate definition of binding point parm: %s",p->name);
        SyntaxError(buf);
    }

    // return this parameter
    $$ = p;
}

| ACTUATOR INLINE_NAME ACTUATOR_CLASS {
    Symbol *p = $3->inst_of($2);

    Symbol *dup;
    if((dup = DefineName(p)) != NULL)
    {
        char buf[256];
        sprintf(buf, "Duplicate definition of binding point parm: %s",p->name);
        SyntaxError(buf);
    }

    // return this parameter
    $$ = p;
}
;

DefineBP	: StartBP LP ParmDef RP SEMICOLON {
    if($1)
    {
        // Hookup the params
        $1->parameter_list = *($3);

        // Add "bound_to" as an input
        Symbol *p = new Symbol(PARM_NAME);
        p->name = "bound_to";
        if(DefineName(p) != NULL)
            SyntaxError("The builtin input \"bound_to\" was redefined");
        $1->parameter_list.append(p);

        // Leaving class scope
        LeaveScope($1);
    }

    // Delete the old copies
    delete $3;

    // Leave architecture scope 
    revert_arch();
}

| StartBP LP RP SEMICOLON {
    if($1)
    {
        // Add "bound_to" as an input
        Symbol *p = new Symbol(PARM_NAME);
        p->name = "bound_to";
        if(DefineName(p) != NULL)
            SyntaxError("The builtin input \"bound_to\" was redefined");
        $1->parameter_list.append(p);

        // Leaving class scope
        LeaveScope($1);
    }

    // Leave architecture scope 
    revert_arch();
}
		| StartBP LP error RP SEMICOLON {
      SyntaxError("Incorrect parameter list definition");

      if($1)
      {
         // Leaving class scope
         LeaveScope($1);
      }

      // Leave architecture scope 
      revert_arch();
   }
;

StartBP		: DEFIBP SetArch TYPE_NAME NAME {
    Symbol *p = new Symbol(BP_CLASS);

    p->data_type = $3;
    p->name = $4;
    p->construction = CS_IBP;

    Symbol *dup;
    if((dup = DefineName(p)) != NULL)
    {
        char buf[256];
        sprintf(buf, "Duplicate definition of name '%s'",p->name);
        SyntaxError(buf);
    }

    // Any parm definitions are now local to this class
    EnterScope(p);

    $$ = p;
}

| DEFIBP SetArch NAME NAME {
    char buf[256];
    sprintf(buf, "Undefined input binding point return type: %s",$3);
    SyntaxError(buf);

    $$ = NULL;
}

| DEFOBP SetArch TYPE_NAME NAME {
    Symbol *p = new Symbol(BP_CLASS);

    p->data_type = $3;
    p->name = $4;
    p->construction = CS_OBP;

    Symbol *dup;
    if((dup = DefineName(p)) != NULL)
    {
        char buf[256];
        sprintf(buf, "Duplicate definition of name '%s'",p->name);
        SyntaxError(buf);
    }

    // Any parm definitions are now local to this class
    EnterScope(p);

    $$ = p;
}

| DEFOBP SetArch NAME NAME {
    char buf[256];
    sprintf(buf, "Undefined output binding point return type: %s",$3);
    SyntaxError(buf);

    $$ = NULL;
}

| DEFRBP SetArch NAME {
    Symbol *p = new Symbol(BP_CLASS);

    p->data_type = NULL;
    p->name = $3;
    p->construction = CS_RBP;

    Symbol *dup;
    if((dup = DefineName(p)) != NULL)
    {
        char buf[256];
        sprintf(buf, "Duplicate definition of name '%s'",p->name);
        SyntaxError(buf);
    }

    // Any parm definitions are now local to this class
    EnterScope(p);

    $$ = p;
}
;

StartSensor	: DEFSENSOR SetArch BINDS BP_CLASS TYPE_NAME NAME {
    Symbol *p = new Symbol(SENSOR_CLASS);

    p->arch = $2;
    p->binds_to = $4;
    p->data_type = $5;
    p->name = $6;

    Symbol *dup;
    if((dup = DefineName(p)) != NULL)
    {
        char buf[256];
        sprintf(buf, "Duplicate definition of name '%s'",p->name);
        SyntaxError(buf);
    }

    // Any parm definitions are now local to this class
    EnterScope(p);

    $$ = p;
}

| DEFSENSOR SetArch BINDS BP_CLASS NAME NAME {
    char buf[256];
    sprintf(buf, "Undefined sensor output type: %s",$5);
    SyntaxError(buf);

    $$ = NULL;
}
;

StartActuator	: DEFACTUATOR SetArch BINDS BP_CLASS TYPE_NAME NAME {
    Symbol *p = new Symbol(ACTUATOR_CLASS);

    p->arch = $2;
    p->binds_to = $4;
    p->data_type = $5;
    p->name = $6;

    Symbol *dup;
    if((dup = DefineName(p)) != NULL)
    {
        char buf[256];
        sprintf(buf, "Duplicate definition of name '%s'",p->name);
        SyntaxError(buf);
    }

    // Any parm definitions are now local to this class
    EnterScope(p);

    $$ = p;
}

| DEFACTUATOR SetArch BINDS BP_CLASS TYPE_NAME ACTUATOR_CLASS {
    char buf[256];
    sprintf(buf, "Duplicate definition of Actuator: %s",$6->name);
    SyntaxError(buf);
    $$ = NULL;
}

| DEFACTUATOR SetArch BINDS BP_CLASS NAME {
    char buf[256];
    sprintf(buf, "You forgot the output type for Actuator: %s",$5);
    SyntaxError(buf);
    $$ = NULL;
}
;

SandA		: StartSensor
| StartActuator
;

DefineSandA	: SandA Desc LP ParmDef RP SEMICOLON {
    if($1)
    {
        // Leaving class scope
        LeaveScope($1);
        $1->description = $2;

        // Hookup the params
        $1->parameter_list = *($4);

        // Leave architecture scope 
        revert_arch();
    }

    // Delete the old copies
    delete $4;
}

| SandA Desc LP RP SEMICOLON {
    if($1)
    {
        // Leaving class scope
        LeaveScope($1);
        $1->description = $2;

        // Leave architecture scope 
        revert_arch();
    }
}

| SandA Desc LP error RP SEMICOLON {
    SyntaxError("Incorrect parameter list definition");

    if($1)
    {
        // Leaving class scope
        LeaveScope($1);

        // Leave architecture scope 
        revert_arch();
    }
}
;

DefineClass	: StartClass Desc LP ParmDef RP SEMICOLON {
    if($1)
    {
        // Leaving class scope
        LeaveScope($1);
        $1->description = $2;

        if($4)
        {
            // Hookup the params
            $1->parameter_list = *($4);
        }
    }

    // Leave architecture scope 
    revert_arch();

    if($4)
    {
        // Delete the old copies
        delete $4;
    }
}

| StartClass Desc LP RP SEMICOLON {
    if($1)
    {
        // Leaving class scope
        LeaveScope($1);
        $1->description = $2;
    }

    // Leave architecture scope 
    revert_arch();
}

| StartClass Desc LP error RP SEMICOLON {
    SyntaxError("Incorrect parameter list definition");

    if($1)
    {
        // Leaving class scope
        LeaveScope($1);
    }

    // Leave architecture scope 
    revert_arch();
}
;

ReqArch		: LS ARCH_NAME RS {
    // Any definitions are local to this architecture

    // Leave the free architecture
    LeaveScope();

    // Enter the specified architecture
    EnterScope($2);

    $$ = $2;
}

| LS NAME RS {
    char msg[256];
    sprintf(msg,"Undefined architecture: %s",$2);
    SyntaxError(msg);
    $$ = NULL;
}
;

StartClass	: DEFAGENT TYPE_NAME NAME {
    Symbol *p = new Symbol(AGENT_CLASS);

    p->arch = (Symbol *)choosen_architecture;
    p->data_type = $2;
    p->name = $3;

    Symbol *dup;
    if((dup = DefineName(p)) != NULL)
    {
        char buf[256];
        sprintf(buf, "Duplicate definition of name '%s'",p->name);
        SyntaxError(buf);
    }

    // Any parm definitions are now local to this class
    EnterScope(p);

    $$ = p;
}

| DEFAGENT ReqArch BINDS AGENT_CLASS TYPE_NAME NAME {
    Symbol *p = new Symbol(AGENT_CLASS);

    p->arch = $2;
    p->binds_to = $4;
    p->data_type = $5;
    p->name = $6;

    Symbol *dup;
    if((dup = DefineName(p)) != NULL)
    {
        char buf[256];
        sprintf(buf, "Duplicate definition of name '%s'",p->name);
        SyntaxError(buf);
    }

    // Any parm definitions are now local to this class
    EnterScope(p);

    $$ = p;
}

| DEFAGENT ReqArch BINDS NAME TYPE_NAME NAME {
    Symbol *bt_rec = LookupName(&free_arch->table,$4);
    if(bt_rec == NULL)
    {
        char buf[256];
        sprintf(buf, "Undefined binding source '%s'",$4);
        SyntaxError(buf);
    }

    Symbol *p = new Symbol(AGENT_CLASS);
    p->arch = $2;
    p->binds_to = bt_rec;
    p->data_type = $5;
    p->name = $6;

    Symbol *dup;
    if((dup = DefineName(p)) != NULL)
    {
        char buf[256];
        sprintf(buf, "Duplicate definition of name '%s'",p->name);
        SyntaxError(buf);
    }

    // Any parm definitions are now local to this class
    EnterScope(p);

    $$ = p;
}

| DEFAGENT TYPE_NAME AGENT_CLASS {
    char buf[256];
    sprintf(buf, "Duplicate definition of agent: %s",$3->name);
    SyntaxError(buf);
    $$ = NULL;
}

| DEFAGENT ReqArch TYPE_NAME NAME {
    char buf[256];
    sprintf(buf, "Missing binding clause in definition of agent %s",$4);
    SyntaxError(buf);
    $$ = NULL;
}

| DEFAGENT ReqArch BINDS AGENT_CLASS TYPE_NAME AGENT_CLASS {
    char buf[256];
    sprintf(buf, "Duplicate definition of agent: %s",$6->name);
    SyntaxError(buf);
    $$ = NULL;
}
;

typed_parm	: TYPE_NAME NAME {
    Symbol *p = new Symbol(PARM_NAME);

    p->data_type = $1;
    p->name = $2;

    Symbol *dup;
    if((dup = DefineName(p)) != NULL)
    {
        char buf[256];
        sprintf(buf, "Duplicate definition of parameter '%s'",p->name);
        SyntaxError(buf);
    }

    // return this parameter
    $$ = p;
}

| CONST TYPE_NAME NAME {
    Symbol *p = new Symbol(PARM_NAME);

    p->data_type = $2;
    p->name = $3;
    p->constant = true;

    Symbol *dup;
    if((dup = DefineName(p)) != NULL)
    {
        char buf[256];
        sprintf(buf, "Duplicate definition of parameter '%s'",p->name);
        SyntaxError(buf);
    }

    // return this parameter
    $$ = p;
}

| CONST TYPE_NAME NAME ASSIGN INITIALIZER {
    Symbol *p = new Symbol(PARM_NAME);

    p->data_type = $2;
    p->name = $3;
    p->constant = true;

    // Build a data record for the constant and attach it as a default value
    Symbol *inst = new Symbol(INITIALIZER);
    inst->name = $5;
    inst->data_type = $2;
    p->input_generator = inst;

    Symbol *dup;
    if((dup = DefineName(p)) != NULL)
    {
        char buf[256];
        sprintf(buf, "Duplicate definition of parameter '%s'",p->name);
        SyntaxError(buf);
    }

    // return this parameter
    $$ = p;
}
;

AParm 		: typed_parm {
    // If we are a member of a related parm list, remember the head
    if(parm_list_head)
    {
        $1->parm_list_head = parm_list_head;
        $1->is_list = true;
        $1->symbol_type = PARM_HEADER;
    }

    $$ = $1;
}

| typed_parm RS {
    // If we are a member of a related parm list, remember the head
    if(parm_list_head)
    {
        $1->parm_list_head = parm_list_head;
        $1->is_list = true;
        $1->symbol_type = PARM_HEADER;
    }
    else
        SyntaxError("Stray ']'");

    parm_list_head = NULL;

    $$ = $1;
}

| LIST typed_parm {
    if(parm_list_head)
        SyntaxError("Already within a related list");

    $2->is_list = true;
    $2->symbol_type = PARM_HEADER;
    $$ = $2;
}

| LIST LS typed_parm {
    // If we are not already in a related parm list, remember the head
    if(parm_list_head == NULL)
        parm_list_head = $3;
    else
        SyntaxError("Already within a related list");

    $3->is_list = true;
    $3->symbol_type = PARM_HEADER;
    $$ = $3;
}
;

ParmDef		: ParmDef COMMA AParm {
    $$ = $1;
    $$->append($3);
}

| AParm {
    // return this parameter
    $$ = new SymbolList($1);
}
;

SorA		: ACTUATOR_NAME
| SENSOR_NAME
;

Loc		: LA NUMBER COMMA NUMBER RA {
    $$.x = (int)$2;
    $$.y = (int)$4;
}

| {
    $$.x = 100;
    $$.y = 100;
}
		;

StartAgent	: INSTAGENT Loc Desc NAME FROM AGENT_CLASS {
    Symbol *p = new Symbol(AGENT_NAME);
    p->construction = CS_CLASS;

    p->name = $4;
    p->defining_rec = $6;
    p->location = $2;
    p->description = $3;

    Symbol *dup;
    if((dup = DefineName(p)) != NULL)
    {
        char buf[256];
        sprintf(buf, "Duplicate definition of name '%s'",p->name);
        SyntaxError(buf);
    }

    // Any parm definitions are now local to this class
    EnterScope(p);

    $$ = p;
}

| INSTAGENT Loc Desc NAME FROM SorA {
    Symbol *p = new Symbol(AGENT_NAME);
    p->construction = $6->construction;

    p->name = $4;
    p->defining_rec = $6;
    p->location = $2;
    p->description = $3;

    // Force the binding operation
    p->bound_to = $6;
    if($6->bound_to)
    {
        char buf[256];
        sprintf(buf, "Unable to bind %s.  The binding point %s is already bound to %s",
                p->name, $6->name, $6->bound_to->name);
        SyntaxError(buf);
    }
    else
    {
        $6->bound_to = p;
    }

    Symbol *dup;
    if((dup = DefineName(p)) != NULL)
    {
        char buf[256];
        sprintf(buf, "Duplicate definition of name '%s'",p->name);
        SyntaxError(buf);
    }

    // Any parm definitions are now local to this class
    EnterScope(p);

    $$ = p;
}

| INSTAGENT Loc Desc NAME FROM NAME {
    char buf[256];
    sprintf(buf, "Attempt to instantiate undefined agent: %s",$6);
    SyntaxError(buf);

    $$ = NULL;
}

/* This rule is here to allow loading files created with bad sensor/actuator
   rules.  Take something like:

	instAgent get_objects from detect_objs:DETECT_OBJECTS(
  		max_sensor_range = {9999});

   and pretend it was:

	   instBP get_objects from sense_objects(
     		bound_to = detect_objs:DETECT_OBJECTS(
	     		max_sensor_range = {9999}));
*/

| INSTAGENT Loc Desc NAME FROM StartSorARef {
    // Use the name on the hardware record.
    Symbol *hdw = $6;
    hdw->location = $2;
    hdw->description = $3;

    // Add it to the symbol table, and make sure not a dup name
    Symbol *dup;
    if((dup = DefineName(hdw)) != NULL)
    {
        char buf[256];
        sprintf(buf, "Duplicate definition of name '%s'",hdw->name);
        SyntaxError(buf);
    }       
                 
    Symbol *bp_class = hdw->defining_rec->binds_to;

    // Define the binding point
    Symbol *p = bp_class->inst_of($4);
      
    // Add it to the symbol table, and make sure not a dup name
    if((dup = DefineName(p)) != NULL)
    {
        char buf[256];
        sprintf(buf, "Duplicate definition of name '%s'",p->name);
        SyntaxError(buf);
    }       
                 
    // Force the binding operation
    p->bound_to = hdw;
    if(hdw->bound_to)
    {
        char buf[256];
        sprintf(buf, "Unable to bind %s.  The binding point %s is already bound to %s",
                p->name, hdw->name, hdw->bound_to->name);
        SyntaxError(buf);
    }
    else
    {
        hdw->bound_to = p;
    }

    // Any parm definitions are now local to this class
    EnterScope(hdw);

    $$ = hdw;
}
;

InstAgent	: StartAgent ParmSet SEMICOLON {
    if($1)
    {
        // Leaving class scope
        LeaveScope($1);

        if($2)
        {
            AddUser($2,$1);
            // Hookup the params
            $1->parameter_list = *($2);
        }

        // Reset the record class for the parms
        $1->ImportParmList();
    }

    // Delete the old copies
    if($2)
        delete $2;
}
;

ParmSet		: LP Glist RP {
    // ::::::: WATCH THIS ::::::
    // We are looking at the prev stack symbol
    Symbol *base_rec = $<symbol>0;

    if(base_rec && base_rec->defining_rec)
    {
        if (base_rec->symbol_type == COORD_NAME && 
            base_rec->defining_rec->operator_style == FSA_STYLE) 
        {
            // Insert a group record before the member agents that
            // don't have one.
            Symbol *p;
            void *cur;           
            if ((cur = $2->first(&p)) != NULL)
            {     
                do 
                {  
                    if (p->symbol_type == PARM_HEADER &&
                        p->data_type == member_type)
                    {     
                        Symbol *lp;
                        void *cur;           
                        if ((cur = p->parameter_list.first(&lp)) != NULL)
                        {     
                            do 
                            {  
                                // insert a group record before each child that
                                // is a simple agent.
                                if(lp->input_generator &&
                                   lp->input_generator->symbol_type != GROUP_NAME)
                                {
                                    Symbol *g = new Symbol(GROUP_NAME);
                                    g->name = AnonymousName();
                                    g->children.append(lp->input_generator);
                                    lp->input_generator = g;
                                }
                            } while ((cur = p->parameter_list.next(&lp, cur)) != NULL);
                        }
                        break;
                    }
                } while ((cur = $2->next(&p, cur)) != NULL);
            }     
        }

        fill_parm_list(base_rec->defining_rec, &($2));
    }

    $$ = $2;
}

| LP RP {
    // ::::::: WATCH THIS ::::::
    // We are looking at the prev stack symbol
    Symbol *base_rec = $<symbol>0;
    SymbolList *p = NULL;

    if(base_rec && base_rec->defining_rec)
    {
        fill_parm_list(base_rec->defining_rec, &p);
    }
    $$ = p;
}

| LP error RP {
    SyntaxError("Incorrect parameter list");
    $$ = NULL;
}
		;

Glist		: Link {
    if($1)
    {
        // Return the parm list
        $$ = new SymbolList($1);
    }
    else
        $$ = NULL;
}

| Glist COMMA Link {
    if($1)
    {
        // Merge the new parm into the list, preventing duplicates
        if($3)
            $1->merge($3);

        $$ = $1;
    }
    else if($3)
    {
        // Return the parm list
        $$ = new SymbolList($3);
    }
    else
        $$ = NULL;
}
;

RobotParms	: LP RobotLinks RP {
    // ::::::: WATCH THIS ::::::
    // We are requiring that the prev stack symbol is the robot record
    Symbol *robot_rec = $<symbol>0;
    if(robot_rec == NULL ||
       (robot_rec->symbol_type !=ROBOT_NAME && robot_rec->symbol_type !=BP_NAME))
    {
        char buf[256];
        sprintf(buf, "Illegal parent record on parse stack in RobotParms");
        SyntaxError(buf);
    }
    else
    {
        // Look for the group entry
        bool found_one = false;
        bool restart;
        do
        {
            restart = false;
            Symbol *p;
            void *cur;           
            if ((cur = $2->first(&p)) != NULL)
            {     
                do 
                {  
                    if (p->symbol_type == GROUP_NAME)
                    {     
                        // Remove it
                        $2->remove(p);
                        restart = true;

                        if(found_one)
                        {
                            char buf[256];
                            sprintf(buf, "Only one group of children is allowed in the parameter list");
                            SyntaxError(buf);
                        }
                        else
                        {
                            // Add the children to our parent
                            robot_rec->children = p->children;

                            // The children are now used by our parent, not the group
                            Symbol *kid;
                            void *cur_ptr;           
                            if ((cur_ptr = robot_rec->children.first(&kid)) != NULL)
                            {     
                                do 
                                {  
                                    kid->users.remove(p);
                                    kid->users.append(robot_rec);
                                } while((cur_ptr=robot_rec->children.next(&kid, cur_ptr)) != NULL);
                            }     
                        }

                        // Get rid of the group record
                        delete p;
                        found_one = true;
                    }
                } while (!restart && (cur = $2->next(&p, cur)) != NULL);
            }     
        } while(restart);
    }

    $$ = $2;
}
| LP RP {
	$$ = NULL;
}
;

RobotLink	: Agent
| Link
;

RobotLinks	: RobotLink {
    if($1)
    {
        // Return the parm list
        $$ = new SymbolList($1);
    }
    else
        $$ = NULL;
}

| RobotLinks COMMA RobotLink {
    if($1)
    {
        // Merge the new parm into the list, preventing duplicates
        if($3)
            $1->merge($3);

        $$ = $1;
    }
    else if($3)
    {
        // Return the parm list
        $$ = new SymbolList($3);
    }
    else
        $$ = NULL;
}
;

Link		: LHS ASSIGN Agent {
    if($1)
    {
        // Attach the input
        // Note: The new parameter record is the LAST one.
        Symbol *p;
        if(!$1->parameter_list.last(&p))
            p = $1;
        p->input_generator = $3;
    }

    // Return the symbol
    $$ = $1;

    // RHS is done
    is_lhs = true;
}

| LHS ASSIGN UP {
    if($1)
    {
        // Attach the input
        // Note: The new parameter record is the LAST one.
        Symbol *p;
        if(!$1->parameter_list.last(&p))
            p = $1;
        p->input_generator = (Symbol *)UP;
    }

    // Return the symbol
    $$ = $1;

    // RHS is done
    is_lhs = true;
}

| LHS ASSIGN START_PU_INITIALIZER RB {
    // Standard pushed up parameter reference.
    if($1)
    {
        // Build a data record for the constant
        Symbol *inst = new Symbol(PU_INITIALIZER);

        // Parameter keeps the same name
        inst->name = NULL;

        // Parameter keeps the same type
        inst->data_type = NULL;
   
        // Attach the input
        // Note: The new parameter record is the LAST one.
        Symbol *p;
        if(!$1->parameter_list.last(&p))
            p = $1;
        p->input_generator = inst;
    }

    // Return the symbol
    $$ = $1;

    // RHS is done
    is_lhs = true;
}

| LHS ASSIGN START_PU_INITIALIZER TYPE_NAME PU_PARM_NAME RB {
    // Pushed up parm reference with parameter rename and retype.
    if($1)
    {
        // Build a data record for the constant
        Symbol *inst = new Symbol(PU_INITIALIZER);
   
        // Set the new name for this parameter
        inst->name = $5;

        // Set the new type for this parameter
        inst->data_type = $4;

        // Attach the input
        // Note: The new parameter record is the LAST one.
        Symbol *p;
        if(!$1->parameter_list.last(&p))
        {
            p = $1;
        }

        p->input_generator = inst;
    }

    // Return the symbol
    $$ = $1;

    // RHS is done
    is_lhs = true;
}

| LHS ASSIGN START_PU_INITIALIZER_ADV TYPE_NAME PU_PARM_NAME RB {
    // Pushed up parm reference with parameter rename and retype.
    if($1)
    {
        // Build a data record for the constant
        Symbol *inst = new Symbol(PU_INITIALIZER);
   
        // Set the new name for this parameter
        inst->name = $5;

        // Set the new type for this parameter
        inst->data_type = $4;

        // Set as the advanced parameter.
        inst->is_advanced_parm = true;

        // Attach the input
        // Note: The new parameter record is the LAST one.
        Symbol *p;
        if(!$1->parameter_list.last(&p))
        {
            p = $1;
        }

        p->input_generator = inst;
    }

    // Return the symbol
    $$ = $1;

    // RHS is done
    is_lhs = true;
}

| LHS ASSIGN START_PU_INITIALIZER TYPE_NAME START_PAIR_PU_PARM_NAMES PU_PARM_NAME PU_PARM_NAME RB RB {
    // Pushed up parm reference with parameter rename and retype.
    if($1)
    {
        // Build a data record for the constant
        Symbol *inst1 = new Symbol(PU_INITIALIZER);
        Symbol *inst2 = new Symbol(PU_INITIALIZER);
   
        // Set the new names for this parameter
        inst1->name = $6;
        inst2->name = $7;

        // Set the new type for this parameter
        inst1->data_type = $4;
        inst2->data_type = $4;

        // Set the inst2 as inst1's pair symbol.
        inst2->is_pair = true;
        inst1->pair = inst2;

        // Attach the input
        // Note: The new parameters record is the LAST one.
        Symbol *p;

        if(!$1->parameter_list.last(&p))
        {
            p = $1;
        }

        p->input_generator = inst1;
    }

    // Return the symbol
    $$ = $1;

    // RHS is done
    is_lhs = true;
}

| LHS ASSIGN START_PU_INITIALIZER_ADV TYPE_NAME START_PAIR_PU_PARM_NAMES PU_PARM_NAME PU_PARM_NAME RB RB {
    // Pushed up parm reference with parameter rename and retype.
    if($1)
    {
        // Build a data record for the constant
        Symbol *inst1 = new Symbol(PU_INITIALIZER);
        Symbol *inst2 = new Symbol(PU_INITIALIZER);
   
        // Set the new names for this parameter
        inst1->name = $6;
        inst2->name = $7;

        // Set the new type for this parameter
        inst1->data_type = $4;
        inst2->data_type = $4;

        // Set as the advanced parameter.
        inst1->is_advanced_parm = true;
        inst2->is_advanced_parm = true;

        // Set the inst2 as inst1's pair symbol.
        inst2->is_pair = true;
        inst1->pair = inst2;

        // Attach the input
        // Note: The new parameter record is the LAST one.
        Symbol *p;

        if(!$1->parameter_list.last(&p))
        {
            p = $1;
        }

        p->input_generator = inst1;
    }

    // Return the symbol
    $$ = $1;

    // RHS is done
    is_lhs = true;
}

| LHS ASSIGN Rule {
    if($1)
    {
        // Attach the input
        // Note: The new parameter record is the LAST one.
        Symbol *p;
        if(!$1->parameter_list.last(&p))
            p = $1;
        p->input_generator = $3;
    }

    // Return the symbol
    $$ = $1;

    // RHS is done
    is_lhs = true;
}

| LHS ASSIGN INITIALIZER {
    if($1)
    {
        // Build a data record for the constant
        Symbol *inst = new Symbol(INITIALIZER);
        inst->name = $3;
   
        // Attach the input
        // Note: The new parameter record is the LAST one.
        Symbol *p;

        if(!$1->parameter_list.last(&p))
        {
            p = $1;
        }

        p->input_generator = inst;

        // Copy the data type to the initilizer record
        inst->data_type = p->data_type;
    }

    // Return the symbol
    $$ = $1;

    // RHS is done
    is_lhs = true;
}

| LHS ASSIGN {
    if($1)
    {
        // Attach the input
        // Note: The new parameter record is the LAST one.
        Symbol *p;
        if(!$1->parameter_list.last(&p))
        {
            p = $1;
        }

        p->input_generator = NULL;
    }

    // Return the new LHS symbol
    $$ = $1;

    // RHS is done
    is_lhs = true;
}

| LHS ASSIGN LP RobotLinks RP {
    if($1)
    {
        // Make sure lhs is a list
        if($1->symbol_type != PARM_HEADER)
        {
            char buf[256];
            sprintf(buf, "Group lists are only supported with list parameters: %s",
                    $1->name);
            SyntaxError(buf);
            $1 = NULL;
        }
        else
        {
            // Find the index record, Note: The new parameter record is the LAST one.
            Symbol *rec;
            if(!$1->parameter_list.last(&rec))
            {
                char buf[256];
                sprintf(buf, "Group lists are only supported with list parameters: %s",
                        $1->name);
                SyntaxError(buf);
            }
            else
            {
                Symbol *index = rec->list_index;

                // Move any pu parms from the RobotLinks list to the index record
                Symbol *p;
                void *cur;           
                bool restart;
                do
                {
                    restart = false;
                    if ((cur = $4->first(&p)) != NULL)
                    {     
                        do 
                        {  
                            if (p->symbol_type == PU_INPUT_NAME)
                            {     
                                // Remove it
                                $4->remove(p);
   
                                // Add the parm to the index record
                                index->parameter_list.merge(p);
   
                                // Since mucked with the list, need to restart processing
                                restart = true;
                                break;
                            }
                        } while ((cur = $4->next(&p, cur)) != NULL);
                    }     
                }
                while(restart);

                int len = $4->len();
                if(len > 1)
                {
                    char buf[256];
                    sprintf(buf, "Only allowed 1 group entry: %s",
                            $1->name);
                    SyntaxError(buf);
                }
                else if(len == 1)
                {
                    // Move the agent from the list
                    rec->input_generator = $4->get();
                }

                // Dump the list
                delete $4;
            }
        }
    }
    $$ = $1;

    // RHS is done
    is_lhs = true;
}
;

LHS		: PARM_NAME Loc Desc { // Left hand side of assignment
    // Build a data record for this instance of the parameter reference
    Symbol *pr = $1;
    Symbol *inst = new Symbol;
    inst->name = strdup(pr->name);
    inst->data_type = pr->data_type;
    inst->symbol_type = INPUT_NAME;
    inst->location = $2;
    inst->description = $3;

    // Return the data record
    $$ = inst;

    // LHS is done
    is_lhs = false;
}

| PARM_NAME LS INDEX_NAME RS Loc Desc {
    char buf[256];
    sprintf(buf, "list reference to non-list parameter '%s'",$1->name);
    SyntaxError(buf);
    $$ = $1;

    // LHS is done
    is_lhs = false;
}

| PARM_NAME LS NAME RS Loc Desc {
    char buf[256];
    sprintf(buf, "list reference to non-list parameter '%s'",$1->name);
    SyntaxError(buf);
    $$ = $1;

    // LHS is done
    is_lhs = false;
}

| PARM_HEADER LS NAME RS Loc Desc {

    /* +++++++DCM: Start of changes to eliminate extra memory useage +++++++*/
    //fprintf(stderr,"\nA: %s[%s]",$1->name, $3);

    // Make sure we only make one copy per parm list so we can merge list assignments
    Symbol *hdr = NameIsDefined($1);

    if(hdr == NULL)
    {
        // Make sure that we have a user copy of the parm_header symbol to attach our indicies.
        hdr = $1->dup(false, $1->name);
        hdr->record_class = current_class;

        if(DefineName(hdr))
        {
            fprintf(stderr,"Internal Error in cdl_gram.y: DefineName failed after NameIsDefined succeeded\n");
        }
    }
    /* ++++++++DCM: End of changes to eliminate extra memory useage ++++++++*/


    // References to [xxx] on the left hand side define the name xxx
    Symbol *c = new Symbol(INDEX_NAME);
    c->name = $3;
    c->location = $5;
    c->description = $6;
    c->index_value = hdr->index_value;
    hdr->index_value++;
    Symbol *dup;
    if((dup = DefineName(c)) != NULL)
    {
        char buf[256];
        sprintf(buf, "Duplicate definition of name '%s'",c->name);
        SyntaxError(buf);

	    c = dup;
    }

    // Add this index to the parm_header's symbol table
    hdr->table.put(c->name,c);

    /* +++++++DCM: Start of changes to eliminate extra memory useage +++++++*/
    // Build a data record for this instance of the parameter reference
    Symbol *inst = new Symbol;
    inst->name = strdup(hdr->name);
    inst->symbol_type = INPUT_NAME;
    inst->list_index = c;
    inst->record_class = current_class;

    /* ++++++++DCM: End of changes to eliminate extra memory useage ++++++++*/

    // Add it to the list in the header record
    hdr->parameter_list.append(inst);

    // Return the header record
    $$ = hdr;

    // LHS is done
    is_lhs = false;
}

| PARM_HEADER LS INDEX_NAME RS Loc Desc {

    /* +++++++DCM: Start of changes to eliminate extra memory useage +++++++*/
    //fprintf(stderr,"\nB: %s[%s]",$1->name, $3->name);

    // Make sure we only make one copy per parm list so we can merge list assignments
    Symbol *hdr = NameIsDefined($1);

    if(hdr == NULL)
    {
        // Make sure that we have a user copy of the parm_header symbol to attach our indicies.
        hdr = $1->dup(false, $1->name);
        hdr->record_class = current_class;

        if(DefineName(hdr))
        {
            fprintf(stderr,"Internal Error in cdl_gram.y: DefineName failed after NameIsDefined succeeded\n");
        }
    }

    // Build a data record for this instance of the parameter reference
    Symbol *inst = new Symbol;
    inst->name = strdup(hdr->name);
    inst->symbol_type = INPUT_NAME;
    inst->list_index = $3;
    inst->record_class = current_class;

    /* ++++++++DCM: End of changes to eliminate extra memory useage ++++++++*/


    // If was defined as a forward reference, then give it a value
    if($3->index_value == UNDEFINED_INDEX)
    {
	    $3->index_value = hdr->index_value;
	    hdr->index_value++;

	    // Add this index to the parm_header's symbol table
        hdr->table.put($3->name,$3);
    }

    // Define the state location
    if(loc_is_default($3->location))
	    $3->location = $5;
    if($3->description == NULL)
        $3->description = $6;

    // Add it to the list in the header record
    hdr->parameter_list.append(inst);

    // Return the header record
    $$ = hdr;

    // LHS is done
    is_lhs = false;
}

| PARM_HEADER Loc Desc { 
    char buf[256];
    sprintf(buf, "non-list reference to list parameter '%s'",$1->name);
    SyntaxError(buf);
    $$ = $1;

    // LHS is done
    is_lhs = false;
}

| PU_PARM_NAME Loc Desc { // Left hand side of assignment
    // Build a data record for this pushed-up parameter
    Symbol *inst = new Symbol;
    inst->name = $1;
    inst->symbol_type = PU_INPUT_NAME;
    inst->location = $2;
    inst->description = $3;

    // Return the data record
    $$ = inst;

    // LHS is done
    is_lhs = false;
}

| NAME Loc Desc {  // Error handling
    char buf[256];
    sprintf(buf, "Assignment to undefined parameter '%s'",$1);
    SyntaxError(buf);
    $$ = NULL;

    // LHS is done
    is_lhs = false;
}
;

RuleHead	: IF LS AgentList RS Loc Desc GOTO {
    Symbol *p = new Symbol(RULE_NAME);
    p->name = AnonymousName();

    // enclose the agent in a group
    Symbol *g = new Symbol(GROUP_NAME);
    g->name = AnonymousName();
    g->location = $5;
    g->description = $6;

    if($3)
    {
        Symbol *t;
        while(!$3->isempty())
        { 
            // Get the next member
            t = $3->get();

            // If it is a pushed up parm, add it to the parm list for the group
            if(t->symbol_type == PU_INPUT_NAME)
                g->parameter_list.append(t);
            else
            {
                // Add the new child
                g->children.append(t);

                // Add the group as a user
                t->users.append(g);
            }
        }

        // Delete the container
        delete $3;
    }

    // Connect the generator
    p->input_generator = g;
    $$ = p;
}

| IF LS RS Loc Desc GOTO {
    Symbol *p = new Symbol(RULE_NAME);
    p->name = AnonymousName();

    // hang a group under the transition
    Symbol *g = new Symbol(GROUP_NAME);
    g->name = AnonymousName();
    g->location = $4;
    g->description = $5;
    // No children

    // Connect the generator
    p->input_generator = g;
    $$ = p;
}

| IF LS NAME RS Loc Desc GOTO {
    char buf[256];
    sprintf(buf, "Reference to undefined trigger: %s",$3);
    SyntaxError(buf);
    $$ = NULL;

    // LHS is done
    is_lhs = false;
    Symbol *p = new Symbol(RULE_NAME);
    p->name = AnonymousName();

    // hang a group under the transition
    Symbol *g = new Symbol(GROUP_NAME);
    g->name = AnonymousName();
    g->location = $5;
    g->description = $6;
    // No children

    // Connect the generator
    p->input_generator = g;
    $$ = p;
}

| IF GOTO {
    Symbol *p = new Symbol(RULE_NAME);
    p->name = AnonymousName();

    // hang a group under the transition
    Symbol *g = new Symbol(GROUP_NAME);
    g->name = AnonymousName();
    // Default location
    // No children

    // Connect the generator
    p->input_generator = g;
    $$ = p;
}
;

Rule		: RuleHead INDEX_NAME {
    $1->list_index = $2;
    $$ = $1;
}

| RuleHead NAME {
    // forward References to [xxx] on the right hand side define the name xxx
    Symbol *c = new Symbol(INDEX_NAME);
    c->name = $2;
    c->index_value = UNDEFINED_INDEX;
    Symbol *dup;
    if((dup = DefineName(c)) != NULL)
    {
        char buf[256];
        sprintf(buf, "Duplicate definition of name '%s'",c->name);
        SyntaxError(buf);
    }
    $1->list_index = c;

    $$ = $1;
}
;

MaybeName	: INLINE_NAME {
    $$ = $1;
}
| {
    $$ = AnonymousName();
}
;

StartRefAgent	: MaybeName AGENT_CLASS {
    Symbol *p = new Symbol(AGENT_NAME);
    p->construction = CS_CLASS;

    p->defining_rec = $2;
    p->name = $1;

    Symbol *dup;
    if((dup = DefineName(p)) != NULL)
    {
        char buf[256];
        char *new_name = AnonymousName();
        sprintf(buf, "Duplicate definition of name: %s\nName changed to %s",p->name,new_name);
        SyntaxError(buf);
        p->name = new_name;
        dup = DefineName(p);
        assert(dup == NULL);
    }

    // Any parm definitions are now local to this agent
    EnterScope(p);

    $$ = p;
}
;

RefAgent		: StartRefAgent ParmSet Loc Desc {
    // Hmm, do I need to return a value here?
    if($1)
    {
        $1->location = $3;
        $1->description = $4;
        LeaveScope($1);
        if($2)
        {
            AddUser($2,$1);
            $1->parameter_list = *($2);
        }
        else
        {
            // Copy the parm defs from our parent
            $1->parameter_list = $1->defining_rec->parameter_list;
        }

        // Reset the record class for the parms
        $1->ImportParmList();
    }
    if($2)
        delete $2;

    $$ = $1;
}
;

InstRobot	: StartRobotInst RobotParms {
    if($1)
    {
        LeaveScope($1);

        if($2)
        {
            AddUser($2,$1);
            $1->parameter_list = *($2);
        }

        // Reset the record class for the parms
        $1->ImportParmList();
    }
    if($2) 
        delete $2;
}
;

StartRobotInst	: INSTROBOT Loc Desc NAME FROM ROBOT_CLASS {

    // Define a symbol for this SEQ coord instance.
    Symbol *p = new Symbol(ROBOT_NAME);
    p->name = $4;
    p->defining_rec = $6;
    p->location = $2;
    p->description = $3;

    // Add it to the symbol table, and make sure not a dup name
    Symbol *dup;
    if((dup = DefineName(p)) != NULL)
    {
        char buf[256];
        sprintf(buf, "Duplicate definition of name '%s'",p->name);
        SyntaxError(buf);
    }

    // Any parm definitions are now local
    EnterScope(p);

    $$ = p;
}
;

InstCoord	: StartCoordInst ParmSet SEMICOLON {
    if($1)
    {
        LeaveScope($1);
        if($2)
        {
            AddUser($2,$1);
            $1->parameter_list = *($2);
        }

        // Reset the record class for the parms
        $1->ImportParmList();
    }
    if($2) 
        delete $2;
}
;

StartCoordInst	: INSTCOORD Loc Desc NAME FROM COORD_CLASS {

    // Define a symbol for this SEQ coord instance.
    Symbol *p = new Symbol(COORD_NAME);
    p->name = $4;
    p->defining_rec = $6;
    p->location = $2;
    p->description = $3;

    // Add it to the symbol table, and make sure not a dup name
    Symbol *dup;
    if((dup = DefineName(p)) != NULL)
    {
        char buf[256];
        sprintf(buf, "Duplicate definition of name '%s'",p->name);
        SyntaxError(buf);
    }

    // Any parm definitions are now local
    EnterScope(p);

    $$ = p;
}
;

RefRobot	: StartRefRobot RobotParms Loc Desc {
    assert ($1);

    $1->location = $3;
    $1->description = $4;
    LeaveScope($1);

    if($2)
    {
        AddUser($2,$1);
        $1->parameter_list = *($2);
        delete $2;

        // Reset the record class for the parms
        $1->ImportParmList();
    }

    $$ = $1;
}
;

StartRefRobot	: MaybeName ROBOT_CLASS {
    Symbol *p = new Symbol(ROBOT_NAME);

    p->defining_rec = $2;
    p->name = $1;

    // Define the new symbol name
    Symbol *is_dup = DefineName(p);

    // Make sure isn't a duplicate
    assert(is_dup == NULL);

    // Any parm definitions are now local to this agent
    EnterScope(p);

    $$ = p;
}
;

RefCoord	: StartCoordRef ParmSet Loc Desc {
    assert($1);

    $1->location = $3;
    $1->description = $4;
    LeaveScope($1);

    if($2)
    {
        AddUser($2,$1);
        $1->parameter_list = *($2);

        delete $2;
    }
    else
    {
        // Copy the parm defs from our parent
        $1->parameter_list = $1->defining_rec->parameter_list;
    }


    // Reset the record class for the parms
    $1->ImportParmList();
    $$ = $1;
}
;

StartCoordRef	: MaybeName COORD_CLASS {
    Symbol *p = new Symbol(COORD_NAME);

    p->defining_rec = $2;
    p->name = $1;

    // Define the new symbol name
    Symbol *is_dup;
    if((is_dup = DefineName(p)) != NULL)
    {
        char buf[256];
        char *new_name = AnonymousName();
        sprintf(buf, "Duplicate definition of name: %s\nName changed to %s",p->name,new_name);
        SyntaxError(buf);
        p->name = new_name;
        is_dup = DefineName(p);
        assert(is_dup == NULL);
    }

    // Make sure isn't a duplicate
    assert(is_dup == NULL);

    // Any parm definitions are now local to this agent
    EnterScope(p);

    $$ = p;
}
;

RefSorA		: StartSorARef ParmSet Loc Desc {
    if($1)
    {
        LeaveScope($1);
        $1->location = $3;
        $1->description = $4;
        if($2)
        {
            AddUser($2,$1);
            $1->parameter_list = *($2);
        }

        // Reset the record class for the parms
        $1->ImportParmList();
    }
    if($2)
        delete $2;

    $$ = $1;
}
;

SorAclass	: ACTUATOR_CLASS
| SENSOR_CLASS
;

StartSorARef	: MaybeName SorA Loc Desc {
    Symbol *p = $2->inst_of($1);
    p->location = $3;
    p->description = $4;

    Symbol *dup;
    if((dup = DefineName(p)) != NULL)
    {
        char buf[256];
        char *new_name = AnonymousName();
        sprintf(buf, "Duplicate definition of name: %s\nName changed to %s",p->name,new_name);
        SyntaxError(buf);
        p->name = new_name;
        dup = DefineName(p);
        assert(dup == NULL);
    }

    // Any parm definitions are now local to this agent
    EnterScope(p);

    $$ = p;
}

| INLINE_NAME INLINE_NAME SorAclass Loc Desc {
    Symbol *n = $3->inst_of($2);
    n->location = $4;
    n->description = $5;

    Symbol *dup;
    if((dup = DefineName(n)) != NULL)
    {
        char buf[256];
        sprintf(buf, "Duplicate name: %s",n->name);
        SyntaxError(buf);
    }

    Symbol *agent = n->inst_of($1);
    agent->location = $4;

    // The agent should be bound to the SorA_name
    agent->bound_to = n;
    n->bound_to = agent;
    n->users.append(agent);
   

    if((dup = DefineName(agent)) != NULL)
    {
        char buf[256];
        char *new_name = AnonymousName();
        sprintf(buf, "Duplicate definition of name: %s\nName changed to %s",agent->name,new_name);
        SyntaxError(buf);
        agent->name = new_name;
        dup = DefineName(agent);
        assert(dup == NULL);
    }

    // Any parm definitions are now local to this agent
    EnterScope(agent);

    $$ = agent;
}

| INLINE_NAME SorAclass Loc Desc {
    Symbol *n = $2->inst_of($1);
    n->location = $3;
    n->description = $4;

    Symbol *dup;
    if((dup = DefineName(n)) != NULL)
    {
        char buf[256];
        sprintf(buf, "Duplicate name: %s",n->name);
        SyntaxError(buf);
    }

    Symbol *agent = n->inst_of();
    agent->location = $3;

    // The agent should be bound to the SorA_name
    agent->bound_to = n;
    n->bound_to = agent;
    n->users.append(agent);
   
    if((dup = DefineName(agent)) != NULL)
    {
        char buf[256];
        char *new_name = AnonymousName();
        sprintf(buf, "Duplicate definition of name: %s\nName changed to %s",agent->name,new_name);
        SyntaxError(buf);
        agent->name = new_name;
        dup = DefineName(agent);
        assert(dup == NULL);
    }

    // Any parm definitions are now local to this agent
    EnterScope(agent);

    $$ = agent;
}
;

StartSorAInst	: INSTACTUATOR Loc Desc NAME FROM ACTUATOR_CLASS {
    Symbol *p = $6->inst_of($4);
    p->location = $2;
    p->description = $3;

    // Add it to the symbol table, and make sure not a dup name
    Symbol *dup;
    if((dup = DefineName(p)) != NULL)
    {
        char buf[256];
        sprintf(buf, "Duplicate definition of name '%s'",p->name);
        SyntaxError(buf);
    }

    // Any parm definitions are now local
    EnterScope(p);

    $$ = p;
}

| INSTSENSOR Loc Desc NAME FROM SENSOR_CLASS {
    Symbol *p = $6->inst_of($4);
    p->location = $2;
    p->description = $3;

    // Add it to the symbol table, and make sure not a dup name
    Symbol *dup;
    if((dup = DefineName(p)) != NULL)
    {
        char buf[256];
        sprintf(buf, "Duplicate definition of name '%s'",p->name);
        SyntaxError(buf);
    }

    // Any parm definitions are now local
    EnterScope(p);

    $$ = p;
}
;

InstSorA	: StartSorAInst ParmSet SEMICOLON {
    if($1)
    {
        LeaveScope($1);
        if($2)
        {
            AddUser($2,$1);
            $1->parameter_list = *($2);

            // Reset the record class for the parms
            $1->ImportParmList();
        }
    }
    if($2) 
        delete $2;
}
;


StartBPRef	: MaybeName BP_CLASS {
    Symbol *p = new Symbol(BP_NAME);
    p->construction = $2->construction;
    p->defining_rec = $2;
    p->name = $1;

    // Define the new symbol name
    Symbol *dup;
    if((dup = DefineName(p)) != NULL)
    {
        char buf[256];
        sprintf(buf, "Duplicate name for binding point: %s",p->name);
        SyntaxError(buf);
    }

    // Any parm definitions are now local to this agent
    EnterScope(p);

    $$ = p;
}
;

RefBP		: StartBPRef ParmSet Loc Desc {
    if($2)
    {
        // SPECIAL CASE HERE
        // the input name "bound_to" indicates bindings.
        // Look for any assignments to this name and move them to "bound_to" parm.
        Symbol *p;
        void *cur;           
        if ((cur = $2->first(&p)) != NULL)
        {     
            do 
            {  
                if(strcmp(p->name,"bound_to") == 0)
                {     
                    // Remove it
                    $2->remove(p);

                    if(p->input_generator)
                    {
                        // retain the bound record (double linked)
                        p->input_generator->bound_to = $1;
                        if($1->bound_to)
                        {
                            char buf[256];
                            sprintf(buf, "Unable to bind %s.  The binding point %s is already bound to %s",
                                    p->name, $1->name, $1->bound_to->name);
                            SyntaxError(buf);
                        }
                        else
                        {
                            $1->bound_to = p->input_generator;
                        }
   
                        // Add the user
                        $1->bound_to->users.append($1);
                    }
   
                    delete p;
                    break;
                }
            } while ((cur = $2->next(&p, cur)) != NULL);
        }     
    }

    if($1)
    {
        LeaveScope($1);
        $1->location = $3;
        $1->description = $4;
        if($2)
        {
            AddUser($2,$1);
            $1->parameter_list = *($2);

            // Reset the record class for the parms
            $1->ImportParmList();
        }
    }
    if($2)
        delete $2;

    $$ = $1;
}
;

InstBP		: StartBPInst RobotParms SEMICOLON {
    if($1)
    {
        if($2)
        {
            // SPECIAL CASE HERE
            // the input name "bound_to" indicates bindings.
            // Look for any assignments to this name and move them to the
            // real "bound_to" parm.
            Symbol *p;
            void *cur;           
            if ((cur = $2->first(&p)) != NULL)
            {     
                do 
                {  
                    if(strcmp(p->name,"bound_to") == 0)
                    {     
                        // Remove it
                        $2->remove(p);
   
                        if(p->input_generator)
                        {
                            // retain the bound record (double linked)
                            p->input_generator->bound_to = $1;
                            if($1->bound_to)
                            {
                                char buf[256];
                                sprintf(buf, "Unable to bind %s.  The binding point %s is already bound to %s",
                                        p->name, $1->name, $1->bound_to->name);
                                SyntaxError(buf);
                            }
                            else
                            {
                                $1->bound_to = p->input_generator;
                            }
                        }
                        delete p;
                        break;
                    }
                } while ((cur = $2->next(&p, cur)) != NULL);
            }     
        }

        // Second special case, robotparms may have given us children
        if($1->construction != CS_RBP && $1->children.len())
        {
            char buf[256];
            sprintf(buf, "Only robot binding points can have children: %s",$1->name);
            SyntaxError(buf);
        }
        else if($1->construction == CS_RBP && $1->bound_to && $1->children.len())
        {
            char buf[256];
            sprintf(buf, "The children should be attached to the hardware record: %s",$1->name);
            SyntaxError(buf);
        }
        LeaveScope($1);
        if($2)
        {
            AddUser($2,$1);
            $1->parameter_list = *($2);

            // Reset the record class for the parms
            $1->ImportParmList();
        }
    }
    if($2) 
        delete $2;
}
;

StartBPInst	: INSTBP Loc Desc NAME FROM BP_CLASS {

    // Define a symbol for this instance.
    Symbol *p = new Symbol(BP_NAME);
    p->construction = $6->construction;
    p->name = $4;
    p->defining_rec = $6;
    p->location = $2;
    p->description = $3;

    // Add it to the symbol table, and make sure not a dup name
    Symbol *dup;
    if((dup = DefineName(p)) != NULL)
    {
        char buf[256];
        sprintf(buf, "Duplicate definition of name '%s'",p->name);
        SyntaxError(buf);
    }

    // Any parm definitions are now local
    EnterScope(p);

    $$ = p;
}

| INSTBP Loc Desc BP_NAME FROM BP_CLASS {
    char buf[256];
    sprintf(buf, "Duplicate definition of binding point '%s'",$4->name);
    SyntaxError(buf);

    $$ = NULL;
}
;

RefGroup	: MaybeName LS AgentList RS Loc Desc {
    Symbol *p = new Symbol(GROUP_NAME);
    p->location = $5;
    p->description = $6;
    p->name = $1;
    if($3)
    {
        Symbol *t;
        while(!$3->isempty())
        { 
            // Get the next member
            t = $3->get();

            // If it is a pushed up parm, add it to the parm list for the group
            if(t->symbol_type == PU_INPUT_NAME)
                p->parameter_list.append(t);
            else
            {
                // Add the new child
                p->children.append(t);

                // Add the group as a user
                t->users.append(p);
            }
        }

        // Delete the container
        delete $3;
    }

    Symbol *dup;
    if((dup = DefineName(p)) != NULL)
    {
        char buf[256];
        char *new_name = AnonymousName();
        sprintf(buf, "Duplicate definition of agent name: %s\nName changed to %s",p->name,new_name);
        SyntaxError(buf);
        p->name = new_name;
        dup = DefineName(p);
        assert(dup == NULL);
    }

    $$ = p;
}

| MaybeName LS RS Loc Desc {
    Symbol *p = new Symbol(GROUP_NAME);
    p->location = $4;
    p->description = $5;
    p->name = $1;

    Symbol *dup;
    if((dup = DefineName(p)) != NULL)
    {
        char buf[256];
        char *new_name = AnonymousName();
        sprintf(buf, "Duplicate definition of agent name: %s\nName changed to %s",p->name,new_name);
        SyntaxError(buf);
        p->name = new_name;
        dup = DefineName(p);
        assert(dup == NULL);
    }

    $$ = p;
}
;

InstGroup	: INSTGROUP Loc Desc NAME FROM LS AgentList RS SEMICOLON {
    Symbol *p = new Symbol(GROUP_NAME);
    p->location = $2;
    p->description = $3;
    p->name = $4;

    if($7)
    {
        Symbol *t;
        while(!$7->isempty())
        { 
            // Get the next member
            t = $7->get();

            // If it is a pushed up parm, add it to the parm list for the group
            if(t->symbol_type == PU_INPUT_NAME)
                p->parameter_list.append(t);
            else
            {
                // Add the new child
                p->children.append(t);

                // Add the group as a user
                t->users.append(p);
            }
        }

        // Delete the container
        delete $7;
    }


    // Add it to the symbol table, and make sure not a dup name
    Symbol *dup;
    if((dup = DefineName(p)) != NULL)
    {
        char buf[256];
        sprintf(buf, "Duplicate definition of group name: %s",p->name);
        SyntaxError(buf);
    }
}
;

LG		: Agent 
| Link {
    if($1 && $1->symbol_type != PU_INPUT_NAME)
    {
        char buf[256];
        sprintf(buf, "Only pushed-up parameters allowed in groups: %s",$1->name);
        SyntaxError(buf);
    }
    $$ = $1;
}
;

AgentList	: LG {
    if($1)
    {
        // Return the parm list
        $$ = new SymbolList($1);
    }
    else
        $$ = NULL;
}

| AgentList COMMA LG {
    SymbolList *p = $1;
    if($3)
    {
        if(p)
        {
            // Add the parm to the list
            p->append($3);
        }
        else
        {
            // Make a new list
            p = new SymbolList($3);
        }
    }
    $$ = p;
}
;

%%

/*=========================== End of CDL Grammer ============================*/

/************************************************************************/
/* yyerror is called by yacc when it finds an error. */

void yyerror(char *s)
{
    SyntaxError(s);
}

/**********************************************************************
 * $Log: cdl_gram.y,v $
 * Revision 1.1.1.1  2008/07/14 16:44:22  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.4  2006/12/05 01:47:02  endo
 * cdl parser now accepts {& a b} and Symbol *pair added.
 *
 * Revision 1.3  2006/11/28 04:13:28  ebeowulf
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
 * Revision 1.3  2006/05/14 07:38:31  endo
 * gcc-3.4 upgrade
 *
 * Revision 1.2  2005/02/07 23:56:26  endo
 * Mods from usability-2004
 *
 * Revision 1.1.1.1  2005/02/06 23:00:06  endo
 * AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
 *
 * Revision 1.102  2003/04/06 12:58:48  endo
 * gcc 3.1.1
 *
 * Revision 1.101  2001/09/24 21:14:47  ebeowulf
 * added QLEARN as a operator type
 *
 * Revision 1.100  2000/04/22 19:02:34  endo
 * Checked in for Doug.
 * This patch fixes a problem where cfgedit is unable to duplicate large FSAs.
 * Due to a bug, duplicating FSAs uses exponential amounts of memory, based
 * on the number of states in the FSA.  This patch corrects this problem,
 * making memory useage linear in the size of the FSA.
 *
 * The problem was that during loading of the configuration file, duplicate
 * information was stored in the FSA's internal representation.  When the
 * FSA was duplicated, the duplicate pointers caused records to be replicated
 * many times.
 *
 * Revision 1.99  2000/04/16 15:46:20  endo
 * TypeInBox modified.
 *
 * Revision 1.98  2000/04/13 22:02:59  endo
 * Checked in for Doug.
 * This patch extends MissionLab to allow the user to import read-only
 * library code.  When the user attempts to change something which is
 * read-only, cfgedit pops up a dialog box and asks if it should import
 * the object so it can be edited.  If OK'd, it imports the object.
 *
 * This fixes the problem with FSA's (and other assemblages) not being
 * editable when they are loaded from the library.
 *
 * Revision 1.97  2000/02/18 02:45:38  endo
 * TypeInBox.hpp added.
 *
 * Revision 1.96  1999/11/09 17:45:04  endo
 * The modification made by Doug MacKenzie.
 * He fixed the problem of CfgEdit not being able to
 * load more than 15 states mission by adding some
 * checking procedure.
 *
 * Revision 1.95  1999/10/25 18:13:58  endo
 * rolled back to this version.
 *
 * Revision 1.93  1996/06/13 15:23:59  doug
 * fixing problems with record_class not getting correctly set
 *
 * Revision 1.92  1996/05/14  23:17:39  doug
 * added twopoints case
 *
 * Revision 1.91  1996/05/05  21:45:27  doug
 * fixing compile warnings
 *
 * Revision 1.90  1996/04/30  20:43:38  doug
 * *** empty log message ***
 *
 * Revision 1.89  1996/04/11  15:11:32  doug
 * Fixed sliders
 *
 * Revision 1.88  1996/04/11  04:07:16  doug
 * *** empty log message ***
 *
 * Revision 1.87  1996/04/06  23:54:20  doug
 * *** empty log message ***
 *
 * Revision 1.86  1996/03/04  00:00:34  doug
 * improved error handling
 *
 * Revision 1.85  1996/02/25  01:13:50  doug
 * *** empty log message ***
 *
 * Revision 1.84  1996/02/19  21:57:42  doug
 * library components and permissions now work
 *
 * Revision 1.83  1996/02/18  23:56:55  doug
 * support default initializers in parm definitions
 *
 * Revision 1.82  1996/02/18  00:03:53  doug
 * *** empty log message ***
 *
 * Revision 1.81  1996/02/15  19:30:32  doug
 * fixed pushed up parms in groups
 *
 * Revision 1.80  1996/02/07  17:51:35  doug
 * *** empty log message ***
 *
 * Revision 1.79  1996/02/04  23:22:25  doug
 * *** empty log message ***
 *
 * Revision 1.78  1996/02/01  19:47:04  doug
 * *** empty log message ***
 *
 * Revision 1.77  1996/01/27  00:10:14  doug
 * added grouped lists support to parser
 *
 * Revision 1.76  1996/01/17  18:46:06  doug
 * *** empty log message ***
 *
 * Revision 1.75  1995/12/14  21:54:03  doug
 * *** empty log message ***
 *
 * Revision 1.74  1995/12/01  21:16:50  doug
 * *** empty log message ***
 *
 * Revision 1.73  1995/11/30  23:30:28  doug
 * *** empty log message ***
 *
 * Revision 1.72  1995/11/29  23:12:30  doug
 * *** empty log message ***
 *
 * Revision 1.71  1995/11/29  15:40:47  doug
 * redid SetArch so only on copy
 *
 * Revision 1.70  1995/11/21  23:09:22  doug
 * *** empty log message ***
 *
 * Revision 1.69  1995/11/17  21:56:39  doug
 * add a new case where change the name if dup
 *
 * Revision 1.68  1995/11/12  22:37:48  doug
 * *** empty log message ***
 *
 * Revision 1.67  1995/11/08  16:50:30  doug
 * *** empty log message ***
 *
 * Revision 1.66  1995/11/07  14:29:58  doug
 * *** empty log message ***
 *
 * Revision 1.65  1995/10/27  20:29:18  doug
 * *** empty log message ***
 *
 * Revision 1.64  1995/10/10  20:41:12  doug
 * *** empty log message ***
 *
 * Revision 1.63  1995/10/09  20:40:41  doug
 * undoing last change
 *
 * Revision 1.62  1995/10/09  20:11:22  doug
 * Handle "if [ ] then" rules by deleting the group
 *
 * Revision 1.61  1995/10/09  20:06:25  doug
 * handle undefined paramters
 *
 * Revision 1.60  1995/09/26  21:37:32  doug
 * allow names on groups
 *
 * Revision 1.59  1995/09/19  15:31:53  doug
 * change so all parms defined in the def record exist in any instances
 *
 * Revision 1.58  1995/09/15  15:37:40  doug
 * *** empty log message ***
 *
 * Revision 1.57  1995/09/07  14:23:14  doug
 * works
 *
 * Revision 1.56  1995/07/14  21:09:49  doug
 * handle empty arg list in coordination operators
 *
 * Revision 1.55  1995/07/10  19:53:40  doug
 * *** empty log message ***
 *
 * Revision 1.54  1995/07/07  18:18:07  doug
 * allow two names on sensor/actuators, 1st defines the agent name
 * and the second defines the sensor name
 *
 * Revision 1.53  1995/07/06  16:54:15  doug
 * refbp didn't handle $2 being null
 * ./
 *
 * Revision 1.52  1995/06/29  14:14:24  jmc
 * Added copyright notice and RCS id and log strings.
 **********************************************************************/
