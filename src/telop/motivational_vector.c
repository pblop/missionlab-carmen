/**********************************************************************
 **                                                                  **
 **                  motivational_vector.c                           **
 **                                                                  **
 **  Written by:  Alexander Stoytchev                                **
 **                                                                  **
 **  Copyright 2000, Georgia Tech Research Corporation               **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: motivational_vector.c,v 1.1.1.1 2008/07/14 16:44:24 endo Exp $ */

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>

#include <Xm/MessageB.h>
#include <Xm/PushB.h>
#include <Xm/Scale.h>

#include "gt_com.h"
#include "console.h"
#include "shared.h"
#include "motivational_vector.h"
#include "motivational_vector_window.h"

//---------------------------------------------------------------------
// Constants
//---------------------------------------------------------------------
#define MOTIVATIONAL_VECTOR_FILENAME "motivational_vector_file"

//---------------------------------------------------------------------
// Globals
//---------------------------------------------------------------------
motivational_vector_type gMotivationalVector_vectors[9];
int gMotivationalVector_numVectors = 0;

Widget MotivationalVector_interface;
bool  MotivationalVector_isUp = false;

//---------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------
Boolean my_proc(XtPointer client_data)
{
    static int count=0;
    char *res;
    double val;

    count= (count+1) %50000;

    if((count==0) && (gMotivationalVector_sliderWidget != NULL))
    {
	res=(char *)gt_inquire(1, "anger_level");
	val=atof(res)*100;
	XmScaleSetValue(gMotivationalVector_sliderWidget[0],(int)val);

	res=(char *)gt_inquire(1, "fear_level");
	val=atof(res)*100;
	XmScaleSetValue(gMotivationalVector_sliderWidget[1],(int)val);

	res=(char *)gt_inquire(1, "hunger_level");
	val=atof(res)*100;
	XmScaleSetValue(gMotivationalVector_sliderWidget[2],(int)val);

	res=(char *)gt_inquire(1, "curiousity_level");
	val=atof(res)*100;
	XmScaleSetValue(gMotivationalVector_sliderWidget[3],(int)val);
    }

    return false; // means we want to be called again
}


void gt_popup_motivational_vector_interface(void)
{
    if (gMotivationalVector_numVectors > 0)
    {
	// Pop up the interface
	XtPopup(MotivationalVector_interface, XtGrabNone);
	MotivationalVector_isUp = true;
	 
    }
    else
    {
	perror("Warning: No motivational vectors are defined.");
	MotivationalVector_isUp = false;
    } 
}

void  gt_end_motivational_vector(int window)
{
    if(window == 1) MotivationalVector_isUp = false;
}

void  read_motivational_vector_file(char * filename)
{
    FILE  *fp;
    int  i, j;
    int warnUser = FALSE;

    // DCM: June 23, 1995.
    //  Need to make sure that you got the file before you read from it.
    if ( (fp = fopen(filename, "r")) == NULL )
    {
	if (warnUser)
	{
	    perror("Warning: did not find motivational_vector_file, using defaults");
	}

	// Use default values
	gMotivationalVector_numVectors = 4;

	strcpy(gMotivationalVector_vectors[0].title, "Anger");
	gMotivationalVector_vectors[0].num_params = 1;
	strcpy(gMotivationalVector_vectors[1].params[0].key, "anger_level");
	gMotivationalVector_vectors[0].params[0].base = 0.5;
	gMotivationalVector_vectors[0].params[0].inc = 1;
   
	strcpy(gMotivationalVector_vectors[1].title, "Fear");
	gMotivationalVector_vectors[1].num_params = 1;
	strcpy(gMotivationalVector_vectors[1].params[0].key, "fear_level");
	gMotivationalVector_vectors[1].params[0].base = 0.5;
	gMotivationalVector_vectors[1].params[0].inc = 1;
 
	strcpy(gMotivationalVector_vectors[2].title, "Hunger");
	gMotivationalVector_vectors[2].num_params = 1;
	strcpy(gMotivationalVector_vectors[2].params[0].key, "hunger_level");
	gMotivationalVector_vectors[2].params[0].base = 0.5;
	gMotivationalVector_vectors[2].params[0].inc = 1;
  
	strcpy(gMotivationalVector_vectors[3].title, "Curiousity");
	gMotivationalVector_vectors[3].num_params = 1;
	strcpy(gMotivationalVector_vectors[3].params[0].key, "curiousity_level");
	gMotivationalVector_vectors[3].params[0].base = 0.5;
	gMotivationalVector_vectors[3].params[0].inc = 1;
    }

    if (fp != NULL)
    {

	fscanf(fp, "Number of mv_sliders: %d\n", &gMotivationalVector_numVectors);

	for (i=0; i<gMotivationalVector_numVectors; i++)
	{
	    fscanf(fp, "title: %s\n", gMotivationalVector_vectors[i].title);
	    fscanf(fp, "num_params: %d\n", &(gMotivationalVector_vectors[i].num_params));
	    for (j=0; j<gMotivationalVector_vectors[i].num_params; j++)
	    {
		fscanf(fp, "key: %s\n", gMotivationalVector_vectors[i].params[j].key);
		fscanf(fp, "base: %lf\n", &(gMotivationalVector_vectors[i].params[j].base));
		fscanf(fp, "inc: %d\n", &(gMotivationalVector_vectors[i].params[j].inc));
	    }
	}
 
	fclose(fp);
    }
}

