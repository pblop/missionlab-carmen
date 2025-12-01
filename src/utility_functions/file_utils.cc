/**********************************************************************
 **                                                                  **
 **                           file_utils.c                           **
 **                                                                  **
 **                                                                  **
 **       Implement some functions for files and filenames.          **
 **                                                                  **
 **                                                                  **
 **  Written by:  Jonathan M. Cameron                                **
 **                                                                  **
 **  Copyright 1995 - 1997, 1999 - 2003 Georgia Tech Research        **
 **  Corporation Atlanta, Georgia  30332-0415                        **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: file_utils.cc,v 1.1.1.1 2008/07/14 16:44:25 endo Exp $ */

#include <stdio.h>
#include <string.h>

#include <string>

#include "gt_std.h"
#include "string_utils.h"

#include "file_utils.h"
#include "utilities.h"

//-----------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------
const string FILE_UTILS_EMPTY_STRING = "";

/**********************************************************************
 **                                                                  **
 **                    implement global functions                    **
 **                                                                  **
 **********************************************************************/




/**********************************************************************
 **            file_exists             **
 ****************************************/

int file_exists( const char* filename )
{
    FILE* tmp = fopen( filename, "r" );

    if ( tmp == NULL )
    {
        return FALSE;
    }

    // must have existed because we opened it, so close it
    fclose( tmp );
    return TRUE;
}


/**********************************************************************
 **         find_full_filename         **
 ****************************************/

char* find_full_filename( const char* filename,
			 const char* directory,
			 char* buff )
{
//    Tries to find the file.  Returns NULL if it fails.
//
//    FIND THE FILE:
//       1. If the filename already has an embedded directory component, it
//	    must exist in the specified location.
//
//	 2. If the filename does not have an embedded directory component, and
//	    if "directory" exists, it points to the first place to look for
//	    the filename.
//
//	 3. If it isn't in "directory", just look for "filename" without any
//	    directory.
   
    char full_filename[256];

    strcpy( full_filename, filename );

    if ( filename_has_directory( filename ) )
    {
        // 1. If filename includes a directory, it had better be in there!
        if ( !file_exists( filename ) )
        {
            buff[0] = '\0';
            return NULL;
        }
        else
        {
            strcpy( buff, filename );
            return buff;
        }
    }
    else
    {
        // 2. If the filename does not include a directory, add it if given
        if ( !NO_STRING( directory ) )
        {
            // add the directory and try it
            strcpy( full_filename, directory );
            strcat( full_filename, "/" );
            strcat( full_filename, filename );
            if ( file_exists( full_filename ) )
            {
                strcpy( buff, full_filename );
                return buff;
            }
        }

        // 3. Finally, see if the file exists in the current directory
        if ( file_exists( filename ) )
        {
            strcpy( buff, filename );
            return buff;
        }
        else
        {
            buff[0] = '\0';
            return NULL;
        }
    }

    return NULL;
}



/**********************************************************************
 **       filename_has_directory       **
 ****************************************/

// Checks for the presense of a directory in the filename.
// Returns TRUE if present, FALSE, if not.
bool filename_has_directory( const char* szFileName )
{
   return ( strpbrk( szFileName, "/" ) != NULL );
}

// Checks for the presense of a directory in the filename.
// Returns TRUE if present, FALSE, if not.
bool filename_has_directory( const string& strFileName )
{
    return filename_has_directory( strFileName.c_str() );
}



/**********************************************************************
 **         extract_directory          **
 ****************************************/

// Returns the directory part of the filename, if it is present, or
//   NULL if it is not present.
//   The caller must provide the string space in "buff".
char* extract_directory( const char* filename, char* buff )
{
    char* end;

    // Return NULL if there is no directory part
    if ( !filename_has_directory( filename ) )
    {
        buff[0] = '\0';
        return NULL;
    }

    // Copy the directory
    strcpy( buff, filename );
    end = strrchr( buff, '/' );
    end[0] = '\0';

    return buff;
}

// Returns the directory part of the filename if it is present, or
// an empty string if it is not present.
string extract_directory( const string& strFileName )
{
    int iPos = -1;
    string strDirectory = "";

    // Return NULL if there is no directory part
    if ( ( iPos = strFileName.rfind( '/' ) ) > -1 )
    {
        // Copy the directory, getting rid of the last '/' and anything following
        strDirectory = strFileName;
        strDirectory.erase( iPos );

        // if we were in the root directory, we'll have an empty string, so 
        // set it to "/"
        if ( strDirectory == "" )
        {
            strDirectory = "/";
        }
    }

    return strDirectory;
}

/**********************************************************************
 **          remove_directory          **
 ****************************************/

