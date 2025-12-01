#ifndef dynamicLoader_H
#define dynamicLoader_H
/**********************************************************************
 **                                                                  **
 **  dynamicLoader.h                                                 **
 **                                                                  **
 **  Manage dynamicLoadable classes                                  **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-2002.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Alto, Michigan, USA                                             **
 **                                                                  **
 **********************************************************************/

/* $Id: dynamicLoader.h,v 1.1.1.1 2008/07/14 16:44:16 endo Exp $ */

///////////////////////////////////////////////////////////////////////
// $Log: dynamicLoader.h,v $
// Revision 1.1.1.1  2008/07/14 16:44:16  endo
// MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
//
// Revision 1.1.1.1  2006/07/20 17:17:46  endo
// MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
//
// Revision 1.1.1.1  2006/07/12 13:37:55  endo
// MissionLab 7.0
//
// Revision 1.1  2006/07/01 00:30:29  endo
// CMDLi from MARS 2020 migrated into AO-FNC repository.
//
// Revision 1.1.1.1  2006/06/29 20:42:07  endo
// cmdli local repository.
//
// Revision 1.4  2004/05/11 19:34:29  doug
// massive changes to integrate with USC and GaTech
//
// Revision 1.3  2003/08/29 21:40:42  doug
// moved ccl procedures to Procedure instead of ProcedureBase
//
// Revision 1.2  2003/08/27 22:37:24  doug
// refactored dynamicLoader and dynamicLoadable
//
// Revision 1.1  2003/08/27 20:42:09  doug
// create a generic dynamic loader base class
//
///////////////////////////////////////////////////////////////////////

#include "mic.h"
#include "dynamicLoadable.h"

namespace sara
{
// *********************************************************************
// Base class for all runtime data objects
class dynamicLoader 
{
public:
   /// convience types
   typedef class dynamicLoadable *factory_t();
   typedef std::map<std::string, factory_t *, std::less<std::string> > entries_t;
   typedef std::map<std::string, entries_t, std::less<std::string> > category_t;

   /// singleton design pattern: Destructor
   ~dynamicLoader();

   // Dump the list of registered classes to stdout.
   static void print();

   // Load a dyanamically linked class
   // Returns true on success, false if error.
   static bool loadPrimitive(const std::string &fileName);

   // Register a class we have loaded
   static void registerFactory(const std::string &category, const std::string &classname, factory_t factory);

   // get a reference to the map of factories for this category
   static entries_t *find(const std::string &category);

   // get a pointer to the factory for this category/name pair
   // returns NULL if the class is not currently loaded
   static factory_t *find(const std::string &category, const std::string &name);

   // set verbose mode true or false
   static void setVerbose(const bool verboseValue);

   // Add library search paths for class files
   static void addPaths(const strings &paths);

private:
   /// singleton design pattern: Constructor
   dynamicLoader();

   /// singleton design pattern: get handle
   static dynamicLoader *getHandle();

   /// singleton design pattern: the object
   static dynamicLoader *theLoader;

   /// The list of registered class factories.
   /// index like:  classFactories[category][classname]
   category_t classFactories;

   /// List of handles for all dynamically loaded classes
   /// Used to unload them when exiting
   typedef std::vector<void *> handles_t;
   handles_t handles;

   /// The library paths to search for classes
   strings libdirs;

   /// Are we in verbose mode?
   bool verbose;

   /// convienence types
   typedef std::string getstring_t();
};

/*********************************************************************/
}
#endif
