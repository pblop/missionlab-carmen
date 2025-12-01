/**********************************************************************
 **                                                                  **
 **                          terrainmap.h                            **
 **                                                                  **
 ** Written by: Patrick Daniel Ulam                                  **
 **                                                                  **
 ** Copyright 2005 - 2006 Georgia Tech Research Corporation          **
 ** Atlanta, Georgia  30332-0415                                     **
 ** ALL RIGHTS RESERVED, See file COPYRIGHT for details.             **
 **                                                                  **
 **********************************************************************/

/* $Id: terrainmap.h,v 1.1.1.1 2008/07/14 16:44:24 endo Exp $ */

#ifndef TERRAIN_H
#define TERRAIN_H

#include "pvector.h"

class TerrainMap {
public:
    TerrainMap(int x, int y, int sl, int scale);
    ~TerrainMap(void);
    bool Load_HeightMap(char *name);
    float Get_Height(float x, float y);
    bool Does_CrossLand(float x1, float y1, float x2, float y2);
    bool Does_CrossWater(float x1, float y1, float x2, float y2);
    bool Is_NearLand(float x, float y, float range);
    bool Is_NearWater(float x, float y, float range);
    bool Is_Water(float x, float y);  
    bool Is_Water(float x, float y, float z);  
    bool Is_OutsideMap(float x, float y);
    float Get_SeaLevel(void);
    int Get_XSize(void);
    int Get_YSize(void);
    int Get_HighestPoint(int x1, int y1, int x2, int y2, int& x_max, int& y_max);

    void Draw(void);
    void Draw_Water(void);

private:
    void Generate_Normals(void);
    void Generate_DList(void);

    int MapX;
    int MapY;
    int **HeightMap;
    int SeaLevel;
    float Scale;


    bool UseGraphics;
    PVector ***HM_Normals;

    int TerrainDList;
    bool HaveHeightMap;

    static const int MAX_MISSION_AREA;
};

#endif

/**********************************************************************
 * $Log: terrainmap.h,v $
 * Revision 1.1.1.1  2008/07/14 16:44:24  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/20 17:17:52  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:38:01  endo
 * MissionLab 7.0
 *
 * Revision 1.6  2006/05/06 18:20:11  endo
 * Bugs for TrackTask fixed.
 *
 * Revision 1.5  2006/05/02 04:12:36  endo
 * TrackTask improved for the experiment.
 *
 **********************************************************************/
