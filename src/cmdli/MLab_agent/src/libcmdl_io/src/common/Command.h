#ifndef Command_H
#define Command_H
/**********************************************************************
 **                                                                  **
 **  Command.h                                                       **
 **                                                                  **
 **  Abstact base class for classes implementing a type of command   **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-2002.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Alto, Michigan, USA                                             **
 **                                                                  **
 **********************************************************************/

/* $Id: Command.h,v 1.1.1.1 2008/07/14 16:44:15 endo Exp $ */

///////////////////////////////////////////////////////////////////////
// $Log: Command.h,v $
// Revision 1.1.1.1  2008/07/14 16:44:15  endo
// MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
//
// Revision 1.1.1.1  2006/07/20 17:17:45  endo
// MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
//
// Revision 1.1.1.1  2006/07/12 13:37:54  endo
// MissionLab 7.0
//
// Revision 1.1  2006/07/01 00:19:14  endo
// CMDLi from MARS 2020 migrated into AO-FNC repository.
//
// Revision 1.1.1.1  2006/06/29 20:42:07  endo
// cmdli local repository.
//
// Revision 1.14  2004/10/29 22:38:36  doug
// working on waittimeout
//
// Revision 1.13  2004/10/22 21:40:29  doug
// added goto and command exception support
//
// Revision 1.12  2004/05/11 19:34:49  doug
// massive changes to integrate with USC and GaTech
//
// Revision 1.11  2004/03/17 21:53:33  doug
// working on subroutine calls
//
// Revision 1.10  2004/03/15 21:41:06  doug
// coded ActionCommand and Call
//
// Revision 1.9  2004/03/12 20:36:58  doug
// snap
//
// Revision 1.8  2004/03/09 21:36:08  doug
// promote Blocks to mission segments and procedures
//
// Revision 1.7  2004/03/01 23:37:51  doug
// working on real overlay
//
// Revision 1.6  2004/02/20 14:39:35  doug
// cmdl interpreter runs a bit
//
// Revision 1.5  2003/11/13 22:29:41  doug
// adding transitions
//
// Revision 1.4  2003/11/10 22:48:30  doug
// working on cmdli
//
// Revision 1.3  2003/11/07 22:54:51  doug
// working on cmdl_io
//
// Revision 1.2  2003/11/07 20:29:27  doug
// added cmdli interpreter library
//
// Revision 1.1  2003/11/03 16:45:16  doug
// snapshot
//
///////////////////////////////////////////////////////////////////////

#include "mic.h"
#include "resources.h"

// ********************************************************************
namespace sara 
{

class Command 
{
   public:
      /// Constructors
      /// the object takes over ownership of the options object
      /// The label should be the empty string "" if not labeled
      Command(const string &label, class Unit *unit);

      /// Destructor
      virtual ~Command();

      // maintain the label on this Command.
      bool is_labeled() {return isLabeled;}
      void set_label(const string &theLabel) {isLabeled = true; label = theLabel;}
      const string &get_label() const {return label;}

      // Maintain the unit specified for this command
      class Unit *getUnit() {return unit;}

      /// Get the list of robots mentioned in this mission
      const stringSet &getRobotNames() const {return names;}

      /// Get a reference to the set of command-specific options
      /// NOTE: The caller must not delete the returned object!
      resources *getOptions() {return &options;}

      /// does the specified name exist in the list of robots?
      bool robotNameExists(const string &name) const {return names.find(name) != names.end();}

      /// return the number of robots mentioned
      uint numRobotNames() const {return names.size();}

      /// Stream out in a form suitable for the loader
      virtual string print() = 0;

   private:
      bool isLabeled;
      string label;

      // The unit specified for this command
      class Unit *unit;

      /// The list of robots mentioned in the block
      stringSet names;

      /// Any command-specific execution options
      resources options;
};

/***************************************************************************/
}
#endif
