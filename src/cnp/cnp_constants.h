/**********************************************************************
 **                                                                  **
 **                          cnp_constants.h                         **
 **                                                                  **
 **  Written by: Yoichiro Endo                                       **
 **                                                                  **
 **  Copyright 2006 Georgia Tech Research Corporation                **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/
#ifndef CNP_CONSTANTS_H
#define CNP_CONSTANTS_H

/* $Id: cnp_constants.h,v 1.1.1.1 2008/07/14 16:44:17 endo Exp $ */

const double CNP_MAXVEL_UUV = 2.5;
const double CNP_MAXVEL_UGV = 10.0;
const double CNP_MAXVEL_USV = 20.0;
const double CNP_MAXVEL_UAV = 20.0;
const double CNP_FUEL_EFFICIENCY_UUV = 90.0;
const double CNP_FUEL_EFFICIENCY_UGV = 576.0;
const double CNP_FUEL_EFFICIENCY_USV = 1152.0;
const double CNP_FUEL_EFFICIENCY_UAV = 144.0;
/*
const int CNP_UUV = 1;
const int CNP_USV = 2;
const int CNP_UAV = 3;
const int CNP_SLUAV = 4;
const int CNP_UGV = 5;
*/

#endif
/**********************************************************************
 * $Log: cnp_constants.h,v $
 * Revision 1.1.1.1  2008/07/14 16:44:17  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/20 17:17:47  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:37:55  endo
 * MissionLab 7.0
 *
 * Revision 1.6  2006/05/02 04:12:58  endo
 * TrackTask improved for the experiment.
 *
 * Revision 1.5  2006/04/28 22:50:42  pulam
 * Constraint checking update, Terrainmap disable for large maps, renegging overhaul
 *
 * Revision 1.4  2006/03/05 23:27:20  pulam
 * Addition of SL-UAV code and CNP code for calculating bids for intercept/inspect tasks
 *
 * Revision 1.3  2006/02/19 22:41:10  endo
 * CNP_SAVE_TARGET_LOCATION stuff.
 *
 * Revision 1.2  2006/02/19 18:00:50  endo
 * Experiment related modifications
 *
 * Revision 1.1  2006/02/17 06:25:29  endo
 * *** empty log message ***
 *
 **********************************************************************/
