/**********************************************************************
 **                                                                  **
 **                           model_3ds.cpp                          **
 **                                                                  **
 ** Written by: Patrick Daniel Ulam                                  **
 **                                                                  **
 ** Copyright 2006 Georgia Tech Research Corporation                 **
 ** Atlanta, Georgia  30332-0415                                     **
 ** ALL RIGHTS RESERVED, See file COPYRIGHT for details.             **
 **                                                                  **
 **********************************************************************/

/* $Id: model_3ds.cpp,v 1.1.1.1 2008/07/14 16:44:24 endo Exp $ */

#include <stdio.h>
#include <sys/stat.h>
#include <GL/gl.h>
#include <GL/glu.h>

#include "model_3ds.h"



Model_3ds::Model_3ds(char *filename, float scalex, float scaley, float scalez,
		     float rx, float ry, float rz, bool norminv, Rot_Axis ra) {

  Name = strdup(filename);

  ScaleX = scalex;
  ScaleY = scaley;
  ScaleZ = scalez;
  
  RotX = rx;
  RotY = ry;
  RotZ = rz;

  RotAx = ra;
  
  NumVerts = 0;
  NumPoly = 0;
  
  Load_Model();
  Calculate_Normals(norminv);
}


Model_3ds::~Model_3ds(void) {

  free(Name);
  if (NumVerts != 0) {
    delete []Vertex;
    for (int i = 0; i < NumVerts; i++) {
      delete Normal[i];
    }
    delete []Normal;
  }
  if (NumPoly != 0) {
    delete []Polygon;
  }
 
}


long Find_FileLength(int fileid) {
    struct stat filebuf; 
    fstat(fileid, &filebuf); 
    return(filebuf.st_size);
}

void Model_3ds::Load_Model(void) {
  int i;
  
  unsigned short chunkid;
  unsigned int chunklength;
  char tc;
  unsigned short faceflags;
  unsigned short a;  

  FILE *infile;
  
  infile= fopen(Name, "rb");
  if (!infile) {
    fprintf(stderr, "Could not open model: %s\n", Name);
    exit(1);
  }
  printf("Loading model: %s\n", Name);

    while (ftell(infile) < Find_FileLength(fileno(infile))) {

    fread (&chunkid, 2, 1, infile);
    fread (&chunklength, 4, 1, infile);

    switch (chunkid) {
			
    case 0x4d4d: 
    case 0x3d3d:
    case 0x4100:
      break;    

      // throw away the name block
    case 0x4000: 
      i=0;
      do {
	  fread (&tc, 1, 1, infile);
	  i++;
	} while((tc != '\0') && (i < 20));
      break;

    case 0x4110: 
      fread (&a, sizeof(unsigned short), 1, infile);
      NumVerts = (int)a;
      printf("Number of vertices: %d\n",NumVerts);
 
      Vertex = new PPoint[NumVerts];
      Normal = new PVector *[NumVerts];
      
      for (i = 0; i < NumVerts; i++) {
	Normal[i] = new PVector();

	fread(&Vertex[i].X, sizeof(float), 1, infile);
	fread(&Vertex[i].Y, sizeof(float), 1, infile);
	fread(&Vertex[i].Z, sizeof(float), 1, infile);
	//	printf("Vert[%d]: %f %f %f\n", i, Vertex[i].X, Vertex[i].Y, Vertex[i].Z);
      }
      break;

    case 0x4120:
      fread (&a, sizeof(unsigned short), 1, infile);
      NumPoly = (int)a;
      printf("Number of polygons: %d\n",NumPoly); 
      Polygon = new PolygonInd[NumPoly];
      				
      for (i = 0; i < NumPoly; i++) {
	fread(&a, sizeof(unsigned short), 1, infile);
	Polygon[i].A = (int)a;
	fread(&a, sizeof(unsigned short), 1, infile);
	Polygon[i].B = (int)a;
	fread(&a, sizeof(unsigned short), 1, infile);
	Polygon[i].C = (int)a;
	fread (&faceflags, sizeof (unsigned short), 1, infile);
	//	printf("Poly[%d]: %d %d %d %d\n", i, Polygon[i].A,Polygon[i].B, Polygon[i].C,faceflags);
      }
      break;

    default:
      fseek(infile, chunklength-6, SEEK_CUR);
    } 
  }
  
  fclose (infile);

}

