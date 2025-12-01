/**********************************************************************
 **                                                                  **
 **                        terrainmap.cpp                            **
 **                                                                  **
 ** Written by: Patrick Daniel Ulam                                  **
 **                                                                  **
 ** Copyright 2005 - 2006 Georgia Tech Research Corporation          **
 ** Atlanta, Georgia  30332-0415                                     **
 ** ALL RIGHTS RESERVED, See file COPYRIGHT for details.             **
 **                                                                  **
 **********************************************************************/

/* $Id: terrainmap.cpp,v 1.1.1.1 2008/07/14 16:44:24 endo Exp $ */

#include <stdio.h>
#include <stdlib.h>
#include "3d_vis_wid.h"
                                                                                
#include "pvector.h"
#include "terrainmap.h"

const bool DEBUG_TERRAIN = false;
const bool USE_NORMAL = false;

// The maximum mission area that 3D can display the terrain.
const int TerrainMap::MAX_MISSION_AREA = 100000000;

// Creates the teraain map with size x,y, sealevel at sl altitude and scale(not currently used) scl per z unit
TerrainMap::TerrainMap(int x, int y, int sl, int scl)
{
    int cnt, cnt2;

    MapX = x;
    MapY = y;
    SeaLevel = sl;
    Scale = (float)scl;

    if (DEBUG_TERRAIN)
      {
        fprintf(
            stderr,
            "Initializing Map: %d %d %d %f\n",
            MapX,
            MapY,
            SeaLevel,
            Scale);
      }
    
    if (x > 2000) {
      UseGraphics = false;
      return;
    }

    if (x*y > MAX_MISSION_AREA) {
      fprintf(stderr, "Heightmap to large for graphical rendering, disabling 3d view\n");
      UseGraphics = false;
      return;
    }
    else {
      UseGraphics = true;
    }


    HeightMap = new int *[MapX];

    if (!HeightMap) {
      fprintf(stderr, "Out of memory in heightmap creation!\n");
      exit(1);
    }
    
    if (UseGraphics) {
      HM_Normals = new PVector**[MapX];
      if (!HM_Normals) {
	fprintf(stderr, "Out of memory in heightmap creation!\n");
	exit(1);
      }
    }

    for (cnt = 0; cnt < MapX; cnt++) {

      HeightMap[cnt] = new int[MapY];
      if (!HeightMap[cnt]) {
	fprintf(stderr, "Out of memory in heightmap creation!\n");
	exit(1);
      }
      
      if (UseGraphics) {
	HM_Normals[cnt] = new PVector *[MapY];
	if (!HM_Normals[cnt]) {
	  fprintf(stderr, "Out of memory in heightmap creation!\n");
	  exit(1);
	}
      } 
      for (cnt2 = 0; cnt2 < MapY; cnt2++) {
	HeightMap[cnt][cnt2] = SeaLevel + 1;
      }
    }

    if (UseGraphics) {
      for (int x = 0; x < MapX; x++) {
	for (int y = 0; y < MapY; y++) {
	  HM_Normals[x][y] = new PVector();
	}
      }
    }

    HaveHeightMap = false;
}




// free up memory used my heightmap
TerrainMap::~TerrainMap(void) {

  for (int cnt = 0; cnt < MapX; cnt++) {
    delete []HeightMap[cnt];
    if (UseGraphics) {
      delete []HM_Normals[cnt];
    }
  }
  delete []HeightMap;
  if (UseGraphics) {
    delete []HM_Normals;
  }
}


int TerrainMap::Get_XSize(void) {
  return MapX;
}

int TerrainMap::Get_YSize(void) {
  return MapY;
}

float TerrainMap::Get_SeaLevel(void) {
  return SeaLevel;
}


// loads a heightmap from file
bool TerrainMap::Load_HeightMap(char *name) {
    int cnt, cnt2;
    FILE *infile;

    if (!strcmp(name, "blank")) {
      HaveHeightMap = false;
    }

    else {
      HaveHeightMap = true;
      
      infile = fopen(name, "r+b");
      if (!infile) {
	fprintf(stderr, "Could not open heightmap file: %s\n", name);
	return false;
      }

      printf("loading heightmap: %d %d\n", MapX, MapY);
      
      for (cnt = 0; cnt < MapY; cnt++) {
	for (cnt2 = 0; cnt2 < MapX; cnt2++) {
	  fread(&HeightMap[cnt2][cnt], sizeof(char), 1, infile);
	}
      }
      fclose(infile);
    }

    if ((HaveHeightMap) && (UseGraphics)) {
	Generate_Normals();
	Generate_DList();
    }
    
    return true;
}



