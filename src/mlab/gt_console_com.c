/****************************************************************************
*                                                                           *
*                                                                           *
*                      Doug MacKenzie                                       *
*                                                                           *
* gt_console_com - manage console side of communication between operator    *
*                  console and robots                                       *
*                                                                           *
*    Copyright 1995, 1996, 1997 Georgia Tech Research Corporation           *
*    Atlanta, Georgia  30332-0415                                           *
*    ALL RIGHTS RESERVED, See file COPYRIGHT for details.                   *
*                                                                           *
****************************************************************************/

/* $Id: gt_console_com.c,v 1.1.1.1 2008/07/14 16:44:22 endo Exp $ */

#include <X11/Xlib.h>
#include <X11/Intrinsic.h>
#include <stdio.h>
#include <malloc.h>

#include "gt_simulation.h"
#include "gt_com.h"
#include "gt_sim.h"
#include "console.h"
#include "gt_command.h"
#include "console_side_com.h"
// ENDO - gcc 3.4
/*
extern "C" {
  void exit(int);
}
*/

/*
 * The symbol table is layed out as follows:
 *
 * The SymbolTable array holds the base records for each symbol name.
 * That is, it holds the records broadcast to all robots (robot_id = -1).
 * The specific_robots pointer points at records which supercede the default value.
 *
 * To write a broadcast value, copy it to the base record and free all
 * specific robot ID records.
 *
 * To write a specific robot ID value, insert a new record in ascending order
 * of robot_id in the specific_robots list.
 *
 * To read a specific robot ID value, follow the specific_robots list to check
 * if that robot has a particular value.  If not, check if the base record is
 * valid.
 *
 */


#define SymbolTableSize    1024    /* Size for symbol table (Closed hashing) */
struct SymbolTableEntry
{
   char *name;
   char *value;
   int   val_buf_len;
   int   valid;
   int   robot_id;
   struct SymbolTableEntry *specific_robots;
};

static struct SymbolTableEntry SymbolTable[SymbolTableSize];

/************************************************************************
*                                                                       *
*                         int Hash(char *)                              *
*                                                                       *
************************************************************************/

/*
 * This routine will return an integer index into the SymbolTable. The
 * location that is returned must be then checked.  If the name is NULL then
 * the symbol is not in the table and should be inserted in this position.  If
 * this is the symbol then it was found at this loc.  Return of -1 means error.
 */

static int Hash(char *str)
{
   int             val;
   int             loc;
   int             start;

   val = 0;
   loc = 0;
   while (str[loc] != 0)
   {
      val += str[loc++];
   }

   loc = (val * 3) % SymbolTableSize;
   start = loc;

   while (SymbolTable[loc].name != NULL && (strcmp(SymbolTable[loc].name, str) != 0) )
   {
      loc = (loc + 1) % SymbolTableSize;
      if (loc == start)
      {
	 /* Have gone through entire table and all locations are full! */
	 warn_user("Error: The robot communication hash symbol table is full. \n\
SymbolTableSize must be increased in the file gt_console_com.c\n");
	 return -1;
      }
   }

   return (loc);
}

/************************************************************************
*                                                                       *
*                                                                       *
************************************************************************/

static struct SymbolTableEntry *
FindSpecificRobotRec(struct SymbolTableEntry *base, int robot_id)
{
   struct SymbolTableEntry *cur;
   int                      i;

   if( base == NULL )
      return NULL;

   cur = base;
   i = 0;
   while( cur->specific_robots && cur->specific_robots->robot_id <= robot_id )
   {
      cur = cur->specific_robots;

      if( i++ > 10000 )
      {
	 fprintf(stderr,"Cycle in SymbolTable records\n");
	 exit(1);
      }
   }

   if( cur == NULL )
      cur = base;

   return cur;
}

/****************************************************************************
*                                                                           *
* int gt_update(int robotid,char *parm_name,char *value);                   *
*                                                                           *
* robotid:                                                                  *
*    BROADCAST_ROBOTID  : all robots                                        *
*    else               : a specific robot's id                             *
*                                                                           *
* parm_name:                                                                *
*    Any text string parameter name.  It is case and white space sensitive. *
*                                                                           *
* value:                                                                    *
*    Any text string.                                                       *
*                                                                           *
* Returns:                                                                  *
*    0: success                                                             *
*   -1: failure                                                             *
*                                                                           *
****************************************************************************/
/*
 * add the parameter to the symbol table if it isn't already there.
 * If it is found then update its value.
 */

