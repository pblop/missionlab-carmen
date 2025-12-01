#ifndef EXTRA_command_H
#define EXTRA_command_H
/**********************************************************************
 **                                                                  **
 **  extra_command.h                                                 **
 **                                                                  **
 **  Keeps track of information for a specific command               **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-1998.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Alto, Michigan, USA                                             **
 **                                                                  **
 **********************************************************************/

/* $Id: extra_command.h,v 1.1.1.1 2008/07/14 16:44:16 endo Exp $ */

/**********************************************************************
* $Log: extra_command.h,v $
* Revision 1.1.1.1  2008/07/14 16:44:16  endo
* MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
*
* Revision 1.1.1.1  2006/07/20 17:17:45  endo
* MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
*
* Revision 1.1.1.1  2006/07/12 13:37:54  endo
* MissionLab 7.0
*
* Revision 1.1  2006/07/01 00:14:58  endo
* CMDLi from MARS 2020 migrated into AO-FNC repository.
*
* Revision 1.1.1.1  2006/06/29 20:42:07  endo
* cmdli local repository.
*
* Revision 1.2  2004/05/11 19:34:50  doug
* massive changes to integrate with USC and GaTech
*
* Revision 1.1.1.1  2003/11/07 20:27:33  doug
* initial import
*
**********************************************************************/

#include "Command.h"
#include "Command_extra.h"

/*********************************************************************/
namespace sara
{

class extra_command : Command_extra
{
   /*********************************************************************/
   public:
      /// Functional constructor
      static Command_extra *create(Command *p);

      /// Constructor
      extra_command(Command *p);

      /// Destructor
      ~extra_command() {};

      /// Load the cache
      /// returns true on success.
      void load_cache(const int action_index, const uints &triggers_required);

      /// Get action index
      uint getAction() const;

      /// Get list of required triggers (vector of indicies)
      const uints &getTriggersRequired() const;

   /******************************************************************/
   private:
      bool   cache_valid;
      int    cached_action_index;       // Index of the associated action.
      uints  cached_triggers_required;  // vector of indicies for triggers used
      Command *main;                    // Pointer to the main data record
};

/*********************************************************************/
}
#endif
