/**********************************************************************
 **                                                                  **
 **                             port.hpp                             **
 **                                                                  **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1995, 1996, 1997 Georgia Tech Research Corporation    **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **********************************************************************/

/* $Id: port.hpp,v 1.1.1.1 2008/07/14 16:44:15 endo Exp $ */


#ifndef PORT_HPP
#define PORT_HPP

/*-------------------------------------------------------------------*/


struct S_users
{
   struct S_bh    *user;	/* record with input link to this output */
   struct S_users *next;	/* pointer to the next users record */
};
typedef struct S_users T_users;

/*-------------------------------------------------------------------*/

class port
{
   char   	*name;		/* port name */
   PORTTYPES    type;		/* data type of this port */

   Widget input_button;		/* this button widget for lookups */
   Widget input_label;		/* The corresponding label widget */
   struct S_bh  *src;		/* pointer to the data record for the source */
   int           is_const;	/* is this a constant? */
   char         *value;		/* initial value for the constant */
   int           is_list;	/* are multiple inputs allowed? */
};

/*-------------------------------------------------------------------*/

struct S_bh
{
   int          is_page_header;         /* TRUE or FALSE */
   struct S_bh *parent;	                /* pointer to our parent rec
					 * In page_headers, points to parent pg
					 * else, points to this page's hdr rec.
					 */
   struct S_bh *next;          		/* pointer to next glyph in chain */
   char 	behavior_name[NODE_NAME_SIZE+1];  /* behavior name */
   char 	node_name[NODE_NAME_SIZE+1];  /* node name */
   Widget       glyph;			/* widget for this glyph */
   Widget       output_button;		/* this output button for lookups */
   Widget       name_field;		/* name text widget for lookups */
   T_users     *users;          	/* chain of pointers to buttons using */
					/* this output */
   int          type;			/* type of this output */
   int          x,y;			/* initial screen location */
   MOD_CLASS    mod_class;		/* class of primitive module:
					 *  CLS_Motor, CLS_Perceptual, 
					 *  CLS_Actuator, CLS_Sensor, 
					 *  CLS_Coordination
					 * or page type:
					 *  CLS_Complex_Perception, CLS_Behavior
					 *  CLS_Assemblage, CLS_Robot, 
					 *  CLS_Society
					 */
   int          bp_class;		/* if binding point, then class of obj*/
   struct S_bh *producer;	        /* if mod_class is not primitive, then*/
					/* pointer to our child producer rec */
   int 		num_inputs;		/* number of inputs in 'inputs' */
   T_input     *inputs;		      	/* allocated to be size in num_inputs */
};
typedef struct S_bh T_bh;

/*-------------------------------------------------------------------*/

typedef struct T_key_val
{
   struct T_key_val *next; 
   char 	    *key;
   char             *val;
} KEY_VAL;



Widget
PostDialog(Widget parent, int dialog_type, char *msg);

int
bl_load(char *file_name,
        T_bh **rtn_list,
        XmString *behavior_names[NUM_NODE_TYPES],
        int      num_behaviors[NUM_NODE_TYPES]);


void select_behavior(Widget w,int mod_class);
int bh_find_button(Widget w, T_bh **rtn,int *is_output,int *input_num);
T_bh *bh_find_name(char *behavior_name);
T_bh *bh_new_node(T_bh *behavior);
T_bh *bh_find_node_name(char *node_name);
T_bh *bh_find_node(Widget w);
void grow_bh_node(T_bh *cur, int extra);
void shrink_bh_node(T_bh *cur, int slot);
int bh_behavior_link(T_bh *cur, char *input_name);
Widget add_const(Widget w, T_bh *this_glyph, int input_num);
void bh_print_node_list(void);
T_bh *create_box(T_bh *behavior, KEY_VAL *key_val);
T_bh *new_bh(int num_inputs,MOD_CLASS mod_class, int type, char *bhv_name);
T_bh *new_bhp(int num_inputs, MOD_CLASS mod_class, int type, char *bhv_name);
void set_options(T_bh *cur, KEY_VAL *key_val);
void update_node_info(T_bh *dst, T_bh *src);


extern T_bh       *book;	/* Current stack of pages */
extern T_bh       *cur_page;	/* Current pages */
extern T_bh       *cur_templates;	/* list of behaviors in current arch */

#ifndef BINDING_H
#include "binding.h"
#endif

extern int       num_behaviors[NUM_ARCHES][MAX_ROBOTS][NUM_NODE_TYPES];
extern XmString *behavior_names[NUM_ARCHES][MAX_ROBOTS][NUM_NODE_TYPES];
extern XmString  select_dialog_msg[NUM_NODE_TYPES];

const int BAD_TYPE = -1;
extern int VOID_TYPE;  /* the type id assigned to void */
extern int FSA_STATE_TYPE;  /* the current state of the fsa */
extern int FSA_STATE_DEF_TYPE;  /* the string definition of an fsa state */
extern char *STATE_LINK_NAME;
int find_type(char *type);
int add_type(char *type);
char *name_of_type(int type);

#endif


///////////////////////////////////////////////////////////////////////
// $Log: port.hpp,v $
// Revision 1.1.1.1  2008/07/14 16:44:15  endo
// MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
//
// Revision 1.1.1.1  2006/07/20 17:17:45  endo
// MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
//
// Revision 1.1.1.1  2006/07/12 13:37:54  endo
// MissionLab 7.0
//
// Revision 1.1.1.1  2005/02/06 22:59:33  endo
// AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
//
// Revision 1.3  1997/02/14  15:55:46  zchen
// *** empty log message ***
//
// Revision 1.2  1995/06/29  18:10:57  jmc
// Added header and RCS id and log strings.
///////////////////////////////////////////////////////////////////////
