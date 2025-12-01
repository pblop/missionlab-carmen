/****************************************************************************
*                                                                           *
*                      gt_message_types.h                                   *
*                                                                           *
*                                                                           *
*  Written by: Douglas Mackenzie                                            *
*                                                                           *
*  Copyright 1995 - 2007, Georgia Tech Research Corporation                 *
*  Atlanta, Georgia  30332-0415                                             *
*  ALL RIGHTS RESERVED, See file COPYRIGHT for details.                     *
*                                                                           *
****************************************************************************/

/* $Id: gt_message_types.h,v 1.2 2009/01/27 20:25:39 zkira Exp $ */

#ifndef GT_MESSAGE_TYPES_H
#define GT_MESSAGE_TYPES_H

#include <values.h>
#include <iostream>
#include <cmath>

#include "gt_std.h"

using std::ostream;
using std::min;

/*******************************************************************/

extern char *ComVersion;

/*******************************************************************/
typedef struct robot_pos_id_t
{
    int id;
    double x;
    double y;
    double z;
    double heading;
};

typedef struct gt_Doorway
{
    float x1, y1;
    float x2, y2;
    double distFromRobot;
    int marked;
} gt_Doorway_t;

typedef struct hallway
{
    Point start;
    Point end;
    double width;
} hallway_t;

typedef struct DoorwayList {
    int num_doors;
    gt_Doorway_t *doors;
} DoorwayList;

typedef struct HallwayList {
    int num_hways;
    hallway_t *hways;
} HallwayList;

/* NOTE: The following three types (gt_Behavior, gt_Formation, and
         gt_Technique) have parallel id strings defined in the file
         include/gt_command.h.  Be sure to edit them if these are changed. */

typedef enum gt_Behavior {
	NO_BEHAVIOR =  0,
	START       =  1,
	MOVETO      =  2,
	TELEOPERATE =  3,
	SWEEPTO     =  4,
	FOLLOW      =  5,
	OCCUPY      =  6,
	STOP        =  7,
	QUIT        =  8,
	SET         =  9,
	PRINT       = 10
} gt_Behavior;


typedef enum gt_Formation {
	NO_FORMATION = 0,
	LINE         = 1,
	LINE_LEFT    = 2,
	LINE_RIGHT   = 3,
	WEDGE        = 4,
	WEDGE_LEFT   = 5,
	WEDGE_RIGHT  = 6,
	DIAMOND      = 7,
	COLUMN       = 8,
	SWEEP        = 9
} gt_Formation;


typedef enum gt_Technique {
	NO_TECHNIQUE = 0,
	TRAVELING = 1,
	TRAVELING_OVERWATCH = 2,
	BOUNDING_OVERWATCH = 3
} gt_Technique;

// IMPORTANT NOTE: If you change this structure, you MUST change the corresponding
// structure REC_SensorReading located in include/gt_ipt_msg_def.h
// and sensor_reading_t located in hardware_drivers/hserver/hserver_ipt.h
// For some reason, one gets cast to the other in (at least) hclient_get_obstacles
// so if you do not do this, BAD THINGS WILL HAPPEN!
typedef struct SensorReading {
    int type;
    int color;
    float angle;
    float reading;
} SensorReading;

typedef struct SensorReadings {
    int robot_id;
    int num_readings;
    SensorReading *readings;
} SensorReadings;

typedef struct gt_Id_and_v {
    int      id;
    gt_Point v;
    double heading;
} gt_Id_and_v;

class RobotReadings
{
public:
    int 		 num_readings;
    gt_Id_and_v* readings;

    RobotReadings( int num = 0 )
    {
        num_readings = 0;
        readings = NULL;

        if ( num > 0 )
        {
            num_readings = num;
            readings = new gt_Id_and_v[ num ];
        }
    }

    RobotReadings( const RobotReadings& list )
    {
        num_readings = 0;
        readings = NULL;

        *this = list;
    }

    /* Assignment operator.  We have to really careful about this memory stuff. */
    RobotReadings& operator=( const RobotReadings& list )
    {
        if ( this != &list )
        {
            cleanup();

            if( list.num_readings > 0 )
            {
                num_readings = list.num_readings;
                readings = new gt_Id_and_v[ num_readings ];
                for( int i = 0; i < num_readings; i++ )
                {
                    readings[ i ].v.x = list.readings[ i ].v.x;
                    readings[ i ].v.y = list.readings[ i ].v.y;
                    readings[ i ].v.z = list.readings[ i ].v.z;
                    readings[ i ].heading = list.readings[ i ].heading;
                    readings[ i ].id = list.readings[ i ].id;
                }
            }
        }

        return *this;
    }

