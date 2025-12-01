/**********************************************************************
 **                                                                  **
 **                             tanfit.c                             **
 **                                                                  **
 **  Functions for fitting lines through points.                      **
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

/* $Id: tanfit.c,v 1.1.1.1 2006/07/12 13:37:57 endo Exp $ */

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

#include "ezxwin.h"
#include "data_structures.h"
#include <assert.h>

/* Function calculates the bestfit line for a number of points. The 
   representation of the line is an angle and a distance. The angle is 
   the perpendicular of the line with respect to the x-axis and is clamped between -PI and PI.
   The distance is the perpendicular to the line and is always positive */
double tanfit(double x[], double y[], int ndata, 
	      double *phi, double *rho)
{
  double tx = 0.0, ty = 0.0, txx = 0.0, tyy = 0.0, txy = 0.0;
  double mx, my, Sx2, Sy2, Sxy, p, r, Sdiff, err;
  int i;

  for (i = 0; i < ndata; i++) {
    tx  += x[i];
    ty  += y[i];
    txx += x[i] * x[i];
    tyy += y[i] * y[i];
    txy += x[i] * y[i];
  }
  mx  = tx/ndata;
  my  = ty/ndata;
  /* Sx2 = sum for all i, (xi - mx)^2 
     Sx2 = sum for all i, xi^2 - 2mx*xi + mx^2 */
  Sx2 = txx - 2*mx*tx + mx*mx*ndata;
  Sy2 = tyy - 2*my*ty + my*my*ndata;
  /* Sxy = sum for all i, (xi - mx)(yi - my)
     Sxy = sum for all i, xi*yi - mx*yi - xi*my + mx*my */
  Sxy = txy - mx*ty - tx*my + mx*my*ndata;
  
  Sdiff = Sy2 - Sx2;

  p   = 0.5 * atan2(-2*Sxy, Sdiff);
  r   = mx*cos(p) + my*sin(p);
  err = 0.5 * (Sx2 + Sy2 - sqrt(4*Sxy*Sxy + Sdiff*Sdiff));

  /* The above procedure returns phi in the range -M_PI/2..M_PI/2
     and rho as positive and negative. We will now convert phi
     to the range -M_PI..M_PI and rho will become the perpendicular 
     distance to the line
  */
  if (r < 0) {
    p += M_PI;
    r = -r;
  }
  if (p > M_PI) {
    p -= (2*M_PI);
  }
  //assert(p >= -M_PI && p <= M_PI);

  *phi = p;
  *rho = r;
  return err;
}

#define NUM_TANFIT_POINTS 3
#define MAX_TANFIT_ERROR  5

/* For each point compute, using the neighborhood of points about that point, 
   the bestfit line in (phi, rho) notation and store this in
   the sv data structure. Lines are not stored if the error is too large or
   nearly parallel to the sensor. */
int computeAllTanfits(double x[], double y[], int ndata, tanfitRec_t *sv)
{
  int i, ntan = 0;
  double err, phi, rho, theta, incidence;

  for (i = 0; i < ndata; i++) {
    // Too early or late in the array to have a large enough neighborhood 
    if (i < NUM_TANFIT_POINTS) continue;
    if (i >= (ndata - NUM_TANFIT_POINTS)) continue;
    
    // Compute the bestfit line
    err = tanfit(&x[i - NUM_TANFIT_POINTS], &y[i - NUM_TANFIT_POINTS], 
		 NUM_TANFIT_POINTS*2 + 1, &phi, &rho);
    if (err > MAX_TANFIT_ERROR) continue;
    theta = atan2(y[i], x[i]);
    incidence = 180/M_PI*fabs(theta - phi);
    /* When incidence is near 90 degrees the laser beam is
       near parallel to the wall */
    if (incidence > 85 && incidence < 95) continue; 
      
    // Save the information;
    sv->phi[ntan] = phi;
    sv->rho[ntan] = rho;
    sv->x[ntan] = x[i];
    sv->y[ntan] = y[i];
    ntan++;
  }
  sv->num = ntan;
  return SUCCESS;
}
//True if a is within 2*tol of b
#define NEAR(a, b, tol) (((a) + tol) >= b && ((a) - tol) <= b)

// ENDO  
//#define NUM_TANFIT_BINS 360
#define NUM_TANFIT_BINS 90

