/**********************************************************************
 **                                                                  **
 **                         draw_military.c                          **
 **                                                                  **
 ** Written by: Patrick Ulam                                         **
 **                                                                  **
 ** Copyright 2007 - 2008 Georgia Tech Research Corporation          **
 ** Atlanta, Georgia  30332-0415                                     **
 ** ALL RIGHTS RESERVED, See file COPYRIGHT for details.             **
 **                                                                  ** 
 **********************************************************************/

/* $Id: draw_military.h,v 1.2 2008/07/16 21:16:08 endo Exp $ */

#ifndef DRAWMILITARY_H
#define DRAWMILITARY_H


void Draw_MilitaryStyle(float x, float y, float nx, float ny, int mobility, GC gc, int erase, char *color);
void Draw_Neutral(float x, float y, float nx, float ny, int mobility, int erase, GC gc);
void Draw_Friendly(float x, float y, float nx, float ny, int mobility, int erase, GC gc);
void Draw_Enemy(float x, float y, float nx, float ny, int mobility, int erase, GC gc);

extern bool gt_draw_robot_military_style;

#endif

/**********************************************************************
# $Log: draw_military.h,v $
# Revision 1.2  2008/07/16 21:16:08  endo
# Military style is now a toggle option.
#
#**********************************************************************/
