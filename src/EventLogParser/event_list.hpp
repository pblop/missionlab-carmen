#ifndef EVENT_LIST_HPP
#define EVENT_LIST_HPP
/**********************************************************************  
 **                                                                  **  
 **                            EVENT_list.hpp                              **  
 **                                                                  **  
 **  list container class                                            **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1995, Georgia Tech Research Corporation               **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **  
 **********************************************************************/ 

/* $Id: event_list.hpp,v 1.1.1.1 2008/07/14 16:44:14 endo Exp $ */


//#include <iostream.h>
#include <stdlib.h>
#include <iostream>

// ENDO - gcc 3.4
//class events;

// #define DEBUG_LIST 1
#define NO_NULL_DATA 1

#ifdef NO_NULL_DATA
#include <assert.h>
#endif

using std::cerr;

/************************************************************************
*			template for symbol tables 			*
*************************************************************************/
template<class T> class event_list
{
public:
    struct SLink
    {
        struct SLink *next;
        T             data;

        // constructor: create objects
        SLink(const T val) {
#ifdef NO_NULL_DATA
            assert(val);
#endif
            data = val;
        }

        // destructor: needed so don't delete the data objects
        ~SLink() {

            // Nothing to do, here to keep from deleting the data
        }
    };

    SLink *last_ptr;


public:

    // ...

    // constructor: create objects
    event_list() {

        last_ptr = NULL;
    }

    event_list(const T data) {

#if DEBUG_LIST
        cerr << "starting create with data\n";
#endif
        last_ptr = NULL;
      
#ifdef NO_NULL_DATA
        assert(data);
#endif

        // Insert the data item
        insert(data);

#if DEBUG_LIST
        cerr << "after create with data\n";
        dump();
#endif
    }


    // copy constructor
    event_list(const event_list& a) {

        last_ptr = NULL;

        // Copy the list from the source.
        T data;
        void *cur;
        if( (cur = ((event_list *)&a)->first(&data)) != NULL )
        {
            do
            {
#ifdef NO_NULL_DATA
                assert(data);
#endif
                append(data);
            } while( (cur = ((event_list *)&a)->next(&data,cur)) != NULL );
        }
    }

    // assignment: cleanup and copy
    event_list& operator=(const event_list& a) {

        if( &a == NULL )
        {
            cerr << "Internal error: assignment from NULL list\n";
            abort();
            exit(1);
        }

        if( this != &a )
        {
            // Delete our current records
            while( ! isempty() )
                get();

            // Copy the list
            last_ptr = NULL;

            // Copy the list from the source.
            T data;
            void *cur;
            if( (cur = ((event_list *)&a)->first(&data)) != NULL )
            {
                do
                {
#ifdef NO_NULL_DATA
                    assert(data);
#endif
                    append(data);
                } while( (cur = ((event_list *)&a)->next(&data,cur)) != NULL );
            }
        }
        return *this;
    }

    // duplicate the contents of the list
    event_list dup() const {

        event_list *p = new event_list;

        // Copy the list from the source.
        T data;
        void *cur;
        if( (cur = first(&data)) != NULL )
        {
            do
            {
#ifdef NO_NULL_DATA
                assert(data);
#endif
                T copy_of_data = data;
                p->append(copy_of_data);
            } while( (cur = next(&data,cur)) != NULL );
        }

        return *p;
    }

    // destructor: cleanup
    ~event_list() {

        // Delete our current records
        while( ! isempty() )
            get();
    }

    // Is list empty?
    int isempty(void) const {

        return last_ptr == NULL;
    }

    // alias for get
    const T pop() {return get();}

    // Return and remove the first object
    const T get(void) {

        if( isempty() )
        {
            cerr << "Internal error: get from empty list\n";
            abort();
            exit(1);
        }
      
        SLink *head = last_ptr->next;
        if( head == last_ptr )
            last_ptr = NULL;
        else
        {
            last_ptr->next = head->next;
            assert(last_ptr->next);
        }

        const T data = head->data;
        delete head;

#ifdef NO_NULL_DATA
        assert(data);
#endif
#if DEBUG_LIST
        cerr << "after get\n";
        dump();
#endif

        return data;
    }

    // Append to end of list
    // Note: last_ptr->next is first record
    void append(const T data) {

#ifdef NO_NULL_DATA
        assert(data);
#endif
        SLink *rec = new SLink(data);
        if( last_ptr )
        {
            rec->next = last_ptr->next;
            assert(rec->next);

            last_ptr->next = rec;
            assert(last_ptr->next);
        }
        else
        {
            rec->next = rec;
            assert(rec->next);
        }
        last_ptr = rec;
#if DEBUG_LIST
        cerr << "after append of rec = " << (unsigned long)rec << '\n';
        dump();
#endif
    }

    // alias for insert
    void push(const T data) {insert(data);}

    // Insert at head of list
    // Note: last_ptr->next is first record
    void insert(const T data) {

#ifdef NO_NULL_DATA
        assert(data);
#endif
        struct SLink *rec = new SLink(data);
        if( last_ptr )
        {
            rec->next = last_ptr->next;
            assert(rec->next);

            last_ptr->next = rec;
            assert(last_ptr->next);
        }
        else
        {
            last_ptr = rec;
        }
        last_ptr->next = rec;
        assert(last_ptr->next);
#if DEBUG_LIST
        cerr << "after insert of rec = " << (unsigned long)rec << '\n';
        dump();
#endif
    }

    // Return last element without removing it
    // Returns false if list is empty
    bool last(T *rec) {

        if( this == NULL )
        {
            cerr << "Internal Error: call to first on NULL list object\n";
            abort();
            exit(1);
        }

        if( last_ptr == NULL )
            return false;
      
        *rec = last_ptr->data;

#ifdef NO_NULL_DATA
        assert(*rec);
#endif
        return true;
    }

    // Return first element without removing it
    // Note: last_ptr->next is first record
    // Returns NULL if list is empty
    void *first(T *rec) const {

        if( this == NULL )
        {
            cerr << "Internal Error: call to first on NULL list object\n";
            abort();
            exit(1);
        }

        if( last_ptr == NULL )
            return NULL;
      
        *rec = last_ptr->next->data;
#ifdef NO_NULL_DATA
        assert(*rec);
#endif

        return (void *)last_ptr->next;
    }

    // Return next element without removing it
    // Note: must call first to setup the iterator
    // Returns NULL if list is empty
    void *next(T *rec, void *cur) const {

        if( (SLink *)cur == last_ptr )
            return NULL;
      
        *rec = ((SLink *)cur)->next->data;
#ifdef NO_NULL_DATA
        assert(*rec);
#endif
        return (void *)((SLink *)cur)->next;
    }

    // Check if the record is already on the list
    bool exists(const T data) const {

#ifdef NO_NULL_DATA
        assert(data);
#endif
        if( last_ptr != NULL )
        {
            SLink *p = last_ptr->next;
            do
            {
                if( p->data == data )
                    return true;
                p = p->next;
            } while(p != last_ptr->next);
        }
        return false;
    }

    // Count number of occurrences of the record in the list
    int occurrences(const T data) const {

#ifdef NO_NULL_DATA
        assert(data);
#endif
        int cnt = 0;
        if( last_ptr != NULL )
        {
            SLink *p = last_ptr->next;
            do
            {
                if( p->data == data )
                    cnt++;
                p = p->next;
            } while(p != last_ptr->next);
        }
        return cnt;
    }

    // Dump the list pointers
    void dump(void) const {

        cerr << "Dumping event_list\n";

        if( last_ptr != NULL )
        {
            SLink *p = last_ptr->next;
            do
            {
                cerr << " : data @" << (unsigned long)p->data 
                     //<< " = " << p->data 
                     << '\n';
                p = p->next;
            } while(p != last_ptr->next);
        }

        cerr << "   End\n";
    }

    // count the list members
    int len() const {

        int count = 0;
        if( last_ptr != NULL )
        {
            SLink *p = last_ptr->next;
            do
            {
                count++;
                p = p->next;
            } while(p != last_ptr->next);
        }

        return count;
    }

    // remove all objects
    void clear(void) {

        while( !isempty() )
        {
            get();
        }
    }

    // Merge into the list, making sure not to duplicate the data item.
    void merge(const T data) {

#ifdef NO_NULL_DATA
        assert(data);
#endif
        if( !exists(data) )
            append(data);
    }

    // if the record is on the list, remove it
    bool remove(const T data) {

#ifdef NO_NULL_DATA
        assert(data);
#endif
        if( last_ptr != NULL )
        {
            SLink *p = last_ptr;
            do
            {
                // Found it, so now remove it
                if( p->next->data == data )
                {
                    // Case where is at head of list
                    // Also catches cases where is only this item in the list
                    if( last_ptr->next == p->next )
                    {
                        get();
                    }
                    else
                    {
                        // Record we are deleting
                        SLink *extra = p->next;

                        // Move chain pointers past it
                        p->next = p->next->next;
                        assert(p->next);

                        // Was it at end of list?  If so, fixup the last_ptr
                        if( extra == last_ptr )
                            last_ptr = extra->next;
            
                        // Discard our list container record.
                        delete extra;
                    }
                    return true;
                }

                p = p->next;
            } while(p->next != last_ptr->next);
        }
        return false;
    }

    // if the record is on the list, replace it with the new value and 
    // return true. Otherwise, do nothing and return false
    bool replace(const T old_data, const T new_data) {

#ifdef NO_NULL_DATA
        assert(new_data);
#endif
        if( last_ptr != NULL )
        {
            SLink *p = last_ptr;
            do
            {
                // Found it, so now replace it
                if( p->next->data == old_data )
                {
                    p->next->data = new_data;
                    return true;
                }

                p = p->next;
            } while(p->next != last_ptr->next);
        }
        return false;
    }

    // ENDO - gcc 3.4
    /*
    // Alphabetize the contents of the list in place
    // This assumes that the data is a pointer to an events record 
    void alphabetize() {

        // Do a bubble sort on the list
        bool made_change;
        do
        {
            made_change = false;
            SLink *p = last_ptr;
            while( p && p->next != last_ptr)
            {
                p = p->next;

                if( ((events *)p->data)->start > ((events *)p->next->data)->start )
                {
                    T data = p->data;
                    p->data = p->next->data;
                    p->next->data = data;
                    made_change = true;
                }
            }
        } while(made_change);
    }
    */
};