int winnerTakeAllTanfit(tanfitRec_t *sv, double *phi)
{
  int i, idx, winner, wnum, ntan = 0, wcur;
  double p, range = M_PI/(double)NUM_TANFIT_BINS;
  int tanfitBins[NUM_TANFIT_BINS + 1];

  //Zero all the bins
  for (i = 0; i <= NUM_TANFIT_BINS; i++) 
    tanfitBins[i] = 0;

  /* Find the index of the bin which corresponds to
     the current phi. The bins store the angle shifted
     to the range of 0..M_PI, instead of -M_PI..M_PI. That is
     angles less than zero have been reflected through the origin. 
     This ensures that lines with similar slope above and below the
     origin will be matched up. */
  for (i = 0; i < sv->num; i++) {
    p = sv->phi[i];
    if (p < 0) p+= M_PI;
    idx = (int)(floor(p/range));
    tanfitBins[idx]++;
  }
  wnum = 0;
  winner = 0;
  // Find the winning bin = the bin with the largest value
  for (i = 2; i < (NUM_TANFIT_BINS-1); i++) {
    // Here we use a triangle filter centered about bin i
      wcur = (int)(tanfitBins[i]*0.4 + tanfitBins[i-1]*0.2 + tanfitBins[i+1]*0.2 + tanfitBins[i-2]*0.1 + tanfitBins[i+2]*0.1);
    if (wcur > wnum) {
      wnum = wcur;
      winner = i;
    }
  }

  // Save the info for all entries with the same winning angle
  for (i = 0; i < sv->num; i++) {
    p = sv->phi[i];
    if (p < 0) p+= M_PI;
    idx = (int)(floor(p/range));
    if (NEAR(idx, winner, 1)) {
      sv->winningX[ntan] = sv->x[i];
      sv->winningY[ntan] = sv->y[i];      
      sv->winningRho[ntan] = sv->rho[i];
      sv->winningPhi[ntan] = sv->phi[i];
      ntan++;
    }
  }
  
  sv->winningNum = ntan;
  
  //The winning phi is the corresponding angle   
  *phi = range*winner;
  return SUCCESS;
}

// Translate then rotate
void translateAndRotateLaserScan(double x[], double y[], int npts, double angle, double tx, double ty)
{
  int i;
  double xx, yy;

  for (i = 0; i < npts; i++) {
    xx = x[i] + tx;
    yy = y[i] + ty;
    x[i] = (cos(angle)*xx - sin(angle)*yy);
    y[i] = (sin(angle)*xx + cos(angle)*yy);
  }
}
// Rotate then translate
void rotateAndTranslateLaserScan(double x[], double y[], int npts, double angle, double tx, double ty)
{
  int i;
  double xx, yy;

  for (i = 0; i < npts; i++) {
    xx = x[i];
    yy = y[i];
    x[i] = (cos(angle)*xx - sin(angle)*yy);
    y[i] = (sin(angle)*xx + cos(angle)*yy);
    x[i] += tx;
    y[i] += ty;
  }
}

#define MIN_POINTS_ON_WALL 4
#define MAX_HWAY_WIDTH 200

void calculateCenterLineRho(tanfitRec_t *sv, double *phi, double *tx, double *ty, double *hwidth, double *robotOffestFromCenter)
{
  double nhump = 0, phump = 0, wnum, wcur, d, avg;
  int i, nnum = 0, pnum = 0, winner;
  double pwall_bins[8000], nwall_bins[8000];

  /* we need to find phi which is the perpendicular to the right hand wall.
     right now we have phi and 180+phi as the angles to the left and 
     right hand side walls. The right hand side wall will have negative 
     y-intercept, thus it suffices to check the sign of sin(phi) and
     adjust by M_PI if necessary. */

  if (sin(*phi) > 0) {
    *phi += M_PI;
    if (*phi > M_PI)
      *phi -= (M_PI*2.0);
  }
  // Zero the bins
  for (i = 0; i < 8000; i++) {
    pwall_bins[i] = 0;
    nwall_bins[i] = 0;
  }
  /* The saved points are those from the hallway walls. They have a tangent angle
     equal to phi or 180+phi as mentioned. We need to sort them into two piles */
  for (i = 0; i < sv->winningNum; i++) {
    if (NEAR(sv->winningPhi[i], *phi, M_PI/NUM_TANFIT_BINS*5)) {
      if (sv->winningRho[i] < MAX_HWAY_WIDTH) {
	nwall_bins[(int)rint(sv->winningRho[i])]++;
	nnum++;
      }
    }
    else {
      if (sv->winningRho[i] < MAX_HWAY_WIDTH) {
	pwall_bins[(int)rint(sv->winningRho[i])]++;
	pnum++;
      }
    }
  }
  // If there are some minimum number of points along the wall find its distance to the y-axis
  if (pnum >= MIN_POINTS_ON_WALL) {
    wnum = 0;
    winner = 0;
    // Find the winning bin = the bin with the largest value
    for (i = 2; i < 8000; i++) {
      wcur = pwall_bins[i]*0.4 + pwall_bins[i-1]*0.2 + pwall_bins[i+1]*0.2 + pwall_bins[i-2]*0.1 + pwall_bins[i+2]*0.1;
      if (wcur > wnum) {
	wnum = wcur;
	winner = i;
      }
    }
    phump = winner;
  }

  // If there are some minimum number of points along the wall find its distance to the y-axis
  if (nnum >= MIN_POINTS_ON_WALL) {
    wnum = 0;
    winner = 0;
    // Find the winning bin = the bin with the largest value
    for (i = 2; i < 8000; i++) {
      wcur = nwall_bins[i]*0.4 + nwall_bins[i-1]*0.2 + nwall_bins[i+1]*0.2 + nwall_bins[i-2]*0.1 + nwall_bins[i+2]*0.1;
      if (wcur > wnum) {
	wnum = wcur;
	winner = i;
      }
    }
    nhump = winner;
  }
  /* Can only see left wall */
  if (nnum < MIN_POINTS_ON_WALL) {
    d = phump;
    // ENDO
    //*hwidth = 170;
    *hwidth = 152;
    // ENDO
    //d -= 85; //for now only assume half width of corridor
    d -= 76; //for now only assume half width of corridor
    *tx = -cos(*phi)*d;
    *ty = -sin(*phi)*d;
  }
  /* Can only see right wall */
  else if (pnum < MIN_POINTS_ON_WALL) {
    d = nhump;
    // ENDO
    //*hwidth = 170;
    *hwidth = 152;
    //for now only assume half width of corridor
    // ENDO
    //d = 85 - nhump;
    d = 76 - nhump;
    *tx = -cos(*phi)*d;
    *ty = -sin(*phi)*d;
  }
  else {
    avg = (nhump + phump)/2.0;
    *hwidth = nhump + phump;
    d   = phump;
    d  -= avg;
    //printf("p %f n %f d %f\n", phump, nhump, d);
    /* Phi gives the angle from along the robot's x-axis to 
       the perpendicular to the right hand wall. In order to translate the
       raw hallway data we need the offsets along the X and Y axes that the 
       robot lies. */
    *tx = -cos(*phi)*d; 
    *ty = -sin(*phi)*d;    
  }
  *robotOffestFromCenter = d;
  return;
}

