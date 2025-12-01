#ifndef _LM_PARAM_DIALOG_H
#define _LM_PARAM_DIALOG_H

extern "C" {

#include <Xm/Xm.h>

#define LM_PARAM_HISTORY_SIZE     500
#define LM_PARAM_GRAPH_COUNT      5
#define LM_PARAM_LABELS_PER_GRAPH 4

typedef struct SuLMParams
{
    float fGoalGain;
    float fAvoidObstacleGain;
    float fObstacleSphere;
    float fWanderGain;
    int   iWanderPersistence;
} SuLMParams;

typedef struct SuLMViewDialog
{
    Widget dialog;
    Widget drawingArea;
    Widget aaLabels[ LM_PARAM_GRAPH_COUNT ][ LM_PARAM_LABELS_PER_GRAPH ];
    int iLastUpdatedIndex;
    int iRobotID;
    SuLMParams asuParamHistory[ LM_PARAM_HISTORY_SIZE ];
} SuLMViewDialog;

/* public functions
 * Instantiate a SuLMViewDialog variable and use only these functions on it.
 * Do not alter the values in the SuLMViewDialog variable directly */

int PopupLMParamDialog( SuLMViewDialog* psuDialog, Widget parent );
int InitializeLMParamDialog( SuLMViewDialog* psuDialog );
int SetLMParamDialogRobotID( SuLMViewDialog* psuDialog, int iRobotID );
int UpdateLMParamDialogValue( SuLMViewDialog* psuDialog, 
                              SuLMParams suValues );
int UpdateLMParamDialogDisplay( SuLMViewDialog* psuDialog );
const Widget GetLMParamDialog( const SuLMViewDialog* psuDialog );
int GetLMParamRobotID( const SuLMViewDialog* psuDialog );
int GetLastUpdatedLMParamIndex( const SuLMViewDialog* psuDialog );

}

#endif // #ifndef _LM_PARAM_DIALOG_H