///////////////////////////////////////////////////////////////////////
// $Log: event_list.hpp,v $
// Revision 1.1.1.1  2008/07/14 16:44:14  endo
// MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
//
// Revision 1.1.1.1  2006/07/20 17:17:45  endo
// MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
//
// Revision 1.1.1.1  2006/07/12 13:37:53  endo
// MissionLab 7.0
//
// Revision 1.2  2006/05/12 22:51:56  endo
// g++-3.4 upgrade.
//
// Revision 1.1.1.1  2005/02/06 22:59:27  endo
// AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
//
// Revision 1.4  2003/04/06 11:39:04  endo
// gcc 3.1.1
//
// Revision 1.3  1999/12/16 23:00:08  endo
// rh-6.0 porting.
//
// Revision 1.2  1996/05/05 17:59:39  doug
// *** empty log message ***
//
// Revision 1.1  1996/04/13  21:55:13  doug
// Initial revision
//
// Revision 1.24  1996/03/05  19:08:35  doug
// added pop and push aliases
//
// Revision 1.23  1996/02/15  19:30:32  doug
// fixed pushed up parms in groups
//
// Revision 1.22  1996/02/08  19:22:31  doug
// *** empty log message ***
//
// Revision 1.21  1995/12/14  21:54:03  doug
// *** empty log message ***
//
// Revision 1.20  1995/11/21  23:09:22  doug
// *** empty log message ***
//
// Revision 1.19  1995/11/07  14:29:58  doug
// *** empty log message ***
//
// Revision 1.18  1995/06/29  14:43:48  jmc
// Added RCS log string.
///////////////////////////////////////////////////////////////////////


#endif
