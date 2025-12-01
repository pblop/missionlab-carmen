/**********************************************************************
 **                                                                  **
 **  test the barrier                                                **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1997, Mobile Intelligence Corporation                 **
 **  7963 Pine Hollow Drive, SE                                      **
 **  Alto, Michigan  49302-9724                                      **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: test_barrier.cc,v 1.1.1.1 2008/07/14 16:44:17 endo Exp $ */

///////////////////////////////////////////////////////////////////////
// $Log: test_barrier.cc,v $
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
// Revision 1.7  2004/05/11 19:34:33  doug
// massive changes to integrate with USC and GaTech
//
// Revision 1.6  1998/02/08 13:50:39  doug
// got win32 version to run test
//
// Revision 1.5  1998/02/06 20:18:13  doug
// made exceptions optional
//
// Revision 1.4  1997/12/10 11:56:51  doug
// *** empty log message ***
//
// Revision 1.3  1997/12/09 15:59:18  doug
// *** empty log message ***
//
// Revision 1.2  1997/11/19 13:08:24  doug
// rtti is back in
//
// Revision 1.1  1997/11/13 11:37:31  doug
// Initial revision
//
///////////////////////////////////////////////////////////////////////

#include "mic.h"
#include "objthreads.h"
#include "barrier.h"

namespace sara 
{
// Exit codes:
//   0   : Successful test
//   <>0 : Test failed

/**********************************************************************/
void failed()
{
   cout << "FAILED barrier test\n";
   exit(1);
}

/**********************************************************************/

// Single barrier
barrier  bar;
int      loop = 0;
int      Max = 97;

/**********************************************************************/
// A thread class
//
class thread: public objthreads 
{
   public:
      thread() {};

      void run(int);

   private:
};

/**********************************************************************/
void
thread::run(int)
{
    for(int i=0; i < Max; i++) 
    {
       if( i != loop )
       {
          cout << "Bad loop count: Wanted " << i << " got " << loop << '\n';
          failed();
       }
       bar.enter();
    }
}

/**********************************************************************/
// callback function
//
void
at_bar(void)
{
   loop ++;
};

/**********************************************************************/

int 
main(int, char *[])
{
   const int NT = 47;
   thread *p[NT];
#ifdef USE_EXCEPTIONS
   try
#endif
   {
      // Create the threads
      {
      for(int i=0; i<NT; i++)
         p[i] = new thread();
      }

      // Add them to the barrier
      {
      for(int i=0; i<NT; i++)
         bar.add_member();
      }

      // Add the callback function
      bar.set_barrier_callback(at_bar);

      // Start them running
      {
      for(int i=0; i<NT; i++)
         p[i]->start_thread();
      }

      // Wait for them to finish
      for(int i=0; i<NT; i++)
      {
         if( !p[i]->wait_for_completion(5000) )
         {
            cout << "thread never completed\n";
            failed();
         }
      }
   }

#ifdef USE_EXCEPTIONS
   catch (const mic_exception &e)
   {
      cout << "barrier test threw an mic_exception: " << e << '\n';
      failed();
   }

   catch (...)
   {
      cout << "barrier test threw an unknown mic_exception\n";
      failed();
   }
#endif

   cout << "SUCESSFUL barrier test\n";
   return 0;
}

}
