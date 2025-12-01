#include <stdio.h>
#include <math.h>

#include "randlib.h"


/**************************************************************************/
/*              STATISTICAL DISTRIBUTIONS ROUTINES                        */
/**************************************************************************/

// 
//   Calculates a probability based on a Gaussian Distribution
//
//                        1                     (x- mu)^2
//        Pm(x) =  ------------------ * exp [ - ---------  ]
//                 sigma * sqrt(2*Pi)           2* sigma^2
//
//
double normalProbability(double x, double mu, double sigma)
{
  return (1/ (sigma* sqrt(2*M_PI))) * exp(- ((x - mu)*(x - mu)) /(2* sigma*sigma));
}


double randomNormal(double mu, double sigma)
{
  /* We use a routine from RANDLIB.C  */
  return (double ) gennor((float) mu, (float) sigma);
}


double randomUniformDouble(double low, double high)
{
  /* We use a routine from RANDLIB.C  */
  return (double) genunf( (float) low, (float) high);
}



//
//          {  0             i=0
//     Pu=  {
//          {  Cr*(1- Sum Pu(Dj))  otherwise.
//                    j<i
//
//    Cr - is the probability that the sensor is deflected by an unknown 
//         obstacle at any range given by the discretization.
//
static double Pu[8000];

void precomputeGeometricDistribution()
{
  int i;
  double sumPu;
  double Cr;


  Pu[0]=0;  /* Initial Condition */
  sumPu=0;
  Cr= 0.0025;

  for (i=1; i<8000; i++)
	{
	   Pu[i]= Cr* (1 - sumPu);
	   sumPu+= Pu[i];
	}
}

double geometricProbability(double distanceReading)
{
   return Pu[(int)distanceReading];
}






/**************************************************************************/
/*              TESTING AND DEBUGGING ROUTINES                            */
/**************************************************************************/




/* These functions test the Normal and Geometric probability distribution
 * functions. They generate a umber of points and save them in files so 
 * that they can be read later fron Matlab for plotting.
 */

void testNormal(double mu, double sigma)
{
  FILE *fd;
  int x;
  double pr;

  fd=fopen("normal.txt","w");

  for(x=0; x < 8000; x++)
	{
	  pr=normalProbability(x, mu, sigma);
	  fprintf(fd, "%d %.3g\n", x, pr);
	}
  fclose(fd);
}


void testRandomNormal(double mu, double sigma)
{
  FILE *fd;
  int x;
  double pr;

  fd=fopen("randomNormal.txt","w");

  for(x=0; x < 1000; x++)
	{
	  pr=randomNormal( mu, sigma);
	  fprintf(fd, "%d %g\n", x, pr);
	}
  fclose(fd);
}

void testGeometric()
{
  FILE *fd;
  int x;
  double pr;

  fd=fopen("geometric.txt","w");

  for(x=0; x < 8000; x++)
	{
	  pr=geometricProbability(x);
	  fprintf(fd, "%d %.3g\n", x, pr);
	}
  fclose(fd);
}



void testAllRoutines()
{
  testNormal(230, 5);
  testRandomNormal(230, 5);
  testGeometric();
}


