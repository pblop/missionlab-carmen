/**********************************************************************
 **                                                                  **
 **  test the objthreads                                             **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1997, Mobile Intelligence Corporation                 **
 **  7963 Pine Hollow Drive, SE                                      **
 **  Alto, Michigan  49302-9724                                      **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: test2.cc,v 1.1.1.1 2008/07/14 16:44:17 endo Exp $ */

///////////////////////////////////////////////////////////////////////
// $Log: test2.cc,v $
// Revision 1.1.1.1  2008/07/14 16:44:17  endo
// MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
//
// Revision 1.1.1.1  2006/07/20 17:17:46  endo
// MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
//
// Revision 1.1.1.1  2006/07/12 13:37:55  endo
// MissionLab 7.0
//
// Revision 1.1  2006/07/01 00:33:51  endo
// CMDLi from MARS 2020 migrated into AO-FNC repository.
//
// Revision 1.1.1.1  2006/06/29 20:42:07  endo
// cmdli local repository.
//
// Revision 1.9  2004/05/11 19:34:33  doug
// massive changes to integrate with USC and GaTech
//
// Revision 1.8  1998/02/08 14:13:35  doug
// linux can only handle ~255 threads
//
// Revision 1.7  1998/02/08 14:03:41  doug
// got win32 to test
//
// Revision 1.6  1998/02/06 20:16:25  doug
// made exceptions optional
//
// Revision 1.5  1997/12/10 11:56:56  doug
// *** empty log message ***
//
// Revision 1.4  1997/12/09 15:59:10  doug
// *** empty log message ***
//
// Revision 1.3  1997/11/19 13:08:16  doug
// rtti is back in
//
// Revision 1.2  1997/11/18 19:07:07  doug
// giving up on rtti
//
// Revision 1.1  1997/11/15 19:38:32  doug
// Initial revision
//
// Revision 1.1  1997/11/11 15:56:57  doug
// Initial revision
//
///////////////////////////////////////////////////////////////////////

#include "mic.h"
#include <stdio.h>
#include <stdlib.h>
#include "barrier.h"
#include "objthreads.h"

namespace sara 
{
// Exit codes:
//   0   : Successful test
//   <>0 : Test failed

/**********************************************************************/
void failed()
{
   cout << "FAILED objthreads test\n";
   exit(1);
}

/**********************************************************************/

// Single buffered com
barrier   bar;

/**********************************************************************/
// a threaded class
//
class threaded: public objthreads 
{
   public:
      threaded(int max);
      ~threaded();
      void run(int);

   private:
      long  max;
};

/**********************************************************************/
threaded::threaded(int howmany):
   max(howmany)
{
   bar.add_member();
}

/**********************************************************************/
threaded::~threaded()
{
   bar.delete_member();
}

/**********************************************************************/
void
threaded::run(int)
{
    for(int i=0; i < max; i++) 
    {
cerr << '.';
       bar.enter();
    }
}

/**********************************************************************/

int 
main(int, char *[])
{
   // Register a handler for new running out of memory
//   set_new_handler(abort);

   const int numthreads = 200;
   threaded *p[numthreads];

   int i;
#ifdef USE_EXCEPTIONS
   try
#endif
   {
      cerr << "Beginning to start the threads\n";
      for(i=0; i<numthreads; i++)
      {
	 p[i] = new threaded(13);
      }
   }
#ifdef USE_EXCEPTIONS
   catch (const mic_exception &e)
   {
      cout << "objthreads test threw an mic_exception after creating " << i 
	   << " threads: " << e << '\n';
      failed();
   }

   catch (...)
   {
      cout << "obthreads test threw an exception which didn't inherit from mic_exception after creating " << i << " threads!\n";
      failed();
   }
#endif

      cerr << "Created " << numthreads << " threads\n";
#ifdef USE_EXCEPTIONS
   try
#endif
   {

      for(i=0; i<numthreads; i++)
	 p[i]->start_thread();

      cerr << "Started the threads running\n";

      for(i=0; i<numthreads; i++)
      {
         if( !p[i]->wait_for_completion(30000) )
         {
            cout << "thread " << i << " never completed\n";
            failed();
         }

	 delete p[i];
      }
   }
#ifdef USE_EXCEPTIONS
   catch (const mic_exception &e)
   {
      cout << "objthreads test threw an mic_exception: " << e << '\n';
      failed();
   }

   catch (...)
   {
      cout << "obthreads test threw an exception which didn't inherit from mic_exception!\n";
      failed();
   }
#endif

   cout << "SUCESSFUL objthreads test\n";
   return 0;
}

}
