/**********************************************************************
 **                                                                  **
 **                     utility.cpp                                  **
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

/* $Id: utility.cpp,v 1.1.1.1 2008/07/14 16:44:18 endo Exp $ */

#include"dstar.hpp"


inline Point2 rotate_z2(Point2 v, double t) 
{
    
    double c=cos(DEGREES_TO_RADIANS(t));
    double s=sin(DEGREES_TO_RADIANS(t));
    Point2 in(v.x*c - v.y*s,v.x*s + v.y*c,0.0);
    return in;
}


void make_safety_g(Graph* G, int x, int y) 
{
    if(G->nodes[x][y].type==OBSTACLE || G->nodes[x][y].type==SAFETY) return;
    G->nodes[x][y].type=SAFETY;
    for(int k=0;k<EDGES;k++) 
    {
	node* n=G->nodes[x][y].neighbrs[k];
	if(n==&G->dummy) continue;
	n->cost[(k+4)%EDGES]=k%2?20:30;
    }
}

void make_obstacle_g(Graph* G, int x, int y, int ngrids) 
{
    for(int i=x-ngrids-1;i<=x+ngrids+1;i++) 
    {
	for(int j=y-ngrids-1;j<=y+ngrids+1;j++) 
	{
	    if(i<0 || i>=G->height || j<0 || j>=G->width) continue;
	    if(G->nodes[i][j].type==OBSTACLE) continue;
	    make_safety_g(G,i,j);
	}
    }
    for(int i=x-ngrids;i<=x+ngrids;i++) 
    {
	for(int j=y-ngrids;j<=y+ngrids;j++) 
	{
	    if(i<0 || i>=G->height || j<0 || j>=G->width) continue;
	    if(G->nodes[i][j].type==OBSTACLE) continue;
	    G->nodes[i][j].type=OBSTACLE;
	    for(int k=0;k<EDGES;k++) 
	    {
		node* n=G->nodes[i][j].neighbrs[k];
		if(n==&G->dummy) continue;
		n->cost[(k+4)%EDGES]=DSTAR_INFINITY;
	    }
	}
    }
}


struct pred 
{
    int type;
    int val;
    
    pred(int v, int i) 
	{
	    type=i;
	    val=v;
	}
    
    bool operator()(double d) 
	{
	    return type? d > val : d < val;
	}
}; 

char* avoid_obs_key = "DSTAR_avoid_obstacle_gain";

double get_avoid_obstacle_gain()
{
    char* szValue = NULL;
    
    szValue = get_state(avoid_obs_key);
    if (szValue != NULL)
    {
        return strtod( szValue, NULL );
    }
    return DSTAR_INFINITY;
}


void put_avoid_obstacle_gain(double gain)
{
    char szGain[ 30 ];

    sprintf( szGain, "%f", gain );
    exec_put_console_state( avoid_obs_key, szGain );
}


Key calc_key_global(node* n, Graph* G) 
{
    double min=n->g<n->rhs?n->g:n->rhs;
    int tmp=G->h(G->start,n);
    return Key(min==DSTAR_INFINITY?DSTAR_INFINITY:(min+tmp),min);
}

double dist_to(Point2 a, Point2 b) 
{
    Point2 diff;
    diff.x=fabs(a.x-b.x);
    diff.y=fabs(a.y-b.y);
    return sqrt(diff.x*diff.x+diff.y*diff.y);
}

/**********************************************************************
 * $Log: utility.cpp,v $
 * Revision 1.1.1.1  2008/07/14 16:44:18  endo
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
