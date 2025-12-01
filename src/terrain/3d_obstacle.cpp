#include <stdio.h>
#include "3d_vis_wid.h"

Obstacle_Cyl::Obstacle_Cyl(float x, float y, float z, float height, float radius) {
  X = x;
  Y = y;
  Z = z;
  Height = height;
  Radius = radius;

  Obs_Quad = gluNewQuadric();
  Obs_Top = gluNewQuadric();
}

Obstacle_Cyl::~Obstacle_Cyl(void) {
 
}

int Obstacle_Cyl::Get_Y(void) {
	return (int)Y;
}



// draw the obstacles as a cylinder with a disk as the top
void Obstacle_Cyl::Draw_Obstacle(float yoffset) {

  glPushMatrix();
  glTranslatef(X,yoffset,Z);
  glColor4f(1.0f, 1.0f, 1.0f, 0.5f);
  glRotatef(90.0f, -1.0f, 0.0f, 0.0f);
  gluCylinder(Obs_Quad, Radius, Radius, Height, 10,1);
  glPopMatrix();

  glColor4f(1.0f, 1.0f, 1.0f, 0.25f);
  glPushMatrix();
  glTranslatef(X, yoffset + Height, Z);
  glRotatef(90.0f, -1.0f, 0.0f, 0.0f);
  gluDisk(Obs_Top, 0, Radius, 10, 1);
  glPopMatrix();

}
