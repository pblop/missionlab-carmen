#ifndef INDENT_H
#define INDENT_H
/**********************************************************************
 **                                                                  **
 **  indent.h                                                        **
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

/* $Id: indent.h,v 1.1.1.1 2008/07/14 16:44:16 endo Exp $ */

/**********************************************************************
* $Log: indent.h,v $
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

namespace sara
{
/**********************************************************************/
class iendlT
{
   friend std::ostream& operator <<(std::ostream& out, const iendlT &obj);
};

extern class iendlT iendl;

/**********************************************************************/
class addtabT
{
   friend std::ostream& operator <<(std::ostream& out, const addtabT &obj);
};

extern class addtabT addtab;

/**********************************************************************/
class deltabT
{
   friend std::ostream& operator <<(std::ostream& out, const deltabT &obj);
};

extern class deltabT deltab;

/**********************************************************************/
class cleartabsT
{
   friend std::ostream& operator <<(std::ostream& out, const cleartabsT &obj);
};

extern class cleartabsT cleartabs;

/*********************************************************************/
}
#endif
