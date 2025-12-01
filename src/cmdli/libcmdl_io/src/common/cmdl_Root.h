#ifndef cmdl_Root_H
#define cmdl_Root_H
/**********************************************************************
 **                                                                  **
 **  cmdl_Root.h                                                     **
 **                                                                  **
 **  Keep track of information that doesn't change                   **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-2002.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Alto, Michigan, USA                                             **
 **                                                                  **
 **********************************************************************/

/* $Id: cmdl_Root.h,v 1.1.1.1 2008/07/14 16:44:15 endo Exp $ */

///////////////////////////////////////////////////////////////////////
// $Log: cmdl_Root.h,v $
// Revision 1.1.1.1  2008/07/14 16:44:15  endo
// MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
//
// Revision 1.1.1.1  2006/07/20 17:17:45  endo
// MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
//
// Revision 1.1.1.1  2006/07/12 13:37:54  endo
// MissionLab 7.0
//
// Revision 1.2  2006/07/11 10:30:45  endo
// Compiling error fixed.
//
// Revision 1.1  2006/07/01 00:19:15  endo
// CMDLi from MARS 2020 migrated into AO-FNC repository.
//
// Revision 1.1.1.1  2006/06/29 20:42:07  endo
// cmdli local repository.
//
// Revision 1.9  2004/11/04 21:36:39  doug
// moved test.cc to util
//
// Revision 1.8  2004/10/29 22:38:36  doug
// working on waittimeout
//
// Revision 1.7  2004/05/11 19:34:49  doug
// massive changes to integrate with USC and GaTech
//
// Revision 1.6  2004/02/20 14:39:35  doug
// cmdl interpreter runs a bit
//
// Revision 1.5  2003/11/10 22:48:30  doug
// working on cmdli
//
// Revision 1.4  2003/11/07 22:54:51  doug
// working on cmdl_io
//
// Revision 1.3  2003/11/07 20:29:27  doug
// added cmdli interpreter library
//
// Revision 1.2  2003/11/03 16:43:31  doug
// snapshot
//
// Revision 1.1  2003/10/24 21:03:39  doug
// finally compiles
//
///////////////////////////////////////////////////////////////////////

#include "mic.h"
#include "environment.h"
#include "TimeOfDay.h"
#include "resources.h"

namespace sara 
{
/***************************************************************************/
    class cmdl_Root 
    {
    public:
        /// Constructor
        cmdl_Root();

        /// Destructor
        ~cmdl_Root();

        // Stream out in a form suitable for the loader
        string print() const;

        // ***** A RobotType is a definition of a type of physical robot.
        // Has the robot type been defined?
        bool robotTypeExists(const string &name) const;

        // Define the robot type
        // takes over ownership of robot_rec
        // returns true on success, false if failed because already exists
        bool defineRobotType(class RobotType_Info *robot_rec);

        // look up the robot type record and return a pointer to it.
        // returns a pointer to the robot record (the user must not delete it)
        //         returns NULL if the robot is not defined
        class RobotType_Info *getRobotType(const string &name) const;

        // ***** access the symbolic environmental objects 

        // Get a pointer to the current environment
        class environment *currentEnvironment() { return theEnvironment;}

        // set the current environment
        void setEnvironment(class environment *env) { theEnvironment = env;}

        /// Get a reference to the named object or return NULL if not found
        /// The user must not delete the returned pointer
        class env_object *getObject(const string &name) const;

        // ***** A unit is a group of real robots (instances of a RobotType) and/or units that can be tasked.
        // Has the unit name been defined?
        bool unitExists(const string &name) const;

        // Define the unit 
        // takes over ownership of rec
        // returns true on success, false if failed because already exists
        bool defineUnit(class Unit *rec);

        // look up the unit record and return a pointer to it.
        // returns a pointer to the robot record (the user must not delete it)
        //         returns NULL if the robot is not defined
        class Unit *getUnit(const string &name) const;

        // ************ misc ************
        // set the name of this robot
        void set_ourName(const string &name) {ourName = name;}
        string get_ourName() const {return ourName;}

        /// Get the list of robots mentioned in this mission
        stringSet getRobotNames() const;

        /// set a default syncronization timeout
        void setDefaultSyncTimeout(const TimeOfDay &timeout); 

        /// is there a default syncronization timeout?
        bool isDefaultSyncTimeout() const {return defaultSyncTimeoutSpecified;}

        /// what is the default syncronization timeout?
        TimeOfDay getDefaultSyncTimeout() const {return defaultSyncTimeout;}

        /// Get a reference to the set of default options
        /// NOTE: The caller must not delete the returned object!
        resources *getOptions() {return &options;}

    private:
        friend class cmdl;

        // the simulated environment
        environment *theEnvironment;

        // the name of this robot
        string ourName;

        // The list of robots types that have been defined
        typedef std::map<std::string, class RobotType_Info *> Robots_T;
        Robots_T robots;

        // The list of units that have been defined
        typedef std::map<std::string, class Unit *> Units_T;
        Units_T units;

        /// The default synronization timeout value
        bool defaultSyncTimeoutSpecified;
        TimeOfDay defaultSyncTimeout;

        /// Any default execution options
        resources options;
    };

/***************************************************************************/
}
#endif
