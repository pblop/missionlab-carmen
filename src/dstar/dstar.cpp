/**********************************************************************
 **                                                                  **
 **                           dstar.cpp                              **
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


/* $Id: dstar.cpp,v 1.1.1.1 2008/07/14 16:44:17 endo Exp $ */

#include"dstar.hpp"
#include"utility.cpp"  //this has to be included after dstar.hpp!

using std::endl;

#if DEBUG
fstream fout("obsdata",fstream::out);
#endif

Dstar::Dstar(double height, double width, double gridsize) 
{
    //cout<<"Dstar constructor\n"<<flush;
    G=new Graph(int(height), int(width));    // the D*Lite grid
    U=new Pqueue;                            // priority queue
    started=false; 
    firsttime=true;
    this->gridsize=gridsize;
    unreachable=0;
    planning_only=planning_only_flag=planning_leave_flag=0;
    planning=0;
    progress_counter=0;
    lastpos.x=lastpos.y=0.0; 
    po_time=0;
    start_time=time(0);
    planning_stuck = false;
//used in simulation only 
    counter=0;
//used in actual robot only
    find_dir=0;
    stored_wall=1000;
    correction=0;
    mapping=0;
    path_blocked=false;
    //cout<<"leaving dstar constructor\n"<<flush;
}


Dstar::~Dstar() 
{
    delete U;
    G->write_map();
    delete G;
    if(planning_only) po_time+=(time(0)-po_start_time);
    cout<<"po_time = "<<po_time<<endl<<flush;
    cout<<"total time = "<<time(0)-start_time<<endl<<flush;
} 


//The D*Lite init function

void Dstar::init() 
{
    for(int i=0;i<G->height;i++)
	for(int j=0;j<G->width;j++) 
	    G->nodes[i][j].g=G->nodes[i][j].rhs=DSTAR_INFINITY;
    G->goal->rhs=0;
    U->insert(G->goal,Key(G->h(G->start,G->goal),0));
} 


//The D*Lite calc_key function

Key Dstar::calc_key(node* n) 
{
    double min=n->g<n->rhs?n->g:n->rhs;
    int tmp=G->h(G->start,n);
    return Key(min==DSTAR_INFINITY?DSTAR_INFINITY:(min+tmp),min);
}


//The D*Lite update function

void Dstar::update(node* n) 
{
    //cout<<"update "<<n->x<<" "<<n->y<<"\n"<<flush;
    if(!(*n==*G->goal)) 
    {
	double min=DSTAR_INFINITY; 
	for(int i=0;i<EDGES;i++) 
	{
	    node* p=n->neighbrs[i];
	    if(p==&G->dummy) continue;
	    double tmp = (p->g==DSTAR_INFINITY || n->cost[i]==DSTAR_INFINITY) ? DSTAR_INFINITY:(n->cost[i]+p->g);
	    min=min<tmp?min:tmp;
	}
	n->rhs=min;
    }
    if(U->exists(n)) U->remove(n);
    if(n->g!=n->rhs) U->insert(n,calc_key(n));
    //cout<<"end update\n"; 
} 



//The D*Lite compute function

bool Dstar::compute() 
{
    //cout<<"compute\n"<<flush;
    while( U->minkey() < calc_key(G->start) || G->start->rhs != G->start->g) 
    {
	if(U->empty()) 
	{
	    cout<<"Queue empty - Unable to reach goal\n"<<flush;
	    return false;
	}
	node* u=U->pop();
	if(u->g > u->rhs)
	{
	    u->g=u->rhs;
	    for(int i=0;i<EDGES;i++)
	    {
		node* n=u->neighbrs[i];
		if(n==&G->dummy) continue;
		update(n);
	    }
	}
	else 
	{
	    u->g=DSTAR_INFINITY;
	    for(int i=0;i<EDGES;i++)
	    {
		node* n = u->neighbrs[i];
		if(n==&G->dummy) continue;
		update(n);   
	    }
	    update(u);
	}
    }
    return true;
    //cout<<"leaving compute\n"<<flush;
}


//The next couple of functions are helpers to change state of a grid cell
//in the graph

