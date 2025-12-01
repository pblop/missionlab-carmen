/**********************************************************************
 **                                                                  **
 **                               map_glob.c                         **
 **                                                                  **
 **                                                                  **
 **                                                                  **
 **                                                                  **
 **  Written by:                                                     **
 **                                                                  **
 **  Copyright 2000, Georgia Tech Research Corporation               **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **********************************************************************/

/* $Id: map_glob.c,v 1.1.1.1 2008/07/14 16:44:24 endo Exp $ */

/* global declarations */


#include "clipstruct.h"
#include "new_long_term_memory.h"
#include "short_term_memory.h"

struct clip_board_t clipboard;
char            clipboard_filename[50] = "clipboard.dat";

struct long_term_memory_t long_term_memory;
char            long_term_memory_filename[50] = "long_term_memory.dat";

struct short_term_memory_t short_term_memory;
char            short_term_memory_filename[50] = "short_term_memory.dat";



///////////////////////////////////////////////////////////////////////
// $Log: map_glob.c,v $
// Revision 1.1.1.1  2008/07/14 16:44:24  endo
// MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
//
// Revision 1.1.1.1  2006/07/20 17:17:51  endo
// MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
//
// Revision 1.1.1.1  2006/07/12 13:38:00  endo
// MissionLab 7.0
//
// Revision 1.1.1.1  2005/02/06 23:00:24  endo
// AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
//
// Revision 1.2  2003/04/06 10:16:11  endo
// Checked in for Robert R. Burridge. Various bugs fixed.
//
// Revision 1.1  2000/03/22 04:40:07  saho
// Initial revision
//
///////////////////////////////////////////////////////////////////////