    /* Important:  We need to delete the allocated memory when we fall out of scope. */
    ~RobotReadings()
    {
        cleanup();
    }

protected:

    void cleanup()
    {
        num_readings = 0;
        if ( readings != NULL )
        {
            delete [] readings;
            readings = NULL;
        }
    }
};


class Obs_List
{
public:
    int       num_obs;
    gt_Point* obs;
    float*    r;

    /* Regular old boring constructor */
    Obs_List( int num = 0 )
    {
        num_obs = 0;
        obs = NULL;
        r = NULL;


        if ( num > 0 )
        {
            num_obs = num;
            obs = new gt_Point[ num ];
            r = new float[ num ];
        }
    }

    /* Copy constructor
       We need this because it will get called when Obs_List is used as a return value and we don't want
       to do a dumb copy of pointers.  We need to make the copy allocate it's own space. */
    Obs_List( const Obs_List& list )
    {
        num_obs = 0;
        obs = NULL;
        r = NULL;

        *this = list;
    }

    /* Assignment operator.  We have to really careful about this memory stuff. */
    Obs_List& operator=( const Obs_List& list )
    {
        if ( this != &list )
        {
            cleanup();

            if ( list.num_obs > 0 )
            {
                num_obs = list.num_obs;
                obs = new gt_Point[ list.num_obs ];
                r = new float[ list.num_obs ];

                for( int i = 0; i < list.num_obs; i++ )
                {
                    obs[ i ].x = list.obs[ i ].x;
                    obs[ i ].y = list.obs[ i ].y;
		    obs[ i ].z = list.obs[ i ].z;
                    r[ i ] = list.r[ i ];
                }
            }
        }

        return *this;
    }

    /* Important:  We need to delete the allocated memory when we fall out of scope. */
    ~Obs_List()
    {
        cleanup();
    }

protected:

    void cleanup()
    {
        num_obs = 0;
        if ( obs != NULL )
        {
            delete [] obs;
            obs = NULL;
        }
        if ( r != NULL )
        {
            delete [] r;
            r = NULL;
        }
    }
};


typedef enum ObjectShapes
{
	CircleObject,  // Circles are centered at x1,y1 with radius r
	LineObject,    // Lines have one vertex at x1,y1 and the second at x2,y2
	PointObject    // Points have their center at x1,y1
} ObjectShapes;

const int NOID = MAXINT;



//  Class to hold readings of various objects
struct ObjectReading
{
    char*        objcolor;
    int          id;    // or NOID if not applicable or not available
    ObjectShapes objshape;
    float        x1;
    float        y1;
    float        r;
    float        x2;
    float        y2;
    bool         ismovable;

    // Please use this function when setting color.  DON'T use strdup, you'll have problems...
    void setColor( char* color )
    {
        objcolor = color;
    }

    gt_Point center()  {
        gt_Point rtn;
        rtn.x = x1;
        rtn.y = y1;

        return rtn;
    }

