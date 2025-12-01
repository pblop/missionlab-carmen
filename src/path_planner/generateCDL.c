/**********************************************************************
 **                                                                  **
 **                           generateCDL.c                          **
 **                                                                  **
 **                                                                  **
 **                                                                  **
 **                                                                  **
 **  Written by:                                                     **
 **                                                                  **
 **  Copyright 2000, Georgia Tech Research Corporation               **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **********************************************************************/

/* $Id: generateCDL.c,v 1.1.1.1 2008/07/14 16:44:24 endo Exp $ */

/***********************************************************/
/* generateCDL.c                                           */
/*                                                         */
/* AUTHOR: Alexander Stoytchev                             */
/*                                                         */
/* COMMENTS:                                               */
/*   This file generates CDL output files. In cooperation  */
/*   with the mapper it can build an FSA that can          */
/*   take the robot from the start to the end position.    */
/*                                                         */
/***********************************************************/

#include <stdio.h>

FILE *fp;

/* The $AN_### numbers are used to link different components 
   together. We arbitrary start from 550.  */
#define FIRST_AN_GROUP_NUMBER 550
#define FIRST_AN_SOCIETY_NUMBER 650

int AN_GroupNumber=FIRST_AN_GROUP_NUMBER;

struct state
{
   int AN_Num;
   int screenX;
   int screenY;
   int AN_linkNum;
   float goalX;
   float goalY;
};

struct state states[100];
int numberOfStates=0;


#define HEADER \
	"/*************************************************\n" \
	"*                                                 \n" \
	"* This CDL file %s was created                    \n" \
	"* with A* Path Planner                            \n" \
	"* version 1.0                                     \n" \
	"*                                                 \n" \
	"*************************************************/\n" \
	"\n"    \
	"bindArch AuRA; \n" \
	"\n\n"

void  dumpHeader(char *filename)
{
  fprintf(fp,HEADER, filename);
}






#define START_AND_STOP \
	"instGroup $AN_501 from [ \n" \
	"  Stop]; \n"                 \
	"\n"                          \
	"instGroup $AN_502 from [\n"  \
	"  Stop];\n\n"

void dumpStartAndStop()
{
  fprintf(fp,START_AND_STOP);
}









#define GOTO_GROUP  \
   "instGroup $AN_%d from [ \n" \
   "    %%Goal_Location = {%g,%g},    \n" \
   "    %%move_to_location_gain = {1.0},  \n" \
   "    %%avoid_obstacle_gain = {1.0},    \n" \
   "    %%avoid_obstacle_sphere = {%g},  \n" \
   "    %%avoid_obstacle_safety_margin = {%g} ,\n" \
   "  GoTo]; \n\n"

void dumpGoToGroup(float x, float y)
{
   float avoid_obstacle_sphere=0.35;
   float avoid_obstacle_safety_margin=0.2;

   fprintf(fp,GOTO_GROUP, AN_GroupNumber, x, y, 
             avoid_obstacle_sphere,
             avoid_obstacle_safety_margin);

   states[numberOfStates].goalX= x;
   states[numberOfStates].goalY= y;

   AN_GroupNumber++;
   numberOfStates++;
}
















#define BODY_BEGIN \
   "instBP<540,110> |The Wheels Binding Point| $AN_505 from movement( \n" \
   "  v<0,0> = , \n" \
   "  base_vel = {0.1}, \n" \
   "  bound_to = base:DRIVE( \n" \
   "        v<292,156> = FSA( \n" \
   "            society[Start]<160,40>|Start|     = $AN_501, \n" 


#define BODY_END \
   "        )<292,156>|The State Machine| , \n" \
   "                                        \n" \
   "        max_vel = {0.2}, \n" \
   "        base_vel = {1},  \n" \
   "        cautious_vel = {0.05},  \n" \
   "        cautious_mode = {false} \n" \
   "                                        \n" \
   "     )<565,111>|The Wheels Actuator| \n" \
   " \n" \
   ");\n\n"

#define START_RULE \
   "            rules[Start]<160,40>|Start| = \n" \
   "               if [ FirstTime]<0,0>|Trans1| goto $AN_%d" 

#define OTHER_RULE \
   "            rules[$AN_%d]<%d,%d>|State%d| = \n" \
   "             if [ %%Goal_Tolerance = {0.5}, \n" \
   "                  %%Goal_Location = {%g, %g} ,  \n" \
   "                  AtGoal  \n" \
   "                ] <0,0>|Trans%d| goto $AN_%d" 

 
