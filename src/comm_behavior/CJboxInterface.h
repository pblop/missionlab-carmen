/*********************************************************************
 **                                                                  **
 **                         CJboxInterface.h                         **
 **                                                                  **
 **                                                                  **
 **  Written by:  Alan Wagner                                        **
 **                                                                  **
 **  Copyright 2002 - 2006 Georgia Tech Research Corporation         **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: CJboxInterface.h,v 1.1.1.1 2008/07/14 16:44:17 endo Exp $ */

#ifndef JBOX_INTERFACE_H
#define JBOX_INTERFACE_H


#define MAX_JBOX_ROBOTS    100


#include "comm_data.h"

class CJboxInterface {
 public:
  CJboxInterface(int numrobo);
  ~CJboxInterface(void);

  // update a robots information with latitude and longitude
  void Update_CommData(int id, int hops, float sig_strength, double lat, double lng);

  // update comm data with overlay coordinates
  int Update_CommData(int id, int hops, float sig_strength, float x, float y, float z);

  // update comm data using mlab format
  void Update_CommData(Comm_Sensor_Data *commdata);

  // return the comm data using mlab format
  Comm_Sensor_Data Get_CommData(void);

  // get out position from based on jbox data
  Comm_Vector Get_Position(void);

  bool Get_ErrorFlag(void);
  
 private:
  void Add_Robot(int numrobo);
  int Count_Used(void);
  void Load_BaseInfo(void);

  int Num_Robots;
  Comm_Vector *Position;
  double *Hop;
  double *SignalStrength;
  int *Used;

  double Base_X;
  double Base_Y;
  double Base_Lat;
  double Base_Long;
  double MPerLat;
  double MPerLong;
  bool ErrorFlag;

};


#endif

/**********************************************************************
 * $Log: CJboxInterface.h,v $
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
