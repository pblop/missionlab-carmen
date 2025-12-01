#ifndef SYMBOL_TABLE_HPP
#define SYMBOL_TABLE_HPP
/**********************************************************************
 **                                                                  **
 **                            symbol_table.hpp                      **
 **                                                                  **
 **  A symbol table container class                                  **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1995, Georgia Tech Research Corporation               **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: symbol_table.hpp,v 1.1.1.1 2008/07/14 16:44:25 endo Exp $ */


#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <iostream>

using std::cerr;

/************************************************************************
*			template for symbol tables 			*
*************************************************************************/
template<class T> class symbol_table
{
   int      	  size;
   char 	**key;
   const T	**val;
   int            cur;

   //***********************************************************************
   // Using closed hashing
   // Returns true if pos is valid
   bool string2pos(const char *string,int *pos) const
   {
      if( size == 0 )
	 return false;

      // Compute a hash value
      unsigned short cksum = 0;
      int loc = 0;
      while (string[loc] != 0)
      {
         cksum += string[loc++];
      }

      loc = (cksum * 3) % size;
      int start = loc;

      while (key[loc] != NULL && strcmp(key[loc], string) != 0)
      {
         loc = (loc + 1) % size;

	 // If are back to start, then table is full and rec is not here
         if (loc == start)
            return false;
      }

      *pos = loc;
      return true;
   }

   //****************************************************************
   // Grow : Grow the symbol table to allow new symbols
   // !! Expensive !!
   void grow(void)
   {
      // Save existing data
      int       old_size = size;
      char    **old_key = key;
      const T **old_val = val;

      // Allocate arrays double the current size
      size = (old_size + 7) * 2;
      key = new char *[size];
      val = new const T *[size];

      // Need to clear the key array since use that to check if in use.
      for(int i=0; i<size; i++)
      {
    	  key[i] = NULL;
    	  val[i] = NULL;
      }

      if( old_size > 0 )
      {
         // Copy the data over to the new arrays
         for(int old_pos=0; old_pos<old_size; old_pos++)
         {
	    if( old_key[old_pos] )
	    {
               int loc;
               if( !string2pos(old_key[old_pos],&loc) )
	       {
                  // Just grown the table and string2pos still failed?
                  cerr << "Giving up: While growing the symbol table string2pos(" << old_key[old_pos] << ") failed\n";
                  abort();
	          exit(1);
	       }

               key[loc] = old_key[old_pos];
               val[loc] = old_val[old_pos];
	    }
         }

         // ENDO - MEM LEAK
         //delete old_val;
         //delete old_key;
         delete [] old_val;
         delete [] old_key;
      }
   }

   //***********************************************************************
   // Hash : Closed hashing
   int hash(const char *string)
   {
      // If empty table, grow it
      if( size == 0 )
	 grow();

      // Compute a hash value
      int loc;
      if( !string2pos(string,&loc) )
      {
	 // Table is full, so grow it
	 grow();
         if( !string2pos(string,&loc) )
	 {
            cerr << "Giving up: Just grew the symbol table and string2pos("
		 << string << ") still failed\n";
            abort();
	    exit(1);
	 }
      }

      return loc;
   }

public:
   const char *name;		// Available to users to name tables

   // constructor: create objects
   symbol_table(int num=0)
   {
      if( num < 0 )
	 num = 0;

      if( num > 0 )
      {
         key = new char *[num];
         val = new const T *[num];

         for(int i=0; i<num; i++)
         {
        	 key[i] = NULL;
        	 val[i] = NULL;
         }
      }
      size = num;
      name = NULL;
   }

   // assignment: cleanup and copy
   symbol_table& operator=(const symbol_table& a)
   {
      if( &a == NULL )
      {
         cerr << "Internal error: assignment from NULL symbol table\n";
         abort();
         exit(1);
      }

      if( this != &a )
      {
         // Delete our current records
         if( size > 0 )
         {
            for(int i=0; i<size; i++)
            {
            	if( key[i] )
            	{
            		free(key[i]);
            		key[i] = NULL;
            	}
            }
            delete [] key;
            delete [] val;
            val = NULL;
            key = NULL;
            size = 0;
         }

         // Copy the table
         for(int i=0; i<a.size; i++)
         {
	    if( a.key[i] )
	    {
               put(a.key[i], a.val[i]);
	    }
         }
      }
      return *this;
   }

   // remove all objects
   void clear(void)
   {
      if( size > 0 )
      {
         for(int i=0; i<size; i++)
         {
        	 if( key[i] )
        	 {
        		free(key[i]);
        		 key[i] = NULL;
        	 }
         }
         delete [] key;
         delete [] val;
         val = NULL;
         key = NULL;
         size = 0;
      }
   }


   // destructor: cleanup
   // Does not delete the user data, only the keys which were dup'd
   ~symbol_table()
   {
      // remove all objects
      clear();
   }

#if 0
   // Access an object
   T& operator[](const char *p)
   {
      int loc = hash(p);

      if( key[loc] == NULL )
      {
         key[loc] = strdup(p);
      }

      return val[loc];
   }
#endif

   // Check if an object exists
   bool exists(const char *string) const
   {
      int loc;
      if( !string2pos(string,&loc) )
	 return false;

      return (key[loc] != NULL);
   }

   // Access an object
   const T *get(const char *string) const
   {
      int loc;
      if( !string2pos(string,&loc) )
	 return NULL;

      if( key[loc] == NULL )
	 return NULL;

      return val[loc];
   }

   // put an object
   void put(const char *p, const T *data)
   {
      assert(this);
      assert(p);
      assert(strlen(p));

      int loc = hash(p);

      if( key[loc] == NULL )
         key[loc] = strdup(p);

      val[loc] = data;
   }

   // remove an object, returns true if object existed
   bool remove(const char *string)
   {
      int loc;
      if( !string2pos(string,&loc) )
      {
	 return false;
      }

      if (key[loc] != NULL)
      {
	 assert( strcmp(string,key[loc]) == 0 );

		 delete key[loc];
         key[loc] = NULL;

         // Big mess.
	 // When you remove a record you need to make sure any records
	 // that skipped over this slot because it was used are bumped back up.
	 for(int i=0; i<size; i++)
	 {
	    // Get the next record that would be checked.
            loc = (loc + 1) % size;

            // Quit if found an empty slot because records beyond it can't move
            if( key[loc] == NULL )
	       break;

	    int  new_loc;
            assert( string2pos(key[loc],&new_loc) );

	    // If required, move the record
	    if( new_loc != loc )
	    {
	       key[new_loc] = key[loc];
	       key[loc] = NULL;
	       val[new_loc] = val[loc];
	    }
	 }

	 return true;
      }

      return false;
   }

   // Return the first valid element
   const T *first(void)
   {
      for(cur=0; cur<size; cur++)
	 if( key[cur] )
            return val[cur];

      return NULL;
   }

   // Return the next valid element or NULL
   const T *next(void)
   {
      for(cur++; cur<size; cur++)
	 if( key[cur] )
            return val[cur];

      return NULL;
   }

   // Return the first valid element
   // returns true if valid
   bool first(const char **pkey, const T **pdata)
   {
      for(cur=0; cur<size; cur++)
	 if( key[cur] )
	 {
	    *pkey = key[cur];
	    *pdata = val[cur];
            return true;
	 }

      return false;
   }

   // Return the next valid element
   // returns true if valid
   bool next(const char **pkey, const T **pdata)
   {
      for(cur++; cur<size; cur++)
	 if( key[cur] )
	 {
	    *pkey = key[cur];
	    *pdata = val[cur];
            return true;
	 }

      return false;
   }

   // dump the keys to stderr
   void dump()
   {
      cerr << "Symbol table size = " << size << "\n";
      cerr << "Symbol table keys:\n";
      for(cur=0; cur<size; cur++)
	 if( key[cur] )
	    cerr << "   " << cur << ": " << key[cur] << "\n";
      cerr << "\n";
   }


   // Return the current table size
   int tablesize(void) const
   {
      return size;
   }

};