// Returns just the simple filename part of the filename ( ie, removes the
// directory component. )  If no directory is present, it just copies the
// filename into the string "buff" and returns it.  The caller must provide
// the string space in "buff".
char* remove_directory( const char* filename, char* buff )
{
    const char* str;

    if ( filename_has_directory( filename ) )
    {
        str = strrchr( filename, '/' );
        str++;
        strcpy( buff, str );
    }
    else
    {
        strcpy( buff, filename );
    }

    return buff;
}

// Returns just the simple filename part of the filename ( ie, removes the
// directory component. )  If no directory is present, it just copies the
// filename into the string "buff" and returns it.  The caller must provide
// the string space in "buff".
string remove_directory( const string& strFileName )
{

    int iPos = -1;
    string strNoDir = strFileName;

    // get the last '/' to see if there's a directory
    if ( ( iPos = strFileName.rfind( '/' ) ) > -1 )
    {
        strNoDir.erase( 0, iPos + 1 );
    }

    return strNoDir;
}

/**********************************************************************
 **         extract_extension          **
 ****************************************/

// Returns the file extension part of the filename.  If none is present,
// return NULL.  The caller must provide the string space in "buff".
char* extract_extension( const char* filename, char* buff )
{
    const char* ext;

    // find the last period
    if ( ( ext = strrchr( filename, '.' ) ) == NULL )
    {
        buff[0] = '\0';
        return NULL;
    }

    // Copy the extension
    strcpy( buff, &( ext[1] ) );

    return buff;
}

//-----------------------------------------------------------------------
// This function tries to finds the file and returns the full pathname to
// it.
//-----------------------------------------------------------------------
string find_file_in_envpath(const string filename)
{
    FILE *file = NULL;
    string fullFilename;
    char *homeDir = NULL;
    char **dirList = NULL;
    int index;  
    bool fileFound = false;
            
    
    // Check current directory first.
    fullFilename = "./";
    fullFilename += filename;

    file = fopen(fullFilename.c_str(),"r");

    if (file != NULL)
    {        
	fclose(file);
	return fullFilename;
    } 

    // Check user's home directory next
    homeDir = getenv("HOME");
    if (homeDir != NULL)
    {
	fullFilename = homeDir;
	fullFilename += "/";
	fullFilename += filename;

	//free(homeDir);
	//homeDir = NULL;

	file = fopen(fullFilename.c_str(),"r");

	if (file != NULL)
	{        
	    fclose(file);
	    return fullFilename; 
	} 
    }

    // As last resort, follow the PATH directory list to see if is in the
    // directory this executable was ran from.
    dirList = extract_directories("PATH");

    if (dirList != NULL)
    {
	index = 0;
	fileFound = false;

	// Check all the directories
	while (dirList[index] != NULL)
	{
	    fullFilename = dirList[index];
	    fullFilename += "/";
	    fullFilename += filename;
	    
	    file = fopen(fullFilename.c_str(),"r");

	    if (file != NULL)
	    {
		fclose(file);
		fileFound = true;
		break;
	    }

   	    index++;
	}

	// Clean up.
	while (dirList[index] != NULL)
	{
	    free(dirList[index]);
	    dirList[index] = NULL;
	}

	dirList = NULL;

	if (fileFound)
	{
	    return fullFilename;
	}
    }

    // File not found.
    return FILE_UTILS_EMPTY_STRING;
}

/**********************************************************************
 * $Log: file_utils.cc,v $
 * Revision 1.1.1.1  2008/07/14 16:44:25  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/20 17:17:52  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:38:01  endo
 * MissionLab 7.0
 *
 * Revision 1.2  2006/06/29 18:11:06  endo
 * FC5 upgrade
 *
 * Revision 1.1.1.1  2005/02/06 23:00:22  endo
 * AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
 *
 * Revision 1.3  2003/04/06 09:15:27  endo
 * Updated for CBR Wizard Prototype II.
 *
 * Revision 1.2  2002/04/04 18:37:44  blee
 * Rewrote filename_has_directory(), overloaded filename_has_directory(),
 * overloaded extract_directory(), and and overloaded remove_directory().
 *
 * Revision 1.1  2002/01/13 01:57:23  endo
 * Initial revision
 *
 * Revision 1.7  1997/02/12 05:42:50  zchen
 * *** empty log message ***
 *
 * Revision 1.6  1996/03/01  00:47:09  doug
 * *** empty log message ***
 *
 * Revision 1.5  1995/06/14  18:21:46  jmc
 * Added function find_full_filename().
 *
 * Revision 1.4  1995/06/09  21:43:54  jmc
 * Added extract_extension() function.
 *
 * Revision 1.3  1995/04/26  14:42:29  jmc
 * Added function remove_directory().
 *
 * Revision 1.2  1995/04/14  18:25:57  jmc
 * Changed include of doug.h to gt_std.h.
 *
 * Revision 1.1  1995/04/12  21:10:14  jmc
 * Initial revision
 **********************************************************************/
