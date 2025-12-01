/**********************************************************************
 **                                                                  **
 **                     graph.cpp                                    **
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

/* $Id: graph.cpp,v 1.1.1.1 2008/07/14 16:44:17 endo Exp $ */

#include"graph.hpp"
#include<ctime>
#include<cstring>
#include<string>

using std::string;

Graph::Graph() 
{
  height=HEIGHT;
  width=WIDTH;
  dummy.type=OBSTACLE;
  init_nodes();
}


Graph::Graph(int he, int we) 
{
  height=he;
  width=we;
  dummy.type=OBSTACLE;
  start=goal=&dummy;
  nodes=new node*[height];
  if(!nodes)
  {
      cout<<"Memory panic\n"<<flush;
      exit(0);
  }
  for(int i=0;i<height;i++)
  {
      nodes[i]=new node[width];
      if(!nodes[i])
      {
	  cout<<"Memory panic\n"<<flush;
	  exit(0);
      }
  }
  init_nodes();
}


void Graph::clear() {
    for(int i=0;i<height;i++)
	delete [] nodes[i];
    delete [] nodes;
    nodes=new node*[height];
    if(!nodes)
    {
	cout<<"Memory panic\n"<<flush;
	exit(0);
    }
    for(int i=0;i<height;i++)
    {
	nodes[i]=new node[width];
	if(!nodes[i])
	{
	    cout<<"Memory panic\n"<<flush;
	    exit(0);
	}
    }
    init_nodes();
}


bool Graph::init_nodes_from_file()
{
    if(!read_map()) return false;
    for(int i=0;i<height;i++)
    {
	for(int j=0;j<width;j++) 
	{
	    if(i==0 || j==0) 
	    {
		nodes[i][j].cost[NORTHWEST]=DSTAR_INFINITY;
		nodes[i][j].neighbrs[NORTHWEST]=&dummy;
	    }
	    else 
	    {
		nodes[i][j].cost[NORTHWEST]=nodes[i-1][j-1].type==NORMAL?1.41:
		                            nodes[i-1][j-1].type==SAFETY?30:
		                            DSTAR_INFINITY;
		nodes[i][j].neighbrs[NORTHWEST]=&nodes[i-1][j-1];
	    }
	    if(i==0) 
	    {
		nodes[i][j].cost[NORTH]=DSTAR_INFINITY;
		nodes[i][j].neighbrs[NORTH]=&dummy;
	    }
	    else
	    {
		nodes[i][j].cost[NORTH]=nodes[i-1][j].type==NORMAL?1:
		    nodes[i-1][j].type==SAFETY?20:
		    DSTAR_INFINITY;
		nodes[i][j].neighbrs[NORTH]=&nodes[i-1][j];
	    }
	    if(i==0 || j==width-1) 
	    {
		nodes[i][j].cost[NORTHEAST]=DSTAR_INFINITY;
		nodes[i][j].neighbrs[NORTHEAST]=&dummy;
	    }
	    else 
	    {
		nodes[i][j].cost[NORTHEAST]=nodes[i-1][j+1].type==NORMAL?1.41:
		    nodes[i-1][j+1].type==SAFETY?30:
		    DSTAR_INFINITY;
		nodes[i][j].neighbrs[NORTHEAST]=&nodes[i-1][j+1];
	    }       
	    if(j==width-1) 
	    {
		nodes[i][j].cost[EAST]=DSTAR_INFINITY;
		nodes[i][j].neighbrs[EAST]=&dummy; 
	    }
	    else
	    {
		nodes[i][j].cost[EAST]=nodes[i][j+1].type==NORMAL?1:
		    nodes[i][j+1].type==SAFETY?20:
		    DSTAR_INFINITY;
		nodes[i][j].neighbrs[EAST]=&nodes[i][j+1];
	    }
	    if(i==height-1 || j==width-1)
	    {
		nodes[i][j].cost[SOUTHEAST]=DSTAR_INFINITY;
		nodes[i][j].neighbrs[SOUTHEAST]=&dummy; 
	    } 
	    else 
	    {
		nodes[i][j].cost[SOUTHEAST]=nodes[i+1][j+1].type==NORMAL?1.41:
		    nodes[i+1][j+1].type==SAFETY?30:
		    DSTAR_INFINITY;
		nodes[i][j].neighbrs[SOUTHEAST]=&nodes[i+1][j+1]; 
	    }
	    if(i==height-1) 
	    {
		nodes[i][j].cost[SOUTH]=DSTAR_INFINITY;
		nodes[i][j].neighbrs[SOUTH]=&dummy;  
	    }
	    else
	    {
		nodes[i][j].cost[SOUTH]=nodes[i+1][j].type==NORMAL?1:
		    nodes[i+1][j].type==SAFETY?20:
		    DSTAR_INFINITY;
		nodes[i][j].neighbrs[SOUTH]=&nodes[i+1][j];             
	    }
	    if(i==height-1 || j==0) 
	    {
		nodes[i][j].cost[SOUTHWEST]=DSTAR_INFINITY;
		nodes[i][j].neighbrs[SOUTHWEST]=&dummy;            
	    }
	    else
	    {
		nodes[i][j].cost[SOUTHWEST]=nodes[i+1][j-1].type==NORMAL?1.41:
		    nodes[i+1][j-1].type==SAFETY?30:
		    DSTAR_INFINITY;
		nodes[i][j].neighbrs[SOUTHWEST]=&nodes[i+1][j-1];         
	    }
	    if(j==0) 
	    {
		nodes[i][j].cost[WEST]=DSTAR_INFINITY;
		nodes[i][j].neighbrs[WEST]=&dummy;            
	    }           
	    else 
	    {
		nodes[i][j].cost[WEST]=nodes[i][j-1].type==NORMAL?1:
		    nodes[i][j-1].type==SAFETY?20:
		    DSTAR_INFINITY;
		nodes[i][j].neighbrs[WEST]=&nodes[i][j-1];         
	    }      
	}
    }
    return true;
}