#define FILTER_WIDTH     100.0
#define FILTER_START_Y   -80.0
#define FILTER_END_Y     200.0
#define FILTER_TOLERANCE 2
#define FILTER_HEIGHT    50.0
#define MIN_BIN_ENTRIES  2
#define FILTER_RANGE     (FILTER_END_Y - FILTER_START_Y)
#define FILTER_INCREMENT 5.0
#define FILTER_NUM_BINS  ((int)(FILTER_RANGE/FILTER_INCREMENT + 1))

/* We look for a door by passing a rectangle filter along the line x = wallX.
   The robot is at (robotX, 0) with heading phi. The doorway endpoints are returned
   in the last four parameters. */
int doorMatchedFilter(double x[], double y[], int ndata, double wallX, double robotX, 
		      double phi, rawLaserScan_t *rawdata,
		      double *sdx, double *sdy, double *edx, double *edy)
     
{
  double min, max;
  int i, j, idx, count, bins[FILTER_NUM_BINS], num;
  int start, end;
  double ainc, sang, eang, dang, dist;

  // Zero the bins
  for (i = 0; i < FILTER_NUM_BINS; i++) 
    bins[i] = 0;

  /* The idea here is whenever there is an opening the
     size of the filter we sprinkle some hits into the 
     corresponding bins. */
  min = FILTER_START_Y;
  max = FILTER_HEIGHT + FILTER_START_Y;
  while (max <= FILTER_END_Y) {
    count = 0;
    // Count up all the data points that lie within the filter
    for (i = 0; i < ndata; i++) {
      if (NEAR(x[i], wallX, FILTER_WIDTH/2.0)) {
	if (y[i] >= min && y[i] <= max) {
	  count++;
	}
      }
    }
    /* If the filter is empty within some tolerance then set the 
       corresponding bins. */
    if (count < FILTER_TOLERANCE) {
        idx = (int)(floor((min - FILTER_START_Y)/FILTER_INCREMENT));
      for (j = 0; j < FILTER_HEIGHT/FILTER_INCREMENT; j++) {
	if (idx >= FILTER_NUM_BINS) continue;
	bins[idx]++;
	idx++;
      }
    }
    // Shift the filter up and look again
    max += FILTER_INCREMENT;
    min += FILTER_INCREMENT;
  }

  /* The bin entries have now been set for openings in the wall. This
     loop will now remove two types of phantom openings; those that appear
     because the sensor's FoV does not include that part of the wall, and
     those that appear because the wall is occluded by obstacles. */
  ainc = M_PI/ndata;
  for (i = 0; i < FILTER_NUM_BINS; i++) {
    // If a bin is set, ensure that it is really an opening
    if (bins[i] >= MIN_BIN_ENTRIES) {
      /* Remember each bin represents FILTER_INCREMENT centimeters so sang and
	 eang represent the start and end angle of the opening corresponding to the bin.
	 The angle will be ego-centric to the robot. */
      sang = atan2((i*FILTER_INCREMENT) + FILTER_START_Y, wallX - robotX) - phi;
      eang = atan2(((i+1)*FILTER_INCREMENT) + FILTER_START_Y, wallX - robotX) - phi;
      if (sang >  M_PI) sang -= (2*M_PI);
      if (sang < -M_PI) sang += (2*M_PI);
      if (eang >  M_PI) eang -= (2*M_PI);
      if (eang < -M_PI) eang += (2*M_PI);
      
      if (sang > M_PI/2.0 || sang < -M_PI/2.0) {
	//the angle corresponding to the bin is outside the sensor FoV
	bins[i] = 0;

      }
      else if (eang > M_PI/2.0 || eang < -M_PI/2.0) {
	//the angle corresponding to the bin is outside the sensor FoV
	bins[i] = 0;

      }
      else {
	// Check for points in front of the opening
	// sang should be the smaller of the two angles
	if (sang > eang) {
	  //swap sang and eang
	  dang = sang;
	  sang = eang;
	  eang = dang;
	}
	dang = fabs(sang - eang);
	// num is the number of laser readings that fall between sang and eang
	num = (int)(dang/ainc);
	if (num == 0) num = 1;

	//shift sang to the range 0..M_PI
	idx = (int)(floor((sang + (M_PI/2.0))/ainc));
	for (j = 0; j < num; j++) {
	  //avoid divide by zero
	  if (cos(sang + phi) == 0.0) continue;
	  //avoid out of range entries
	  if ((idx + j) > ndata) continue;
	  dist = (wallX - robotX)/cos(sang + phi);
	  if (rawdata->dist[idx+j] < dist)
	    bins[i] = 0;
	  
	  sang += ainc; 
	}
      }
    }
  }

  /* We have removed all phantom doors, now look if
     there is any opening of atleast size FILTER_HEIGHT/FILTER_INCREMENT
     centimeters. */
  for (i = 0; i < FILTER_NUM_BINS; i++) { 
    if (bins[i] >= MIN_BIN_ENTRIES) {
      start = i;
      do {
	i++;
      } while (i < FILTER_NUM_BINS && bins[i] >= MIN_BIN_ENTRIES);
      end = (i < FILTER_NUM_BINS)? i: FILTER_NUM_BINS - 1;
      *sdx = wallX;
      *sdy = (start*FILTER_INCREMENT) + FILTER_START_Y;
      *edx = wallX;
      *edy = (end*FILTER_INCREMENT) + FILTER_START_Y;
      /* If the opening is still long enough */
      if ((end - start) >= (FILTER_HEIGHT/FILTER_INCREMENT)) {
	return SUCCESS;
      }
    }
  }

  // No door return false
  return FAILURE;
}