    gt_Point closest_point()
    {
        gt_Point rtn;
        rtn.x = GT_INFINITY;
        rtn.y = GT_INFINITY;

        switch(objshape)
        {
        case LineObject:
        {
            rtn.x = x1;
            rtn.y = y1;
            double dist = vlength(x1, y1);

            double new_dist = vlength(x2, y2);
            if( new_dist < dist )
            {
                dist = new_dist;
                rtn.x = x2;
                rtn.y = y2;
            }

            // create a unit vector along the line
            double Ux = x2 - x1;
            double Uy = y2 - y1;
            double length = vlength(Ux,Uy);
            Ux /= length;
            Uy /= length;

            // create a vector from the x,y location to the 1st point
            double Vx = - x1;
            double Vy = - y1;

            // calculate the dot product, to see if is along the line segment
            // recall: dot product gives parallel component of resultant vector
            double h = Ux * Vx + Uy * Vy;

            if (h < 0 || h > length)
            {
                /* The perpendicular from the point to the line does
                   not intersect the line between the two end points */
            }
            else
            {
                // now calculate the distance via the cross product.
                // recall cross product gives perpendicular component of
                // resultant vector
                new_dist = fabs(Ux * Vy - Uy * Vx);
                if( new_dist < dist )
                {
                    // Closest point is "h" units along unit vector
                    rtn.x = Ux * h + x1;
                    rtn.y = Uy * h + y1;
                }
            }
        }
        break;

        case CircleObject:
        {
            double sqr_dist = x1*x1 + y1*y1;

            // Kick out case where points are coincident
            if( sqr_dist <= 0 )
            {
                rtn.x = 0;
                rtn.y = 0;
            }
            else
            {
                double dist = sqrt(sqr_dist) - r;

                // Kick out case where point is inside the circle
                if( dist <= 0 )
                {
                    rtn.x = 0;
                    rtn.y = 0;
                }
                else
                {
                    // compute closest point on edge of circle

                    // create a unit vector along the line
                    double Vx = x1;
                    double Vy = y1;
                    double length = vlength(Vx,Vy);
                    rtn.x = Vx * dist / length;
                    rtn.y = Vy * dist / length;
                }
            }
        }
        break;

        case PointObject:
        {
            double sqr_dist = x1*x1 + y1*y1;

            // Kick out case where points are coincident
            if( sqr_dist <= 0.001 )
            {
                rtn.x = 0;
                rtn.y = 0;
            }
            else
            {
                double dist = sqrt(sqr_dist);

                // create a unit vector along the line
                double Vx = x1;
                double Vy = y1;
                double length = vlength(Vx,Vy);
                rtn.x = Vx * dist / length;
                rtn.y = Vy * dist / length;
            }
        }
        break;
        }
        return rtn;
    };

    double distance(double x, double y)
    {
        switch(objshape)
        {
        case LineObject:
        {
            double dist = vlength(x1 - x, y1 - y);
            dist = min(dist, vlength(x2 - x, y2 - y));

            // create a unit vector along the line
            double Ux = x2 - x1;
            double Uy = y2 - y1;
            double length = vlength(Ux,Uy);
            Ux /= length;
            Uy /= length;

            // create a vector from the x,y location to the 1st point
            double Vx = x - x1;
            double Vy = y - y1;

            // calculate the dot product, to see if is along the line segment
            // recall: dot product gives parallel component of resultant vector
            double h = Ux * Vx + Uy * Vy;

            if (h < 0 || h > length)
            {
                /* The perpendicular from the point to the line does
                   not intersect the line between the two end points */
            }
            else
            {
                // now calculate the distance via the cross product.
                // recall cross product gives perpendicular component of
                // resultant vector
                dist = min(dist, fabs(Ux * Vy - Uy * Vx));
            }

            // return distance from line to the point
            return dist;
        }

        case CircleObject:
        {
            double dx = x1 - x;
            double dy = y1 - y;
            double sqr_dist = dx*dx + dy*dy;

            // Kick out case where points are coincident
            if( sqr_dist <= 0 )
                return 0;

            double dist = sqrt(sqr_dist) - r;

            // Kick out case where point is inside the circle
            if( dist <= 0 )
                return 0;

            // return distance from edge of circle to the point
            return dist;
        }

        case PointObject:
        {
            double dx = x1 - x;
            double dy = y1 - y;
            double sqr_dist = dx*dx + dy*dy;

            // Kick out case where points are coincident
            if( sqr_dist <= 0.001 )
                return 0;

            return sqrt(sqr_dist);
        }
        }

        /* Not reached */
        return 0;
    };

friend ostream& operator << (ostream& s, ObjectReading &r);
};



class ObjectList
{
public:
    int count;
    ObjectReading* objects;

    // constructor: create objects
    ObjectList( int size = 0 )
    {
        objects = NULL;
        count = 0;

        if ( size > 0 )
        {
            objects = new ObjectReading[ size ];
            count = size;
        }
    }
    // copy constructor
    ObjectList(const ObjectList& a)
    {
        objects = NULL;
        count = 0;

        *this = a;
    }

    // assignment: cleanup and copy
    ObjectList& operator=(const ObjectList& a)
    {
        if( this != &a )
        {
            cleanup();

            if ( a.count > 0 )
            {
                objects = new ObjectReading[ a.count ];
                count = a.count;
                for( int i=0; i<a.count; i++ )
                {
                    objects[i] = a.objects[ i ];
                }
            }
        }
        return *this;
    }

    // destructor: delete objects
    ~ObjectList()
    {
        cleanup();
    }

protected:

    void cleanup()
    {
        count = 0;
        if ( objects != NULL )
        {
            delete [] objects;
            objects = NULL;
        }
    }
};



