#ifndef PARTICLE_FILTER_H
#define PARTICLE_FILTER_H

#include "PoseCalculatorTypes.h"
#include "PCSensorFuser.h"
#include "PCSensoryDataBusInterface.h"
#include "pfilter.h"

#include <string.h>

class ParticleFilter : public PoseCalcSensorFuser{

protected:
    
public:
    ParticleFilter(PoseCalcSensoryDataBusInterface *sensoryDataBus);
    ~ParticleFilter(void);
    PoseCalcFusedPose_t getOutput(void);
    PoseCalcFusedPose_t initialize(void);
    PoseCalcFusedPose_t calcMaxConf(vector<PoseCalcLocationInfo_t> locData,
                                    vector<PoseCalcRotationInfo_t> rotData);
    PFilter filter;
//	FilterViz viz;
    bool useMeanParticle;
//	bool visualize;
};

#endif
