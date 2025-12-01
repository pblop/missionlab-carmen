#ifndef LMADJUST_H
#define LMADJUST_H

#define WINDOW_SIZE	        5
#define HISTORY_INTERVAL    10
#define ADJUSTABLE_WEIGHTS  3

extern double history[ WINDOW_SIZE ]; // performance history
extern int    hist_offset;
extern double distance_to_goal[ WINDOW_SIZE ];
extern int    affecting_obstacle_history[ WINDOW_SIZE ];

enum EnMovementType
{
    EnMovementType_NO_MOVEMENT = 0,
    EnMovementType_PROGRESS,
    EnMovementType_NO_PROGRESS_WITH_OBSTACLES,
    EnMovementType_NO_PROGRESS_WITHOUT_OBSTACLES,
    EnMovementType_COUNT     // this must be declared last
};

enum EnLMParamType
{ 
    EnLMParamType_GOAL = 0, 
    EnLMParamType_OBJECT, 
    EnLMParamType_NOISE, 
    EnLMParamType_SPHERE, 
    EnLMParamType_NOISE_PERSISTENCE, 
    EnLMParamType_COUNT      // this must be declared last
};

union UnLMParamValue
{
    int iIntValue;
    double dDoubleValue;
};

enum EnLMParamDataType
{
    EnLMParamDataType_INT, 
    EnLMParamDataType_DOUBLE
};

void AdjustLMValues();

#define szMOVE_TO_GOAL_GAIN_LABEL   ("LM_move_to_goal_gain")
#define szOBJECT_GAIN_LABEL         ("LM_object_gain")
#define szWANDER_GAIN_LABEL         ("LM_wander_gain")
#define szWANDER_PERSISTENCE_LABEL  ("LM_wander_persistence")
#define szSPHERE_OF_INFLUENCE_LABEL ("LM_sphere_of_influence")
#define szLM_ENABLED_LABEL          ("LM_enabled")
#define szSAVE_LM_WEIGHTS_LABEL     ("LM_save_weights")

typedef struct SuDoubleRange
{
  double dLo, dHi;
} SuDoubleRange;

extern SuDoubleRange g_aasuLMParams[ EnMovementType_COUNT ][ EnLMParamType_COUNT ];

#endif // LMADJUST_H