// generate normal vectors for prettier terrain
void TerrainMap::Generate_Normals(void) {
  int x, y;
  PVector v1, v2, va ,vb, vc, vd, ve, vf, tv;
  
  for (y = 1; y < MapY - 1; y++) {
    for (x = 1; x < MapX - 1; x++) {
      
      
  
      v2 = PVector( x + 1, HeightMap[x + 1][y], y,
		    x, HeightMap[x][y], y);
      v1 = PVector( x, HeightMap[x][y + 1], y + 1,
		    x, HeightMap[x][y], y);
      va = v1.Cross(v2);
      va.Normalize();


      v2 = PVector( x + 1, HeightMap[x + 1][y - 1], y - 1,
		    x, HeightMap[x][y], y);
      v1 = PVector( x + 1, HeightMap[x + 1][y], y,
		    x, HeightMap[x][y], y);
      vb = v1.Cross(v2);
      vb.Normalize();


      v2 = PVector( x, HeightMap[x][y - 1], y - 1,
		    x, HeightMap[x][y], y);
      v1 = PVector( x + 1, HeightMap[x + 1][y - 1], y - 1,
		    x, HeightMap[x][y], y);
      vc = v1.Cross(v2);
      vc.Normalize();
      



      v2 = PVector( x - 1, HeightMap[x - 1][y], y,
		    x, HeightMap[x][y], y);
      v1 = PVector( x, HeightMap[x][y - 1], y - 1,
		    x, HeightMap[x][y], y);
      vd = v1.Cross(v2);
      vd.Normalize();



      v2 = PVector( x - 1, HeightMap[x - 1][y + 1], y + 1,
		    x, HeightMap[x][y], y);
      v1 = PVector( x - 1, HeightMap[x - 1][y], y,
		    x, HeightMap[x][y], y);
      ve = v1.Cross(v2);
      ve.Normalize();
      


      v2 = PVector( x, HeightMap[x][y + 1], y + 1,
		    x, HeightMap[x][y], y);
      v1 = PVector( x - 1, HeightMap[x - 1][y + 1], y + 1,
		    x, HeightMap[x][y], y);
      vf = v1.Cross(v2);
      vf.Normalize();      


      tv = va + vb + vc + vd + ve + vf;
      tv = tv / 6.0f;
      tv.Normalize();

      *HM_Normals[x][y] = tv;
    }
  }
} 

// create a display list to speed up rendering
void TerrainMap::Generate_DList(void) {

  TerrainDList = glGenLists(1);

  GLfloat mat_colors[4];
  /*
  mat_colors[0] = .69;
  mat_colors[1] = .64;
  mat_colors[2] = .41;
  */
  // Based on color khaki4
  mat_colors[0] = .55;
  mat_colors[1] = .53;
  mat_colors[2] = .31;
  mat_colors[3] = 1.0;
	 
  glNewList(TerrainDList, GL_COMPILE);

  glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, mat_colors);


  glPushMatrix();
  for (int cntz = 0; cntz < MapY - 1; cntz += 1) {
    for (int cntx = 0; cntx < MapX; cntx += 1) {
      
      glBegin(GL_TRIANGLE_STRIP);
      glNormal3f(HM_Normals[cntx][cntz]->X, HM_Normals[cntx][cntz]->Y, HM_Normals[cntx][cntz]->Z);
      glVertex3f(cntx, HeightMap[cntx][cntz], cntz);
      glNormal3f(HM_Normals[cntx][cntz + 1]->X, HM_Normals[cntx][cntz + 1]->Y, HM_Normals[cntx][cntz + 1]->Z);
      glVertex3f(cntx, HeightMap[cntx][cntz + 1], cntz + 1); 
    }
    glEnd();
  }
  glPopMatrix();
  
  glEndList();

}

bool TerrainMap::Is_OutsideMap(float x, float y)
{
    int xi, yi;

    xi = (int)x;
    yi = (int)y;

    if ((xi < 0) || (xi >= MapX) || (yi < 0) || (yi >= MapY))
    {
        return true;
    }

    return false;
}

float TerrainMap::Get_Height(float x, float y) {

  if (!UseGraphics) {
    return 0.0;
  }
   
  if (Is_OutsideMap(x, y))
    {
        fprintf(
            stderr,
            "Error: TerrainMap::Get_Height(): Point <%f %f> outside the map [%d x %d]\n",
            x, y, MapX, MapY);
        return 0;
    }

    return HeightMap[(int)x][MapY - (int)y];
}

