/**********************************************************************
 **                                                                  **
 **                          string_utils.h                          **
 **                                                                  **
 **  This file declares a few routines for dealing with strings.     **
 **                                                                  **
 **  Written by:  Jonathan M. Cameron                                **
 **                                                                  **
 **  Copyright 1995 - 2006 Georgia Tech Research Corporation         **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: string_utils.h,v 1.1.1.1 2008/07/14 16:44:25 endo Exp $ */

#ifndef STRING_UTILS_H
#define STRING_UTILS_H

#include <malloc.h>
#include <string>

using std::string;

/**********************************************************************
 **                                                                  **
 **                         inline functions                         **
 **                                                                  **
 **********************************************************************/

inline int NO_STRING(const char *str)
{
   return((!str) || (strlen(str) == 0));
}


inline char *COPY_STRING(const char *str)
{
   return strcpy((char *)malloc(strlen(str)+1), str);
}


/**********************************************************************
 **                                                                  **
 **                      Function Declarations                       **
 **                                                                  **
 **********************************************************************/

int string_equal(const char *str1,
		 const char *str2,
		 const int case_sensitive = false);
/* Compare the two strings (case insensitive by default).
   Returns true if the two strings are equal and false if not. */

/* Constants for the case_insensitive option: */
const int CASE_SENSITIVE = true;
const int CASE_INSENSITIVE = false;


char *string_upcase(char *str);
/* Convert any lower case letters in str to upper case (in place).
   RETURN: str. */


char *trim_white_space(char *str);
/* Trim off any leading or trailing whitespace from str (in place).
   RETURN: str.  */


char *remove_comments(char *str,
		      char delimeter);
/* Remove any comment characters after the delimeter character.
   Remove the delimeter too. 
   RETURN: str. */


char *boolean_string(const int val,
		     char *str);
/* Returns string "true" or "false" depending on its argument.
   Copies the string into str and returns it. */


char *pop_word(char *str,
	       char *buff);
/* Pop the first word off the front of the string str, stripping off
   any adjacent white space off the word and the rest of the string.
   SUCCESS: Return the first word (in buff) and modify the string str.
   FAILURE: Return NULL if the string is zero length or NULL 
            (and the string str is not modified).
   SIDE-EFFECT: If successful, the string str is modified. */


char *pop_part(char *str,
	       char *buff,
	       char delimeter);
/* Pop off the part of the string up to the indicated delimeter.
   Remove the part and the delimeter from the original string.
   SUCCESS: Return part (in buff) and modify string str.
   FAILURE: Return NULL if the string is zero length or NULL
            (and the string str is not modified).
   SIDE-EFFECT: If successful, the string str is modified. */

string replaceCharInString(
    const string originalString,
    const char unwantedChar,
    const char fillerChar);

string replaceStringInString(
    const string originalString,
    const string unwantedString,
    const string fillerString);

string removeCharInString(
    const string originalString,
    const char unwantedChar);

#endif  /* STRING_UTILS_H */


/**********************************************************************
 * $Log: string_utils.h,v $
 * Revision 1.1.1.1  2008/07/14 16:44:25  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/20 17:17:52  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:38:01  endo
 * MissionLab 7.0
 *
 * Revision 1.3  2006/06/18 19:41:46  endo
 * .cfgeditrc now accepts MLAB_HOME
 *
 * Revision 1.2  2005/07/16 08:50:45  endo
 * removeCharInString() added.
 *
 * Revision 1.1.1.1  2005/02/06 23:00:22  endo
 * AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
 *
 * Revision 1.2  2003/04/06 09:15:27  endo
 * Updated for CBR Wizard Prototype II.
 *
 * Revision 1.1  2002/01/13 01:57:23  endo
 * Initial revision
 *
 * Revision 1.3  1999/12/16 22:49:31  mjcramer
 * rh6 port
 *
 * Revision 1.2  1997/02/12  05:41:58  zchen
 * *** empty log message ***
 *
 * Revision 1.1  1995/06/09  21:28:21  jmc
 * Initial revision
 **********************************************************************/