void  gt_create_motivational_vector_interface(Widget top_level, XtAppContext app)
{
    char value[10];
    gMotivationalVectorTopLevel = top_level;
    gMotivationalVectorAppContext = app;  
    gMotivationalVectorDisplay = XtDisplay(gMotivationalVectorTopLevel);
    gMotivationalVectorScreen = XDefaultScreen(gMotivationalVectorDisplay);

    read_motivational_vector_file(MOTIVATIONAL_VECTOR_FILENAME);

    sprintf(value, "%f", 0.5);
    gt_update(BROADCAST_ROBOTID, "anger_level", value);
    gt_update(BROADCAST_ROBOTID, "fear_level", value);
    gt_update(BROADCAST_ROBOTID, "hunger_level", value);
    gt_update(BROADCAST_ROBOTID, "curiousity_level", value);

    if (gMotivationalVector_numVectors > 0)
    {
	MotivationalVector_interface = create_motivational_vector_window();
    }

    add_workproc(my_proc, NULL);
}

/**********************************************************************
 * $Log: motivational_vector.c,v $
 * Revision 1.1.1.1  2008/07/14 16:44:24  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/20 17:17:52  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:38:01  endo
 * MissionLab 7.0
 *
 * Revision 1.1.1.1  2005/02/06 23:00:21  endo
 * AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
 *
 * Revision 1.12  2002/07/18 17:02:58  endo
 * Fixed the problem of not displaying correctly with OpenMotif.
 *
 * Revision 1.11  2002/01/13 01:48:10  endo
 * Commented  perror("Warning: did not find motivational_vector_file, ...")
 *
 * Revision 1.10  2001/12/23 20:33:40  endo
 * RH 7.1 porting.
 *
 * Revision 1.9  2001/01/24 23:13:52  saho
 * Fixed the update of all motivational variables. So far Only Curiosity was
 * updatad after a slider bar bm[3~  move.
 * ,.
 *
 * Revision 1.8  2000/03/24 18:22:21  saho
 * *** empty log message ***
 *
 * Revision 1.7  2000/03/24 17:41:31  saho
 * *** empty log message ***
 *
 * Revision 1.6  2000/03/24 17:39:50  saho
 * *** empty log message ***
 *
 * Revision 1.5  2000/03/24 17:33:51  saho
 * Disabled a debug frintf.
 *
 * Revision 1.4  2000/03/24 17:08:51  saho
 * The motivational vector window sliders are now updated if the values of the
 * variables in the database change.
 *
 * Revision 1.3  2000/03/01 00:42:13  saho
 * *** empty log message ***
 *
 * Revision 1.2  2000/03/01 00:35:29  saho
 * Bug fix. Default motivational_vector_file did not work.
 *
 * Revision 1.1  2000/02/29 21:16:20  saho
 * Initial revision
 *
 **********************************************************************/