// This function returns true if the line between the two points crosses land.
bool TerrainMap::Does_CrossLand(float x1, float y1, float x2, float y2)
{
    float x, y, minX, minY, maxX, maxY, diffx, diffy, dx, dy;

    if (x1 < x2)
    {
        minX = x1;
        minY = y1;
        maxX = x2;
        maxY = y2;
    }
    else
    {
        minX = x2;
        minY = y2;
        maxX = x1;
        maxY = y1;
    }

    diffx = maxX - minX;
    diffy = maxY - minY;

    if ((fabs(diffx) < 1.0) && (fabs(diffy) < 1.0))
    {
        // It's a point.
        return (!(Is_Water(minX, minY)));
    }

    if (fabs(diffx) > fabs(diffy))
    {
        dx = 1.0;
        dy = diffy/diffx;
    }
    else
    {
        if (diffy > 0)
        {
            dx = diffx/diffy;
            dy = 1.0;
        }
        else
        {
            dx = -diffx/diffy;
            dy = -1.0;
        }
    }

    x = minX;
    y = minY;

    while (x <= maxX)
    {
        if (!(Is_OutsideMap(x, y)))
        {
            if (!(Is_Water(x, y)))
            {
                return true;
            }
        }

        x += dx;
        y += dy;
    }

    return false;
}

bool TerrainMap::Is_NearLand(float x, float y, float range)
{
    float dist, dx, dy;
    int xi, yi, minX, minY, maxX, maxY;

    minX = (int)(x - range);
    maxX = (int)(x + range);
    minY = (int)(y - range);
    maxY = (int)(y + range);

    for (xi = minX; xi <= maxX; xi++)
    {
        dx = x - (float)xi;

        for (yi = minY; yi <= maxY; yi++)
        {
            dy = y - (float)yi;

            dist = sqrt((dx*dx) + (dy*dy));

            if (dist <= range)
            {
                if (!(Is_OutsideMap((float)xi, (float)yi)))
                {
                    if (!(Is_Water((float)xi, (float)yi)))
                    {
                        return true;
                    }
                }
            }
        }
    }

    return false;
}

// This function returns true if the line between the two points crosses water.
bool TerrainMap::Does_CrossWater(float x1, float y1, float x2, float y2)
{
    float x, y, minX, minY, maxX, maxY, diffx, diffy, dx, dy;

    if (x1 < x2)
    {
        minX = x1;
        minY = y1;
        maxX = x2;
        maxY = y2;
    }
    else
    {
        minX = x2;
        minY = y2;
        maxX = x1;
        maxY = y1;
    }

    diffx = maxX - minX;
    diffy = maxY - minY;

    if ((fabs(diffx) < 1.0) && (fabs(diffy) < 1.0))
    {
        // It's a point.
        return (Is_Water(minX, minY));
    }

    if (fabs(diffx) > fabs(diffy))
    {
        dx = 1.0;
        dy = diffy/diffx;
    }
    else
    {
        if (diffy > 0)
        {
            dx = diffx/diffy;
            dy = 1.0;
        }
        else
        {
            dx = -diffx/diffy;
            dy = -1.0;
        }
    }

    x = minX;
    y = minY;

    while (x <= maxX)
    {
        if (!(Is_OutsideMap(x, y)))
        {
            if (Is_Water(x, y))
            {
                return true;
            }
        }

        x += dx;
        y += dy;
    }

    return false;
}

bool TerrainMap::Is_NearWater(float x, float y, float range)
{
    float dist, dx, dy;
    int xi, yi, minX, minY, maxX, maxY;

    minX = (int)(x - range);
    maxX = (int)(x + range);
    minY = (int)(y - range);
    maxY = (int)(y + range);

    for (xi = minX; xi <= maxX; xi++)
    {
        dx = x - (float)xi;

        for (yi = minY; yi <= maxY; yi++)
        {
            dy = y - (float)yi;

            dist = sqrt((dx*dx) + (dy*dy));

            fprintf(stderr, "dist %f range %f\n", dist, range);
                    
            if (dist <= range)
            {
                if (!(Is_OutsideMap((float)xi, float(yi))))
                {
                    if (Is_Water((float)xi, (float)yi))
                    {
                        return true;
                    }
                }
            }
        }
    }

    return false;
}

bool TerrainMap::Is_Water(float x, float y) {
  if (!UseGraphics) {
    return false;
  }

    if (Is_OutsideMap(x, y))
    {
        fprintf(
            stderr,
            "Warning: TerrainMap::Is_Water(): Point <%f %f> outside the map [%d x %d]\n",
            x, y, MapX, MapY);
        return false;
    }

    return (HeightMap[(int)x][MapY - (int)y] <= (SeaLevel - 1));
}

