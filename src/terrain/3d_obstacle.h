
class Obstacle_Cyl {
public:
  Obstacle_Cyl(float x, float y, float z, float height, float radius);
  ~Obstacle_Cyl(void);
  void Draw_Obstacle(float yoffset);
  int Get_Y(void);

private:
  GLUquadric *Obs_Quad;
  GLUquadric *Obs_Top;

  float X;
  float Y;
  float Z;
  float Height;
  float Radius;
};
