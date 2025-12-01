/**********************************************************************
 **                                                                  **
 **                         string_utils.c                           **
 **                                                                  **
 **  This file implements a few routines for dealing with strings.   **
 **                                                                  **
 **  Written by:  Jonathan M. Cameron                                **
 **                                                                  **
 **  Copyright 1995 - 1997, 1999 - 2003 Georgia Tech Research        **
 **  Corporation Atlanta, Georgia  30332-0415                        **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: string_utils.cc,v 1.1.1.1 2008/07/14 16:44:25 endo Exp $ */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <string>

#include "string_utils.h"


/**********************************************************************
 **            string_equal            **
 ****************************************/

int string_equal(const char *str1,
		 const char *str2,
		 const int case_sensitive)

/* Compare the two strings (case insensitive by default).
   Returns TRUE if the two strings are equal and false if not. */
{
   /* Make sure we have strings */
   if ((str1 == NULL) || (str2 == NULL))
      return false;

   /* If the strings aren't the same length, they can't possibly be equal */
   int len = strlen(str1);
   if (len != (int)strlen(str2))
      return false;

   /* Compare the strings character by character */
   char ch1, ch2;
   int i;
   for (i=0; i<len; i++) {

      /* grab the next characters */
      ch1 = str1[i];
      ch2 = str2[i];

      /* If case insensitive, convert both to upper case */
      if (!case_sensitive) {
	 if (islower(ch1))
	    ch1 = toupper(ch1);
	 if (islower(ch2))
	    ch2 = toupper(ch2);
	 }

      /* Compare the characters, return false immediately if not equal */
      if (ch1 != ch2)
	 return false;
      }
   
   /* the comparison completed without detecting any difference,
   therefore, the strings must be equal */

   return true;
}



/**********************************************************************
 **            string_upcase           **
 ****************************************/

char *string_upcase(char *str)

/* Convert any lower case letters in str to upper case (in place).
   Returns str. */
{
   int i;

   /* Check for NULL arguments */
   if (str == NULL)
      return NULL;

   /* Convert to upper case */
   for (i=0; i<(int)strlen(str); i++)
      if (islower(str[i]))
	 str[i] = toupper(str[i]);

   return str;
   }
   


/**********************************************************************
 **          trim_white_space          **
 ****************************************/

char *trim_white_space(char *str)

/* Trim off any leading or trailing whitespace from str.
   RETURN: str.  */
{
   int i, j, start, len;

   /* Check for a null string */
   if (str == NULL)
      return NULL;
   if (strlen(str) == 0)
      return str;

   /* Kill any white space at the end of the string */
   i = strlen(str) - 1;
   while (isspace(str[i]) && (i>=0)) {
      str[i] = '\0';
      i--;
      };

   /* Kill any spaces at the beginning */
   i=0;
   start = 0;
   while (isspace(str[i]) && (i < (int)strlen(str))) {
      start = ++i;
      }

   /* Shift the string left to kill leading white space */
   if (start>0) {
      len = strlen(str);
      for (i=start, j=0; i<=len; i++, j++)
	 str[j] = str[i];
      }

   return str;
}






/**********************************************************************
 **           remove_comments          **
 ****************************************/

char *remove_comments(char *str,
		      char delimeter)

/* Remove any comment characters after the delimeter character.
   Remove the delimeter too. 
   RETURN: str. */
{
   char *d;

   /* Make sure we have a string */
   if (str == NULL) 
      return NULL;
   if (strlen(str) == 0)
      return NULL;

   /* Locate the delimeter, if any */
   if ((d = strchr(str, delimeter)) == NULL)
      return str;
   else {
      d[0] = '\0';
      return str;
      }
}



/**********************************************************************
 **           boolean_string           **
 ****************************************/

char *boolean_string(const int val,
		     char *str)

/* Returns string "TRUE" or "FALSE" depending on its argument.
   Copies the string into str and returns it. */
{
   if (val)
      strcpy(str, "TRUE");
   else
      strcpy(str, "FALSE");

   return str;      
}



/**********************************************************************
 **              pop_word              **
 ****************************************/

char *pop_word(char *str,
	       char *buff)