// PU ??? Why (HeightMap[(int)x][MapY - (int)y] <= z)?
bool TerrainMap::Is_Water(float x, float y, float z) {

    if (Is_OutsideMap(x, y))
    {
        fprintf(
            stderr,
            "Warning: TerrainMap::Is_Water(): Point <%f %f> outside the map [%d x %d]\n",
            x, y, MapX, MapY);
        return false;
    }

    return (HeightMap[(int)x][MapY - (int)y] <= z);
}


void TerrainMap::Draw(void) {
    GLfloat mat_colors[4];

    if (UseGraphics) {

      mat_colors[0] = .69;
      mat_colors[1] = .64;
      mat_colors[2] = .41;
      mat_colors[3] = 1.0;
      
      if (!glIsList(TerrainDList))
	{
	  //	  Draw_Water();
	}
   
      glCallList(TerrainDList);
    }
    else {
      fprintf(stderr, "Can not render terrain heightmap too large to fit in memory\n");
    }
}
void TerrainMap::Draw_Water(void) {
    GLfloat mat_colors[4];
    float maxX, maxY, minX, minY;

    mat_colors[0] = 0.0;
    mat_colors[1] = 0;
    mat_colors[2] = 1.0;
    mat_colors[3] = 0.4;

    // Let us make the water 9 times as big as the regular terrain.
    maxX = MapX * 2.0;
    maxY = MapY * 2.0;
    minX = -MapX;
    minY = -MapY;

    if (SeaLevel != 0) {	
      glBegin(GL_QUADS);
      glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, mat_colors);
      
      glVertex3f(minX,0.0f, minY);					// Top Right Of The Quad (Bottom)
      glVertex3f(maxX, 0.0f, minY);					// Top Left Of The Quad (Bottom)
      glVertex3f(maxX, SeaLevel,minY);					// Bottom Left Of The Quad (Bottom)
      glVertex3f(minY,SeaLevel,minY);					// Bottom Right Of The Quad (Bottom)
      
      glVertex3f(minY, 0.0f, minY);					// Top Right Of The Quad (Front)
      glVertex3f(minY, 0.0f, maxY);					// Top Left Of The Quad (Front)
      glVertex3f(minY,SeaLevel, maxY);					// Bottom Left Of The Quad (Front)
      glVertex3f(minY,SeaLevel, minY);					// Bottom Right Of The Quad (Front)
      
      glVertex3f(minY, SeaLevel, minY);					// Top Right Of The Quad (Back)
      glVertex3f(maxX, SeaLevel, minY);					// Top Left Of The Quad (Back)
      glVertex3f(maxX, SeaLevel, maxY);					// Bottom Left Of The Quad (Back)
      glVertex3f(minY, SeaLevel,maxY);					// Bottom Right Of The Quad (Back)
      
      glVertex3f(maxX, 0.0f, minY);					// Top Right Of The Quad (Left)
      glVertex3f(maxX, SeaLevel,minY);					// Top Left Of The Quad (Left)
      glVertex3f(maxX,SeaLevel,maxY);					// Bottom Left Of The Quad (Left)
      glVertex3f(maxX,0.0f, maxY);					// Bottom Right Of The Quad (Left)
      
      glVertex3f(minY, 0.0f,maxY);					// Top Right Of The Quad (Right)
      glVertex3f(minY, SeaLevel,maxY);					// Top Left Of The Quad (Right)
      glVertex3f(maxX,SeaLevel, maxY);					// Bottom Left Of The Quad (Right)
      glVertex3f(maxX,0.0f,maxY);					// Bottom Right Of The Quad (Right)
      glEnd();
    }
}