///////////////////////////////////////////////////////////////////////
// $Log: symbol_table.hpp,v $
// Revision 1.1.1.1  2008/07/14 16:44:25  endo
// MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
//
// Revision 1.1.1.1  2006/07/20 17:17:52  endo
// MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
//
// Revision 1.1.1.1  2006/07/12 13:38:01  endo
// MissionLab 7.0
//
// Revision 1.3  2006/05/12 22:49:05  endo
// g++-3.4 upgrade.
//
// Revision 1.2  2005/11/08 17:10:55  endo
// Memory leak problem solved.
//
// Revision 1.1.1.1  2005/02/06 23:00:22  endo
// AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
//
// Revision 1.19  2003/04/06 11:55:38  endo
// gcc 3.1.1
//
// Revision 1.18  1995/12/14 21:14:46  doug
// added clear function
//
// Revision 1.17  1995/12/01  21:17:23  doug
// fixed the remove function.  Wasn't shifting records up after removing one
//
// Revision 1.16  1995/11/19  21:37:22  doug
// *** empty log message ***
//
// Revision 1.15  1995/11/19  21:26:01  doug
// made functions const that can be
//
// Revision 1.14  1995/11/07  14:30:57  doug
// *** empty log message ***
//
// Revision 1.13  1995/09/07  14:23:14  doug
// works
//
// Revision 1.12  1995/08/24  16:40:24  doug
// fixed interface on new functions
//
// Revision 1.11  1995/08/24  16:21:26  doug
// Added new versions of first,next that return key also.
//
// Revision 1.10  1995/06/29  14:45:22  jmc
// Added RCS log string.
///////////////////////////////////////////////////////////////////////

#endif
