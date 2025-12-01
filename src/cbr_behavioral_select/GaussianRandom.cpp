/**********************************************************************
 **                                                                  **
 **                       GaussianRandom.cpp                         **
 **                                                                  **
 ** An Gaussian random number generator                              **
 **                                                                  **
 ** By: Zsolt Kira                                                   **
 **                                                                  **
 ** Copyright 2006, Georgia Tech Research Corporation                **
 ** Atlanta, Georgia  30332-0415                                     **
 ** ALL RIGHTS RESERVED, See file COPYRIGHT for details.             **
 **********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

double gaussianRandom() {
  double x1, x2;

  // Uniform random from 0...1
  x1 = rand() / (double) RAND_MAX;
  x2 = rand() / (double) RAND_MAX;

  x1 = sqrt(-2.0 * log(x1)) * cos ( 2.0 * M_PI * x2);

  return x1;

}

/**********************************************************************
 * $Log: GaussianRandom.cpp,v $
 * Revision 1.1.1.1  2008/07/14 16:44:14  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/20 17:17:45  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:37:53  endo
 * MissionLab 7.0
 *
 * Revision 1.1  2006/07/04 18:42:16  endo
 * gasdev() replaced by GaussianRandom().
 *
 **********************************************************************/
