#include <stdio.h>
#include <math.h>

main(){
  double x,y;
  int i;
  double theta, theta1, theta2;



  printf("WALLS 4\n");
  printf("10.0\t10.0\n");
  printf("10.0\t110.0\n");
  printf("110.0\t110.0\n");
  printf("110.0\t10.0\n");

  for(theta = 0.0;theta < 6.28;theta += 2.0*M_PI/3.0){
  printf("\nOBSTACLES 3\n");

    x = 60.0 + 10.0*cos(theta);
    y = 60.0 + 10.0*sin(theta);
    printf("%5.2f %5.2f\n", x, y);

    theta2 = theta + M_PI/30;
    x = 60.0 + 20.0*cos(theta2);
    y = 60.0 + 20.0*sin(theta2);
    printf("%5.2f %5.2f\n", x, y);

    theta2 = theta - M_PI/30;
    x = 60.0 + 20.0*cos(theta2);
    y = 60.0 + 20.0*sin(theta2);
    printf("%5.2f %5.2f\n", x, y);
  }

  printf("\nOBSTACLES 0\n");

  printf("\nLANDMARKS 0\n");

}



main2(){
  double x,y;
  int i;
  double theta;



  printf("WALLS 30\n");
  for(theta = 0.0;theta < 6.28;theta += M_PI/15){
    x = 30.0 + 20.0*cos(theta);
    y = 30.0 + 20.0*sin(theta);
    printf("%5.2f %5.2f\n",x, y);
  }

  printf("\nOBSTACLES 30\n");
  for(theta = 0.0;theta < 6.28;theta += M_PI/15){
    x = 30.0 + 10.0*cos(-theta);
    y = 30.0 + 10.0*sin(-theta);
    printf("%5.2f %5.2f\n",x, y);
  }

  printf("\nOBSTACLES 0\n");

  printf("\nLANDMARKS 0\n");

}
