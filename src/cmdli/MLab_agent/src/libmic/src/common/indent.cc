/**********************************************************************
 **                                                                  **
 **  indent.cc                                                       **
 **                                                                  **
 **  Simplify indenting nested stuff.                                **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-1999.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Alto, Michigan, USA                                             **
 **                                                                  **
 **********************************************************************/

/* $Id: indent.cc,v 1.1.1.1 2008/07/14 16:44:16 endo Exp $ */

/**********************************************************************
* $Log: indent.cc,v $
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
* Revision 1.3  2004/05/11 19:34:29  doug
* massive changes to integrate with USC and GaTech
*
* Revision 1.2  2002/10/22 21:05:59  doug
* updated to gcc 3.0
*
* Revision 1.1  1999/08/01 02:47:08  doug
* Initial revision
*
**********************************************************************/

#include "mic.h"
#include "indent.h"

using namespace std;
namespace sara
{

/**********************************************************************/
class margin
{
   public:
      friend ostream& operator<<(ostream&, const margin &obj);
      ~margin();
      static margin *create_margin(const uint tabsize = 3);
      static margin *themargin();
      void indent(const uint steps);
      void outdent(const uint steps);
      void reset();

   private:
      margin();
      uint columns;
      uint tabsize;
      static margin *the_margin;
};


//**********************************************************************
// print an end of line and enough blanks to get the left margin correct.
ostream & operator <<(ostream & out, const iendlT & obj)
{
	return out << endl << *margin::themargin();
}

// Declare an instance to get prints to work.
iendlT iendl;

//**********************************************************************
// increment the tab column
ostream & operator <<(ostream & out, const addtabT & obj)
{
	margin::themargin()->indent(1);
	return out;
}

// Declare an instance to get prints to work.
addtabT addtab;

//**********************************************************************
// decrement the tab column
ostream & operator <<(ostream & out, const deltabT & obj)
{
	margin::themargin()->outdent(1);
	return out;
}

// Declare an instance to get prints to work.
deltabT deltab;

//**********************************************************************
// reset the tab column
ostream & operator <<(ostream & out, const cleartabsT & obj)
{
	margin::themargin()->reset();
	return out;
}

// Declare an instance to get prints to work.
cleartabsT cleartabs;

//**********************************************************************
margin *margin::the_margin = NULL;

//**********************************************************************
// print enough blanks to get the left margin correct.
ostream & operator <<(ostream & out, const margin & obj)
{
   for(uint i=0; i< obj.columns * obj.tabsize; ++i)
      out << " ";

	return out;
}

//**********************************************************************
// Create with default tab size of 3.
margin::margin() :
   columns(0),
	tabsize(3)
{
}

//**********************************************************************
margin::~margin()
{
   if( the_margin )
	{
      delete the_margin;
	   the_margin = NULL;
	}
}

//**********************************************************************
margin *margin::create_margin(const uint Tsize)
{
   if( the_margin == NULL )
      the_margin = new margin();

	the_margin->tabsize = Tsize;
	the_margin->columns = 0;

	return the_margin;
}

//**********************************************************************
margin *margin::themargin()
{
   if( the_margin == NULL )
      create_margin();

   return the_margin;
}

//**********************************************************************
void margin::indent(const uint steps)
{
   columns += steps;
}

//**********************************************************************
void margin::outdent(const uint steps)
{
   if( columns >= steps )
      columns -= steps;
	else
		columns = 0;
}

//**********************************************************************
void margin::reset()
{
   columns = 0;
}

//*********************************************************************
}
