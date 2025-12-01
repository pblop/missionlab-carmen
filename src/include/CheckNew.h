/**********************************************************************
 **                                                                  **
 **                       CheckNew.h                                 **
 **                                                                  **
 **  Abstract class used to draw stuff in the simulation world.      **
 **                                                                  **
 **                                                                  **
 **  Written by:  J. Brian Lee                                       **
 **                                                                  **
 **  Copyright 1995, 1996, 1997 Georgia Tech Research Corporation    **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **********************************************************************/

#include <iostream>

using std::cerr;

// This is used to check memory allocation so you don't have to repeat this code 
// every single time new memory is allocated.
inline void CheckNew( void* pvToCheck )
{
    if ( pvToCheck == NULL )
    {
        cerr << "Fatal error: couldn't allocate memory\n";
        exit( -1 );
    }
}