void Dstar::make_obstacle(int x, int y, int ngrids, bool upd) 
{
    for(int i=x-ngrids-1;i<=x+ngrids+1;i++) 
    {
	for(int j=y-ngrids-1;j<=y+ngrids+1;j++) 
	{
	    if(i<0 || i>=G->height || j<0 || j>=G->width) continue;
	    if(G->nodes[i][j].type==OBSTACLE) continue;
	    make_safety(i,j,upd);
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
		if(upd) update(n);
	    }
	}
    }
}


void Dstar::make_safety(int x, int y, bool upd) 
{
    if(G->nodes[x][y].type==OBSTACLE || G->nodes[x][y].type==SAFETY) return;
    G->nodes[x][y].type=SAFETY;
    for(int k=0;k<EDGES;k++) 
    {
	node* n=G->nodes[x][y].neighbrs[k];
	if(n==&G->dummy) continue;
	n->cost[(k+4)%EDGES]=k%2?20:30;  //cost of safety cells
	if(upd) update(n);
    }
}


//returns current position so that robot does not move

Point2 Dstar::unreachable_output(robot_position& pos) 
{
    Point2 output(pos.v.x,pos.v.y,0.0);
    return output;
}


//Process readings from sensors

void Dstar::convert_obstacles(robot_position& pos, obs_array& readings) 
{
    if(run_type!=SIMULATION)
    {
	find_correction(pos);  
//wall finding, to correct for orientation errors, can be commented out 
//if localization is present 
    }
    obstacles.clear();
    Point2 tmp;
    for(int i=0;i<readings.size;i++) 
    {
	Point2 tmpvec=(Point2)readings.val[i].center;
	if(run_type!=SIMULATION)
	{
	    tmpvec=rotate_z2(tmpvec,pos.heading-correction); 
//correct for orientation errors
	}
	else
	{
	    tmpvec=rotate_z2(tmpvec,pos.heading);
	}  
	tmp.x=int((tmpvec.x+pos.v.x)/gridsize);
	tmp.y=int((tmpvec.y+pos.v.y)/gridsize);
	tmp.z=int(ceil(readings.val[i].r/gridsize)); 
	if(i) 
	{
	    Point2 t2=obstacles[obstacles.size()-1];
	    if(t2.x!=tmp.x || t2.y!=tmp.y || t2.z!=tmp.z) 
	    {
		obstacles.push_back(tmp);	
#if DEBUG
		fout<<"Obstacle:"<<tmp.x<<" "<<tmp.y<<" "<<tmp.z<<endl<<flush;
#endif
	    }  
	}
	else 
	{
	    obstacles.push_back(tmp);	
#if DEBUG
	    fout<<"Obstacle:"<<tmp.x<<" "<<tmp.y<<" "<<tmp.z<<endl<<flush;
#endif
	}  
    }
    //cout<<"Exiting convert obstacles\n"<<flush;
}


//performs line-finding by histogramming over angle space

void Dstar::get_wall_directions(vector<int>& histogram) 
{
    int sz=obstacles.size();
    for(int i=3;i<sz-3;i++) 
    {
	double sigmaxsq=0,sigmax=0,sigmay=0,sigmaxy=0,n=7;
	for(int j=-3;j<=3;j++) 
	{
	    double x = obstacles[i+j].x;
	    double y = obstacles[i+j].y;
	    sigmaxsq+=x*x;
	    sigmax+=x;
	    sigmay+=y;
	    sigmaxy+=x*y;
	}
	double den=n*sigmaxsq-sigmax*sigmax;
	int a;
	if(!den) 
	{
	    a=180;
	}
	else 
	{
	    a=int(180.0*atan((n*sigmaxy-sigmax*sigmay)/den)/3.14159)+90;
	}
	histogram[a/2]++;
    }
	//double b=(sigmax*sigmaxy-sigmaxsq*sigmay)/(sigmax*sigmax-sigmaxsq*n);
}
         
	    

//finds correction angle using the previous known orientation of a wall

void Dstar::find_correction(robot_position& pos) 
{
    vector<int> histogram(190,0);
    get_wall_directions(histogram);
    int max=0;
    for(int i=1;i<182/2;i++) 
    {
      if(histogram[i]>histogram[max]) 
      {
	  max=i;
      }
    }
    int number=histogram[max];
    int slope=max*2-90;
    if(number < (int)(obstacles.size())/9) 
    {
	stored_wall=1000;
	return;   //No wall if below this threshold
    }
    if(stored_wall==1000 || abs(stored_wall-slope)>75) 
    {
	stored_wall=slope;
    }
    else 
    {
	correction=slope-stored_wall;
    }
}