int TerrainMap::Get_HighestPoint(int x1, int y1, int x2, int y2,
				int& x_max, int& y_max)
{
  int max_elevation = 0;
  int i;
  int ystep, xstep;
  int error;
  int errorprev;
  int y = y1, x = x1;
  int ddy, ddx;
  int dx = x2 - x1;
  int dy = y2 - y1;
  
  if (dx < 0)  {
      xstep = -1;
      dx = -dx;
  }
  else {
      xstep = 1;
  }
  
  if (dy < 0) {
      ystep = -1;
      dy = -dy;
  }
  else {
      ystep = 1;
  }

  ddx = 2 * dx;
  ddy = 2 * dy;

  if (ddy >= ddx) {
    errorprev = error = dy;
    for (i = 0; i < dy; i++) {
      y+= ystep;
      error +=ddx;
      if (error > ddy) {
	x += xstep;
	error -= ddy;
	if (error + errorprev < ddy) {
	  if (HeightMap[x- xstep][MapY - y] > max_elevation) {
	    max_elevation = HeightMap[x - xstep][MapY - y];
	    x_max = x - xstep;
	    y_max = y;
	  }
	}
	
	else if (error + errorprev > ddy) {
      if (HeightMap[x][MapY - y - ystep] > max_elevation) { // PU ??? [MapY - y + ystep] instead?
	    max_elevation = HeightMap[x][MapY - y - ystep];
	    x_max = x;
	    y_max = y - ystep;
	  }
	}
	else {
	  if (HeightMap[x - xstep][MapY - y] > max_elevation) {
	    max_elevation = HeightMap[x - xstep][MapY - y];
	    x_max = x - xstep;
	    y_max = y;
	  }

	  if (HeightMap[x][MapY - y - ystep] > max_elevation) { // ENDO [MapY - y + ystep] instead???
	    max_elevation = HeightMap[x][MapY - y - ystep];
	    x_max = x;
	    y_max = y - ystep;
	  }
	}
      }
      if (HeightMap[x][y] > max_elevation) {
	max_elevation = HeightMap[x][MapY - y];
	x_max = x;
	y_max = y;
      }
      errorprev = error;
    }
  }
  else {
    errorprev = error = dx;
    for (i = 0; i < dx; i++) {
      x += xstep;
      error +=ddy;
      if (error > ddx) {
	y += ystep;
	error -= ddx;
	if (error + errorprev < ddx) { 
	  if (HeightMap[x][MapY - y - ystep] > max_elevation) {
	    max_elevation = HeightMap[x][MapY - y - ystep];
	    x_max = x;
	    y_max = y - ystep;
	  }
	}
	else if (error + errorprev > ddx) {
	  if (HeightMap[x - xstep][MapY - y] > max_elevation) {
	    max_elevation = HeightMap[x - xstep][MapY - y];
	    x_max = x - xstep;
	    y_max = y;
	  }
	}
	else {
	  if (HeightMap[x][MapY - y - ystep] > max_elevation) {
	    max_elevation = HeightMap[x][MapY - y - ystep];
	    x_max = x;
	    y_max = y - ystep;
	  }
	  if (HeightMap[x - xstep][MapY - y] > max_elevation)  {
	    max_elevation = HeightMap[x - xstep][MapY - y];
	    x_max = x - xstep;
	    y_max = y;
	  }
	}
      }
      
      if (HeightMap[x][MapY - y] > max_elevation) {
	max_elevation = HeightMap[x][MapY - y];
	x_max = x;
	y_max = y;
      }
      errorprev = error;
    }
  }

  if (DEBUG_TERRAIN)  {
    fprintf(stderr, "max: %d\n", max_elevation);
  }
  
  return max_elevation;
}

/**********************************************************************
 * $Log: terrainmap.cpp,v $
 * Revision 1.1.1.1  2008/07/14 16:44:24  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.2  2006/10/20 14:30:06  nadeem
 * Added Additional checks to ensure that 3D is turned off for overlays > 2000 x 2000:
 *
 * Revision 1.1.1.1  2006/07/20 17:17:52  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:38:01  endo
 * MissionLab 7.0
 *
 * Revision 1.15  2006/07/10 17:47:13  endo
 * Clean-up for MissionLab 7.0 release.
 *
 * Revision 1.14  2006/06/29 21:49:22  pulam
 * New 3ds model loader, normal calculation and other fixes
 *
 * Revision 1.13  2006/05/15 01:04:51  endo
 * gcc-3.4 upgrade
 *
 * Revision 1.12  2006/05/06 18:20:11  endo
 * Bugs for TrackTask fixed.
 *
 * Revision 1.11  2006/05/02 04:12:36  endo
 * TrackTask improved for the experiment.
 *
 * Revision 1.10  2006/04/28 22:50:42  pulam
 * Constraint checking update, Terrainmap disable for large maps, renegging overhaul
 *
 * Revision 1.9  2006/03/15 01:09:10  endo
 * Normal disabled.
 *
 * Revision 1.8  2006/03/05 23:27:20  pulam
 * Addition of SL-UAV code and CNP code for calculating bids for intercept/inspect tasks
 *
 * Revision 1.7  2005/07/28 04:55:35  endo
 * 3D visualization improved more.
 *
 * Revision 1.6  2005/07/27 20:39:45  endo
 * 3D visualization improved.
 *
 * Revision 1.5  2005/07/23 06:02:06  endo
 * 3D visualization improved.
 *
 **********************************************************************/
