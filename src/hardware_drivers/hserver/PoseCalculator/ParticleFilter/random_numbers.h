#ifndef _RANDOM_NUMBERS_H_
#define _RANDOM_NUMBERS_H_

#include "randlib.h"

double normalProbability(double x, double mu, double sigma);
double randomNormal(double mu, double sigma);

double randomUniformDouble(double low, double high);
double geometricProbability(double distanceReading);
void precomputeGeometricDistribution();

#endif