void dumpBody()
{
   int i;
   fprintf(fp,BODY_BEGIN);

   /* print out society clauses */

   for(i=0; i< numberOfStates; i++)
	 {

	    states[i].AN_Num= FIRST_AN_SOCIETY_NUMBER+i;
		states[i].screenX = 160;
		states[i].screenY = 300+400*i;
		states[i].AN_linkNum = FIRST_AN_GROUP_NUMBER+i;


        fprintf(fp,"            society[$AN_%d]<%d,%d>|State%d| = $AN_%d,\n",
                   states[i].AN_Num,
                   states[i].screenX,
                   states[i].screenY,
                   i+1,
                   states[i].AN_linkNum);
	 }

   /* Add the stop State */
   fprintf(fp,"            society[$AN_%d]<160,%d>|State%d|     = $AN_502, \n\n",
                FIRST_AN_SOCIETY_NUMBER+numberOfStates,
                300+400*numberOfStates,
                numberOfStates+1 );



   /*== print out the rules (transitions)== */

   /* the start state is connected to the first group (GoTo) */
   fprintf(fp,START_RULE, FIRST_AN_SOCIETY_NUMBER ); 

   for(i=0; i< numberOfStates; i++)
	 {
	   fprintf(fp,", \n\n"); /* no ',' after the last rule */
         fprintf(fp,OTHER_RULE, 
                   states[i].AN_Num, 
                   states[i].screenX,
                   states[i].screenY,
                   i+1,
                   states[i].goalX,
                   states[i].goalY,
                   i+1,
                   states[i].AN_Num+1);
	 }

   fprintf(fp,"\n\n");
   fprintf(fp,BODY_END);

}









#define TRAILER \
   "instBP<0,0> $AN_800 from vehicle( \n" \
   "  bound_to = moveToRobot1:MRV2(   \n" \
   "moveToRobot1:[                    \n" \
   "          $AN_505]                \n" \
   ")<0,0>|The robot|                 \n" \
   ");                                \n" \
   "                                  \n" \
   "NoName:[                          \n" \
   "[                                 \n" \
   "    $AN_800]<0,0>|The Configuration| \n" \
   "]<10,10>                          \n"


void  dumpTrailer()
{
   fprintf(fp,TRAILER);
}





void generateCDLfile(float *x, float *y, int NPoints)
{

  int i;
  char filename[128];

  printf("\nEnter CDL filename: ");
  scanf("%s", filename);

  fp=fopen(filename, "w");
  if(fp == NULL)
	{
     fprintf(stderr, "Cantopen file %s for writing\n", filename);
     return;
	}

  dumpHeader(filename);
  dumpStartAndStop();

  for(i=0; i< NPoints; i++)
    dumpGoToGroup(x[i], y[i]);
  
  dumpBody();

  dumpTrailer();

}



/* This function outputs a text file with one point per line.
   In addition to the point x and y coordinates the five other 
   arguments are also printed on the line. 
   They are used by Cfgedit to set the parametes of the schemas.
   The first four are used by the GoTo state and the last one is used
   by the AtGoal trigger coming out of that state.

            x
            y
            move_to_location_gain
            avoid_obstacle_gain
            avoid_obstacle_sphere
            avoid_obstacle_safety_margin
            Goal_Tolerance

*/
void generateTextFile(float *x, float *y, int NPoints, char* filename)
{

  int i;

  fp=fopen(filename, "w");
  if(fp == NULL)
	{
     fprintf(stderr, "Cannot open file %s for writing\n", filename);
     return;
	}


  for(i=0; i< NPoints; i++)
	{
      fprintf(fp,"%g %g 1.0 0.45 0.40 0.23 0.5\n", x[i], y[i]);
	}  

  fclose(fp);
}


/*
int main()
{
  float x[]={10, 30, 40};
  float y[]={12, 17, 32};

  generateCDLfile(x, y, 3);
}
*/



///////////////////////////////////////////////////////////////////////
// $Log: generateCDL.c,v $
// Revision 1.1.1.1  2008/07/14 16:44:24  endo
// MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
//
// Revision 1.1.1.1  2006/07/20 17:17:51  endo
// MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
//
// Revision 1.1.1.1  2006/07/12 13:38:00  endo
// MissionLab 7.0
//
// Revision 1.2  2006/07/10 21:51:16  pulam
// Fixed pathplanner
//
// Revision 1.1.1.1  2005/02/06 23:00:23  endo
// AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
//
// Revision 1.2  2003/04/06 10:16:11  endo
// Checked in for Robert R. Burridge. Various bugs fixed.
//
// Revision 1.1  2000/03/22 04:40:00  saho
// Initial revision
//
///////////////////////////////////////////////////////////////////////