// ENDO - gcc 3.4
//int
//gt_update(int robotid, char *parm_name, char *value)
int gt_update(int robotid, char *parm_name, const char *value)
{
   int                      loc;
   struct SymbolTableEntry *rec;
   struct SymbolTableEntry *cur;

   if( debug )
   {
      fprintf(stderr,"gt_update: robot %d parm '%s' =  '%s'\n",robotid,parm_name,value);
   }

   if( strcmp(parm_name,RSP_MSG) == 0 &&
       strncmp(value, DONE_MSG, strlen(DONE_MSG)) == 0)
   {
      some_command_completed ++;
   }

   if( (loc = Hash(parm_name)) < 0)
   {
      fprintf(stderr,"gt_update: Unable to hash parameter name '%s'\n",parm_name);
      return -1;
   }

   if (SymbolTable[loc].name == NULL)
   {
      /* Not in table so add string */
      SymbolTable[loc].name = strdup(parm_name);
      SymbolTable[loc].robot_id = -1;
      SymbolTable[loc].specific_robots = NULL;
   }

   if( robotid == -1 )
   {
      /* free any robot specific records */
      cur = SymbolTable[loc].specific_robots;
      while( cur )
      {
         rec = cur->specific_robots;

         if( cur->value )
            free(cur->value);

         free(cur);

         cur = rec;
      }
      SymbolTable[loc].specific_robots = NULL;

      /* mark to update this record */
      rec = &SymbolTable[loc];
   }
   else
   {
      cur = FindSpecificRobotRec(&SymbolTable[loc], robotid);
      if( cur->robot_id != robotid )
      {
	 /* add one at this spot */
         rec=(struct SymbolTableEntry *)calloc(sizeof(struct SymbolTableEntry),1);
	 if( rec == NULL )
	 {
	    fprintf(stderr,"gt_update: Unable to allocate memory for robot record '%s'\n",parm_name);
	    return -1;
	 }

         rec->specific_robots = cur->specific_robots;
         cur->specific_robots = rec;

         rec->value = NULL;
      }
      else
      {
	 rec = cur;
      }
   }

   rec->robot_id = robotid;
   rec->valid = TRUE;

   if(rec->value != NULL)
	   free(rec->value);

   rec->val_buf_len = strlen(value) + 1;
   rec->value = strdup(value);

   /* send new value to the robot(s) */
   // ENDO - gcc 3.4
   //sim_put_state(robotid, parm_name, value);
   sim_put_state(robotid, parm_name, (char *)value);

   return 0;
}


/****************************************************************************
*                                                                           *
* int gt_delete(int robotid,char *parm_name);                                  *
*                                                                           *
* robotid:                                                                  *
*    -1  : all robots                                                       *
*    else: a specific robot's id                                            *
*                                                                           *
* parm_name:                                                                *
*    Any text string parameter name.  It is case and white space sensitive. *
*                                                                           *
* Returns:                                                                  *
*    0: success                                                             *
*   -1: failure                                                             *
*                                                                           *
****************************************************************************/
/*
 * gt_delete the parameter from the symbol table if it is there.
 */

int
gt_delete(int robotid, char *parm_name)
{
   int             loc;
   struct SymbolTableEntry *cur;
   struct SymbolTableEntry *rec;


   if( (loc = Hash(parm_name)) < 0)
   {
      fprintf(stderr,"gt_delete: Unable to hash parameter name '%s'\n",parm_name);
      return -1;
   }

   cur = &SymbolTable[loc];
   if( robotid == -1 )
   {
      /* skip the main record */
      cur = cur->specific_robots;
      while( cur )
      {
         rec = cur;
         cur = cur->specific_robots;

         /* free the record */
         if( rec->specific_robots && rec->specific_robots->value != NULL )
         {
            free(rec->specific_robots->value);
         }
         free(rec);
      }

      /* free the main record */
      if( SymbolTable[loc].value)
         free( SymbolTable[loc].value );
      if( SymbolTable[loc].name)
         free( SymbolTable[loc].name );
      SymbolTable[loc].specific_robots = NULL;
      SymbolTable[loc].name = NULL;
      SymbolTable[loc].value = NULL;
      SymbolTable[loc].valid = FALSE;
   }
   else
   {
      /* run down the chain of robot specific records and see if it is here */
      while(cur->specific_robots && cur->specific_robots->robot_id <= robotid )
      {
         cur = cur->specific_robots;
      }

      if( cur->specific_robots && cur->specific_robots->robot_id == robotid )
      {
         /* found it */

         rec = cur;
         cur = cur->specific_robots;

         /* remove from chain */
         rec->specific_robots = rec->specific_robots->specific_robots;

         /* free the record and its data */
         if( cur->value != NULL )
         {
            free(cur->value);
         }
         free(cur);
      }
   }

   return 0;
}



/****************************************************************************
*                                                                           *
* char *gt_inquire(int robotid,char *parm_name);                               *
*                                                                           *
* robotid:                                                                  *
*    -1  : all robots (dealer's choice)                                     *
*    else: a specific robot's id                                            *
*                                                                           *
* parm_name:                                                                *
*    Any text string parameter name.  It is case and white space sensitive. *
*                                                                           *
* Returns:                                                                  *
*    NULL: failure                                                          *
*    else: pointer to the string value                                      *
*                                                                           *
****************************************************************************/

