/**********************************************************************
 **                                                                  **
 **                         TypeInfo.hpp                             **
 **                                                                  **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1995, Georgia Tech Research Corporation               **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **********************************************************************/

/* $Id: TypeInfo.hpp,v 1.1.1.1 2008/07/14 16:44:21 endo Exp $ */


#ifndef TypeInfo_HPP
#define TypeInfo_HPP

#include <Xm/Xm.h>

const int TYPEINFO_DEFAULT_WIDTH = 475;

// Define the TypeInfo class
class TypeInfo {

public:
    char *TypeName;

    TypeInfo(const char *name) {

        TypeName = strdup(name);
    }

    virtual ~TypeInfo(void) {

        free(TypeName);
        TypeName = NULL;
    }

    bool IsThisOurName(const char *name) {
		return strcmp(TypeName,name) == 0;
    };
	
    virtual Widget CreateDisplayWidget(Widget parent, struct Symbol *parm) = 0;
    //virtual void UpdateFromDisplayWidget(Widget w, char **value) = 0;
    virtual void UpdateFromDisplayWidget(Widget w, struct Symbol *parm) = 0;
    virtual char *PrintValue(struct Symbol *parm, const char *separator) = 0;
};

#endif


///////////////////////////////////////////////////////////////////////
// $Log: TypeInfo.hpp,v $
// Revision 1.1.1.1  2008/07/14 16:44:21  endo
// MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
//
// Revision 1.3  2006/12/05 01:47:02  endo
// cdl parser now accepts {& a b} and Symbol *pair added.
//
// Revision 1.2  2006/08/29 15:13:57  endo
// Advanced parameter flag added to symbol.
//
// Revision 1.1.1.1  2006/07/20 17:17:49  endo
// MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
//
// Revision 1.1.1.1  2006/07/12 13:37:59  endo
// MissionLab 7.0
//
// Revision 1.1.1.1  2005/02/06 23:00:06  endo
// AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
//
// Revision 1.2  1996/04/06  23:54:20  doug
// *** empty log message ***
//
// Revision 1.2  1996/04/06  23:54:20  doug
// *** empty log message ***
//
// Revision 1.1  1996/04/06  20:34:13  doug
// Initial revision
//
///////////////////////////////////////////////////////////////////////