void Graph::init_nodes()
{
  for(int i=0;i<height;i++)
    {
      for(int j=0;j<width;j++) 
	{
	  nodes[i][j].x=i;
	  nodes[i][j].y=j;
	  nodes[i][j].type=NORMAL;
	  nodes[i][j].path=nodes[i][j].removed=0;
	}
    }
 
  if(!read_map()) cout<<"Map not found, beginning with empty map....\n";

  for(int i=0;i<height;i++)
  {
      for(int j=0;j<width;j++) 
      {
	  if(i==0 || j==0) 
	  {
	      nodes[i][j].cost[NORTHWEST]=DSTAR_INFINITY;
	      nodes[i][j].neighbrs[NORTHWEST]=&dummy;
	  }
	  else 
	  {
	      nodes[i][j].cost[NORTHWEST]=nodes[i-1][j-1].type==NORMAL?1.41:
		                          nodes[i-1][j-1].type==SAFETY?30:
                                          DSTAR_INFINITY;
	      nodes[i][j].neighbrs[NORTHWEST]=&nodes[i-1][j-1];
	  }
	  if(i==0) 
	  {
	      nodes[i][j].cost[NORTH]=DSTAR_INFINITY;
	      nodes[i][j].neighbrs[NORTH]=&dummy;
	  }
	  else
	  {
	      nodes[i][j].cost[NORTH]=nodes[i-1][j].type==NORMAL?1:
                                      nodes[i-1][j].type==SAFETY?20:
                                      DSTAR_INFINITY;
	      nodes[i][j].neighbrs[NORTH]=&nodes[i-1][j];
	  }
	  if(i==0 || j==width-1) 
	  {
	      nodes[i][j].cost[NORTHEAST]=DSTAR_INFINITY;
	      nodes[i][j].neighbrs[NORTHEAST]=&dummy;
	  }
	  else 
	  {
	      nodes[i][j].cost[NORTHEAST]=nodes[i-1][j+1].type==NORMAL?1.41:
                                          nodes[i-1][j+1].type==SAFETY?30:
                                          DSTAR_INFINITY;
	      nodes[i][j].neighbrs[NORTHEAST]=&nodes[i-1][j+1];
	  }       
	  if(j==width-1) 
	  {
	      nodes[i][j].cost[EAST]=DSTAR_INFINITY;
	      nodes[i][j].neighbrs[EAST]=&dummy; 
	  }
	  else
	  {
	      nodes[i][j].cost[EAST]=nodes[i][j+1].type==NORMAL?1:
                                     nodes[i][j+1].type==SAFETY?20:
                                     DSTAR_INFINITY;
	      nodes[i][j].neighbrs[EAST]=&nodes[i][j+1];
	  }
	  if(i==height-1 || j==width-1)
	  {
	      nodes[i][j].cost[SOUTHEAST]=DSTAR_INFINITY;
	      nodes[i][j].neighbrs[SOUTHEAST]=&dummy; 
	  } 
	  else 
	  {
	      nodes[i][j].cost[SOUTHEAST]=nodes[i+1][j+1].type==NORMAL?1.41:
                                          nodes[i+1][j+1].type==SAFETY?30:
                                          DSTAR_INFINITY;
	      nodes[i][j].neighbrs[SOUTHEAST]=&nodes[i+1][j+1]; 
	  }
	  if(i==height-1) 
	  {
	      nodes[i][j].cost[SOUTH]=DSTAR_INFINITY;
	      nodes[i][j].neighbrs[SOUTH]=&dummy;  
	  }
	  else
	  {
	      nodes[i][j].cost[SOUTH]=nodes[i+1][j].type==NORMAL?1:
                                      nodes[i+1][j].type==SAFETY?20:
                                      DSTAR_INFINITY;
	      nodes[i][j].neighbrs[SOUTH]=&nodes[i+1][j];             
	  }
	  if(i==height-1 || j==0) 
	  {
	      nodes[i][j].cost[SOUTHWEST]=DSTAR_INFINITY;
	      nodes[i][j].neighbrs[SOUTHWEST]=&dummy;            
	  }
	  else
	  {
	      nodes[i][j].cost[SOUTHWEST]=nodes[i+1][j-1].type==NORMAL?1.41:
                                          nodes[i+1][j-1].type==SAFETY?30:
                                          DSTAR_INFINITY;
	      nodes[i][j].neighbrs[SOUTHWEST]=&nodes[i+1][j-1];         
	  }
	  if(j==0) 
	  {
	      nodes[i][j].cost[WEST]=DSTAR_INFINITY;
	      nodes[i][j].neighbrs[WEST]=&dummy;            
	  }           
	  else 
	  {
	      nodes[i][j].cost[WEST]=nodes[i][j-1].type==NORMAL?1:
                                     nodes[i][j-1].type==SAFETY?20:
                                     DSTAR_INFINITY;
	      nodes[i][j].neighbrs[WEST]=&nodes[i][j-1];         
	  }      
      }
  }
}