char *
gt_inquire(int robotid, char *parm_name)
{
   int    loc;
   char  *rtn = NULL;
   struct SymbolTableEntry *cur;

   if( (loc = Hash(parm_name)) < 0)
   {
      fprintf(stderr,"gt_inquire: Unable to hash parameter name '%s'\n",parm_name);
      return NULL;
   }


   if (SymbolTable[loc].name != NULL)
   {
      cur = FindSpecificRobotRec(&SymbolTable[loc], robotid);
      if( cur->robot_id == robotid )
      {
         rtn = cur->value;
      }
      else
      {
         rtn = SymbolTable[loc].value;
      }
   }

   if( debug )
   {
      fprintf(stderr,"gt_inquire: robot %d parm '%s' -> '%s'\n",robotid,parm_name,rtn);
   }

   if( rtn == NULL )
      rtn = "";

   return rtn;
}




/****************************************************************************
*                                                                           *
* char *gt_get_value(int robotid,char *parm_name);                          *
*                                                                           *
* read and delete a parameter value from the database                       *
*                                                                           *
* robotid: the specific robot's id                                          *
*                                                                           *
* parm_name:                                                                *
*    Any text string parameter name.  It is case and white space sensitive. *
*                                                                           *
* Returns:                                                                  *
*    NULL: failure                                                          *
*    else: pointer to the string value                                      *
*                                                                           *
****************************************************************************/

char *
gt_get_value(int robotid, char *parm_name)
{
   int    loc;
   char  *rtn = NULL;
   struct SymbolTableEntry *cur;

   if( (loc = Hash(parm_name)) < 0)
   {
      fprintf(stderr,"gt_get_value: Unable to hash parameter name '%s'\n",parm_name);
      return NULL;
   }

   if (SymbolTable[loc].name != NULL)
   {
      cur = FindSpecificRobotRec(&SymbolTable[loc], robotid);
      if( cur->robot_id == robotid )
      {
         rtn = cur->value;
         cur->value = NULL;
      }
   }

   if( debug )
   {
      fprintf(stderr,"gt_get_value: robot %d parm '%s' -> '%s'\n",robotid,parm_name,rtn);
   }

   return rtn;
}




/**********************************************************************
 * $Log: gt_console_com.c,v $
 * Revision 1.1.1.1  2008/07/14 16:44:22  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/20 17:17:50  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:37:59  endo
 * MissionLab 7.0
 *
 * Revision 1.2  2006/05/15 01:50:11  endo
 * gcc-3.4 upgrade
 *
 * Revision 1.1.1.1  2005/02/06 23:00:10  endo
 * AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
 *
 * Revision 1.20  1997/02/12  05:42:50  zchen
 * *** empty log message ***
 *
 * Revision 1.19  1996/03/13  03:29:38  doug
 * Fixed problem in gt_delete where it didn't watch out for NULL pointers
 *
 * Revision 1.18  1995/04/20  18:05:07  jmc
 * Corrected spelling.
 *
 * Revision 1.17  1995/04/03  20:09:39  jmc
 * Added copyright notice.
 *
 * Revision 1.16  1995/02/23  17:11:31  doug
 * fixed comments
 *
 * Revision 1.15  1994/11/14  21:39:16  doug
 * gt_delete deletes the wrong value record
 *
 * Revision 1.14  1994/10/18  20:55:53  doug
 * fixing prototypes
 *
 * Revision 1.13  1994/10/06  20:29:26  doug
 * fixing execute
 *
 * Revision 1.12  1994/10/05  22:29:11  doug
 * working on executive
 *
 * Revision 1.11  1994/10/05  18:50:32  doug
 * fix keypress stuff
 *
 * Revision 1.10  1994/10/04  22:20:50  doug
 * Getting execute to work
 *
 * Revision 1.9  1994/09/06  14:03:23  jmc
 * Modified debugging message in gt_update.
 *
 * Revision 1.8  1994/09/03  17:02:56  doug
 * Getting execution to work
 *
 * Revision 1.7  1994/08/19  17:08:16  doug
 * rpc is working
 *
 * Revision 1.6  1994/08/18  20:03:50  doug
 * moved gt_std.h to doug.h
 *
 * Revision 1.5  1994/08/04  14:18:48  jmc
 * Modified includes because prototypes for warn_user... were
 * moved to console.h.
 *
 * Revision 1.4  1994/07/25  16:53:24  jmc
 * Converted error messages over to use the warn_user
 * function (which pops up an error dialog box).
 *
 * Revision 1.3  1994/07/12  19:14:59  jmc
 * Tweaked RCS automatic identification strings
 *
 * Revision 1.2  1994/07/12  19:05:33  jmc
 * Added RCS automatic identification strings
 **********************************************************************/
