#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gt_simulation.h"

void fill_los_string(char **&robcol)
{
   robcol = (char**)malloc(4*sizeof (char*));
   for (int i = 0; i < 4; i++)
        robcol[i] = (char*)malloc(16*sizeof (char));

   strcpy(robcol[0], "PSblue");
   strcpy(robcol[1], "PSred");
   strcpy(robcol[2], "PSgreen");
   strcpy(robcol[3], "PSyellow");
}

void fill_goal_string(char **&robgoal)
{
   robgoal = (char**)malloc(4*sizeof (char*));
   for (int i = 0; i < 4; i++)
        robgoal[i] = (char*)malloc(16*sizeof (char));

   strcpy(robgoal[0], "TGblu");
   strcpy(robgoal[1], "TGred");
   strcpy(robgoal[2], "TGgre");
   strcpy(robgoal[3], "TGyel");
}

void fill_id_string(char **&robcid)
{
   robcid = (char**)malloc(4*sizeof (char*));
   for (int i = 0; i < 4; i++)
        robcid[i] = (char*)malloc(16*sizeof (char));

   strcpy(robcid[0], "Rblu");
   strcpy(robcid[1], "Rred");
   strcpy(robcid[2], "Rgre");
   strcpy(robcid[3], "Ryel");
}

bool get_colID(char *&colId)
{
   if (!strcmp(colId = get_state("ColID"), ""))
   {
        colId = (char*)malloc(10*sizeof(char));
        strcpy(colId, "Rblu");
	put_state("ColID", colId);
	return false;
   }
   else
        return true;

}

bool has_less_stale(int oth)
{
   char *ptr;
   char *value = get_state("Stale");
   if (strcmp(value, ""))
   {
	int stale = strtol(value, &ptr, 10);
	value = ptr++;
	int nstale = strtol(value, &ptr, 10);
	if (stale > 5*nstale && oth == 0)
		return true;
	else 
		return false; 
   }
   else
	return false;
}

void set_helper(char *rob)
{
   char *value = (char*) malloc(100*sizeof(char));
   sprintf(value, "%s", rob);
   put_state("Helper", value);
   free(value);
}

char* get_helper()
{
   char *value = get_state("Helper");
   return value;
}

void reset_my_stale()
{
   //printf("\n-----------ResetState");
   char *value = (char*) malloc(100*sizeof(char));
   sprintf(value, "%d %d %d", 0, 1, 0);
   put_state("Stale", value);
   free(value);
}

void update_my_stale(int stale)
{
   char *ptr;
   char *value = get_state("Stale");
   int oldstale;
   int nstale;
   int stceil;
   int instale;

   if (strcmp(value, ""))
   {
	oldstale = strtol(value, &ptr, 10);
	value = ptr+1;
	nstale = strtol(value, &ptr, 10);
	value = ptr+1;
	instale = strtol(value, &ptr, 10);
   }
   else
   {
	oldstale = 0;
	nstale   = 1;
	instale  = 0;
   }	

   stceil   = min(30000, 60*nstale);
   if (stale == 1)
   {
	if (oldstale < stceil)
		oldstale = oldstale + nstale;
	instale = 1;
   }
   if (stale == 0)
   {
	if (oldstale > 0)
		oldstale --;
	if (oldstale == 0 && instale)	
	{
		instale = 0;
		nstale = min( 1000, nstale + 1);
	}
   }
   if (stale == -1)
   {
	oldstale = 0;
	instale = 0;
   }
   value = (char*) malloc(100*sizeof(char));
   sprintf(value, "%d %d %d", oldstale, nstale, instale);
   put_state("Stale", value);
   free(value);
}

int get_my_stale()
{
   char *ptr;
   char *value = get_state("Stale");
   if (strcmp(value, ""))
   {
	int stale = strtol(value, &ptr, 10);
	return stale;
   }
   else
	return 0;
}

int get_state_info(char *about)
{
   char *sumid = (char*)malloc(100*sizeof(char));
   strcpy(sumid, "Stale");
   strcat(sumid, about);

   char *ptr;
   char *value = get_state(sumid);
   if (strcmp(value, ""))
   {
	int state = strtol(value, &ptr, 10);
	free(sumid);
	return state;
   }
   else
   {
	free(sumid);
	return 0;
   }
}

