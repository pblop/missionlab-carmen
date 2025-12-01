#include "matt_lib.h"
#include <cstdlib>
#include <math.h>
#include <stdio.h>
//#include <iostream.h>
#include "randlib.h"

using namespace std;
double scaled_rand(double min_val, double max_val){
  int temp_rand = rand();
  double diff = max_val - min_val;
  double return_rand;
  return_rand = diff * ((double)temp_rand / (RAND_MAX + 1.0)) 
    + min_val;
  return return_rand;
  // return (double)temp_rand / (RAND_MAX + 1.0);
}

double unit_rand(){
  return scaled_rand(0,1);
}

double normal_pdf(double x, double mu, double sigma){
    double pdf, exponent;
  exponent = (x - mu) * (x - mu) / (2 * (sigma*sigma));
  pdf = exp(-exponent) / (sigma * sqrt(2 * PI));
  return pdf;
}

/*double normal_pdf(double x, double mu, double sigma){
  double pdf;
  pdf = (double) gennor((float) mu, (float) sigma);
  return pdf;
  }*/

double scaled_normal_pdf(double x, double mu, double sigma){
  double scaled_pdf;
  scaled_pdf = normal_pdf(x,mu,sigma) 
    / normal_pdf(mu,mu,sigma);
  return scaled_pdf;
}

double normal_cdf(double x, double mu, double sigma){
  double delta = .001;
  double integral = 0;
  
  for(double i = mu - 6 * sigma; i <= x; i = i + delta){
    integral += normal_pdf(i, mu, sigma) * delta;
  }
  return integral;
}
    
double normal_inv(double y, double mu, double sigma){
  double delta = .001;
  double x = mu - 6 * sigma;
  double integral = 0;
  while(integral < y){
    integral += normal_pdf(x, mu, sigma) * delta;
    x += delta;
  }
  return x - delta;
}

double normal_sample_calc(double mu, double sigma){
  return normal_inv(unit_rand(), mu, sigma);
}
  
double normal_sample(double mu, double sigma){
  double pdf;
  pdf = (double) gennor((float) mu, (float) sigma);
  return pdf;
}

double angle_pdf(double theta, double mu, double sigma){
  double pdf;
  if(theta - mu > 180)
    pdf = normal_pdf(360 - theta + mu, mu, sigma);
  else if(mu - theta > 180)
    pdf = normal_pdf(360 - mu + theta, mu, sigma);
  else
    pdf = normal_pdf(theta, mu, sigma);
  return pdf;

}

double scaled_angle_pdf(double theta, double mu, double sigma){
  double scaled_pdf;
  scaled_pdf = angle_pdf(theta,mu,sigma) 
    / angle_pdf(mu,mu,sigma);
  return scaled_pdf;
}
