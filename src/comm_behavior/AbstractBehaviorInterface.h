/*********************************************************************
 **                                                                  **
 **                     AbstractBehaviorInterface.h                  **
 **                                                                  **
 **                                                                  **
 **  Written by:  Alan Wagner                                        **
 **                                                                  **
 **  Copyright 2002 - 2006 Georgia Tech Research Corporation         **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 ** Purpose: Abstract Interface class for all behaviors and the      **
 ** behavior coordination class.                                     **
 **                                                                  **
 **********************************************************************/
#ifndef ABSTRACTBEHAVIORINTERFACE_H
#define ABSTRACTBEHAVIORINTERFACE_H


//Preprocessor defines
#define ERRNUM int
#define LoadParameters int
#define UnLoadParameters int
#define ClearParameters int

//Include files
#include <stdlib.h>
#include "VectorConfidenceAdvice.h"
#include "SensorData.h"

//Forward declarations
struct InitializeParameters
{
	int iRobotID;
	int iInitialNumberOfRobots;
};


class AbstractBehaviorInterface
{
public:

	virtual ERRNUM Initialize( InitializeParameters& )=0;
    /// Called once when an instance of the behavior is created to allow
    /// the behavior to construct any necessary objects and load any data files
    /// necessary PRIOR to execution. This function returns zero = EXIT_SUCCESS
    /// if the update was successful, some positive integer if initialization failed.
    /// Intialization parameters are included in the InitializeParameters class.
    /// Implementing this method is optional for behaviors

	virtual ERRNUM Load( LoadParameters& )=0;
    //This function loads data and files necessary PRIOR to execution. This
    //function returns zero = EXIT_SUCCESS if the update was successful some
    //positive integer if the update was unsuccessful. Parameters for
    //construction are included in the BehaviorLoadParameters class.

	virtual ERRNUM UpdateSensoryData( SensorData& )=0;
    //This function updates the Behavior with a SensoryData data structure
    //providing the behavior with all the data necessary to execute. This
    //function returns zero = EXIT_SUCCESS if the update was successful
    //some positive integer if the update was unsuccessful.

	virtual VectorConfidenceAdvice Execute()=0;
    /// This function excutes the behavior using the data currently available
    /// based on its last update. The default return value is a zero vector with
    /// zero confidence.
    /// All behaviors are required to implement this method.

	virtual void Cleanup()=0;
    /// Called once when the behavior instance is going to be destroyed
    /// to allow it to free its resources and persist its state to data files.
    /// Implementing this method is optional for behaviors

    virtual void SetSignalThresholdHigh(float threshold){}
    virtual void SetSignalThresholdLow(float threshold){}
};

#endif

