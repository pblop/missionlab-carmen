/**********************************************************************
 **                                                                  **
 **                            report_level.c                        **
 **                                                                  **
 **                                                                  **
 **  Written by:  William Halliburton                                **
 **                                                                  **
 **  Copyright 2000 - 2006, Georgia Tech Research Corporation        **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: report_level.c,v 1.1.1.1 2006/07/12 13:37:56 endo Exp $ */

#include <string.h>
#include <stdlib.h>
#include <panel.h>

#include "hserver.h"
#include "report_level.h"

#include "nomad.h"
#include "pioneer.h"
#include "ipt_handler.h"
#include "cognachrome.h"
#include "video.h"
#include "xwindow.h"
#include "apm_module.h"

ReportLevel *reportlevel;

char *ReportLevel::levStr(int report_level){
    switch(report_level){
    case HS_REP_LEV_NONE:
	return("none");
	break;
    case HS_REP_LEV_ERR:
	return("error");
	break;
    case HS_REP_LEV_NORMAL:
	return("normal");
	break;
    case HS_REP_LEV_DEBUG:
	return("debug");
	break;
    }
    return("unknown");
}

int ReportLevel::drawWindow(){
    int i;
    int line = 1;
    char buf[100];
    char sel[5];
    pthread_cleanup_push((void(*)(void*))pthread_mutex_unlock, (void *) &screenMutex);
    pthread_mutex_lock(&screenMutex);
    werase(window);
    wmove(window,0,0);
    wborder(window,0,0,0,0,0,0,0,0);
    mvwaddstr(window,line++,1,"Report Level");
    line++;
    for(i=0;i<num_lines;i++){
	wmove(window,i+3,3);
	if (curSelect == i) strcpy(sel,">");
	else strcpy(sel," ");
	sprintf(buf,"%1s %-15s %s",sel,names[i],levStr(*lines[i]));
	waddstr(window,buf);
	line++;
    }

    line ++;
    mvwaddstr(window,line++,1,"  Use arrow keys to adjust");
    mvwaddstr(window,line++,1,"  x exit  d debug  n normal");
    pthread_cleanup_pop(1);
    return(0);
}

int ReportLevel::add(char *name,int *line){
    int i;
    int idx = -1;
    if (num_lines == REPORT_LEVEL_MAX_LINES) return(-1);
    lines[num_lines] = line;
    names[num_lines] = name;
    for(i=0;i<REPORT_LEVEL_MAX_LINES;i++){
	if (index[i] == -1) {
	    idx = i;
	    break;
	}
    }
    index[idx] = num_lines;
    num_lines++;
    // size++; Ernest -this variable apparantly not used for anything
    return(idx);
}   


void ReportLevel::remove(int line_idx){
    int i,low;
    low = index[line_idx];
    index[line_idx] = -1;
    for(i=0;i<REPORT_LEVEL_MAX_LINES;i++){
	if (index[i] > low) index[i]--;
	if ((i>=low)&&(i<STATUSBAR_MAX_LINES-1)) lines[i] = lines[i+1];
    }
    num_lines--;
    // size--; Ernest -this variable not used for anything
}


void ReportLevel::capInt(int *i,int l,int u){
  if (*i<l) *i = l;
  if (*i>u) *i = u;
}

void ReportLevel::movePanel(int xdif,int ydif){
    panX += xdif;
    panY += ydif;
    capInt(&panX,0,mainPanelX-REPORTLEVEL_WIN_COL);
    capInt(&panY,0,mainPanelY-REPORTLEVEL_WIN_ROW);
    move_panel(panel,panY,panX);
    redrawWindows();
}

void ReportLevel::movePanelKey(int c){
    switch(c){
    case '1':
	movePanel(-1,1);
	break;
    case '2':
	movePanel(0,1);
	break;
    case '3':
	movePanel(1,1);
	break;
    case '4':
	movePanel(-1,0);
	break;
    case '5':
	break;
    case '6':
	movePanel(1,0);
	break;
    case '7':
	movePanel(-1,-1);
	break;
    case '8':
	movePanel(0,-1);
	break;
    case '9':
	movePanel(1,-1);
	break;
    }
}

