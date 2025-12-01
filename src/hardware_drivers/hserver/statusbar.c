/**********************************************************************
 **                                                                  **
 **                            statusbar.c                           **
 **                                                                  **
 **                                                                  **
 **  Written by:  William Halliburton                                **
 **                                                                  **
 **  Status bar updates for HServer                                  **
 **                                                                  **
 **  Copyright 2000 - 2006, Georgia Tech Research Corporation        **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: statusbar.c,v 1.1.1.1 2006/07/12 13:37:56 endo Exp $ */

#include <string.h>

#include "hserver.h"
#include "cognachrome.h"
#include "nomad.h"
#include "statusbar.h"


Statusbar *statusbar;


char srstr[100];

Statusbar::Statusbar(int size)
{
    this->size = size;
    memset(lines,0,sizeof(char*)*STATUSBAR_MAX_LINES);
    memset(index,-1,sizeof(char)*STATUSBAR_MAX_LINES);
    num_lines = 0;
    window = newwin(size,screenX-2,HS_STATUS_Y,HS_STATUS_X);
    panel = new_panel(window);
    empty_line = new char[300];
    memset(empty_line,' ',screenX-2);
    draw();
    show_panel(panel);
}

void Statusbar::draw(void)
{
    int i;
    if ( SilentMode )
    {
	return;
    }

    pthread_cleanup_push((void(*)(void*))pthread_mutex_unlock, (void *) &screenMutex);
    pthread_mutex_lock(&screenMutex);
    werase(window);
    for(i=0;i<num_lines;i++){
	wmove(window,i,0);
	waddstr(window,lines[i]);
    }
    pthread_cleanup_pop(1);
    redrawWindows();
}

int Statusbar::add(char *line)
{
    int i;
    int idx = -1;
    if (num_lines == STATUSBAR_MAX_LINES) return(-1);
    lines[num_lines] = line;
    for(i=0;i<STATUSBAR_MAX_LINES;i++){
	if (index[i] == -1) {
	    idx = i;
	    break;
	}
    }
    index[idx] = num_lines;
    num_lines++;
    size++;
    statusbarDevideLine++;
    resizeWindows(false);
    resize();
    return(idx);
}

void Statusbar::update(int line_idx)
{
    if (SilentMode) {
       return;
    }

    pthread_cleanup_push((void(*)(void*))pthread_mutex_unlock, (void *) &screenMutex);
    pthread_mutex_lock(&screenMutex);
    wmove(window,index[line_idx],0);
    waddnstr(window,empty_line,screenX-2);
    wmove(window,index[line_idx],0);
    waddnstr(window,lines[index[line_idx]],screenX-2);
    pthread_cleanup_pop(1);
    redrawWindows();
}

void Statusbar::remove(int line_idx)
{
    int i,low;
    low = index[line_idx];
    index[line_idx] = -1;
    for(i=0;i<STATUSBAR_MAX_LINES;i++){
	if (index[i] > low) index[i]--;
	if ((i>=low)&&(i<STATUSBAR_MAX_LINES-1)) lines[i] = lines[i+1];
    }
    num_lines--;
    size--;
    statusbarDevideLine--;
    resizeWindows(false);
    draw();
}

void Statusbar::resize(void)
{
    wresize(window,size,screenX-2);
}

StatusbarSpinner::StatusbarSpinner(void) :
    counter_(0)
{
}

StatusbarSpinner::~StatusbarSpinner(void)
{
}

char StatusbarSpinner::getStatus(void)
{
    char status = '\0';

    switch(counter_) {

    case 0:
        status = '-';
        break;
    case 1:
        status = '\\';
        break;
    case 2:
        status = '|';
        break;
    case 3:
        status = '/';
        break;
    }

    counter_ = (counter_ + 1) % 4;

    return status;
}

/**********************************************************************
# $Log: statusbar.c,v $
# Revision 1.1.1.1  2006/07/12 13:37:56  endo
# MissionLab 7.0
#
# Revision 1.2  2006/06/05 21:35:52  endo
# PoseCalculator from MARA 2020 migrated
#
# Revision 1.3  2004/09/10 19:41:08  endo
# New PoseCalc integrated.
#
# Revision 1.2  2004/04/06 02:07:40  pulam
# Added code for Silent Mode
#
# Revision 1.1.1.1  2004/02/27 22:30:51  endo
# Initial MARS 2020 revision
#
# Revision 1.2  2000/09/19 03:49:46  endo
# RCS log added.
#
#
#**********************************************************************/