//mainly debugging purposes, keeps track of the robot's path
	    
void Dstar::track_path(robot_position& pos) 
{
    int newx=int(pos.v.x/gridsize);
    int newy=int(pos.v.y/gridsize);
    if(newx<0 || newx>=G->height || newy<0 || newy>=G->width) 
    {
	cout<<"Robot position out of grid\n"<<flush;
	return;
    }  
    Point2 tmp(newx,newy,0.0);
    if(path.size()) 
    {
	Point2 v=path[path.size()-1];
	if(v.x!=tmp.x || v.y!=tmp.y)
	{
	    path.push_back(tmp);
#if DEBUG
	    fout<<"Robbie's been here "<<tmp.x<<","<<tmp.y<<"\n"<<flush;
#endif
	}
    }
    else path.push_back(tmp);
    if(path.size()==60) path.erase(path.begin());

    if(run_type!=SIMULATION)
    {
	if(!path_blocked) 
	{
	    int sz=path.size();
	    for(int i=0;i<sz;i++) 
	    {
		Point2 tmp=path[i];
		if(G->nodes[int(tmp.x)][int(tmp.y)].type==OBSTACLE) 
		{
		    path_blocked=true;
		}
	    }
	}
	else 
	{
	    int sz=path.size();
	    for(int i=0;i<sz;i++) 
	    {
		Point2 tmp=path[i];
		if(G->nodes[int(tmp.x)][int(tmp.y)].type==OBSTACLE)  
		{
		    return;
		}
	    }
	    path_blocked=false;
	}	
    }

}


//throws map if goal cannot be reached 
//necessary only if no localization is present

void Dstar::throw_map() 
{
    //cout<<"Throwing map\n"<<flush;
    started=path_blocked=false;
    if(run_type==SIMULATION)
    {
	planning=false;
    }
    delete U;
    U=new Pqueue;
    G->clear();
    acct_distance=0;
    progress_counter=0;
    correction=0;
    if(run_type!=SIMULATION) 
    {
	path.clear();
    }
}


Point2 Dstar::test_output(Point2 output,robot_position& pos,Point2 mygoal) 
{
    if(output==unreachable_output(pos)) 
    {
	unreachable++;
	if(unreachable>=3) return output;
	else 
	{
	    cout<<"Goal unreachable "<<unreachable<<" times\n"<<flush;
	    throw_map();
	    return mygoal;
	}
    }
    else 
    {
	unreachable=0;
	return output;
    }
}


Point2 Dstar::get_plan_output(Point2 v, robot_position& pos) 
{
    Point2 plan=v;
    minus_2d(plan,pos.v,plan);
    plan=rotate_z2(plan,-1*pos.heading );
    unit_2d(plan);
    return plan;
}


Point2 Dstar::get_reactive_output(Point2 ao_vec, Point2 goal, double goal_gain) 
{
    Point2 a = ao_vec;
    unit_2d(a);
    mult_2d(a,ao_val);
    Point2 b = goal;
    unit_2d(b);
    mult_2d(b,goal_gain);
    add_2d(a,b,b);
    unit_2d(b);
    return b;
}


Point Point2_to_Point(Point2 p)
{
    Point r;
    r.x=p.x;
    r.y=p.y;
    r.z=p.z;
    return r;
}

//entry function for the D*Lite behavior