void ReportLevel::adjust(int adj, int set)
{
    int *lev;
    
    lev = lines[curSelect];
    if (set!=-1) *lev = set;
    else *lev += adj;
    if (*lev<0) *lev = 0;
    if (*lev>HS_REP_LEV_DEBUG) *lev = HS_REP_LEV_DEBUG;
}


int ReportLevel::runKey(int c){
    switch(c){
    case 'r':
	refreshScreen();
	break;
    case 'x':
    case 'Q':
    case KEY_ESC:
	return(1);
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
	movePanelKey(c);
	break;
    case 'd':
	adjust(0,HS_REP_LEV_DEBUG);
	drawWindow();
	redrawWindows();
	break;
    case 'n':
	adjust(0,HS_REP_LEV_NORMAL);
	drawWindow();
	redrawWindows();
	break;
    case KEY_DOWN:
	curSelect++;
	if (curSelect>=num_lines) curSelect = num_lines-1;
	drawWindow();
	redrawWindows();
	break;
    case KEY_UP:
	curSelect--;
	if (curSelect<0) curSelect = 0;
	drawWindow();
	redrawWindows();
	break;
    case KEY_LEFT:
	adjust(-1);
	drawWindow();
	redrawWindows();
	break;
    case KEY_RIGHT:
	adjust(1);
	drawWindow();
	redrawWindows();
	break;
    }
    return(0);
}

void ReportLevel::show(){
    int c,done;
    
    done = false;
    show_panel(panel);
    drawWindow();
    redrawWindows();
    do {
	c = getch();
	done = runKey(c);
    } while (!done);
    hide_panel(panel);
    redrawWindows();
    return;
}


// Ernest
ReportLevel::~ReportLevel()
{

    delete [] names;
 
}

ReportLevel::ReportLevel(){
    int i;
    
    curSelect = 0;
    maxNameLen = 20;
    for(i=0;i<REPORT_LEVEL_MAX_LINES;i++){
	names[i] = new char[maxNameLen];
    }
    memset(index,-1,sizeof(char)*REPORT_LEVEL_MAX_LINES);
    memset(lines,0,sizeof(int*)*REPORT_LEVEL_MAX_LINES);
    memset(names,0,sizeof(char*)*REPORT_LEVEL_MAX_LINES);
    num_lines = 0;
    panX = REPORTLEVEL_WIN_X;
    panY = REPORTLEVEL_WIN_Y;
    panH = REPORTLEVEL_WIN_ROW;
    panW = REPORTLEVEL_WIN_COL;
    if (panH>screenY) panH = screenY;
    if (panW>screenX) panW = screenX;
    if (panX+panW>screenX) panX = screenX-panW;
    if (panY+panH>screenY) panY = screenY-panH;
    window = newwin(panH,panW,panY,panX);
    panel = new_panel(window);
    drawWindow();
    hide_panel(panel);
}

/**********************************************************************
# $Log: report_level.c,v $
# Revision 1.1.1.1  2006/07/12 13:37:56  endo
# MissionLab 7.0
#
# Revision 1.2  2006/06/05 21:35:52  endo
# PoseCalculator from MARA 2020 migrated
#
# Revision 1.1.1.1  2005/02/06 22:59:42  endo
# AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
#
# Revision 1.6  2003/04/06 13:32:37  endo
# gcc 3.1.1
#
# Revision 1.5  2002/01/16 22:21:49  ernest
# Explicit deletion of names array.
#
# Revision 1.4  2000/11/13 20:09:23  endo
# ARTV-Jr class added to HServer.
#
# Revision 1.3  2000/09/19 03:47:14  endo
# RCS log added.
#
#
#**********************************************************************/
