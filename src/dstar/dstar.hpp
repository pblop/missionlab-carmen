/**********************************************************************
 **                                                                  **
 **                           dstar.hpp                              **
 **                                                                  **
 **                                                                  **
 **  Written by:  Ananth Ranganathan                                 **
 **                                                                  ** 
 **  The D*Lite algorithm was developed by Sven Koenig and Maxim     ** 
 **  Likhachev.                                                      **
 **                                                                  **
 **  This code adheres closely to the D*Lite pseudo-code in the      **
 **  paper:                                                          **
 **                                                                  **
 **  S. Koenig and M. Likhachev                                      **
 **  "Improved Fast Replanning for Robot Navigation in Unknown       **
 **  Terrain", ICRA-2002.                                            **
 **                                                                  **
 **  Copyright 2003, Georgia Tech Research Corporation               **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: dstar.hpp,v 1.1.1.1 2008/07/14 16:44:17 endo Exp $*/


#ifndef DSTAR_H
#define DSTAR_H

#include"pq.hpp"
#include"gt_simulation.h"
#include<vector>
#include<deque>
#include<utility>
#include<algorithm>
#include<cmath>
#include<iterator>
#include<ctime>
#include <string.h>	//Old compilation error: several functions not defined

//using namespace std;
using std::vector;
using std::deque;
using std::pair;


//Use own version of Point and not the one in gt_std.h

 struct Point2
{
    double x;
    double y;
    double z;
    Point2(double p,double q,double r): x(p),y(q),z(r) {}
    Point2() { Point2(0.0,0.0,0.0); }
    Point2(const Point2& p) { x=p.x; y=p.y; z=p.z; }
    Point2(const Point& p) { x=p.x; y=p.y; z=p.z; }
    Point2& operator=(Point2 p) { x=p.x; y=p.y; z=p.z; return *this; }
    Point2& operator=(Point p) { x=p.x; y=p.y; z=p.z; return *this; }
    bool operator==(Point2 p) { return x==p.x && y==p.y;}

};

//override Vector definition from gt_std.h

//#define Vector Point2

#define MOVEMENT_THRESHOLD             0.7
#define PLANNING_MOVEMENT_THRESHOLD    0.1
#define DISTANCEOUT                    200
#define PROGRESS_THRESHOLD              30
#define PLANNING_PROGRESS_THRESHOLD     70
#define MIN_PLANNING_DISTANCE            3
#define MAX_PLANNING_DISTANCE           62
//#define PO_START_THRES                   2
//#define LO_ANGLE_THRES                   5
#define HI_ANGLE_THRES                   5

extern int hclient_stop();
extern int run_type;

class Dstar 
{
    Pqueue* U;
    Graph* G;
    double gridsize, dist_to_last_planned, ao_val;
    int unreachable, counter;
    int mapping,find_dir;
    int stored_wall;
    int planning_only, planning_only_flag, planning_leave_flag;
    bool started, firsttime, planning_stuck, path_blocked;
    Point2 lastoutput, lastplanned, stuck_goal; 
    vector<Point2> obstacles; 
    vector<Point2> path;
    double acct_distance;
    int planning;
    Point2 lastpos;
    vector<double> angles, g_angles;
    int progress_counter;
    long po_start_time, po_time, start_time;

    int PO_START_THRES, LO_ANGLE_THRES;

    Point2 exec_dstar(Point, robot_position&, obs_array&);

    void convert_obstacles(robot_position&, obs_array&);
    void throw_map();
    void track_path(robot_position& pos);
    Point2 test_output(Point2, robot_position&, Point2);
    Point2 unreachable_output(robot_position&);
    Point2 get_plan_output(Point2, robot_position&);
    Point2 get_reactive_output(Point2, Point2, double);
    void get_wall_directions(vector<int>&);
    void find_correction(robot_position& pos);

    void init();
    Key calc_key(node*); 
    void update(node*);
    bool compute();

    void make_obstacle(int,int,int,bool);
    void unmake_safety(int,int,int);
    void make_safety(int,int,bool);

    void write_graph(robot_position&,char* s);
  

public:
    int correction;
    Dstar(double gridsize=0.5);
    Dstar(double he,double we,double gridsize=0.5);
    ~Dstar();
    Point sequencer(Point, robot_position&, obs_array&, double, 
		     Point, double, double, double); 
};

#endif

/**********************************************************************
 * $Log: dstar.hpp,v $
 * Revision 1.1.1.1  2008/07/14 16:44:17  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/20 17:17:47  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:37:55  endo
 * MissionLab 7.0
 *
 * Revision 1.2  2006/05/02 04:13:47  endo
 * TrackTask improved for the experiment.
 *
 * Revision 1.1.1.1  2005/02/06 23:00:25  endo
 * AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
 *
 * Revision 1.2  2003/04/06 15:00:50  endo
 * gcc 3.1.1
 *
 * Revision 1.1  2003/04/06 07:23:34  endo
 * Initial revision
 *
 **********************************************************************/
