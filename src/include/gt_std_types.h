/************************************************************************
*                                                                       *
*           gt_std_types.h                                              *
*                                                                       *
*                                                                       *
*   Written by: Douglas C. MacKenzie                                    *
*                                                                       *
*   Copyright 1995, Georgia Tech Research Corporation                   *
*   Atlanta, Georgia  30332-0415                                        *
*   ALL RIGHTS RESERVED, See file COPYRIGHT for details.                *
*	                                                                    *
*************************************************************************/

#ifndef GT_STD_TYPES
#define GT_STD_TYPES

#include <iostream>
#include "gt_std.h"

/************************************************************************
*           template for variable sized arrays                          *
*************************************************************************/
// ENDO - gcc 3.4: The following moved to gt_std.h
/*
struct obs_reading
{
    Vector center;
    double r;
};


struct raw_sonar_reading
{
    double val;
};
*/

template<class Type> class array {

 public:

    int size;
    Type* val;

    // constructor: create objects
    array( int num = 0 )
    {
        val = NULL;
        size = 0;

        if( num > 0 )
      	{
      	    val = new Type[ num ];
            size = num;
      	}
    }

    // copy constructor
    array( const array& a )
    {
        val = NULL;
        size = 0;

        if ( a.size > 0 )
		{
			val = new Type[ a.size ];
			size = a.size;
			memcpy( val, a.val, sizeof( Type ) * size );
		}
    }

    // assignment: cleanup and copy
    array& operator=( const array& a )
    {
        if( this != &a )
        {
            cleanup();

            if ( a.size > 0 )
            {
                val = new Type[ a.size ];
                size = a.size;
                memcpy( val, a.val, sizeof( Type ) * size );
            }
        }
        return *this;
    }

    // destructor: cleanup
    ~array()
    {
        cleanup();
    }

#if 0
    // Print the object
    void print() const
    {
        cout << "Array:\n";
        for(int i = 0; i<size; i++)
        {
            cout << "\t" << val[ i ];
        }
        cout << '\n';
    }

    // Read the object
    void read() const
    {
        for( int i = 0; i < size; i++ )
        {
            cin >> val[ i ];
        }
    }
#endif

 protected:

    // delete all data in the object
    void cleanup()
    {
        size = 0;
        if ( val != NULL )
        {
            delete [] val;
            val = NULL;
        }
    }

};

/************************************************************************/

// declare instances of the array class
typedef ::array<Vector> vector_array;
typedef ::array<obs_reading> obs_array;
typedef ::array<raw_sonar_reading> raw_sonar_array;

/************************************************************************/

#endif  // GT_STD_TYPES_H