Point Dstar::sequencer(
    Point goal,
    robot_position& pos,
    obs_array& readings,
    double ao_value,
    Point ao_vec,
    double goal_gain,
    double persistence,
    double angle_dev) 
{
    Point invalidPoint;

    memset(&invalidPoint, 0x0, sizeof(Point));

    if(firsttime) 
    {
        firsttime=false;
        PO_START_THRES = int(persistence);
        LO_ANGLE_THRES = int(angle_dev);
        lastpos=pos.v;
        ao_val=ao_value;
    }

    if(planning_only)
    { 
        G->nodes[int(pos.v.x/gridsize)][int(pos.v.y/gridsize)].removed=2;
    }

    if(run_type==SIMULATION)
    {
        if(!counter%30)
        {
            for(int i=0;i<(int)(path.size());i++)  
            {
                int x = int(path[i].x), y = int(path[i].y);
                G->nodes[x][y].path=1;
            }
        }
        counter++;
    }

    track_path(pos);
    Point2 mygoal=goal;

    if(run_type!=SIMULATION)
    {
        mygoal=rotate_z2(mygoal,pos.heading-correction);
    }
    else
    {
        mygoal=rotate_z2(mygoal,pos.heading);
    }

    add_2d(mygoal,pos.v,mygoal);
    double delta=dist_to(pos.v,lastpos);

    if(!planning) 
    {
        if(delta > MOVEMENT_THRESHOLD) 
        {
            progress_counter=0;
            lastpos=pos.v;
            acct_distance+=delta;
        }
        else 
        {
            progress_counter++;
            acct_distance+=delta;
        }
        if( acct_distance > DISTANCEOUT || 
            progress_counter > PROGRESS_THRESHOLD) 
        {
            planning=PO_START_THRES;
#if DEBUG
            if(acct_distance > DISTANCEOUT) 
            {
                cout<<"Distance out\n"<<flush;
            }
            else
            {
                cout<<"Robot making no progress\n"<<flush;
            }
            cout<<"Switching to planning mode pos=("<<pos.v.x<<","<<pos.v.y
                <<")\n"<<flush;
            fout<<"Entering planning mode at distance "<<acct_distance
                <<" at location "<<int(pos.v.x/gridsize)<<","
                <<int(pos.v.y/gridsize)<<"\n"<<flush;
#endif
            progress_counter=0;
            acct_distance=0;
            lastpos=pos.v;
        }
        else 
        {
            return Point2_to_Point(mygoal);
        }
    }

    if(planning) 
    {
        lastpos=pos.v;
        if(planning_only)
        {
            planning_only++;
        }

        if(!started) 
        {
            //cout<<"Started\n"<<flush;
            if(run_type!=SIMULATION)
            {
                hclient_stop();   
                //stop robot before taking sensor readings. Can be commented out if 
                //localization is available
            }	
            convert_obstacles(pos,readings);
            Point2 output = exec_dstar(goal,pos,readings);
            return Point2_to_Point(test_output(output,pos,mygoal));
        }

        acct_distance+=delta;
        double dist=dist_to(lastplanned, pos.v);    
        double dist2=dist_to(lastoutput,pos.v);
        if(run_type!=SIMULATION)
        {
            if(abs(correction)>20) 
            {
                throw_map();
                acct_distance=0;
                return Point2_to_Point(lastoutput); 
            }  
            if(!planning_only && path_blocked) 
            {
                //cout<<"Path blocked\n"<<flush;
                throw_map();
                acct_distance=0;
                return Point2_to_Point(lastoutput);
            }
        }

        if(!planning_only && dist2 > 3*gridsize)  
        {
            //cout<<"delta="<<delta<<" dist2="<<dist2<<endl<<flush; 
            if(delta < PLANNING_MOVEMENT_THRESHOLD)//abs(dist-dist_to_last_planned) < MOVEMENT_THRESHOLD) 
            {
                progress_counter++;
            }
            else 
            {
                progress_counter=0;
                dist_to_last_planned=dist;
            }

            if(progress_counter < PLANNING_PROGRESS_THRESHOLD) 
            {
                return Point2_to_Point(lastoutput);
            }
            else 
            { 
                if(planning_stuck && lastoutput==stuck_goal)
                {
                    planning_stuck=false;
                    planning_only=1;
                    planning_only_flag=planning_leave_flag=0;
                    put_avoid_obstacle_gain(0);
                    cout<<"Entering planning only mode due to no progress\n"<<flush;
                    po_start_time=time(0);
                }
                else 
                {
                    planning_stuck=true;
                    stuck_goal=lastoutput;
                    progress_counter=0;
                }
            }
        }
	
        progress_counter=0;
        convert_obstacles(pos,readings);
        Point2 output = exec_dstar(goal,pos,readings);
	
        Point2 react = get_reactive_output(ao_vec,goal,goal_gain);
        Point2 plan = get_plan_output(output,pos);
        double r = atan2(react.y,react.x)*180.0/3.141592;
        double p = atan2(plan.y,plan.x)*180.0/3.141592;  
        // ENDO - gcc 3.4
        //angles.push_back(abs(r-p));
        angles.push_back(fabs(r-p));
#if DEBUG
        copy(angles.begin(), angles.end(), ostream_iterator<double>(cout, " "));
        cout<<endl<<flush;
#endif
        int thres = int(0.5*angles.size());
        if(planning_only) 
        {
            if((int)(angles.size())>=PO_START_THRES) 
            {
                int c1=count_if(angles.end()-PO_START_THRES,angles.end(),
                                pred(LO_ANGLE_THRES,0));
                int c2=count_if(angles.begin(),angles.end(),
                                pred(LO_ANGLE_THRES,0));
                if(c1>=PO_START_THRES || c2>=(thres?thres:1)) 
                {
                    planning_leave_flag++;
                    if(planning_leave_flag>PO_START_THRES) 
                    {
                        planning_only=planning_only_flag=planning_leave_flag=0;
                        put_avoid_obstacle_gain(ao_val);
                        cout<<"Leaving planning only mode react_angle="
                            <<r<<" plan_angle="<<p<<" pos=("<<pos.v.x
                            <<","<<pos.v.y<<")\n"<<flush;
                        po_time+=(time(0)-po_start_time);
                    }
                }
            }
        }
        else if(!planning_only) 
        {
            if((int)(angles.size())>=PO_START_THRES) 
            {
                int c1=count_if(angles.end()-PO_START_THRES,angles.end(),
                                pred(LO_ANGLE_THRES,1));
                int c2=count_if(angles.begin(),angles.end(),
                                pred(LO_ANGLE_THRES,1));
                if(c1>=PO_START_THRES || c2>=(thres?thres:1)) 
                {
                    planning_only_flag++;
                    if(planning_only_flag > PO_START_THRES) 
                    {
                        planning_only=1;
                        planning_only_flag=planning_leave_flag=0;
                        put_avoid_obstacle_gain(0);
                        cout<<"Entering planning only mode react_angle="
                            <<r<<" plan_angle="<<p<<" pos=("<<pos.v.x
                            <<","<<pos.v.y<<")\n"<<flush;
                        po_start_time=time(0);
                    }
                }
            }
        }

        if(!planning_only) 
        {
            Point2 g=goal;
            unit_2d(g);
            double pg = atan2(g.y,g.x)*180.0/3.141592;
            // ENDO - gcc 3.4
            //g_angles.push_back(abs(r-pg));
            g_angles.push_back(fabs(r-pg));
            //cout<<"react_angle="<<r<<" goal_angle="<<pg<<"\n"<<flush;  
            if((int)(angles.size())>=PO_START_THRES && (int)(g_angles.size())>=PO_START_THRES)
            {
                int c1=count_if(angles.begin(),angles.end(),
                                pred(LO_ANGLE_THRES,0));
                int c2=count_if(g_angles.begin(),g_angles.end(),
                                pred(LO_ANGLE_THRES,0));
                int c3=count_if(angles.end()-PO_START_THRES,angles.end(),
                                pred(LO_ANGLE_THRES,0));
                int c4=count_if(g_angles.end()-PO_START_THRES,g_angles.end(),
                                pred(LO_ANGLE_THRES,0));
                int thres2= int(0.5*g_angles.size());
                if( (c1>=(thres?thres:1) && c2>=(thres2?thres2:1)) || 
                    (c3>=PO_START_THRES && c4>=PO_START_THRES) )
                {
                    planning--;
                    if(!planning) 
                    {
                        started=false;
                        acct_distance=0;
                        progress_counter=0;
                        cout<<"Switching out of planning mode react_angle="
                            <<r<<" goal_angle="<<pg<<"\n"<<flush;
                    }
                }
            }
        }
        if((int)(g_angles.size())>PO_START_THRES*2) 
        {
            g_angles.erase(g_angles.begin());
        }
        if((int)(angles.size())> PO_START_THRES*2) 
        {
            angles.erase(angles.begin());
        }

        return Point2_to_Point(test_output(output,pos,mygoal));
    }

    return invalidPoint;
}


