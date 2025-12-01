/**********************************************************************
 **                                                                  **
 **                            utility.h                             **
 **                                                                  **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1995, Georgia Tech Research Corporation               **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **********************************************************************/

/* $Id: utilities.h,v 1.1.1.1 2008/07/14 16:44:25 endo Exp $ */


#ifndef UTILITY_H
#define UTILITY_H

#include "load_rc.h"
#include <sys/types.h>


// Returns true if the copy fails.
bool 
copyfile(const char *src, const char *des);

// Returns an allocated block of mem with a//b and frees a.
// b is not freed.  
// Handles case where a is NULL, but b must be valid
char *
strdupcat(char *a, const char *b);

// Returns an allocated block of mem with a//b//c and frees a.
// b and c are not freed.
// Handles case where a is NULL, but b and c must be valid.
char *
strdupcat(char *a, const char *b, const char *c);


// extract the list of colon seperated directories from an environment variable
char **
extract_directories(const char *env_var_name);

// find the file by looking through the list of directories
char *
find_file(const char *name, const rc_chain *dirs);

// Return the date the file was last modified
// or -1 if not found
time_t  file_date(const char *name);

// Build the list of file names in the specified directory with the
// specified extension.
int 
build_dir_list(const char *dir, const char *extension, char ***names) ;

// Returns the input string with the carriage returns converted to blanks.
// IT MODIFIES THE INPUT STRING
char *
strip_crs(char *a);


#endif


/**********************************************************************
 * $Log: utilities.h,v $
 * Revision 1.1.1.1  2008/07/14 16:44:25  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/20 17:17:52  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:38:01  endo
 * MissionLab 7.0
 *
 * Revision 1.1.1.1  2005/02/06 23:00:22  endo
 * AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
 *
 * Revision 1.8  1996/03/12  17:45:28  doug
 * *** empty log message ***
 *
 * Revision 1.7  1996/02/27  02:28:02  doug
 * *** empty log message ***
 *
 * Revision 1.6  1996/02/07  17:51:19  doug
 * *** empty log message ***
 *
 * Revision 1.5  1995/10/31  19:23:46  doug
 * added find_file
 *
 * Revision 1.4  1995/10/30  21:35:36  doug
 * *** empty log message ***
 *
 * Revision 1.3  1995/10/30  20:56:38  doug
 * Renamed from copyfile.h
 *
 **********************************************************************/
