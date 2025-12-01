/**********************************************************************
 **                                                                  **
 **                       env_change.c                               **
 **                                                                  **
 **  Functions to change the environment at runtime.                 **
 **                                                                  **
 **                                                                  **
 **  Written by:  J. Brian Lee                                       **
 **                                                                  **
 **  Copyright 1995, 1996, 1997 Georgia Tech Research Corporation    **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **********************************************************************/

#include <stdio.h>
#include "env_change_defs.h"
#include "gt_world.h"
#include "CheckNew.h"

list<SuEventDesc*>* g_pclEventDescList = NULL; // this holds a list of time events and 
                                               // their respective commands

// execute a obstacle command
void PerformObsCmd( SuCmd* psuCmd )
{
    // see if it's a region obstacle command
    if ( psuCmd->bRegionCmd )
    {
        // get the command and region info
        SuRegionObsCmd* psuObsCmd = (SuRegionObsCmd*) psuCmd;
        double dWidth  = psuObsCmd->suRegion.suP1.x - psuObsCmd->suRegion.suP0.x;
        double dHeight = psuObsCmd->suRegion.suP1.y - psuObsCmd->suRegion.suP0.y;

        // if we're adding obstacles to a region, do it
        if ( psuObsCmd->enAction == EnActionType_ADD )
        {
            FillRegion( psuObsCmd->suRegion.suP0.x, psuObsCmd->suRegion.suP0.y, 
                        dWidth, dHeight, psuObsCmd->dMinRad, psuObsCmd->dMaxRad, 
                        psuObsCmd->dCoverage, psuObsCmd->iSeed );
        }
        // otherwise, if we're removing obstacles from a region, then do it
        else if ( psuObsCmd->enAction == EnActionType_REMOVE )
        {
            ClearRegion( psuObsCmd->suRegion.suP0.x, psuObsCmd->suRegion.suP0.y, 
                         dWidth, dHeight );
        }

        // increment the random seed for periodic commands
        psuObsCmd->iSeed += psuObsCmd->iSeedInc;
    }
    // otherwise, we're either adding or removing a single obstacle
    else
    {
        SuSingleObsCmd* psuObsCmd = (SuSingleObsCmd*) psuCmd;
        if ( psuObsCmd->enAction == EnActionType_ADD )
        {
            gt_add_obstacle( psuObsCmd->suPosition.x, psuObsCmd->suPosition.y, psuObsCmd->dRadius );
        }
        else if ( psuObsCmd->enAction == EnActionType_REMOVE )
        {
            RemoveObsAt( psuObsCmd->suPosition.x, psuObsCmd->suPosition.y );
        }
    }
}

// dispatch a command to a more specific handler function
void PerformCmd( SuCmd* psuCmd )
{
    // test for obstacle commands
    if ( psuCmd->enType == EnEnvCmdType_OBS )
    {
        PerformObsCmd( psuCmd );
    }
    // if we haven't found a valid handler yet, then we have an error
    else
    {
        fprintf( stderr, "Error: unknown env change command type: %d\n", psuCmd->enType );
    }
}

// execute commands that should happen at or before the specified step
void ChangeEnv( int iStep )
{
    // do nothing if there's no events
    if ( g_pclEventDescList == NULL )
    {
        return;
    }

    // keep executing commands that happen at or before the given time step as long as we have them
    while ( ( g_pclEventDescList->size() > 0 ) && 
            ( g_pclEventDescList->front()->suTime.iChangeTime <= iStep ) )
    {
        // grap the front command
        SuEventDesc* psuEventDesc = g_pclEventDescList->front();
        g_pclEventDescList->pop_front();
        if ( psuEventDesc != NULL )
        {
            // do the events in the event desciptor
            list<SuCmd*>::iterator i = psuEventDesc->pclEvents->begin();
            while ( i != psuEventDesc->pclEvents->end() )
            {
                PerformCmd( *i );
                i++;
            }

            // see when the event should next be performed, and if it is later than 
            // the current time (period > 0), re-insert it into the list
            psuEventDesc->suTime.iChangeTime += psuEventDesc->suTime.iPeriod;
            if ( psuEventDesc->suTime.iChangeTime > iStep )
            {
                InsertEvent( psuEventDesc );
            }
        }
    }
}

// reset the event list
void ClearEventList()
{
    if ( g_pclEventDescList != NULL )
    {
        // delete all events in the list
        list<SuEventDesc*>::iterator i = g_pclEventDescList->begin();
        while ( i != g_pclEventDescList->end() )
        {
            if ( *i != NULL )
            {
                delete *i;
            }
            i++;
        }

        // delete the list
        delete g_pclEventDescList;
        g_pclEventDescList = NULL;
    }
}

void InsertEvent( SuEventDesc* psuEvent )
{
    // if the list doesn't already exist, create it
    if ( g_pclEventDescList == NULL )
    {
        CheckNew( g_pclEventDescList = new list<SuEventDesc*> );
    }

    // insert the new event description into the list in the correct order 
    // (the order is keyed on the event's ChangeTime -- soonest to latest)
    list<SuEventDesc*>::iterator i = g_pclEventDescList->begin();
    while ( i != g_pclEventDescList->end() )
    {
        if ( (*i)->suTime.iChangeTime > psuEvent->suTime.iChangeTime )
        {
            g_pclEventDescList->insert( i, psuEvent );
            break;
        }
        i++;
    }

    // if the new event hasn't been added to the list yet, it belongs at the back
    if ( i == g_pclEventDescList->end() )
    {
        g_pclEventDescList->push_back( psuEvent );
    }
}