//D*Lite main

Point2
Dstar::exec_dstar(Point goal, robot_position& pos, obs_array& readings) 
{ 
    //cout<<"exec_dstar\n"<<flush;
    //cout<<"exec_dstar correction="<<correction<<endl<<flush;
    Point2 mygoal;
    if(run_type!=SIMULATION)
    {
	goal=Point2_to_Point(rotate_z2(goal,pos.heading-correction));
    }
    else
    {
	goal=Point2_to_Point(rotate_z2(goal,pos.heading));
    }
    add_2d(goal,pos.v,mygoal);
    if(unreachable>=3)
    {
	cout<<"Goal unreachable\n"<<flush;
	return unreachable_output(pos);
    }
    
    int newx=int(pos.v.x/gridsize);
    int newy=int(pos.v.y/gridsize);
       
    if(newx<0 || newx>=G->height || newy<0 || newy>=G->width) 
    {
	cout<<"Start position out of grid\n"<<flush;
	return unreachable_output(pos);
    }  
    
    G->start=&G->nodes[newx][newy];
    //cout<<"done preliminaries\n"<<flush;

    if(run_type!=SIMULATION)
    {

    //Code for getting out of an obstacle if inside it. 
    //Not tested on a robot but seems to work in simulation !
	
	if(G->start->type == OBSTACLE) 
	{
	    cout<<"Inside obstacle: getting out\n"<<flush;
	    dist_to_last_planned=0;
	    lastplanned = pos.v;
	    Point2 out;
	    for(int i= path.size();i>=0;i--) 
	    {
		if(G->nodes[int(path[i].x)][int(path[i].y)].type==OBSTACLE)
		    continue;
		out=path[i];
		out.z=0.0;
	    }
	    lastoutput=out;
	    return out;
	}
    }
    
    int gx=int((goal.x+pos.v.x)/gridsize);
    int gy=int((goal.y+pos.v.y)/gridsize);
    
    if(gx<0 || gx>=G->height || gy<0 || gy>=G->width) 
    {
	cout<<"Goal position out of grid\n"<<flush;
	return unreachable_output(pos);
    }
    
    G->goal=&G->nodes[gx][gy];
    
    //cout<<"Current position = ("<<pos.v.x<<","<<pos.v.y<<")\n";
    //cout<<"Distance to goal = ("<<goal.x<<","<<goal.y<<")\n";
    //cout<<"Goal grids = ("<<gx<<","<<gy<<")\n";
    
    if(!started) 
    {
	started=true;
	int sz=obstacles.size();
	for(int i=0;i<sz;i++) 
	{
	    Point2 tmpvec=obstacles[i];
	    make_obstacle(int(tmpvec.x),int(tmpvec.y),int(tmpvec.z),false);	  
	}
#if DEBUG
        G->debug(true,false);
#endif
	init();
	if(!compute() || G->start->g==DSTAR_INFINITY) 
	{
	    return unreachable_output(pos);
	}
	int count=0;
	bool done=false;
	node* st=G->start;
	node* nextstart=NULL;
//find next waypoint
	while(!done && (st->x!=G->goal->x || st->y!=G->goal->y)) 
	{
	    if(planning_only) 
	    {
		count++;
	    }
	    double min=DSTAR_INFINITY;
	    int posn=-1;
	    for(int i=0;i<EDGES;i++) 
	    {
		node* p=st->neighbrs[i];
		if(p==&G->dummy) continue;
		double tmp= (p->g==DSTAR_INFINITY || st->cost[i]==DSTAR_INFINITY) ? DSTAR_INFINITY:(st->cost[i]+p->g);
		if(tmp<min) 
		{
		    min=tmp;
		    posn=i;
		}
	    }
	    if(posn==-1) 
	    {
		return unreachable_output(pos);
	    }
	    nextstart=st->neighbrs[posn];
	    if(posn%2==0 && 
	       (st->neighbrs[(posn-1+EDGES)%EDGES]->type!=nextstart->type || 
		st->neighbrs[(posn+1)%EDGES]->type!=nextstart->type) )
	    {
		done=true;
	    }
	    else if(st->type != nextstart->type &&
		    (st->type!=SAFETY && nextstart->type!=NORMAL))
	    {
		done=true;
	    }
	    else if(planning_only && count==2) 
	    {
		done=true;
	    }
	    else 
	    {
            //Point2 tmp(st->x,st->y,0.0);
	        st=nextstart;
            }
	}
	
	Point2 out(nextstart->x*gridsize+gridsize/2,nextstart->y*gridsize+gridsize/2,0.0);
	lastoutput=out;
	dist_to_last_planned=0;
        lastplanned = pos.v;
	return out;
    }
    
    
    G->start->path=1;
    int sz=obstacles.size();
    for(int i=0;i<sz;i++) 
    {
	Point2 tmpvec=obstacles[i];
	make_obstacle(int(tmpvec.x),int(tmpvec.y),int(tmpvec.z),true);	  
    }
    
    U->percolate_all(calc_key_global,G);
    
    if(!compute() || G->start->g==DSTAR_INFINITY) 
    {
	return unreachable_output(pos);
    }

    bool done=false;
    node* st=G->start;
    node* nextstart=NULL;
    int count=0;
    while(!done  && (st->x!=G->goal->x || st->y!=G->goal->y)) 
    {
	if(planning_only) 
	{
	    count++;
	}
	double min=DSTAR_INFINITY;
	int posn=-1;
	for(int i=0;i<EDGES;i++) 
	{
	    node* p=st->neighbrs[i];
	    if(p==&G->dummy) 
	    {
		continue;
	    }
	    double tmp= (p->g==DSTAR_INFINITY || st->cost[i]==DSTAR_INFINITY) ? DSTAR_INFINITY:(st->cost[i]+p->g);
	    if(tmp<min) 
	    {
		min=tmp;
		posn=i;
	    }
	}
	if(posn==-1) 
	{
	    return unreachable_output(pos);
	}
	nextstart=st->neighbrs[posn];
	//if(per_cell) done=true;/*else*/
	if(posn%2==0 && 
	   (  st->neighbrs[(posn-1+EDGES)%EDGES]->type!=nextstart->type  || 
	      st->neighbrs[(posn+1)%EDGES]->type!=nextstart->type) )
	{
	    done=true;
	}
	else if(st->type != nextstart->type &&
		(st->type!=SAFETY && nextstart->type!=NORMAL))
	    done=true;
	else if(planning_only && count==2) 
	{
	    done=true;
	}
	else 
	{
            //Point2 tmp(st->x,st->y,0.0);
            st=nextstart;
        }
    }
    
    //cout<<"Output vector cell x="<<nextstart->x<<", y="<<nextstart->y<<"\n"<<flush;
    Point2 out(nextstart->x*gridsize+gridsize/2,nextstart->y*gridsize+gridsize/2,0.0);
    //cout<<"end exec-dstar\n"<<flush;
    lastoutput=out;
    lastplanned=pos.v;
    dist_to_last_planned=0;
#if DEBUG
    G->debug(true,false);
#endif
    return out;
}


