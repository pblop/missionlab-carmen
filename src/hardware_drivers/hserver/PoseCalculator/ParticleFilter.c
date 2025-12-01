#include "ParticleFilter.h"
#include "PoseCalculatorTypes.h"
#include "PoseCalculator.h"


extern double getCurrentEpochTime(void);
extern void printfTextWindow(const char* arg, ...);


ParticleFilter::ParticleFilter(PoseCalcSensoryDataBusInterface *sensoryDataBus):
  PoseCalcSensorFuser(sensoryDataBus, POSECALC_FUSER_TYPE_PARTICLE_FILTER)
{
    initialize();
}

ParticleFilter::~ParticleFilter(void){

}


PoseCalcFusedPose_t ParticleFilter::initialize(void){

/*  visualize = true;
  if(visualize){
	viz.setupViz();
    }*/

    //Calculate the mean of all the particles rather than ML particle
    useMeanParticle = true;

    PoseCalcFusedPose_t fusedPose;
    vector<PoseCalcLocationInfo_t> locData;
    vector<PoseCalcRotationInfo_t> rotData;
    double maxConfidenceLevel;
    int i;
    // Get the latest location and rotation data from the sensory bus.
    locData = sensoryDataBus_->getLatestLocationData(FUSER_TYPE_);
    rotData = sensoryDataBus_->getLatestRotationData(FUSER_TYPE_);

    // Initialize data.
    memset((PoseCalcFusedPose_t *)&fusedPose, 0x0, sizeof(PoseCalcFusedPose_t));
    fusedPose.fuserType = FUSER_TYPE_;
    fusedPose.confidence.locConfidence.value = PoseCalc::MIN_CONFIDENCE_LEVEL;
    fusedPose.confidence.rotConfidence.value = PoseCalc::MIN_CONFIDENCE_LEVEL;
    fusedPose.extra.validPose = false;

    // Get the location with the highest confidence.
    maxConfidenceLevel = PoseCalc::MIN_CONFIDENCE_LEVEL;

    for (i = 0; i < (int)(locData.size()); i++)
    {
        if ((locData[i].confidence.value) > maxConfidenceLevel)
        {
            fusedPose.pose.loc = locData[i].loc;
            fusedPose.confidence.locConfidence = locData[i].confidence;
            fusedPose.variance.locVariance = locData[i].variance;
            fusedPose.extra.locSensorType = locData[i].sensorType;
            fusedPose.extra.validLoc = true;

            maxConfidenceLevel = locData[i].confidence.value;
        }
    }

    // Get the rotation with the highest confidence.
    maxConfidenceLevel = PoseCalc::MIN_CONFIDENCE_LEVEL;

    for (i = 0; i < (int)(rotData.size()); i++)
    {
        if ((rotData[i].confidence.value) > maxConfidenceLevel)
        {
            fusedPose.pose.rot = rotData[i].rot;
            fusedPose.confidence.rotConfidence = rotData[i].confidence;
            fusedPose.variance.rotVariance = rotData[i].variance;
            fusedPose.extra.rotSensorType = rotData[i].sensorType;
            fusedPose.extra.validRot = true;

            maxConfidenceLevel = rotData[i].confidence.value;
        }
    }

    if ((fusedPose.extra.validLoc) && (fusedPose.extra.validRot))
    {
        fusedPose.extra.validPose = true;
    }

    filter.setup(1000, fusedPose, locData, rotData);
    filter.setFusedPose(&fusedPose);  

    return fusedPose;
}

PoseCalcFusedPose_t ParticleFilter::calcMaxConf(vector<PoseCalcLocationInfo_t> locData,
                                  vector<PoseCalcRotationInfo_t> rotData){
    
    PoseCalcFusedPose_t fusedPose;
    double maxConfidenceLevel;  
    memset((PoseCalcFusedPose_t *)&fusedPose, 0x0, sizeof(PoseCalcFusedPose_t));
    fusedPose.fuserType = FUSER_TYPE_;
    fusedPose.confidence.locConfidence.value = PoseCalc::MIN_CONFIDENCE_LEVEL;
    fusedPose.confidence.rotConfidence.value = PoseCalc::MIN_CONFIDENCE_LEVEL;
    fusedPose.extra.validPose = false;
    
    maxConfidenceLevel = PoseCalc::MIN_CONFIDENCE_LEVEL;

    for (int i = 0; i < (int)(locData.size()); i++)
    {
        if ((locData[i].confidence.value) > maxConfidenceLevel)
        {
            fusedPose.pose.loc = locData[i].loc;
            fusedPose.confidence.locConfidence = locData[i].confidence;
            fusedPose.variance.locVariance = locData[i].variance;
            fusedPose.extra.locSensorType = locData[i].sensorType;
            fusedPose.extra.validLoc = true;

            maxConfidenceLevel = locData[i].confidence.value;
        }
    }

    // Get the rotation with the highest confidence. Do not use the data if
    // it was already read previously.
    maxConfidenceLevel = PoseCalc::MIN_CONFIDENCE_LEVEL;

    for (int i = 0; i < (int)(rotData.size()); i++)
    {
        if ((rotData[i].confidence.value) > maxConfidenceLevel)
        {
            fusedPose.pose.rot = rotData[i].rot;
            fusedPose.confidence.rotConfidence = rotData[i].confidence;
            fusedPose.variance.rotVariance = rotData[i].variance;
            fusedPose.extra.rotSensorType = rotData[i].sensorType;
            fusedPose.extra.validRot = true;

            maxConfidenceLevel = rotData[i].confidence.value;
        }
    }

    if ((fusedPose.extra.validLoc) && (fusedPose.extra.validRot))
    {
        fusedPose.extra.validPose = true;
    }

    return fusedPose;
}

PoseCalcFusedPose_t ParticleFilter::getOutput(void){
    PoseCalcFusedPose_t fusedPose;
    PoseCalcFusedPose_t maxConf;
    vector<PoseCalcLocationInfo_t> locData;
    vector<PoseCalcRotationInfo_t> rotData;
  
    locData = sensoryDataBus_->getLatestLocationData(FUSER_TYPE_);
    rotData = sensoryDataBus_->getLatestRotationData(FUSER_TYPE_);
    maxConf = calcMaxConf(locData, rotData);


    memset((PoseCalcFusedPose_t *)&fusedPose, 0x0, sizeof(PoseCalcFusedPose_t));
    fusedPose.fuserType = FUSER_TYPE_;
    fusedPose.confidence.locConfidence.value = PoseCalc::MIN_CONFIDENCE_LEVEL;
    fusedPose.confidence.rotConfidence.value = PoseCalc::MIN_CONFIDENCE_LEVEL;
    fusedPose.extra.validPose = false;
    fusedPose.extra.time = getCurrentEpochTime();
  
    //add calls to particle filter here

    filter.applyMotionModel(locData, rotData);
    filter.applySensorModel(locData, rotData);
    filter.resampleFilter();
    if(useMeanParticle){//if not, uses ML Particle
        filter.calcFilterStats();
    }

    filter.addSamples(maxConf, locData, rotData);
    filter.setFusedPose(&fusedPose);
  
  
    if ((fusedPose.extra.validLoc) && (fusedPose.extra.validRot)){
        fusedPose.extra.validPose = true;
    }

/*	if(visualize){
	  viz.drawViz(&filter);
      }*/


    return fusedPose;
}
