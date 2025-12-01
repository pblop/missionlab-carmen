/**********************************************************************
 **                                                                  **
 **  actionable.cc                                                   **
 **                                                                  **
 **  A baseclass used to flag an environmental object as the target  **
 **  of an action, and to maintain status information on the action. **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-2003.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Livonia, Michigan, USA                                          **
 **********************************************************************/

/* $Id: actionable.cc,v 1.1.1.1 2008/07/14 16:44:16 endo Exp $ */

/**********************************************************************
* $Log: actionable.cc,v $
* Revision 1.1.1.1  2008/07/14 16:44:16  endo
* MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
*
* Revision 1.1.1.1  2006/07/20 17:17:46  endo
* MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
*
* Revision 1.1.1.1  2006/07/12 13:37:54  endo
* MissionLab 7.0
*
* Revision 1.1  2006/07/01 00:21:03  endo
* CMDLi from MARS 2020 migrated into AO-FNC repository.
*
* Revision 1.1.1.1  2006/06/29 20:42:07  endo
* cmdli local repository.
*
* Revision 1.4  2004/08/13 17:32:12  doug
* add streamout capability
*
* Revision 1.3  2004/08/13 17:31:02  doug
* add streamout capability
*
* Revision 1.2  2004/05/11 19:34:39  doug
* massive changes to integrate with USC and GaTech
*
* Revision 1.1  2003/03/25 21:45:13  doug
* moved libccl_code to the sara namespace
*
**********************************************************************/

#include "mic.h"
#include "actionable.h"

namespace sara
{

// **********************************************************************
// Constructor
actionable::actionable() :
   status(UNASSIGNED)
{
}

// **********************************************************************
// destructor
actionable::~actionable()
{
}

// *******************************************************************
// Stream out the value
ostream & operator << (ostream & out, const actionable &obj)
{
   switch (obj.status)
   {
      case actionable::UNASSIGNED:
	 out << "UNASSIGNED";
	 break;
      case actionable::ASSIGNED:
	 out << "ASSIGNED";
	 break;
      case actionable::ACTIVE:
	 out << "ACTIVE";
	 break;
      case actionable::PERMENENT_FAILURE:
	 out << "PERMENENT_FAILURE";
	 break;
      case actionable::TEMPORARY_FAILURE:
	 out << "TEMPORARY_FAILURE";
	 break;
      case actionable::COMPLETED:
	 out << "COMPLETED";
	 break;
   }
   return out;
}

// **********************************************************************
}
