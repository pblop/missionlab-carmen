/**********************************************************************
 **                                                                  **
 **                          CNPInterceptTask.h                      **
 **                                                                  **
 **  Written by: Patrick Ulam                                        **
 **                                                                  **
 **  Copyright 2006 Georgia Tech Research Corporation                **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: CNPInterceptTask.h,v 1.1.1.1 2008/07/14 16:44:17 endo Exp $ */

#ifndef CNP_INTERCEPT_TASK_H
#define CNP_INTERCEPT_TASK_H

#include "AbstractCNPTask.h"
#include "cnp_server_manager.h"
#include <string>
#include <list>

using std::string;
using std::list;

class CNPInterceptTask : public AbstractCNPTask {

 public:
  CNPInterceptTask()  {};

 public:
  const string    GetTaskName();
  double          CalculateBid(list< struct CNP_RobotConstraint> lstRobotConstraints);

};

#endif
/**********************************************************************
 * $Log: CNPInterceptTask.h,v $
 * Revision 1.1.1.1  2008/07/14 16:44:17  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/20 17:17:47  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:37:55  endo
 * MissionLab 7.0
 *
 * Revision 1.4  2006/05/02 04:12:58  endo
 * TrackTask improved for the experiment.
 *
 **********************************************************************/