void Model_3ds::Draw_Model(float x, float y, float z, float theta, float scx, float scy, float scz) {

  glPushMatrix();

  glTranslatef(x, y, z);
  glRotatef(RotX, 1.0f, 0.0f, 0.0f);
  glRotatef(RotY, 0.0f, 1.0f, 0.0f);
  glRotatef(RotZ, 0.0f, 0.0f, 1.0f);

  if (RotAx == X_AXIS) {
    glRotatef(theta, 1.0f, 0.0f, 0.0f);
  }
  else if (RotAx == Y_AXIS) {
    glRotatef(theta, 0.0f, 1.0f, 0.0f);
  }
  else {
    glRotatef(theta, 0.0f, 0.0f, 1.0f);
  }

  glScalef(scx, scy, scz);


  glBegin(GL_TRIANGLES);
  for (int j=0; j < NumPoly; j++) {
    //----------------- FIRST VERTEX -----------------
    glNormal3f(Normal[Polygon[j].A]->X,
	       Normal[Polygon[j].A]->Y,
    	       Normal[Polygon[j].A]->Z);
    
    glVertex3f(Vertex[Polygon[j].A].X,
	       Vertex[Polygon[j].A].Y,
	       Vertex[Polygon[j].A].Z);
    
    //----------------- SECOND VERTEX -----------------
    glNormal3f(Normal[Polygon[j].B]->X,
	       Normal[Polygon[j].B]->Y,
    	       Normal[Polygon[j].B]->Z);
  
    glVertex3f(Vertex[Polygon[j].B].X,
	       Vertex[Polygon[j].B].Y,
	       Vertex[Polygon[j].B].Z);
        
    //----------------- THIRD VERTEX -----------------
    glNormal3f(Normal[Polygon[j].C]->X,
    	       Normal[Polygon[j].C]->Y,
    	       Normal[Polygon[j].C]->Z);

    glVertex3f(Vertex[Polygon[j].C].X,
	       Vertex[Polygon[j].C].Y,
	       Vertex[Polygon[j].C].Z);
  }
  glEnd();
    
  glPopMatrix();
}



void Model_3ds::Draw_Model(float x, float y, float z, float theta) {

  glPushMatrix();

  glTranslatef(x, y, z);
  glRotatef(RotX, 1.0f, 0.0f, 0.0f);
  glRotatef(RotY, 0.0f, 1.0f, 0.0f);
  glRotatef(RotZ, 0.0f, 0.0f, 1.0f);

  if (RotAx == X_AXIS) {
    glRotatef(theta, 1.0f, 0.0f, 0.0f);
  }
  else if (RotAx == Y_AXIS) {
    glRotatef(theta, 0.0f, 1.0f, 0.0f);
  }
  else {
    glRotatef(theta, 0.0f, 0.0f, 1.0f);
  }


  //  RotX += 5.0;
  //  glRotatef((RotZ - 90.0f) + theta,0.0f,0.0f,1.0f);

  glScalef(ScaleX, ScaleY, ScaleZ);


  
  // GLfloat matcolors[4] = {0.10f, 0.50f, 0.0f, 1.0f};
  //  glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, matcolors);

  glBegin(GL_TRIANGLES);
  for (int j=0; j < NumPoly; j++) {
    //----------------- FIRST VERTEX -----------------
    glNormal3f(Normal[Polygon[j].A]->X,
	       Normal[Polygon[j].A]->Y,
    	       Normal[Polygon[j].A]->Z);
    
    glVertex3f(Vertex[Polygon[j].A].X,
	       Vertex[Polygon[j].A].Y,
	       Vertex[Polygon[j].A].Z);
    
    //----------------- SECOND VERTEX -----------------
    glNormal3f(Normal[Polygon[j].B]->X,
	       Normal[Polygon[j].B]->Y,
    	       Normal[Polygon[j].B]->Z);
  
    glVertex3f(Vertex[Polygon[j].B].X,
	       Vertex[Polygon[j].B].Y,
	       Vertex[Polygon[j].B].Z);
        
    //----------------- THIRD VERTEX -----------------
    glNormal3f(Normal[Polygon[j].C]->X,
    	       Normal[Polygon[j].C]->Y,
    	       Normal[Polygon[j].C]->Z);

    glVertex3f(Vertex[Polygon[j].C].X,
	       Vertex[Polygon[j].C].Y,
	       Vertex[Polygon[j].C].Z);
  }
  glEnd();
    
  glPopMatrix();
}

void Model_3ds::Calculate_Normals(bool invert) {
  int *conncnt;

  conncnt = new int[NumVerts];
  for (int i = 0; i < NumVerts; i++) {
    conncnt[i] = 0;
  }

  for (int i = 0; i < NumPoly; i++) {
    PVector v1 = PVector(Vertex[Polygon[i].A], Vertex[Polygon[i].B]);
    PVector v2 = PVector(Vertex[Polygon[i].A], Vertex[Polygon[i].C]);

    PVector nv = v1.Cross(v2);
    nv.Normalize();

    conncnt[Polygon[i].A] += 1;
    conncnt[Polygon[i].B] += 1;
    conncnt[Polygon[i].C] += 1;

    *Normal[Polygon[i].A] += nv;
    *Normal[Polygon[i].B] += nv;
    *Normal[Polygon[i].C] += nv;
  
  }	
	
  for (int i = 0; i < NumVerts; i++) {
    if (conncnt[i] > 0) {
      *Normal[i] /= conncnt[i];
      Normal[i]->Normalize();
      if (invert) {
	*Normal[i] *= -1;
      }
    }
  }

  delete []conncnt;
}


/**********************************************************************
 * $Log: model_3ds.cpp,v $
 * Revision 1.1.1.1  2008/07/14 16:44:24  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/20 17:17:52  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:38:01  endo
 * MissionLab 7.0
 *
 * Revision 1.1  2006/07/04 16:51:34  endo
 * Model_3ds added.
 *
 **********************************************************************/