void Graph::write_map()
{
    fstream fout;
    fout.open("dstar.map",fstream::out);
    if(!fout.is_open()) cout<<"\nCannot write map - File opening error\n";
    else
	for(int i=0;i<height;i++) 
	{
	    for(int j=0;j<width;j++) 
	    {
		//if(nodes[i][j].path==1)
		//{
		//  if(nodes[i][j].type==SAFETY) fout<<"%";
		//  else fout<<"x";
		//}
		//else 
		if(nodes[i][j].type==OBSTACLE) fout<<"0";
		else if(nodes[i][j].type==SAFETY)  fout<<"#";
		else fout<<"-";
	    }
	    fout<<endl;
	}
    fout.close();    
}



Graph::~Graph() 
{ 
    for(int i=0;i<height;i++)
    {
	delete [] nodes[i];
    }
    delete [] nodes;
}


bool Graph::read_map()
{
    fstream fin;
    fin.open("dstar.map", fstream::in);
    if(!fin.is_open()) return false;
    string str;
    for(int i=0;i<height;i++) 
    {
	fin>>str;
	for(int j=0;j<width;j++) 
	{
	    if(str[j]=='%')
	    {
		nodes[i][j].type=SAFETY;
		nodes[i][j].path=1;
	    }
	    if(str[j]=='x') 
	    {
		nodes[i][j].type=NORMAL;
		nodes[i][j].path=1;
	    }
	    if(str[j]=='0')
	    {
		nodes[i][j].type=OBSTACLE;
	    }
	    if(str[j]=='#')
	    {
		nodes[i][j].type=SAFETY;
	    }
	    if(str[j]=='-') 
	    {
		nodes[i][j].type=NORMAL;
	    }
	}
    }
    fin.close();
    return true;
}     



int Graph::h(node* a,node* b) 
{
  // ENDO - gcc 3.4
  //int t1=int(abs(float(a->x-b->x)));
  //int t2=int(abs(float(a->y-b->y)));
  int t1=int(fabs(float(a->x-b->x)));
  int t2=int(fabs(float(a->y-b->y)));
  return t1>t2?t1:t2; 
}


