/**********************************************************************
 **                                                                  **
 **                           3drobot.cpp                            **
 **                                                                  **
 ** Written by: Patrick Daniel Ulam                                  **
 **                                                                  **
 ** Copyright 2005 Georgia Tech Research Corporation                 **
 ** Atlanta, Georgia  30332-0415                                     **
 ** ALL RIGHTS RESERVED, See file COPYRIGHT for details.             **
 **                                                                  **
 **********************************************************************/

/* $Id: 3drobot.cpp,v 1.1.1.1 2008/07/14 16:44:24 endo Exp $ */

#include <stdio.h>
#include <GL/glu.h>
#include <GL/glx.h>
#include <GL/gl.h>

#include "3d_vis_wid.h"
#include "pvector.h"
#include "model_3ds.h"


const string Robot::STRING_RED_ = "red";
const string Robot::STRING_GREEN_ = "green";
const string Robot::STRING_PURPLE_ = "purple";
const string Robot::STRING_BLACK_ = "black";
const string Robot::STRING_GRAY_ = "gray";
const string Robot::STRING_ORANGE_ = "orange";
const string Robot::STRING_YELLOW_ = "yellow";

Robot::Robot(
    int robotnum,
    double x,
    double y,
    double z,
    double heading,
    string color,
    int rtype) :
    Robot_Num(robotnum),
    X(x),
    Y(y),
    Z(z),
    Theta(heading),
    Robot_Scale(1),
    Robot_Type(rtype)
{
    Set_Color(color);

    Uncertainty = false;
}

void Robot::Set_Color(string color)
{
    color_ = color;

    red = 0;
    green = 0;
    blue = 0;

    if (color == STRING_RED_)
    {
        red = 0.5f;
    }
    else if (color == STRING_GREEN_)
    {
        green = 0.5;
    }
    else if (color == STRING_PURPLE_)
    {
        red = 0.5;
        blue = 0.5;
    }
    else if (color == STRING_BLACK_)
    {
        // Do nothing.
    }
    else if (color == STRING_GRAY_)
    {
        /*
        red = .2;
        green = .2;
        blue = .2;
        */
        red = 140.0/255.0;
        green = 140.0/255.0;
        blue = 140.0/255.0;
    }
    else if (color == STRING_ORANGE_)
    {
        red = 0.5;
        green = 75.0/255.0;
    }
    else if (color == STRING_YELLOW_)
    {
        red = 0.5;
        green = 0.5;
    }
    else
    {
        // Blue substitute
        green = 0.5;
	red = 0.1;
    }
}

void Robot::Toggle_Colors(string color1, string color2)
{
    string color = color1;

    if (color_ == color1)
    {
        color = color2;
    }

    Set_Color(color);
}

Robot::~Robot(void) {
}

void Robot::Update_Position(double x, double y, double z, double heading) {
  X = x;
  Y = y;
  Z = z;
  Theta = heading;
}

void Robot::Update_Uncertainty(bool tog) {
  Uncertainty = tog;
}

void Robot::Update_Uncertainty(int x, int y, int dx, int dy) {
  UncertaintyX = x;
  UncertaintyY = y;
  UncertaintyDx = dx;
  UncertaintyDy = dy;
}

void Robot::Set_RobotScale(double scale) {
  Robot_Scale = scale;
}

void Robot::Draw_Robot(float yoffset, Model_3ds *model) {
  
  GLfloat mat_colors[4];


  if (!Uncertainty) {
    mat_colors[0] = red;
    mat_colors[1] = green;
    mat_colors[2] = blue;
    mat_colors[3] = 1.0f;
    
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, mat_colors);
    

    model->Draw_Model(X, Y + yoffset, Z, Theta);
  }
  else {
    mat_colors[0] = 1.0f;
    mat_colors[1] = 1.0f;
    mat_colors[2] = 1.0f;
    mat_colors[3] = 1.0f;
  
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, mat_colors);


    glDisable(GL_LIGHTING);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    glPushMatrix();


    glBegin(GL_LINE_LOOP);

    for (int j = 50; j < 100; j += 5) {
    
    for (int i=0; i < 360; i++)
      {
	float degInRad = i*3.14159/180;
	glVertex3f(cos(degInRad)*UncertaintyDx +  UncertaintyX, j, sin(degInRad)*UncertaintyDx +  UncertaintyY);
      }
    }
   glEnd();
   glPopMatrix();
   glEnable(GL_LIGHTING);

  }
}
/**********************************************************************
 * $Log: 3drobot.cpp,v $
 * Revision 1.1.1.1  2008/07/14 16:44:24  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/20 17:17:52  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:38:01  endo
 * MissionLab 7.0
 *
 * Revision 1.8  2006/06/29 21:49:22  pulam
 * New 3ds model loader, normal calculation and other fixes
 *
 * Revision 1.7  2005/07/30 02:18:07  endo
 * 3D visualization improved.
 *
 * Revision 1.6  2005/07/28 04:55:34  endo
 * 3D visualization improved more.
 *
 * Revision 1.5  2005/07/27 20:39:45  endo
 * 3D visualization improved.
 *
 * Revision 1.4  2005/07/23 06:02:05  endo
 * 3D visualization improved.
 *
 **********************************************************************/