/* Pop the first word off the front of the string str, stripping off
   any adjacent white space off the word and the rest of the string.
   SUCCESS: Return the first word (in buff) and modify the string str.
   FAILURE: Return NULL if the string is zero length or NULL 
            (and the string str is not modified).
   SIDE-EFFECT: If successful, the string str is modified. */
{
   int i, old_len;
   char *start, *rest;

   /* initialize the buffer and check for dud str */
   if (buff == NULL)
      return NULL;
   if (str == NULL)
      return NULL;
   if (strlen(str) == 0)
      return NULL;
   buff[0] = '\0';

   /* find the start of the first token (skip white space) */
   old_len = strlen(str);
   i = 0;
   while (isspace(str[i]) && (i<old_len))
      i++;
   start = str+i;
   
   /* locate the end of the first word */
   while (!isspace(str[i]) && (i<old_len))
      i++;
   str[i] = '\0';

   /* copy the token to the buffer */
   strcpy(buff, start);

   /* check for the case that only one word is present */
   if (old_len == i) {
      str[0] = '\0';
      return buff;
      }

   /* locate the beginning of the rest of the string (skip white space) */
   i++;
   while (isspace(str[i]) && (i<old_len))
      i++;
   rest = str+i;

   /* shift the rest of the string left */
   i = 0;
   while (rest[i]) {
      str[i] = rest[i];
      i++;
      }
   str[i] = '\0';

   /* return the popped word */
   return buff;
}




/**********************************************************************
 **              pop_part              **
 ****************************************/

char *pop_part(char *str,
	       char *buff,
	       char delimeter)
/* Pop off the part of the string up to the indicated delimeter.
   If delimeter is not found, the entire string is returned as "part".
   Remove the part and the delimeter from the original string.
   SUCCESS: Return part (in buff) and modify string str.
   FAILURE: Return NULL if the string is zero length or NULL
            (and the string str is not modified).
   SIDE-EFFECT: If successful, the string str is modified. */

{
   char *rest;

   /* initialize the buffer and check for dud str */
   if (buff == NULL)
      return NULL;
   if (str == NULL)
      return NULL;
   if (strlen(str) == 0)
      return NULL;

   /* Locate the delimeter, if any */
   if ((rest = strchr(str, delimeter)) == NULL) {
      /* return str if delimeter isn't found */
      strcpy(buff, str);
      str[0] = '\0';
      }
   else {
      /* copy the delimeted part to buff */
      rest[0] = '\0';
      strcpy(buff, str);
      rest++;
      strcpy(str, rest);
      }

   return buff;
}

//-----------------------------------------------------------------------
// This function takes a string. If there is any "unwanted charactor", it
// will be replaced with a specified charactor.
//-----------------------------------------------------------------------
string replaceCharInString(
    const string originalString,
    const char unwantedChar,
    const char fillerChar)
{
    string replacedString;
    string::size_type index;
    
    replacedString = originalString;

    while (true)
    {
        index = replacedString.find(unwantedChar);

        if (index == string::npos)
        {
            break;
        }

        replacedString[index] = fillerChar;
    }

    return replacedString;
}

//-----------------------------------------------------------------------
// This function takes a string. If there is a target string, it
// will be replaced with a specified string.
//-----------------------------------------------------------------------
string replaceStringInString(
    const string originalString,
    const string unwantedString,
    const string fillerString)
{
    string subBufString, targetString;
    string bufStringFront, bufStringBack, filledString;
    string::size_type index;

    filledString = originalString;

    while (true)
    {
        index = filledString.find(unwantedString);

        if (index != string::npos)
        {
            bufStringFront = filledString.substr(0, index);
            bufStringBack = filledString.substr(index+unwantedString.size(), string::npos);
            filledString = bufStringFront;
            filledString += fillerString;
            filledString += bufStringBack;
        }
        else
        {
            break;
        }
    }

    return filledString;
}

//-----------------------------------------------------------------------
// This function takes a string. If there is any "unwanted charactor", it
// will be remove it.
//-----------------------------------------------------------------------
string removeCharInString(
    const string originalString,
    const char unwantedChar)
{
    string replacedString;
    string::size_type index;
    int erasePos;
    
    replacedString = originalString;

    while (true)
    {
        index = replacedString.find(unwantedChar);

        if (index == string::npos)
        {
            break;
        }

        erasePos = index;
        replacedString.erase(erasePos, erasePos+1);
    }

    return replacedString;
}

/**********************************************************************
 * $Log: string_utils.cc,v $
 * Revision 1.1.1.1  2008/07/14 16:44:25  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.2  2007/02/13 11:04:00  endo
 * replaceStringInString() now works recursively.
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
 * Revision 1.3  1997/02/12 05:42:50  zchen
 * *** empty log message ***
 *
 * Revision 1.2  1995/06/28  18:58:34  jmc
 * Added copyright messages.  Added RCS id and log.
 **********************************************************************/