bool rec_state_info(char *send, char *rec) 
{
   char *sumid = (char*)malloc(100*sizeof(char));
   strcpy(sumid, "Stale");
   strcat(sumid, send);
   strcat(sumid, rec);

   char *value;
   if ( (value = return_state(sumid))==NULL)
   {
	free(sumid);  
	return false;
   }
   else
   {
        //int state = (int) strtol(value, &ptr, 10);
	//printf("\n State %d", state);
	*(sumid+9)=0;        
	put_state(sumid, value);
	free(sumid);  
	return true;
   } 
}

void send_state_info(char *send, char *rec) 
{
   int stale = get_my_stale();
   char *sumid = (char*)malloc(100*sizeof(char));
   strcpy(sumid, "Stale");
   strcat(sumid, send);
   strcat(sumid, rec);
   
   char *value = (char*)malloc(100*sizeof(char));
   sprintf(value, "%d", stale); 
   exec_broadcast_value(sumid, value, THIS_ROBOT);
   free(value);
   free(sumid);
}

bool get_pos_shot(Vector &pos, double &heading)
{
   char* value, *ptr;
   if (strcmp(value = get_state("PosShot"), ""))
   {
        pos.x = (double)strtod(value, &ptr);
        value = ptr+1;
        pos.y = (double)strtod(value, &ptr);
        value = ptr+1;
        heading = (double)strtod(value, &ptr);
   	return true; 
   }
   else
	return false;
}

bool set_pos_shot(Vector pos, double heading)
{
   char *value = (char*)malloc(100*sizeof(char));
   sprintf(value, "%f %f %f", pos.x, pos.y, heading);
   put_state("PosShot", value);
   free(value);
   return true;
}


bool has_los(char *name, Vector &pos) 
{
   char *ptr;
   char *value = get_state(name+1);
   if(strcmp(value, "") && (bool)strtol(value,&ptr, 10))
   {
        value = ptr+1;
        pos.x = (double)strtod(value, &ptr);
        value = ptr+1;
        pos.y = (double)strtod(value, &ptr);
	return true;
   }
   else
	return false;	
}

int get_my_visual_goal(Vector &vg, int &ptvg)
{
   char *ptr;
   char *value = get_state("VGoal");
   if (strcmp(value, ""))
   {
	int nh = (int)strtol(value, &ptr, 10);
        value = ptr+1;
        vg.x = (double)strtod(value, &ptr);
        value = ptr+1;
        vg.y = (double)strtod(value, &ptr);
        value = ptr+1;
	ptvg = (int)strtol(value, &ptr, 10);
	return nh;
   }
   else
	return 0;
}

void set_my_visual_goal(int see, Vector vg, int ptvg)
{
   char *value = (char*) malloc(100*sizeof(char));
   sprintf(value, "%d %f %f %d", see, vg.x, vg.y, ptvg);
   put_state("VGoal", value);
   free(value);
}

bool get_visual_goal(char *sender, char*about, int &goalID, Vector &pos, int &ptg, int &pptg)
{
   char *name = (char*)malloc(16*sizeof(char));
   strcpy(name, sender);
   strcat(name, about);
   char *ptr; 
   char *value = get_state(name);
   if(strcmp(value, "") && (bool)strtol(value,&ptr, 10))
   {
	value = ptr+1;
        goalID = (int)strtol(value, &ptr, 10);
        value = ptr+1;
        pos.x = strtod(value, &ptr);
        value = ptr+1;
        pos.y = strtod(value, &ptr);
        value = ptr+1;
        ptg = (int)strtol(value, &ptr, 10);
        value = ptr+1;
        pptg = (int)strtol(value, &ptr, 10);
	free(name);
	return true;
   }
   else 
   {
	free(name);
	return false;	
   }
 }

