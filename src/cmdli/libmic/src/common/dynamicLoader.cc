/**********************************************************************
 **                                                                  **
 **  dynamicLoader.cc                                                 **
 **                                                                  **
 **  base file which users subclass for Data types.                  **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-2002.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Alto, Michigan, USA                                             **
 **                                                                  **
 **********************************************************************/

/* $Id: dynamicLoader.cc,v 1.1.1.1 2008/07/14 16:44:16 endo Exp $ */

///////////////////////////////////////////////////////////////////////
// $Log: dynamicLoader.cc,v $
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
// Revision 1.8  2004/09/23 21:52:03  doug
// getting robots to run again
//
// Revision 1.7  2004/05/11 19:34:29  doug
// massive changes to integrate with USC and GaTech
//
// Revision 1.6  2003/11/26 15:21:34  doug
// back from GaTech
//
// Revision 1.5  2003/09/04 16:50:12  doug
// redid the ccl primitive loader sections
//
// Revision 1.4  2003/09/03 20:47:10  doug
// revised to match new loader usage
//
// Revision 1.3  2003/09/03 17:16:25  doug
// fixing loader problems
//
// Revision 1.2  2003/08/27 22:37:24  doug
// refactored dynamicLoader and dynamicLoadable
//
// Revision 1.1  2003/08/27 20:42:09  doug
// create a generic dynamic loader base class
//
// Revision 1.7  2003/07/09 22:19:19  doug
// The robot now exits cleanly
//
// Revision 1.6  2003/07/08 21:09:34  doug
// Cleanup error messages
//
// Revision 1.5  2002/10/02 14:25:24  doug
// *** empty log message ***
//
// Revision 1.4  2002/06/25 19:56:55  doug
// fix so removes dlls
//
// Revision 1.3  2002/06/13 15:12:28  doug
// Works
//
// Revision 1.2  2002/05/08 20:28:19  doug
// *** empty log message ***
//
// Revision 1.1  2002/05/03 18:43:31  doug
// Initial revision
//
// Revision 1.1  2002/04/29 22:11:52  doug
// Initial revision
//
//
///////////////////////////////////////////////////////////////////////

#include "mic.h"
#include "dynamicLoader.h"
#include "dlfcn.h"

using namespace std;
namespace sara
{
// **********************************************************************
/// singleton design pattern: the object
dynamicLoader *dynamicLoader::theLoader = NULL;

/*********************************************************************/
// Constructor
dynamicLoader::dynamicLoader() :
   verbose(false)
{
}

/*********************************************************************/
// destructor: unload all loaded classes
dynamicLoader::~dynamicLoader()
{
   // need to manually unload the classes we loaded
   handles_t::iterator it;
   for(it=handles.begin(); it!=handles.end(); ++it)
   {
      dlclose(*it);
   }
   handles.erase(handles.begin(), handles.end());

   if( verbose )
   {
      cerr << "Unloaded dynamic libraries" << endl;
   }
}

/*********************************************************************/
/// Get handle
dynamicLoader *
dynamicLoader::getHandle()
{
   if( !theLoader )
   {
      theLoader = new dynamicLoader();
   }
   return theLoader;
}

/*********************************************************************/
void 
dynamicLoader::print()
{
   cout << "Registered Data Types:" << endl;

   dynamicLoader *hdl = getHandle();
   category_t::const_iterator cit;
   for(cit=hdl->classFactories.begin(); cit!=hdl->classFactories.end(); cit++) 
   {
      cout << "Category '" << cit->first << "':" << endl;

      entries_t::const_iterator eit;
      for(eit=cit->second.begin(); eit!=cit->second.end(); eit++) 
      {
         cout << "   '" << eit->first << "'" << endl;
      }
   }
   cout << endl;
}

/*********************************************************************/
// Load one dyanamically linked datatype
// Returns true on success, false if error.
bool 
dynamicLoader::loadPrimitive(const string &name)
{
   dynamicLoader *hdl = getHandle();

   string fileName = name + ".so";
   string fullname;
   void *dynLib = NULL;
   strings::const_iterator it;
   for(it=hdl->libdirs.begin(); it!=hdl->libdirs.end(); ++it)
   {
      fullname = (*it) + "/" + fileName;
      if( (dynLib = dlopen(fullname.c_str(), RTLD_NOW | RTLD_GLOBAL )) )
      {
         if( hdl->verbose )
            cerr << "Loading " << fullname << endl;

	 // Found it.
         break;
      }
   }

   // If we didn't find it, try just the name they gave us.
   if( !dynLib )
   {
      fullname = fileName;
      dynLib = dlopen(fullname.c_str(), RTLD_NOW | RTLD_GLOBAL );
      if( hdl->verbose && dynLib )
         cerr << "Loading " << fullname << endl;
   }

   // No luck.
   if( dynLib == NULL )
   {
      cerr << dlerror() << endl << "Tried: '" << fileName << "'" << endl;
      for(it=hdl->libdirs.begin(); it!=hdl->libdirs.end(); ++it)
      {
         string name = (*it) + "/" + fileName;
	 cerr << "       '" << name << "'" << endl;
      }

      return false;
   }

   // remember the handle so we can close it
   hdl->handles.push_back( dynLib );

   return true;
}

// *********************************************************************
// Register a data type 
void
dynamicLoader::registerFactory(const string &category, const string &classname, factory_t factory)
{
   dynamicLoader *hdl = getHandle();

   // add it to our list
   (hdl->classFactories[category])[classname] = factory;

   if( hdl->verbose )
   {
      cerr << "dynamicLoader::registerFactory - Registering name='" << classname << "' in category='" << category << "'" << endl;
   }
}

// *********************************************************************
// get a reference to the map of factories for this category
dynamicLoader::entries_t *
dynamicLoader::find(const string &category)
{
   dynamicLoader *hdl = getHandle();

   category_t::iterator it = hdl->classFactories.find(category);
   if( it == hdl->classFactories.end() )
      return NULL;

   return &it->second;
}

// *********************************************************************
// get a pointer to the factory for this category/name pair
dynamicLoader::factory_t *
dynamicLoader::find(const string &category, const string &name)
{
   dynamicLoader *hdl = getHandle();

   category_t::iterator cit = hdl->classFactories.find(category);
   if( cit == hdl->classFactories.end() )
   {
      if( hdl->verbose )
      {
         cerr << "dynamicLoader::find - Did not find category='" << category << "'" << endl;
      }
      return NULL;
   }

   entries_t::iterator eit = cit->second.find( name );
   if( eit == cit->second.end() )
   {
      if( hdl->verbose )
      {
         cerr << "dynamicLoader::find - Did not find name='" << name << "' in category='" << category << "'" << endl;
      }
      return NULL;
   }

   return eit->second;
}

// *********************************************************************
// Add library search paths for class files
void 
dynamicLoader::addPaths(const strings &paths)
{
   dynamicLoader *hdl = getHandle();

   strings::const_iterator it;
   for(it = paths.begin(); it != paths.end(); ++it)
   {
      hdl->libdirs.push_back( *it );

      if( hdl->verbose )
         cerr << "Adding dynamic library search path " << *it << endl;
   }
}

// *********************************************************************
// set verbose mode true or false
void 
dynamicLoader::setVerbose(const bool verboseValue)
{
   dynamicLoader *hdl = getHandle();

   hdl->verbose = verboseValue;
}

// *********************************************************************
}
