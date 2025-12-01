/**********************************************************************
 **                                                                  **
 **  cmdl_transition.cc                                              **
 **                                                                  **
 **  Keeps track of information for a specific transition            **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-1998.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Alto, Michigan, USA                                             **
 **                                                                  **
 **********************************************************************/

/* $Id: cmdl_transition.cc,v 1.1.1.1 2008/07/14 16:44:15 endo Exp $ */

/**********************************************************************
* $Log: cmdl_transition.cc,v $
* Revision 1.1.1.1  2008/07/14 16:44:15  endo
* MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
*
* Revision 1.1.1.1  2006/07/20 17:17:45  endo
* MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
*
* Revision 1.1.1.1  2006/07/12 13:37:54  endo
* MissionLab 7.0
*
* Revision 1.1  2006/07/01 00:19:15  endo
* CMDLi from MARS 2020 migrated into AO-FNC repository.
*
* Revision 1.1.1.1  2006/06/29 20:42:07  endo
* cmdli local repository.
*
* Revision 1.5  2004/10/25 22:58:16  doug
* working on mars2020 integrated demo
*
* Revision 1.4  2004/10/22 21:40:29  doug
* added goto and command exception support
*
* Revision 1.3  2004/05/11 19:34:49  doug
* massive changes to integrate with USC and GaTech
*
* Revision 1.2  2004/03/19 22:48:21  doug
* working on exceptions.
*
* Revision 1.1  2003/11/13 22:29:41  doug
* adding transitions
*
**********************************************************************/

#include "mic.h"
#include "cmdl_transition.h"

namespace sara 
{
// *********************************************************************
// Constructor
cmdl_transition::cmdl_transition(bool isCall,
      				 const string &theTriggerName, 
				 keyValueMap *theOptions, 
		                 const string &theTargetName) :
	triggerName(theTriggerName),
        options(theOptions),
	targetName(theTargetName),
	isSubroutineCall(isCall)
{ /* empty */ }

// *********************************************************************
// Destructor
cmdl_transition::~cmdl_transition()
{
}

// *********************************************************************
}