bool put_visual_goal(char *sender, char *about, bool see, int goalID, Vector pos, int ptg, int optg)
{
        char *name = (char*)malloc(16*sizeof(char));
        strcpy(name, sender);
        strcat(name, about);
	char *value = (char*)malloc(100*sizeof(char));
        sprintf(value, "%d %d %f %f %d %d", see, goalID, pos.x, pos.y, ptg, optg);
        exec_broadcast_value(name, value, THIS_ROBOT);
        put_state(name, value);
        free (value);
	free (name);
	return true;
}

bool has_visual_goal(Vector goal_rel_loc, obs_array readings, double d_sc)
{
   bool vgoal = false;
   Vector g;
   g.x = -goal_rel_loc.x;
   g.y = -goal_rel_loc.y;
   double aval = sqrt(g.x*g.x+g.y*g.y);

   if (aval==0)
        vgoal = true;
   else
   {
   	g.x = g.x/aval;
   	g.y = g.y/aval;

   	Vector r;
   	for(int i=0; i<readings.size; i++)
   	{
        	r.x = -readings.val[i].center.x;
        	r.y = -readings.val[i].center.y;

        	aval = sqrt(r.x*r.x+r.y*r.y);
        	if (aval>0)
        	{
                	r.x = r.x/aval;
                	r.y = r.y/aval;
        	}

        	if ((r.x*g.x+r.y*g.y) > d_sc)
        	{
                	if (len_2d(readings.val[i].center)
                        	> len_2d(goal_rel_loc))
                        	vgoal = true;
                	else
                	{
                        	vgoal = false;
                        	i = readings.size;
                	}
        	}
   	}
   }
   return vgoal;
}

bool rec_new_goal_info(char *about, char *send, char *rec, int &ts, Vector &goal, Vector &mpos) 
{
   char *sumid = (char*)malloc(100*sizeof(char));
   strcpy(sumid, about);
   strcat(sumid, send);
   strcat(sumid, rec);

   char *value, *ptr;
   if ( (value = return_state(sumid))==NULL)
   {
	free(sumid);  
	return false;
   }
   else
   {
	ts = strtol(value, &ptr, 10);
        value = ptr+1;
        goal.x = strtod(value, &ptr);
        value = ptr+1;
        goal.y = strtod(value, &ptr);
        value = ptr+1;
        mpos.x = strtod(value, &ptr);
        value = ptr+1;
        mpos.y = strtod(value, &ptr);
	free(sumid);  
	return true;
   } 
}

void send_new_goal_info(char *about, char *send, char *rec, int ts, Vector goal, Vector mpos) 
{
   char *sumid = (char*)malloc(100*sizeof(char));
   strcpy(sumid, about);
   strcat(sumid, send);
   strcat(sumid, rec);

   char *value = (char*)malloc(100*sizeof(char));
   sprintf(value, "%d %f %f %f %f", ts, goal.x, goal.y, mpos.x, mpos.y); 
   exec_broadcast_value(sumid, value, THIS_ROBOT);
   free(value);
   free(sumid);
}

bool get_goal_info(char *about, int &ts, Vector &goal) 
{
   char *value, *ptr;
   if ( !strcmp(value = get_state(about), ""))
	return false;
   else
   {
	ts = strtol(value, &ptr, 10);
        value = ptr+1;
        goal.x = strtod(value, &ptr);
        value = ptr+1;
        goal.y = strtod(value, &ptr);
	return true;
   } 
}

void put_goal_info(char *about, int ts, Vector goal) 
{
   char *value;
   value = (char*)malloc(100*sizeof(char));
   sprintf(value, "%d %f %f", ts, goal.x, goal.y);
   put_state(about, value);
   free(value);
}

Vector rel_to_abs(Vector pos, double heading, Vector p_to_pos)
{
   Vector p_to_abs = p_to_pos;
   rotate_z(p_to_abs, heading);
   p_to_abs.x = p_to_abs.x + pos.x;
   p_to_abs.y = p_to_abs.y + pos.y;
   return p_to_abs;
}

Vector abs_to_rel(Vector pos, double heading, Vector p_to_abs)
{
   Vector p_to_pos;
   minus_2d(p_to_abs, pos, p_to_pos);
   rotate_z(p_to_pos, -1 * heading);
   return p_to_pos;
}
