/**********************************************************************
 **                                                                  **
 **                           SensorData.h                           **
 **                                                                  **
 **                                                                  **
 **  Written by:  Alan Wagner                                        **
 **                                                                  **
 **  Copyright 2002 - 2006 Georgia Tech Research Corporation         **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: SensorData.h,v 1.1.1.1 2008/07/14 16:44:17 endo Exp $ */

#ifndef SENSOR_DATA_H
#define SENSOR_DATA_H

/****************************************************************************************************
 * SensorData Class:  This class serves as a means of buffering sensor data that may be recieved
 *                   in a number of different formats so as to provide the behaviors utilizing this
 *                   class with a means in which recieve the buffered sensory data in a manner
 *                   consistant with their implementation
 ****************************************************************************************************/

#include "CJboxInterface.h"
#include <vector>
#include <stdlib.h>	//Old compilation error: EXIT_SUCCESS not defined

using namespace std;

class SensorData {
 public:

  SensorData(void);
  ~SensorData(void);

  // EXTERNAL INTERFACES
  /***************************************************************************************
   * UpdateCommData: Update communications data on a per robot basis using overlay offsets
   *
   *    Parameters:  id (int):  ID of the robot to update
   *                 hops (int): Number of hops to given robot
   *                 sig_strength (double): Signal strength to given robot
   *                 x (double): X position of robot in overlay offsets
   *                 y (double): Y position of robot in overlay offsets
   *                 z (double): Z position of robot in overlay offsets
   ****************************************************************************************/
  int UpdateCommData(int id, int hops, double sig_strength, double x, double y, double z );


  /*************************************************************************************
   * UpdateCommData: Update communications data on a per robot basis using gps data
   *
   *    Parameters:  id (int):  ID of the robot to update
   *                 hops (int): Number of hops to given robot
   *                 sig_strength (double): Signal strength to given robot
   *                 lat (double): Latitude position of robot
   *                 lng (double): Longitude position of robot
   *************************************************************************************/
  int UpdateCommData(int id, int hops, double sig_strength, double lat, double lng );
  void UpdateHeading( double );

  // GATECH INTERFACES

  /*******************************************************************************************
   *  UpdateCommData: Updates the sensor interface with commdata recieved in MissionLab format
   *
   *    Parameters: commdata (Comm_Sensor_Data): The jbox data in the MissionLab format
   ******************************************************************************************/
  int UpdateCommData(Comm_Sensor_Data *commdata);



  /*******************************************************************************************
   * GetCommData: Returns communication data in format utilized by GaTech comm behaviors
   *
   *     Returns: Comm_Sensor_Data: GaTech format for communications data:  (See comm_data.h)
   *******************************************************************************************/
  Comm_Sensor_Data GetCommData(void);



  /*******************************************************************************************
   * GetHeading: Returns heading information in format utilized by GaTech comm behaviors
   *
   *     Returns: double: GaTech format for heading information
   *******************************************************************************************/
  double GetHeading(void);  // get the heading in GaTech format for behavior use


  /*******************************************************************************************
   * GetPosition: Returns position information in format utilized by GaTech comm behaviors
   *              as stored via the jbox data already recieved
   *
   *     Returns: Comm_Vector: GaTech format for vector data
   *******************************************************************************************/
  Comm_Vector GetPosition(void);
  bool Get_ErrorFlag(void);

 private:
  bool Error_Flag;

  CJboxInterface* JBoxI;        // Interface to the JBox
  double Heading;              // Store Heading information

};

#endif

/**********************************************************************
 * $Log: SensorData.h,v $
 * Revision 1.1.1.1  2008/07/14 16:44:17  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/20 17:17:47  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:37:55  endo
 * MissionLab 7.0
 *
 * Revision 1.1  2006/06/08 14:47:42  endo
 * CommBehavior from MARS 2020 migrated.
 *
 **********************************************************************/
