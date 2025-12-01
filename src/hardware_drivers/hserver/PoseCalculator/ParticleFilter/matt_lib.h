#ifndef _MATT_LIB_
#define _MATT_LIB_
#define PI 3.1415926


double scaled_rand(double min_val, double max_val);

double unit_rand();

double normal_pdf(double x, double mu, double sigma);

double normal_pdf_calc(double x, double mu, double sigma);

double scaled_normal_pdf(double x, double mu, double sigma);

double normal_cdf(double x, double mu, double sigma);

double normal_inv(double y, double mu, double sigma);

double normal_sample(double mu, double sigma);

double angle_pdf(double theta, double mu, double sigma);

double scaled_angle_pdf(double theta, double mu, double sigma);

#endif
