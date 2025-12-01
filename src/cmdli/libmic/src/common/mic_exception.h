#ifndef MIC_EXCEPTION_H
#define MIC_EXCEPTION_H
/**********************************************************************
 **                                                                  **
 **  mic_exception.h                                                 **
 **                                                                  **
 **  The mic_exception objects                                       **
 **                                                                  **
 **  Object are:                                                     **
 **     system_error                                                 **
 **     no_resources                                                 **
 **     invalid_operation                                            **
 **     busy                                                         **
 **     already                                                      **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-1998.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Alto, Michigan, USA                                             **
 **                                                                  **
 **********************************************************************/

/* $Id: mic_exception.h,v 1.1.1.1 2008/07/14 16:44:16 endo Exp $ */

/**********************************************************************
* $Log: mic_exception.h,v $
* Revision 1.1.1.1  2008/07/14 16:44:16  endo
* MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
*
* Revision 1.1.1.1  2006/07/20 17:17:46  endo
* MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
*
* Revision 1.1.1.1  2006/07/12 13:37:55  endo
* MissionLab 7.0
*
* Revision 1.1  2006/07/01 00:30:29  endo
* CMDLi from MARS 2020 migrated into AO-FNC repository.
*
* Revision 1.1.1.1  2006/06/29 20:42:07  endo
* cmdli local repository.
*
* Revision 1.7  2004/05/11 19:34:29  doug
* massive changes to integrate with USC and GaTech
*
* Revision 1.6  2002/10/23 20:00:53  doug
* remove deprecated warnings
*
* Revision 1.5  1997/12/30 18:02:09  doug
* *** empty log message ***
*
* Revision 1.4  1997/12/30 17:58:23  doug
* docified
*
* Revision 1.3  1997/12/30 17:39:26  doug
* docifying
*
* Revision 1.2  1997/11/10 12:51:56  doug
* renamed from exception to mic_exception to eliminate name conflicts
*
* Revision 1.1  1997/11/07 17:13:59  doug
* Initial revision
*
* Revision 1.1  1997/11/06 22:32:39  doug
* Initial revision
*
**********************************************************************/


#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>


namespace sara
{
class mic_exception 
{
public:
   friend ostream& operator<< (ostream& ostr, const mic_exception& e)
   {
      e.print(ostr);
      return ostr;
   }

   virtual ~mic_exception() {};

protected:
   virtual void print(ostream& ostr) const
   {
      ostr << "Unexpected throw of base mic_exception class!\n";
   }
};

/*********************************************************************/

/// Class to be thrown when system errors occur
class system_error: public mic_exception
{
   ///
   char *usrmsg;
   ///
   int   error_number;
   
public:
   /// msg is a user message and errno is used to get the system error.
   system_error(const char *msg, int errno)
   {
      usrmsg = strdup(msg);
      error_number = errno;
   }

   ///
   ~system_error()
   {
      if( usrmsg )
      {
         free(usrmsg);
         usrmsg = NULL;
      }  
   }
   
private:  
   void print(ostream& ostr) const
   {
      ostr << usrmsg << "\nError number: " << error_number << "\n"; 
   }
};

/*********************************************************************/

/// Class to be thrown when there are insuffient resources to complete a request
class no_resources: public mic_exception
{
   ///
   char *usrmsg;
   ///
   int   error_number;
   ///
   bool  use_error;
   
public:
   /// msg is a user message 
   no_resources(const char *msg)
   {
      usrmsg = strdup(msg);
      use_error = false;
   }

   /// msg is a user message and errno is used to get the system error.
   no_resources(const char *msg, int errno)
   {
      usrmsg = strdup(msg);
      error_number = errno;
      use_error = true;
   }

   ///
   ~no_resources()
   {
      if( usrmsg )
      {
         free(usrmsg);
         usrmsg = NULL;
      }  
   }
   
private:  
   ///
   void print(ostream& ostr) const
   {
      ostr << "Error: Out of resources " << usrmsg << "\n"; 
      if( use_error )
      {
	 ostr << "Error number: " << error_number << "\n";
      }
   }
};


/*********************************************************************/

/// Class to be thrown when an invalid operation is attempted
class invalid_operation: public mic_exception
{
   ///
   char *usrmsg;
   
public:
   /// msg is a user message 
   invalid_operation(const char *msg)
   {
      usrmsg = strdup(msg);
   }

   ///
   ~invalid_operation()
   {
      if( usrmsg )
      {
         free(usrmsg);
         usrmsg = NULL;
      }  
   }
   
private:  
   ///
   void print(ostream& ostr) const
   {
      ostr << "Error: " << usrmsg << "\n"; 
   }
};

/*********************************************************************/

/// Class to be thrown when an operation is attempted on an invalid object
class invalid_object: public mic_exception
{
   ///
   char *usrmsg;
   
public:
   /// msg is a user message 
   invalid_object(const char *msg)
   {
      usrmsg = strdup(msg);
   }

   ///
   ~invalid_object()
   {
      if( usrmsg )
      {
         free(usrmsg);
         usrmsg = NULL;
      }  
   }
   
private:  
   ///
   void print(ostream& ostr) const
   {
      ostr << "Error: " << usrmsg << "\n"; 
   }
};

/*********************************************************************/

/// An operation can not complete because the target is busy
class busy: public mic_exception
{
   ///
   char *usrmsg;
   
public:
   /// msg is a user message 
   busy(const char *msg)
   {
      usrmsg = strdup(msg);
   }

   ///
   ~busy()
   {
      if( usrmsg )
      {
         free(usrmsg);
         usrmsg = NULL;
      }  
   }
   
private:  
   ///
   void print(ostream& ostr) const
   {
      ostr << "Error: " << usrmsg << "\n"; 
   }
};


/*********************************************************************/

/// An operation found the object already in the desired state
class already: public mic_exception
{
   ///
   char *usrmsg;
   
public:
   /// msg is a user message 
   already(const char *msg)
   {
      usrmsg = strdup(msg);
   }

   ///
   ~already()
   {
      if( usrmsg )
      {
         free(usrmsg);
         usrmsg = NULL;
      }  
   }
   
private:  
   ///
   void print(ostream& ostr) const
   {
      ostr << "Error: " << usrmsg << "\n"; 
   }
};

/*********************************************************************/
}
#endif
