/**********************************************************************
 **                                                                  **
 **                             test.c                               **
 **                                                                  **
 **  Test file for lineutils.c                                       **
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

/* $Id: test.c,v 1.1.1.1 2006/07/12 13:37:57 endo Exp $ */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "data_structures.h"

void printPoint(point2d_t *p, char *txt);
void printLineSeg(lineseg2d_t *l, char *txt);
void printLine(line2d_t *l, char *txt);

void testLineSegInters1(void)
{
  point2d_t p1, p2, inters;
  
  lineseg2d_t xl, yl;

  p1.x = 5;
  p1.y = 1;
  p2.x = 0;
  p2.y = 3;
  
  convPoints2LineSeg(&p1, &p2, &xl);
  printLineSeg(&xl, "x-axis");
  p1.x = 0;
  p1.y = 1;
  p2.x = 5;
  p2.y = 3;
  
  convPoints2LineSeg(&p1, &p2, &yl);
  printLineSeg(&yl, "y-axis");

  if (intersectTwoLineSegs(&xl, &yl, &inters))
  
    printPoint(&inters, "Intersection point:");
  else printf("No intersection point\n");
  printf("Expected (2.5, 2)\n");
}

void testLineSegInters2(void)
{
  point2d_t p1, p2, inters;
  
  lineseg2d_t xl, yl;

  p1.x = 5;
  p1.y = 1;
  p2.x = 0;
  p2.y = 3;
  
  convPoints2LineSeg(&p1, &p2, &xl);
  printLineSeg(&xl, "x-axis");
  p1.x = 2.5;
  p1.y = 2;
  p2.x = 5;
  p2.y = 3;
  
  convPoints2LineSeg(&p1, &p2, &yl);
  printLineSeg(&yl, "y-axis");

  if (intersectTwoLineSegs(&xl, &yl, &inters))
  
    printPoint(&inters, "Intersection point:");
  else printf("No intersection point\n");
  printf("Expected (2.5, 2)\n");
}

void testLineSegInters3(void)
{
  point2d_t p1, p2, inters;
  
  lineseg2d_t xl, yl;

  p1.x = -1;
  p1.y = 3;
  p2.x = 1;
  p2.y = 0;
  
  convPoints2LineSeg(&p1, &p2, &xl);
  printLineSeg(&xl, "x-axis");
  p1.x = 2;
  p1.y = 0;
  p2.x = -1;
  p2.y = 4;
  
  convPoints2LineSeg(&p1, &p2, &yl);
  printLineSeg(&yl, "y-axis");

  if (intersectTwoLineSegs(&xl, &yl, &inters))
  
    printPoint(&inters, "Intersection point:");
  else printf("No intersection point\n");
  printf("Expected no inters\n");
}




void testLineInters1(void)
{
  point2d_t p1, p2, inters;
  
  line2d_t xl, yl;

  p1.x = 5;
  p1.y = 1;
  p2.x = 0;
  p2.y = 3;
  
  convPoints2Line(&p1, &p2, &xl);
  printLine(&xl, "x-axis");
  p1.x = 0;
  p1.y = 1;
  p2.x = 5;
  p2.y = 3;
  
  convPoints2Line(&p1, &p2, &yl);
  printLine(&yl, "y-axis");

  if (intersectTwoLines(&xl, &yl, &inters))
  
    printPoint(&inters, "Intersection point:");
  else printf("No intersection point\n");
  printf("Expected (2.5, 2)\n");
}

void testLineInters2(void)
{
  point2d_t p1, p2, inters;
  
  line2d_t xl, yl;

  p1.x = 5;
  p1.y = 1;
  p2.x = 0;
  p2.y = 3;
  
  convPoints2Line(&p1, &p2, &xl);
  printLine(&xl, "x-axis");
  p1.x = 2.5;
  p1.y = 2;
  p2.x = 5;
  p2.y = 3;
  
  convPoints2Line(&p1, &p2, &yl);
  printLine(&yl, "y-axis");

  if (intersectTwoLines(&xl, &yl, &inters))
  
    printPoint(&inters, "Intersection point:");
  else printf("No intersection point\n");
  printf("Expected (2.5, 2)\n");
}

void testLineInters3(void)
{
  point2d_t p1, p2, inters;
  
  line2d_t xl, yl;

  p1.x = -2;
  p1.y = 4;
  p2.x = 1;
  p2.y = 0;
  
  convPoints2Line(&p1, &p2, &xl);
  printLine(&xl, "x-axis");
  p1.x = 2;
  p1.y = 0;
  p2.x = -1;
  p2.y = 4;
  
  convPoints2Line(&p1, &p2, &yl);
  printLine(&yl, "y-axis");

  if (intersectTwoLines(&xl, &yl, &inters))
  
    printPoint(&inters, "Intersection point:");
  else printf("No intersection point\n");
  printf("Expected no inters\n");
}

void testLineInters4(void)
{
  point2d_t p1, p2, inters;
  
  line2d_t xl, yl;

  p1.x = 3;
  p1.y = -1;
  p2.x = 3;
  p2.y = 2;
  
  convPoints2Line(&p1, &p2, &xl);
  printLine(&xl, "x-axis");
  p1.x = 2;
  p1.y = 0;
  p2.x = -1;
  p2.y = 4;
  
  convPoints2Line(&p1, &p2, &yl);
  printLine(&yl, "y-axis");

  if (intersectTwoLines(&xl, &yl, &inters))
  
    printPoint(&inters, "Intersection point:");
  else printf("No intersection point\n");
  printf("Expected 3, -1.33\n");
}

void testLineInters5(void)
{
  point2d_t p1, p2, inters;
  
  line2d_t xl, yl;

  p1.x = 0;
  p1.y = -1;
  p2.x = 5;
  p2.y = -1;
  
  convPoints2Line(&p1, &p2, &xl);
  printLine(&xl, "x-axis");
  p1.x = 0;
  p1.y = 2;
  p2.x = 4;
  p2.y = -2;
  
  convPoints2Line(&p1, &p2, &yl);
  printLine(&yl, "y-axis");

  if (intersectTwoLines(&xl, &yl, &inters))
  
    printPoint(&inters, "Intersection point:");
  else printf("No intersection point\n");
  printf("Expected 3,-1\n");
}
void testLineInters6(void)
{
  point2d_t p1, p2, inters;
  
  line2d_t xl, yl;

  p1.x = -1;
  p1.y = 3;
  p2.x = 1;
  p2.y = -3;
  
  convPoints2Line(&p1, &p2, &xl);
  printLine(&xl, "x-axis");
  p1.x = 0;
  p1.y = 2;
  p2.x = 4;
  p2.y = 0;
  
  convPoints2Line(&p1, &p2, &yl);
  printLine(&yl, "y-axis");

  if (intersectTwoLines(&xl, &yl, &inters))
  
    printPoint(&inters, "Intersection point:");
  else printf("No intersection point\n");
  printf("Expected no inters parallel\n");
}

int main(int argc, char **agrv)
{
  testLineSegInters1();
  testLineSegInters2();
  testLineSegInters3();

  testLineInters1();
  testLineInters2();
  testLineInters3();  
  testLineInters4();  
  testLineInters5();  
  testLineInters6();  

}

/**********************************************************************
 * $Log: test.c,v $
 * Revision 1.1.1.1  2006/07/12 13:37:57  endo
 * MissionLab 7.0
 *
 * Revision 1.1.1.1  2005/02/06 22:59:40  endo
 * AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
 *
 * Revision 1.1  2000/08/12 18:45:31  saho
 * Initial revision
 *
 **********************************************************************/