//debugging function

void Dstar::write_graph(robot_position& pos,char* s) 
{
    Graph* mg=new Graph(G->height,G->width);
    if(!mg) 
    {
	cout<<"Out of memory\n"<<flush;
    }
    int sz=obstacles.size();
    for(int i=0;i<sz;i++) 
    {
	Point2 tmpvec=obstacles[i];
	make_obstacle_g(mg,int(tmpvec.x),int(tmpvec.y),-1);	  
    }    
    long t=time(0);
    char filename[80];
    sprintf(filename,"/net/hr2/ananth/misc/images/");
    sprintf(filename+strlen(filename),"%ld",t);
    sprintf(filename+strlen(filename),s);
    int newx=int(pos.v.x/gridsize);
    int newy=int(pos.v.y/gridsize);
    if(newx<0 || newx>=mg->height || newy<0 || newy>=mg->width);
    else mg->start=&mg->nodes[newx][newy];
#if DEBUG
    mg->debug(false,false,strdup(filename));
    fout<<"Wrote file "<<filename<<"  heading="<<pos.heading<<endl<<flush;
    fout<<"\n\n\n";
#endif
    delete mg;
}

/**********************************************************************
 * $Log: dstar.cpp,v $
 * Revision 1.1.1.1  2008/07/14 16:44:17  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/20 17:17:47  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:37:55  endo
 * MissionLab 7.0
 *
 * Revision 1.4  2006/05/15 21:34:31  endo
 * gcc-3.4 upgrade
 *
 * Revision 1.3  2006/05/15 02:43:33  endo
 * gcc-3.4 upgrade
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
 * Revision 1.1  2003/04/06 07:21:29  endo
 * Initial revision
 *
 **********************************************************************/
