/**********************************************************************
 **                                                                  **
 **               build_dir_list.cc                                  **  
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1995, Georgia Tech Research Corporation               **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: build_dir_list.cc,v 1.1.1.1 2008/07/14 16:44:25 endo Exp $ */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/dir.h>

#include "utilities.h"

static const char *ext = NULL;   // Has period:  ".ovl"
static int ext_len = 0;

#ifndef linux
extern "C" int scandir(const char *dirname, 
		struct direct ***namelist, 
		int (*select)(const direct *p), 
		int (*compar)(struct direct **d1, struct direct **d2));

extern "C" int alphasort(struct direct **d1, struct direct **d2);
#endif

/*-----------------------------------------------------------------------*/
static int 
select(const direct *p)
{
#ifdef linux
   const int len = strlen(p->d_name);
#else
   const int len = p->d_namlen;
#endif

   // Not long enough for an extension
   if( len <= ext_len )
      return false;

   // Extension doesn't match
   if( strcmp(ext, &p->d_name[len - ext_len]) )
      return false;

   return true;
}

/*-----------------------------------------------------------------------*/

// Returns the number of entries in namelist
// in the directory with the desired extension (extension has period: ".ovl")

int
build_dir_list(const char *dir, const char *extension, char ***names)
{
    // Save stuff in the globals
    ext = extension;
    ext_len = strlen(extension);

    struct direct **namelist;

    // Build the directory.
    int cnt = scandir(dir, &namelist, select, alphasort);

    if( cnt > 0 )
    {
        // ENDO - gcc 3.4
        *names = new char *[cnt];

        for(int i=0; i<cnt; i++)
        {
            (*names)[i] = namelist[i]->d_name;
        }
    }

    return cnt;
}


/**********************************************************************
 * $Log: build_dir_list.cc,v $
 * Revision 1.1.1.1  2008/07/14 16:44:25  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/20 17:17:52  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:38:01  endo
 * MissionLab 7.0
 *
 * Revision 1.2  2006/05/12 22:49:05  endo
 * g++-3.4 upgrade.
 *
 * Revision 1.1.1.1  2005/02/06 23:00:21  endo
 * AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
 *
// Revision 1.3  1996/09/03  15:27:42  doug
// fixed prototype
//
// Revision 1.2  1996/09/03  15:23:45  doug
// fixing for linux
//
 * Revision 1.1  1996/02/27 02:28:16  doug
 * Initial revision
 *
// Revision 1.1  1996/02/07  17:51:08  doug
// Initial revision
//
 **********************************************************************/
