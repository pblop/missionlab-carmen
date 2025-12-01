/**********************************************************************
 **                                                                  **
 **  Grid.h                                                          **
 **                                                                  **
 **  A dynamically sizable two dimensional array                     **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-1998.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Alto, Michigan, USA                                             **
 **                                                                  **
 **********************************************************************/

/* $Id: Grid.h,v 1.1.1.1 2008/07/14 16:44:16 endo Exp $ */

/**********************************************************************
* $Log: Grid.h,v $
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
* Revision 1.1  1998/11/18 03:04:53  doug
* Initial revision
*
* Revision 1.2  1998/11/18 02:21:31  doug
* Cleaned up indentation
*
* Revision 1.1  1998/11/18 02:18:40  doug
* Initial revision
*
**********************************************************************/

#include <stdexcept>

/**********************************************************************/
// The grid class
template <class T> class Grid
{
   public:

	// Constructor
	Grid(const int il, const int ih, const int jl, const int jh);

	// Destructor
	~Grid();

   // Access a location in the grid
   T &location(const int i, const int j)
   {
		if( this == NULL || data == NULL )
			throw std::invalid_argument("Attempt to access invalid Grid object");

      if (i < i_low || i > i_high || j < j_low || j > j_high)
      {
			char buf[128];
			sprintf(buf,"Subscript %d,%d out of range [%d:%d, %d:%d]\n",
					i,j, i_low, i_high, j_low, j_high);

			throw std::range_error(buf);
	 	}

	 	return data[(j - j_low) * num_i + (i - i_low)];
	}

   // functions to return the dimensions of the grid.
	int ibegin()const
	{
		if( this == NULL || data == NULL )
			throw std::invalid_argument("Attempt to access invalid Grid object");

	 	return i_low;
	}

	int iend()const
	{
		if( this == NULL || data == NULL )
			throw std::invalid_argument("Attempt to access invalid Grid object");

	 	return i_high;
	}

	int jbegin()const
	{
		if( this == NULL || data == NULL )
			throw std::invalid_argument("Attempt to access invalid Grid object");

	 	return j_low;
	}

	int jend()const
	{
		if( this == NULL || data == NULL )
			throw std::invalid_argument("Attempt to access invalid Grid object");

	 	return j_high;
	}

private:
   int i_low;
   int i_high;
	int num_i;
   int j_low;
   int j_high;

	T *data;
};

/**********************************************************************/
