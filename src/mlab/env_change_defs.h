/**********************************************************************
 **                                                                  **
 **                       env_change_defs.h                          **
 **                                                                  **
 **  Definitions for changing the environment at runtime.            **
 **                                                                  **
 **                                                                  **
 **  Written by:  J. Brian Lee                                       **
 **                                                                  **
 **  Copyright 1995, 1996, 1997 Georgia Tech Research Corporation    **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **********************************************************************/

/* $Id: env_change_defs.h,v 1.1.1.1 2008/07/14 16:44:22 endo Exp $ */

#ifndef _ENV_CHANGE_DEFS_H
#define _ENV_CHANGE_DEFS_H

#include <list>
#include "gt_std.h"

using std::list;

extern double origin_x;
extern double origin_y;

extern int g_iLineNum;

int env_change_lex();
int env_change_parse();
void env_change_error( char* s );
void env_change_init();

enum EnEnvCmdType
{
    EnEnvCmdType_OBS
};

enum EnActionType
{
    EnActionType_ADD, 
    EnActionType_REMOVE
};

struct SuRegion
{
    Point suP0; // lower left corner
    Point suP1; // upper right corner
};

struct SuCmd
{
    EnEnvCmdType enType; // what type of cmd is this?
    bool bRegionCmd;     // is this a region command?
};

struct SuRegionCmd : public SuCmd
{
    SuRegion suRegion;   // defines the affected area
};

struct SuSingleObsCmd : public SuCmd
{
    EnActionType enAction;   // action to perform
    Point suPosition;        // where to add/remove an obstacle
    double dRadius;          // how big is the obstacle? (only for add)
};

struct SuRegionObsCmd : public SuRegionCmd
{
    EnActionType enAction;   // action to perform
    double dMinRad, dMaxRad; // used for add (also used for single obs cmd)
    double dCoverage;        // used for add
    int iSeed;               // used for add
    int iSeedInc;            // increments the seed for periodic region commands
};

struct SuEnvChangeTime
{
    int iChangeTime;
    int iPeriod;
};

struct SuEventDesc
{
    SuEnvChangeTime suTime;
    list<SuCmd*>*   pclEvents;

    SuEventDesc()
    {
        pclEvents = NULL;
    }

    ~SuEventDesc()
    {
        if ( pclEvents != NULL )
        {
            while ( pclEvents->size() > 0 )
            {
                if ( pclEvents->front() != NULL )
                {
                    delete pclEvents->front();
                }
                pclEvents->pop_front();
            }
        }
    }
};

void ClearEventList();
void InsertEvent( SuEventDesc* psuEvent );
void ChangeEnv( int iStep );

#endif // ndef _ENV_CHANGE_DEFS_H

/**********************************************************************
 * $Log: env_change_defs.h,v $
 * Revision 1.1.1.1  2008/07/14 16:44:22  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/20 17:17:50  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:37:59  endo
 * MissionLab 7.0
 *
 * Revision 1.1.1.1  2005/02/06 23:00:11  endo
 * AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
 *
 * Revision 1.3  2003/04/06 12:44:19  endo
 * gcc 3.1.1
 *
 **********************************************************************/
