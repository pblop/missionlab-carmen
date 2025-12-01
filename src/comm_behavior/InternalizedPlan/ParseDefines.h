/*********************************************************************
 **                                                                  **
 **                     ParseDefines.h                                **
 **                                                                  **
 **                                                                  **
 **  Written by:  Alan Wagner                                        **
 **                                                                  **
 **  Copyright 2002 - 2006 Georgia Tech Research Corporation         **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 ** Purpose: Provides parsing functionality                          **
 **                                                                  **
 **********************************************************************/

/* $Id: ParseDefines.h,v 1.1.1.1 2008/07/14 16:44:17 endo Exp $ */

#ifndef PARSEDEFINES_H
#define PARSEDEFINES_H

#include "ProjectDefinitions.h"

#define MOVE_TO_NEXT_LINE position = s1.find('\n', position+1); position++;
#define FIND_END_OF_NUMBER (s1.find_first_of(' ',position) < s1.find_first_of('\n',position)) ? (endOfNumber= s1.find_first_of(' ',position)):(endOfNumber=s1.find_first_of('\n',position));
#define FIND_END_OF_LINE (s1.find_first_of('\n',position) < s1.find_first_of('\r',position)) ? (endOfNumber = s1.find_first_of('\n',position)):(endOfNumber = s1.find_first_of('\r',position));
#define MOVE_TO_NEXT_NON_COMMENT while(( (position = s1.find('\n', position+1)) != string::npos) && (s1.at(position+1) == '#' )){} position++;
#define MOVE_TO_BEGINNING_OF_NEXT_NUMBER position=endOfNumber;position++;

#define MOVE_TO_MISSION_AREA_TAG if((position = s1.find("MISSION-AREA", 0)) != string::npos)	{	position = position+13;	}	else{	position=0;	}
#define MOVE_TO_MAPTYPE_TAG if((position = s1.find("--MAPTYPE", 0)) != string::npos)	{	position = position+10;	}	else{	position=0;	}
#define MOVE_TO_RESOLUTION_TAG if((position = s1.find("--RESOLUTION", 0)) != string::npos)	{	position = position+13;	}	else{	position=0;	}
#define MOVE_TO_GOALS_TAG if((position = s1.find("GOALS", 0)) != string::npos)	{	position = position+6;	}	else{	position=0;	}
#define MOVE_TO_OBSTACLE_MAP_TAG if((position = s1.find("OBSTACLE_MAP", 0)) != string::npos)	{	position = position+13;	}	else{	position=0;	}
#define MOVE_TO_COMMUNICATIONS_MAP_TAG if((position = s1.find("COMMUNICATION_MAP", 0)) != string::npos)	{	position = position+18;	}	else{	position=0;	}

#define MOVE_TO_BEGIN_MAP_TAG if((position = s1.find("BEGIN_MAP", 0)) != string::npos)	{	position = position+10;	}	else{	break;	}
#define MOVE_TO_END_MAP_TAG if((position = s1.find("END_MAP", 0)) != string::npos)	{	position = position+8;	}	else{	position=0;	}
#define MOVE_TO_BEGIN_PARALLEL_TAG if((position = s1.find("BEGIN_PARALLEL", 0)) != string::npos)	{	position = position+15;	}	else{	break;	}
#define MOVE_TO_END_PARALLEL_TAG if((position = s1.find("END_PARALLEL", 0)) != string::npos)	{	position = position+13;	}	else{	position=0;	}

#define MOVE_TO_BEGIN_PLAN_TAG if((position = s1.find("BEGIN_PLAN", 0)) != string::npos)	{	position = position+11;	}	else{	return EXIT_SUCCESS;	}
#define MOVE_TO_END_PLAN_TAG if((position = s1.find("END_PLAN", 0)) != string::npos)	{	position = position+9;	}	else{	position=0;	}


#define MOVE_TO_YOUR_MAP_MAP_TAG if((position = s1.find("YOUR_MAP", 0)) != string::npos)	{	position = position+9;	}	else{	position=0;	}

#define MOVE_TO_NUMBERED_FIELD FIND_END_OF_NUMBER position = endOfNumber; position++; FIND_END_OF_NUMBER 


const string Obstacle("OBSTACLE");
const string obstacle("Obstacle");
const string Wall("WALL");
const string wall("Wall");
const string exactPoint("EXACTPOINT");
const string newLine("\n");
const string pound("#");

#endif

/**********************************************************************
 * $Log: ParseDefines.h,v $
 * Revision 1.1.1.1  2008/07/14 16:44:17  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/20 17:17:47  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:37:55  endo
 * MissionLab 7.0
 *
 * Revision 1.1  2006/06/08 14:47:42  endo
 * CommBehavior from MARS 2020 migrated.
 *
 **********************************************************************/

