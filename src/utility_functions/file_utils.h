/**********************************************************************
 **                                                                  **
 **                           file_utils.h                           **
 **                                                                  **
 **        Declare some functions for files and filenames.           **
 **                                                                  **
 **  Written by:  Jonathan M. Cameron                                **
 **                                                                  **
 **  Copyright 1995 - 1997, 1999 - 2003 Georgia Tech Research        **
 **  Corporation Atlanta, Georgia  30332-0415                        **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: file_utils.h,v 1.1.1.1 2008/07/14 16:44:25 endo Exp $ */

#ifndef FILE_UTILS_H
#define FILE_UTILS_H

#include <string>

using std::string;

/**********************************************************************
 **                                                                  **
 **                        Declare Constants                         **
 **                                                                  **
 **********************************************************************/

const int MAX_FILENAME_LENGTH = 256;



/**********************************************************************
 **                                                                  **
 **                        Declare Functions                         **
 **                                                                  **
 **********************************************************************/

// Checks for the existence of the file.
// Returns TRUE if it exists, FALSE if not.
int file_exists( const char* filename );

// Checks for the presense of a directory in the filename.
// Returns TRUE if present, FALSE, if not.
bool filename_has_directory( const char* szFileName );
bool filename_has_directory( const string& strFileName );

// Tries to find the file.  Returns NULL if it fails.   It's logic:
// If "filename" has a directory component, it must exist where it is.
// Next it checks for the file in the specified directory.
// Finally it just checks for the file in the current directory.
char* find_full_filename( const char* filename, const char* directory, char* buff );

// Returns the directory part of the filename, if it is present, or NULL
// if it is not present.  The caller must provide the string space "buff".
char* extract_directory( const char* filename, char* buff );
string extract_directory( const string& strFileName );

// Returns just the simple filename part of the filename (ie, removes the
// directory component.  If no directory is present, it just copies the
// filename into the buffer and returns it.  The caller must provide the
// string space in "buff".
char* remove_directory( const char* filename, char* buff );
string remove_directory( const string& strFileName );

// Returns the file extension part of the filename.  If none is present,
// return NULL.  The caller must provide the string space in "buff".
char* extract_extension( const char* filename, char* buff );

string find_file_in_envpath(const string filename);

#endif  // FILE_UTILS_H


/**********************************************************************
 * $Log: file_utils.h,v $
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
 * Revision 1.4  2003/04/06 11:36:37  endo
 * gcc 3.1.1
 *
 * Revision 1.3  2003/04/06 09:15:27  endo
 * Updated for CBR Wizard Prototype II.
 *
 * Revision 1.2  2002/04/04 18:36:52  blee
 * Changed return type of filename_has_directory(), overloaded
 * filename_has_directory(), overloaded extract_directory(),
 * and overloaded remove_directory().
 *
 * Revision 1.1  2002/01/13 01:57:23  endo
 * Initial revision
 *
 * Revision 1.5  1997/02/12 05:41:58  zchen
 * *** empty log message ***
 *
 * Revision 1.4  1995/06/14  18:21:28  jmc
 * Added function prototype for find_full_filename().
 *
 * Revision 1.3  1995/06/09  21:43:15  jmc
 * Added declaration for extract_extension() function.
 *
 * Revision 1.2  1995/04/26  14:42:10  jmc
 * Added function prototype for remove_directory().
 *
 * Revision 1.1  1995/04/12  21:10:07  jmc
 * Initial revision
 **********************************************************************/
