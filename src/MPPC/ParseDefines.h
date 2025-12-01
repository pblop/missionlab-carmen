/*********************************************************************
 **                                                                  **
 **                         ParseDefines.h                           **
 **                                                                  **
 **                                                                  **
 **  Written by:  Alan Wagner                                        **
 **                                                                  **
 **  Copyright 2006 - 2007 Georgia Tech Research Corporation         **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **  Purpose: Provides parsing functionality                         **
 **                                                                  **
 **********************************************************************/

/* $Id: ParseDefines.h,v 1.1.1.1 2008/07/14 16:44:14 endo Exp $ */

#ifndef PARSEDEFINES_H
#define PARSEDEFINES_H

//#include "ProjectDefinitions.h"


#define MOVE_TO_NEXT_LINE position = s1.find('\n', position+1); position++;
#define FIND_END_OF_NUMBER (s1.find_first_of(' ',position) < s1.find_first_of('\n',position)) ? (endOfNumber= s1.find_first_of(' ',position)):(endOfNumber=s1.find_first_of('\n',position));
#define FIND_END_OF_LINE( position, endOfNumber) (s1.find_first_of('\n',position) < s1.find_first_of('\r',position)) ? (endOfNumber = s1.find_first_of('\n',position)):(endOfNumber = s1.find_first_of('\r',position));
#define MOVE_TO_NEXT_NON_COMMENT while(( (position = s1.find('\n', position+1)) != string::npos) && (s1.at(position+1) == '#' )){} position++;

#define MOVE_TO_END_OF_LINE( position ) (s1.find_first_of('\n',position) < s1.find_first_of('\r',position)) ? (position = s1.find_first_of('\n',position)):(position = s1.find_first_of('\r',position));
#define MOVE_TO_NEXT_AGENT_NAME_TAG( position ) if((position = s1.find("AGENT_NAME", position)) != string::npos)	{	position=position;	} else { position=0; }
#define MOVE_TO_NEXT_INSTANCE_START_TAG( position ) if((position = s1.find("INSTANCE START", position)) != string::npos)	{	position = position;	}	else{	position=0;	}


#define MOVE_TO_NEXT_INSTANCE_END_TAG( position ) if((position = s1.find("INSTANCE END", position)) != string::npos)	{	position = position+13;	}	else{	position=0;	}






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




#endif

/**********************************************************************
 * $Log: ParseDefines.h,v $
 * Revision 1.1.1.1  2008/07/14 16:44:14  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.2  2007/09/18 22:37:39  endo
 * Mission time windows in ICARUS objectives can be now updated based on the environment message.
 *
 **********************************************************************/
