/**********************************************************************
 **                                                                  **
 **                    graph.hpp                                     **
 **                                                                  **
 **                                                                  **
 **  Written by:  Ananth Ranganathan                                 **
 **                                                                  ** 
 **  The D* Lite algorithm was developed by Sven Koenig and Maxim    ** 
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

/* $Id: graph.hpp,v 1.1.1.1 2008/07/14 16:44:17 endo Exp $ */

#ifndef GRAPH_H
#define GRAPH_H

#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <utility>
#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>

//using namespace std;
using std::vector;
using std::pair;
using std::make_pair;
using std::ostream;
using std::fstream;
using std::istream;
using std::cout;
using std::flush;
using std::endl;

#define EDGES 8 
#define DSTAR_INFINITY 100000

enum 
  {
    NORTHWEST, NORTH, NORTHEAST, EAST, 
    SOUTHEAST, SOUTH, SOUTHWEST, WEST,
    OBSTACLE=1000, NORMAL, SAFETY
  };

struct node 
{
  int x,y; 
  short type;
  double g;
  short path,removed;
  double rhs;
  double cost[EDGES];
  node* neighbrs[EDGES];
  bool operator==(const node& n) { return x==n.x && y==n.y; }
  //node& operator=(const node& n) { x=n.x;y=n.y;g=n.g;rhs=n.rhs;}
  node& operator=(const node& n) { x=n.x;y=n.y;g=n.g;rhs=n.rhs; return *this;}
  friend ostream& operator<<(ostream& os,node& n) 
  { os<<"("<<n.x<<","<<n.y<<")";return os; }
};
   
typedef pair<double,double> Key;
typedef node* Element;

class Dstar;


class Graph 
{
public:
    
    static const int HEIGHT=200;
    static const int WIDTH=200;
    
public:
    
    node** nodes;
    node dummy;
    int height, width;
    node *start, *goal;

    void debug(bool,bool,char* name=NULL);
    void init_nodes();
    bool init_nodes_from_file();

public:

    Graph();
    Graph(int he, int we);
    ~Graph();
    int h(node* a, node* b);
    void clear();
    bool read_map();
    void write_map();
    friend class Dstar;
    friend istream& operator>>(istream& is,Graph& g);
    friend Key calc_key_global(node*, Graph*);

};

#endif

/**********************************************************************
 * $Log: graph.hpp,v $
 * Revision 1.1.1.1  2008/07/14 16:44:17  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/20 17:17:47  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:37:55  endo
 * MissionLab 7.0
 *
 * Revision 1.2  2006/05/02 04:13:48  endo
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
