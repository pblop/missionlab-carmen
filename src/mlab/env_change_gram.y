%{

/**********************************************************************
 **                                                                  **
 **                       env_change_gram.y                          **
 **                                                                  **
 **  A parser for an environment change descriptor file.             **
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
#include "env_change_defs.h"
#include "CheckNew.h"

extern list<SuEventDesc*>* g_pclEventDescList;
extern bool g_bHadEnvChangeParseError;

// round to the nearest int
inline int Round( double d )
{
    return (int) ( ( d >= 0 ) ? ( d + 0.5 ) : ( d - 0.5 ) );
}

%}

%union {
    double              dNumber;
    Point               suPoint;
    SuRegion            suRegion;
    SuEnvChangeTime     suChangeTime;
    SuCmd*              psuCmd;
    SuRegionCmd*        psuRegionCmd;
    list<SuCmd*>*       pclCmdList;
    SuEventDesc*        psuEventDesc;
    list<SuEventDesc*>* pclEventDescList;
}

// Define the tokens that lex will return
%token COMMA
%token STEP_COUNT
%token ADD
%token REMOVE
%token OBSTACLE
%token REGION

%token <dNumber> NUMBER

%type <suPoint>          Position
%type <suRegion>         RegionSpec
%type <suChangeTime>     TimeDesc
%type <psuCmd>           Command
%type <psuRegionCmd>     RegionCommand
%type <pclCmdList>       CommandList
%type <psuEventDesc>     EventDesc
%type <pclEventDescList> EventDescList

//************************* Start of Grammer *****************************
%%

Start : EventDescList {
    if ( !g_bHadEnvChangeParseError )
    {
        ClearEventList();
        g_pclEventDescList = $1;
    }
}
| { // empty file
    if ( !g_bHadEnvChangeParseError )
    {
        ClearEventList();
    }
}
;

EventDescList : EventDesc {
    CheckNew( $$ = new list<SuEventDesc*> );
    $$->push_back( $1 );
}
| EventDescList EventDesc {
    list<SuEventDesc*>::iterator i = $1->begin();

    // insert the new event description into the list in the correct order 
    // (the order is keyed on the event's ChangeTime -- soonest to latest)
    while ( i != $1->end() )
    {
        if ( (*i)->suTime.iChangeTime > $2->suTime.iChangeTime )
        {
            $1->insert( i, $2 );
            break;
        }
        i++;
    }

    // if the new event hasn't been added to the list yet, it belongs at the back
    if ( i == $1->end() )
    {
        $1->push_back( $2 );
    }

    $$ = $1;
}
;

EventDesc : STEP_COUNT TimeDesc CommandList {
    CheckNew( $$ = new SuEventDesc );
    $$->suTime = $2;
    $$->pclEvents = $3;
}
;

TimeDesc : NUMBER  { // <start time>
    $$.iChangeTime = Round( $1 );
    $$.iPeriod = -1;

    if ( $1 < 0 )
    {
        env_change_error( "start time must be non-negative!" );
    }
}
| NUMBER NUMBER { // <start time> <periodic interval>
    $$.iChangeTime = Round( $1 );
    $$.iPeriod = Round( $2 );

    if ( ( $1 < 0 ) || ( $2 < 0 ) )
    {
        env_change_error( "start time and period must be non-negative!" );
    }
}
;

CommandList : Command  {
    CheckNew( $$ = new list<SuCmd*> );
    $$->push_back( $1 );
}
| CommandList Command {
    $1->push_back( $2 );
    $$ = $1;
}
;

Command : ADD OBSTACLE Position NUMBER { // add obstacle |position| <radius>
    SuSingleObsCmd* psuCmd;

    CheckNew( psuCmd = new SuSingleObsCmd );
    psuCmd->bRegionCmd = false;
    psuCmd->enType = EnEnvCmdType_OBS;
    psuCmd->enAction = EnActionType_ADD;
    psuCmd->suPosition = $3;
    psuCmd->dRadius = $4;

    $$ = psuCmd;
}
| REMOVE OBSTACLE Position { // remove obstacle |position|
    SuSingleObsCmd* psuCmd;

    CheckNew( psuCmd = new SuSingleObsCmd );
    psuCmd->bRegionCmd = false;
    psuCmd->enType = EnEnvCmdType_OBS;
    psuCmd->enAction = EnActionType_REMOVE;
    psuCmd->suPosition = $3;

    $$ = psuCmd;
}
| RegionSpec RegionCommand {
    $2->suRegion = $1;
    $$ = $2;
}
;

Position : NUMBER NUMBER { // <x> <y>
    $$.x = $1 + origin_x;
    $$.y = $2 + origin_y;
    $$.z = 0;
}
;

RegionSpec : REGION Position Position { // region |Position0| |Position1|
    // make sure the later assumption that P0 is the lower left corner is satisfied

    // make sure P0 has the smallest X coord
    if ( $2.x < $3.x )
    {
        $$.suP0.x = $2.x;
        $$.suP1.x = $3.x;
    }
    else
    {
        $$.suP0.x = $3.x;
        $$.suP1.x = $2.x;
    }

    // make sure P0 has the smallest Y coord
    if ( $2.y < $3.y )
    {
        $$.suP0.y = $2.y;
        $$.suP1.y = $3.y;
    }
    else
    {
        $$.suP0.y = $3.y;
        $$.suP1.y = $2.y;
    }

    // we don't use the Z coord, so just init them to 0
    $$.suP0.z = 0;
    $$.suP1.z = 0;
}
;

RegionCommand : ADD OBSTACLE NUMBER NUMBER NUMBER { // add obstacle <coverage> <min_rad> <max_rad>
    SuRegionObsCmd* psuCmd;

    CheckNew( psuCmd = new SuRegionObsCmd );
    psuCmd->enType = EnEnvCmdType_OBS;
    psuCmd->bRegionCmd = true;
    psuCmd->enAction = EnActionType_ADD;
    psuCmd->dCoverage = $3;
    psuCmd->dMinRad = $4;
    psuCmd->dMaxRad = $5;
    psuCmd->iSeed = -1;
    psuCmd->iSeedInc = 0;

    $$ = psuCmd;
}
| ADD OBSTACLE NUMBER NUMBER NUMBER NUMBER { // add obstacle <coverage> <min_rad> <max_rad> <seed>
    SuRegionObsCmd* psuCmd;

    CheckNew( psuCmd = new SuRegionObsCmd );
    psuCmd->enType = EnEnvCmdType_OBS;
    psuCmd->bRegionCmd = true;
    psuCmd->enAction = EnActionType_ADD;
    psuCmd->dCoverage = $3;
    psuCmd->dMinRad = $4;
    psuCmd->dMaxRad = $5;
    psuCmd->iSeed = Round( $6 );
    psuCmd->iSeedInc = 0;

    $$ = psuCmd;
}
| ADD OBSTACLE NUMBER NUMBER NUMBER NUMBER COMMA NUMBER { 
    // add obstacle <coverage> <min_rad> <max_rad> <seed>,<seed inc>

    SuRegionObsCmd* psuCmd;

    CheckNew( psuCmd = new SuRegionObsCmd );
    psuCmd->enType = EnEnvCmdType_OBS;
    psuCmd->bRegionCmd = true;
    psuCmd->enAction = EnActionType_ADD;
    psuCmd->dCoverage = $3;
    psuCmd->dMinRad = $4;
    psuCmd->dMaxRad = $5;
    psuCmd->iSeed = Round( $6 );
    psuCmd->iSeedInc = Round( $8 );

    $$ = psuCmd;
}
| REMOVE OBSTACLE {
    SuRegionObsCmd* psuCmd;

    CheckNew( psuCmd = new SuRegionObsCmd );
    psuCmd->enType = EnEnvCmdType_OBS;
    psuCmd->bRegionCmd = true;
    psuCmd->enAction = EnActionType_REMOVE;

    $$ = psuCmd;
}
;
