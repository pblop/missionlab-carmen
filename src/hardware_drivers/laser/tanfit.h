/**********************************************************************
 **                                                                  **
 **                             tanfit.h                             **
 **                                                                  **
 **  Functions for fitting lines through points.                     **
 **                                                                  **
 **                                                                  **
 **  Written by:  Jonathan Diaz  and                                 **
 **               Alexander Stoytchev                                **
 **                                                                  **
 **  Copyright 2000, Georgia Tech Research Corporation               **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: tanfit.h,v 1.1.1.1 2006/07/12 13:37:57 endo Exp $ */

#ifndef TANFIT_HEADER
#define TANFIT_HEADER

#include "data_structures.h" 

double tanfit(double x[], double y[], int ndata, double *phi, double *rho);
int computeAllTanfits(double x[], double y[], int ndata, tanfitRec_t *sv);
int winnerTakeAllTanfit(tanfitRec_t *sv, double *phi);
void translateAndRotateLaserScan(double x[], double y[], int npts, 
				 double angle, double tx, double ty);
void rotateAndTranslateLaserScan(double x[], double y[], int npts, 
				 double angle, double tx, double ty);
void calculateCenterLineRho(tanfitRec_t *sv, double *phi, double *tx, 
			    double *ty, double *hwidth, double *robotOffestFromCenter);
int doorMatchedFilter(double x[], double y[], int ndata, double wallX, double robotX, 
		      double phi, rawLaserScan_t *rawdata,
		      double *sdx, double *sdy, double *edx, double *edy);
double toterror(double lineseg[4], double x[], double y[], int ndata);
double meanerror(double lineseg[4], double x[], double y[], int ndata);


#endif

/**********************************************************************
 * $Log: tanfit.h,v $
 * Revision 1.1.1.1  2006/07/12 13:37:57  endo
 * MissionLab 7.0
 *
 * Revision 1.1.1.1  2005/02/06 22:59:40  endo
 * AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
 *
 * Revision 1.1  2000/08/12 18:45:51  saho
 * Initial revision
 *
 **********************************************************************/
