/**********************************************************************
 **                                                                  **
 **  test.cc                                                         **
 **                                                                  **
 **  Test the Array class                                            **
 **                                                                  **
 **  Written by Ali Rahimi, published in July 1977 C++ Users Journal **
 **  Modified by:  Douglas C. MacKenzie                              **
 **                                                                  **
 **  Modifications copyright 1996-1998.  All Rights Reserved.        **
 **  Mobile Intelligence Corporation                                 **
 **  Alto, Michigan, USA                                             **
 **                                                                  **
 **********************************************************************/

/* $Id: test.cc,v 1.1.1.1 2008/07/14 16:44:16 endo Exp $ */

/**********************************************************************
* $Log: test.cc,v $
* Revision 1.1.1.1  2008/07/14 16:44:16  endo
* MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
*
* Revision 1.1.1.1  2006/07/20 17:17:46  endo
* MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
*
* Revision 1.1.1.1  2006/07/12 13:37:55  endo
* MissionLab 7.0
*
* Revision 1.1  2006/07/01 00:30:29  endo
* CMDLi from MARS 2020 migrated into AO-FNC repository.
*
* Revision 1.1.1.1  2006/06/29 20:42:07  endo
* cmdli local repository.
*
* Revision 1.2  1999/03/11 22:24:22  doug
* *** empty log message ***
*
* Revision 1.1  1998/11/18 02:24:04  doug
* Initial revision
*
**********************************************************************/

#include <stdio.h>
#include "Grid.h"
     
void main()
{
     
 try{
  Grid<int,10,19,20,29> va2d;
  va2d[11][28] = 11;
     
  int dim1 =va3d.dim();
  int low1 =va3d.begin();
  int high1= va3d.end();
  int dim2=va3d[low1].dim();
  int low2 =va3d[low1].begin();
  int high2 = va3d[low1].end();
     
  int dim3=va3d[low1][low2].dim();
     
  int low3 =va3d[low1][low2].begin();
  int high3 =va3d[low1][low2].end();
  printf("dim: %dX%dX%d\n",dim1,dim2,dim3);
     
     
  for( int i= low1; i<=high1; ++i)
   for( int j=low2; j<=high2; ++j)
    for( int k=low3; k<=high3; ++k)
    va3d[i][j][k] = (i*100+j)*100+k;
     
     
  for( int i= low1; i<=high1; ++i)
   for( int j=low2; j<=high2; ++j)
    for( int k=low3; k<=high3; ++k)
     if(va3d[i][j][k] != (i*100+j)*100+k)
      printf("%2d%2d%2d != %d\n",i,j,k,va3d[i][j][k]);
     
  //Projection
  Array1D<int, 30,39> va3d_1Dsubarr;
  va3d_1Dsubarr = va3d[10][20];
  for(int i=va3d_1Dsubarr.begin(); i<=va3d_1Dsubarr.end(); ++i)
   printf("[%d]=%d\n",i,va3d_1Dsubarr[i]);
     
     
  Array3D<int,10,19,20,29,30,39> *pva3d= new Array3D<int,10,19,20,29,30,39>;
     
  Array3D<int,10,19,20,29,30,39>& rva3d = *pva3d;
     
  //raise range error exception
  //va3d[1][20][30] = 1;
  //va3d[10][2][31] = 2;
  va3d[10][20][3] = 3;
     
     
  puts("\nend");
  while(1);
 }
 catch(RangeException RE)
 {
  char buf[128];
  sprintf(buf,
  "The %d-D(sub)array:\n  The subscript %d is out of range: %d:%d\n",
   RE.m_dim,RE.m_index,RE.m_low,RE.m_high);
  puts(buf);
 }
     
     
}
     
     
     
     
