/**********************************************************************
 **                                                                  **
 **                            CNPConstraint.h                       **
 **                                                                  **
 **  Written by:  Alan Richard Wagner                                **
 **                                                                  **
 **  Copyright 2005 Georgia Tech Research Corporation                **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: CNPConstraint.h,v 1.1.1.1 2008/07/14 16:44:17 endo Exp $ */

#ifndef CNP_CONSTRAINT_H
#define CNP_CONSTRAINT_H

#include <string>
#include "cnp_constants.h"
//#include "cnp_task_constraint.h"

using std::string;

typedef enum CNP_ConstraintType
{
    CNP_STRING = 0,
    CNP_INT,
    CNP_DOUBLE,
    CNP_BOOLEAN,
    CNP_NA
};

struct CNP_RobotConstraint {
  int iRobotID;
  string strConstraintName;
  CNP_ConstraintType constraintType; 
  string strConstraintValue;

  //We need this for sorting the list...
  bool operator<(const CNP_RobotConstraint & right) const {return (this->iRobotID < right.iRobotID); }
};

struct CNP_TaskInfo {
    int id;
    string name;
};

struct CNP_TaskConstraint {
    CNP_TaskInfo taskInfo;
    string strConstraintName;
    CNP_ConstraintType constraintType; 
    string strConstraintValue;
};

#endif
/**********************************************************************
 * $Log: CNPConstraint.h,v $
 * Revision 1.1.1.1  2008/07/14 16:44:17  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/20 17:17:47  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:37:55  endo
 * MissionLab 7.0
 *
 * Revision 1.8  2006/05/14 06:51:24  endo
 * gcc-3.4 upgrade
 *
 * Revision 1.7  2006/05/14 06:48:22  endo
 * gcc-3.4 upgrade
 *
 * Revision 1.6  2006/04/28 22:50:41  pulam
 * Constraint checking update, Terrainmap disable for large maps, renegging overhaul
 *
 * Revision 1.5  2006/02/19 22:41:10  endo
 * CNP_SAVE_TARGET_LOCATION stuff.
 *
 * Revision 1.4  2005/07/05 21:09:10  alanwags
 * Recent changes for constraint names
 *
 * Revision 1.3  2005/06/23 22:51:29  alanwags
 * Add bidding
 *
 * Revision 1.2  2005/06/22 20:01:12  endo
 * TaskInfo added.
 *
 **********************************************************************/
