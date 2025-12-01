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

/* $Id: test1.cc,v 1.1.1.1 2008/07/14 16:44:17 endo Exp $ */

///////////////////////////////////////////////////////////////////////
// $Log: test1.cc,v $
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
// Revision 1.8  2004/05/11 19:34:33  doug
// massive changes to integrate with USC and GaTech
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
// Revision 1.1  1997/11/11 15:56:57  doug
// Initial revision
//
///////////////////////////////////////////////////////////////////////

#include "mic.h"
#include "mutex.h"
#include "condition.h"
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
mutex     guard;
condition changed;
bool      is_full;
int       buffer;

/**********************************************************************/
// producer class
//
class producer: public objthreads 
{
   public:
      producer(long howmany, int parm);

      void run(int);

   private:
      long  max;
};

/**********************************************************************/
producer::producer(long howmany, int parm) :
   objthreads(parm)
{
    max = howmany;
    is_full = false;
}

/**********************************************************************/
void
producer::run(int parm)
{
   if( parm != 2 )
   {
      cout << "Wrong parm passed in consumer\n";
      failed();
   }

    for(int i=0; i < max; i++) 
    {
       guard.lock();
       while( is_full )
       {
	  if( !changed.wait(guard, 5000) )
	  {
             cout << "Deadlock in producer\n";
             failed();
	  }
       }

       buffer = i;
       is_full = true;
       changed.signal();
       guard.unlock();
    }
}

/**********************************************************************/
// consumer class
//
class consumer : public objthreads 
{
   public:
      consumer(long howmany, int parm);

      void run(int);

   private:
      long  max;
};

/**********************************************************************/
consumer::consumer(long howmany, int parm) :
   objthreads(parm)
{
    max = howmany;
}

/**********************************************************************/
void 
consumer::run(int parm)
{
   if( parm != 42 )
   {
      cout << "Wrong parm passed in consumer\n";
      failed();
   }

    for(int i=0; i < max; i++) 
    {
       guard.lock();
       while( !is_full )
       {
	  if( !changed.wait(guard, 5000) )
	  {
             cout << "Deadlock in consumer\n";
             failed();
	  }
       }

       if( buffer != i )
       {
          cout << "Incorrect data in consumer: Wanted " << i 
	       << " got " << buffer << '\n';;
          failed();
       }
       is_full = false;
       changed.signal();
       guard.unlock();
    }
}

/**********************************************************************/

int 
main(int, char *[])
{
#ifdef USE_EXCEPTIONS
   try
#endif
   {

      // start consumer
      consumer *con = new consumer(1000, 42);
      con->start_thread();

      // start producer
      producer *prod = new producer(1000, 2);
      prod->start_thread();

      if( !prod->wait_for_completion(5000) )
      {
         cout << "Producer never completed\n";
         failed();
      }
      if( !con->wait_for_completion(5000) )
      {
         cout << "Consumer never completed\n";
         failed();
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
      cout << "obthreads test threw an unknown mic_exception\n";
      failed();
   }
#endif

   cout << "SUCESSFUL objthreads test\n";
   return 0;
}

}