double vlength(double x, double y) 
{ 
  return sqrt(x*x + y*y); 
}

double perpdist(double lineseg[4], double x, double y)
{
  double dist, Ux, Uy, length, Vx, Vy;

  // create a unit vector along the line 
  Ux = lineseg[2] - lineseg[0];
  Uy = lineseg[3] - lineseg[1];
  length = vlength(Ux,Uy);
  Ux /= length;
  Uy /= length;
  
  // create a vector to the point
  Vx = x - lineseg[0];
  Vy = y - lineseg[1];
  
  // now calculate the distance via the cross product.
  // recall cross product gives perpendicular component of resultant 
  // vector
  dist = fabs(Ux * Vy - Uy * Vx);
   
  
  // return distance from line to the point
  return dist;
}

// Return the sum of the perpendicular ditance to the line for each point
double toterror(double lineseg[4], double x[], double y[], int ndata)
{
  int i;
  double terr = 0.0, err;

  for (i = 0; i < ndata; i++) {
    err = perpdist(lineseg, x[i], y[i]);
    if (err > 4500) err = 0.0;
    terr += err;
  }
  return terr;
}
 
// Return the average perpendicular distance to the line
double meanerror(double lineseg[4], double x[], double y[], int ndata) {
  double ret;
  if (ndata == 0) return 0.0;
  ret = toterror(lineseg, x, y, ndata);
  ret /= (double)ndata;

  return ret;
}


/**********************************************************************
 * $Log: tanfit.c,v $
 * Revision 1.1.1.1  2006/07/12 13:37:57  endo
 * MissionLab 7.0
 *
 * Revision 1.2  2006/05/15 21:03:36  endo
 * gcc-3.4 upgrade
 *
 * Revision 1.1.1.1  2005/02/06 22:59:40  endo
 * AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
 *
 * Revision 1.2  2000/09/19 01:03:48  jdiaz
 * rockville modifications
 *
 * Revision 1.1  2000/08/12 18:45:31  saho
 * Initial revision
 *
 **********************************************************************/







