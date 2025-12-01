/**********************************************************************
 **                                                                  **
 **  Grid.cc                                                         **
 **                                                                  **
 **  A two dimensional array class for grids.                        **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-1998.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Alto, Michigan, USA                                             **
 **                                                                  **
 **********************************************************************/

/* $Id: Grid.cc,v 1.1.1.1 2008/07/14 16:44:16 endo Exp $ */

/**********************************************************************
* $Log: Grid.cc,v $
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
* Revision 1.3  1998/11/19 00:25:51  doug
* compiles
*
* Revision 1.2  1998/11/18 03:14:38  doug
* *** empty log message ***
*
* Revision 1.1  1998/11/18 03:05:05  doug
* Initial revision
*
**********************************************************************/

#include "mic.h"
#include "Grid.h"

/*********************************************************************/
// Constructor
template<class T> 
Grid<T>::Grid(const int il, const int ih, const int jl, const int jh) :
   i_low(il),
	i_high(ih),
	j_low(jl),
	j_high(jh)
{
   // Remember how many items per row to speed accesses.
   num_i = i_high - i_low + 1;

   // Allocate the data.
	data = new T[num_i * (j_high - j_low + 1)];
}


/*********************************************************************/
// Destructor
template<class T>
Grid<T>::~Grid()
{
   delete [] data;
	data = NULL;
}

/*********************************************************************/
