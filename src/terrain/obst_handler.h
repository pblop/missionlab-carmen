#ifndef OBS_HANDLER_H
#define OBS_HANDLER_H

#define    CYLINDER_VIS     1
#define    WALL_VIS         2

typedef struct {
	int Obstacle_Type;
	void *Obs_Rec;
} Obstacle_Record;


class ObstacleHandler {
public:
  ObstacleHandler(TerrainMap *tm);
  ~ObstacleHandler(void);
  float Place_Obstacle(double x, double y, double height, double radius);
  float Place_Obstacle(gt_Measure *walldata, float height, double highpt);
  void Draw(void);
private:
  SLList *ObstacleList;
  TerrainMap *OH_HeightMap;
};

#endif

