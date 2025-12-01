#ifndef strings_H
#define strings_H
/**********************************************************************
 **                                                                  **
 **  strings.h                                                       **
 **                                                                  **
 **  define a vector of strings                                      **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 2002.  All Rights Reserved.                           **
 **  Mobile Intelligence Corporation                                 **
 **  Livonia, Michigan, USA                                          **
 **                                                                  **
 **********************************************************************/

/* $Id: strings.h,v 1.1.1.1 2008/07/14 16:44:16 endo Exp $ */

/**********************************************************************
* $Log: strings.h,v $
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
* Revision 1.8  2004/07/30 13:41:00  doug
* added more wrappers
*
* Revision 1.7  2004/05/11 19:34:29  doug
* massive changes to integrate with USC and GaTech
*
* Revision 1.6  2003/07/11 20:19:38  doug
* cleanup debug messages
*
* Revision 1.5  2002/11/06 20:11:52  doug
* add erase functions
*
* Revision 1.4  2002/11/06 14:02:04  doug
* added size() call
*
* Revision 1.3  2002/11/05 16:48:51  doug
* extended strings functions
*
* Revision 1.2  2002/11/05 16:45:36  doug
* extended strings functions
*
* Revision 1.1  2002/11/04 18:09:49  doug
* added seperate strings class so could enhance constructors
*
**********************************************************************/

#include <vector>
#include <string>

namespace sara
{
// *******************************************************************
/// Define the #strings# data type as a vector of string
class strings
{
public:
   /// typedefs
   typedef std::vector<std::string>  strings_T;
   typedef strings_T::iterator iterator;
   typedef strings_T::const_iterator const_iterator;
   typedef strings_T::size_type size_type;
   typedef strings_T::reference reference;
   typedef strings_T::const_reference const_reference;

   /// Constructor
   strings();

   /// Constructor
   strings(const std::string &str);

   /// Constructor
   strings(const std::vector<std::string> &strs);

   /// mimic the vector push_back function
   void push_back(const std::string &str) {data.push_back(str);}


   /// iterator stuff
   iterator begin() {return data.begin();}
   const_iterator begin() const {return (const_iterator)data.begin();}
   iterator end() {return data.end();}
   const_iterator end() const {return (const_iterator)data.end();}
   size_type size() const {return data.size();}
   bool empty() const {return data.empty();}
   reference operator[](size_type n) {return data[n];}
   const_reference operator[](size_type n) const {return data[n];}

   /// Erase functions
   void pop_back() {data.pop_back();}
   void erase(iterator position) {data.erase(position);}
   void erase(iterator first, iterator last) {data.erase(first, last);}

   /// The data
   strings_T data;

   // Stream out contents of the record for debugging.
   friend std::ostream &operator << (std::ostream & out, const strings &obj);
};

// *******************************************************************
}
#endif