typedef struct gt_Command
{
    int          num_path_points;
	gt_Point*    path;

	int          unit_size;
	int*         unit;

	int          num_pl_points;
	gt_Point*    phase_line;

	gt_Point     goal;
	gt_Behavior  behavior;
	gt_Formation formation;
	gt_Technique technique;
	float        speed;
	float        width;
	int          phase_line_given;
	int          time;    /* Seconds since epoch (see 'man 3 time').
					       0 means time is not a factor (no phase line).
					       Negative means seconds for timeout. */
	char*        phase_line_ack_msg;
	int          wait_at_phase_line;
	int          completion_given;
	char*        completion_msg;
	int          freeze_on_completion;
	int          until_timeout_given;
	int          robot_id;
	int          seq_num;  /* unique command sequence number */

} gt_Command;



#endif  /* GT_TCX_TYPE_DEF_H */


/**********************************************************************
 * $Log: gt_message_types.h,v $
 * Revision 1.2  2009/01/27 20:25:39  zkira
 * Bugfix: Changed SensorReading structures to correspond to that in hserver/hserver_ipt.h
 *
 * Revision 1.1.1.1  2008/07/14 16:44:20  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.2  2007/08/30 18:36:57  endo
 * exec_display_bitmap_icon() added.
 *
 * Revision 1.1.1.1  2006/07/20 17:17:48  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:37:57  endo
 * MissionLab 7.0
 *
 * Revision 1.6  2006/06/08 22:33:55  endo
 * CommBehavior from MARS 2020 migrated.
 *
 * Revision 1.5  2006/05/14 06:23:36  endo
 * gcc-3.4 upgrade
 *
 * Revision 1.4  2006/01/10 06:22:00  endo
 * AO-FNC Type-I check-in.
 *
 * Revision 1.3  2005/04/08 01:55:56  pulam
 * addition of cnp behaviors
 *
 * Revision 1.2  2005/03/23 07:36:53  pulam
 * *** empty log message ***
 *
 * Revision 1.1.1.1  2005/02/06 22:59:46  endo
 * AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
 *
 * Revision 1.7  2003/04/06 11:54:35  endo
 * gcc 3.1.1
 *
 * Revision 1.6  2000/12/02 21:28:53  sgorbiss
 * Fixed bug
 *
 * Revision 1.5  2000/11/03 20:48:41  blee
 * Fixed possible memory leaks.
 *
 * Revision 1.4  2000/07/02 01:08:18  conrad
 * added sensor readings
 *
 * Revision 1.3  2000/02/28 23:52:39  jdiaz
 * doorway struct added field marked
 *
 * Revision 1.2  2000/02/07 05:20:44  jdiaz
 * doorways walls hallways rooms
 *
 * Revision 1.1  1999/12/16 23:00:23  mjcramer
 * Initial revision
 *
 * Revision 1.39  1999/07/03 17:46:46  endo
 * The 3D thing disabled because it was having problem
 * showing obstacles on the mlab-console.
 *
 * Revision 1.38  1999/06/29 02:51:41  mjcramer
 * Added 3D functions
 *
 * Revision 1.37  1999/05/17 16:59:09  jdiaz
 * added suppport for displaying the current compass reading
 *
 * Revision 1.36  1997/02/12 05:41:58  zchen
 * *** empty log message ***
 *
 * Revision 1.35  1996/06/02  16:43:39  doug
 * *** empty log message ***
 *
 * Revision 1.34  1996/05/05  16:11:25  doug
 * added a return to remove a warning
 *
 * Revision 1.33  1996/03/20  00:00:19  doug
 * *** empty log message ***
 *
 * Revision 1.32  1996/03/09  01:08:43  doug
 * *** empty log message ***
 *
 * Revision 1.31  1996/02/28  03:54:51  doug
 * *** empty log message ***
 *
 * Revision 1.30  1996/02/27  05:00:04  doug
 * *** empty log message ***
 *
 * Revision 1.29  1996/02/22  00:56:22  doug
 * *** empty log message ***
 *
 * Revision 1.28  1996/02/20  16:47:27  doug
 * made detect_objects work on the dennings
 *
 * Revision 1.27  1996/02/08  19:24:07  doug
 * handle objects of any color
 *
 * Revision 1.26  1995/11/07  14:31:57  doug
 * added baskets and command to drop oranges into them
 *
 * Revision 1.25  1995/10/12  20:13:34  doug
 * Added pickup function
 *
 * Revision 1.24  1995/10/11  22:05:34  doug
 * *** empty log message ***
 *
 * Revision 1.23  1995/10/11  20:36:33  doug
 * *** empty log message ***
 *
 * Revision 1.22  1995/10/09  19:59:13  doug
 * added distance member function to reading class
 *
 * Revision 1.21  1995/10/09  15:23:11  doug
 * change ObjectClasses to ObjectColors
 *
 * Revision 1.20  1995/10/03  21:25:06  doug
 * add support for get_objects call
 *
 * Revision 1.19  1995/10/03  20:32:00  doug
 * Added the detect_objects messages
 *
 * Revision 1.18  1995/04/26  17:55:25  jmc
 * Added the TELEOPERATE behavior enum to the list for gt_Behavior.
 *
 * Revision 1.17  1995/04/20  18:23:09  jmc
 * Corrected spelling.
 *
 * Revision 1.16  1995/04/03  20:34:33  jmc
 * Added copyright notice.
 *
 * Revision 1.15  1995/02/15  16:28:14  jmc
 * Updated gt_Behavior to add the SWEEPTO command and gt_Formation to
 * add new types for two vehicle formations.
 *
 * Revision 1.14  1995/01/10  19:32:58  doug
 * add notify_config msg
 *
 * Revision 1.13  1994/11/22  20:55:14  doug
 * added ReportObstacles message
 *
 * Revision 1.12  1994/11/16  20:52:48  doug
 * combined get_xy and get_heading messages into get_location
 *
 * Revision 1.11  1994/11/04  03:34:32  doug
 * removed old style detect_obstacles
 *
 * Revision 1.10  1994/11/02  18:39:46  doug
 * added radius to get_obstacle_list info returned
 *
 * Revision 1.9  1994/11/01  23:40:47  doug
 * Name tcx socket with pid
 * handle robot error messages from bad version
 *
 * Revision 1.8  1994/10/28  20:13:39  jmc
 * Changed until_given to until_timeout_given in gt_Command.
 *
 * Revision 1.7  1994/10/28  16:58:55  jmc
 * Removed until_msg from gt_Command.
 *
 * Revision 1.6  1994/10/28  14:48:32  jmc
 * Added comment to gt_Command declaration.
 *
 * Revision 1.5  1994/10/25  15:30:36  jmc
 * Added PRINT behavior type and switched the order to QUIT and SET.
 * Added #ifndef to prevent multiple inclusion problems.
 *
 * Revision 1.4  1994/10/11  17:10:48  doug
 * added seq field to gt_command record
 *
 * Revision 1.4  1994/10/11  17:10:48  doug
 * added seq field to gt_command record
 *
 * Revision 1.3  1994/10/10  15:44:44  jmc
 * Minor tweak: Added a comment.
 *
 * Revision 1.2  1994/10/07  21:16:51  jmc
 * Cleaned up some minor things and added comments.  No real change.
 *
 * Revision 1.1  1994/10/03  21:14:10  doug
 * Initial revision
 *
 * Revision 1.1  1994/09/28  20:46:09  doug
 * Initial revision
 *
 * Revision 1.13  1994/09/03  13:33:06  doug
 * Added defines for messages returned from robots
 *
 * Revision 1.12  1994/08/22  16:38:33  doug
 * Added RSP_MSG define
 *
 * Revision 1.11  1994/08/22  15:52:39  doug
 * Removed command packet code since now in rpc header
 *
 * Revision 1.10  1994/08/19  17:08:54  doug
 * rpc is working
 *
 * Revision 1.9  1994/08/18  20:04:56  doug
 * moved gt_std.h doug.h
 *
 * Revision 1.8  1994/08/18  19:33:43  doug
 * moved gt_std.h to doug.h
 *
 * Revision 1.7  1994/08/17  22:55:09  doug
 * added link to rpc include
 *
 * Revision 1.6  1994/08/16  19:23:00  jmc
 * Added type for gt_Point_array.  Updated definition of gt_Command
 * to add more phase line info, on-completion info, and "until" info.
 *
 * Revision 1.5  1994/08/12  14:36:46  jmc
 * Added speed to gt_Command and fixed_gt_Command.
 *
 * Revision 1.4  1994/08/11  19:36:25  doug
 * new_robot now returns the id that was assigned.
 *
 * Revision 1.3  1994/07/14  13:40:58  jmc
 * Added extern "C"'s on several function prototypes
 *
 * Revision 1.2  1994/07/14  13:20:08  jmc
 * Added RCS automatic id and log strings
 **********************************************************************/