void Graph::debug(bool file, bool screen, char* name) 
{
  fstream fout;
  long t=time(0);
  char filename[80];
  if(!name) {
    sprintf(filename,"/net/hr2/ananth/misc/images/");
    sprintf(filename+strlen(filename),"%ld",t);
  }
  else {
    strcpy(filename,name);
    //free(name);
  }
  if(file) {
      fout.open(filename,fstream::out);
      if(!fout.is_open()) cout<<"\nFILE OPENING ERROR\n";
  }
  for(int i=0;i<height;i++) 
    {
      for(int j=0;j<width;j++) 
	{
	  if(nodes[i][j]==*start)
	    {
		if(screen) cout<<"S";
		if(file) fout<<"s";
	    }
	  else if(nodes[i][j].path==1)
	    {
		if(screen) cout<<"x";
		if(file) {
		    if(nodes[i][j].type==SAFETY) fout<<"%";
		    else fout<<"x";
		} 
	    }
	  else if(nodes[i][j]==*goal)
	    {
		if(screen) cout<<"G";
		if(file) fout<<"g";
	    }
	  else if(nodes[i][j].type==OBSTACLE)
	    {
		if(screen) cout<<"0";
		if(file) fout<<"0";
	    }
	  else if(nodes[i][j].type==SAFETY)
	  {
	      if(screen) cout<<"#";
	      if(file) fout<<"#";
	  }
	  else if(nodes[i][j].removed==1) {
	      if(screen) cout<<"#";
	      if(file) fout<<"$";
	      nodes[i][j].removed=0;
	  }
	  else if(nodes[i][j].removed==2) {
	      if(screen) cout<<"#";
	      if(file) fout<<"*";
	      nodes[i][j].removed=0;
	  }
	  else
	    {
		if(screen) cout<<"-";
		if(file) fout<<"-";
	    }
	}
      if(screen) cout<<endl;
      if(file) fout<<endl;
    }
  if(screen) cout<<"\n\n";
  if(file) fout.close();
}




/*istream& operator>>(istream& is, Graph& g) {
  is>>g.height>>g.width;
  int goalx,goaly,startx,starty;
  is>>startx>>starty>>goalx>>goaly;
  int i=0,j=0;
  is.get();
  while(i<g.height) {
    char c=is.get();
    //cout<<c; 
    if(isspace(c)) continue;
    if(c=='x') g.nodes[i][j].type=OBSTACLE;
    else g.nodes[i][j].type=NORMAL;
    g.nodes[i][j].x=i;
    g.nodes[i][j].y=j;
    j++;
    if(j==g.width) {
       j=0;
       i++;
    }
  }
  for(i=0;i<g.height;i++)
    for(j=0;j<g.width;j++) {
      if(i==0 || j==0) g.nodes[i][j].edges[0]=0;
      else g.nodes[i][j].edges[0]=int(g.nodes[i-1][j-1].type);
      if(i==0) g.nodes[i][j].edges[1]=0;
      else g.nodes[i][j].edges[1]=int(g.nodes[i-1][j].type);
      if(i==0 || j==g.width-1) g.nodes[i][j].edges[2]=0;
      else g.nodes[i][j].edges[2]=int(g.nodes[i-1][j+1].type);
      if(j==g.width-1) g.nodes[i][j].edges[3]=0;
      else g.nodes[i][j].edges[3]=int(g.nodes[i][j+1].type);
      if(i==g.height-1 || j==g.width-1) g.nodes[i][j].edges[4]=0;
      else g.nodes[i][j].edges[4]=int(g.nodes[i+1][j+1].type);
      if(i==g.height-1) g.nodes[i][j].edges[5]=0;
      else g.nodes[i][j].edges[5]=int(g.nodes[i+1][j].type);
      if(i==g.height-1 || j==0) g.nodes[i][j].edges[6]=0;
      else g.nodes[i][j].edges[6]=int(g.nodes[i+1][j-1].type);
      if(j==0) g.nodes[i][j].edges[7]=0;
      else g.nodes[i][j].edges[7]=int(g.nodes[i][j-1].type);
    }
  g.start=&g.nodes[startx][starty];
  g.goal=&g.nodes[goalx][goaly];
  //cin.get();
  return is;
  }*/


/*int main() {
  Graph g(10,10);
  for(int i=0;i<g.height;i++)
    delete [] nodes[i];  
    delete [] nodes;
  cout<<"x="<<g.getxy(1,1)->x<<" y="<<g.getxy(1,1)->y<<flush;
}*/

/**********************************************************************
 * $Log: graph.cpp,v $
 * Revision 1.1.1.1  2008/07/14 16:44:17  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/20 17:17:47  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:37:55  endo
 * MissionLab 7.0
 *
 * Revision 1.4  2006/05/15 02:45:41  endo
 * gcc-3.4 upgrade
 *
 * Revision 1.3  2006/05/15 02:43:33  endo
 * gcc-3.4 upgrade
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
