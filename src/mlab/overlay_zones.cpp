#include <string.h>
#include "overlay_zones.h"

using std::vector;

OverlayZone::OverlayZone(char *name, gt_Point_list *points) {

  Points = points;
  Name = strdup(name);
  ZoneType = NO_ZONE;

  Convert_PointsToLineSegs();
}

OverlayZone::~OverlayZone(void) {
  
  vector<lineRec2d_t *>::iterator lrit = LineSegments.begin();
  while(lrit != LineSegments.end()) {
    free(*lrit);
    lrit++;
  }
  
  free(Name);
}

lineRec2d_t *OverlayZone::Get_LineList(void) {
  if (LineSegments.empty()) {
    return NULL;
  }
  return LineSegments[LineSegments.size() - 1];
}

OverlayZone_Type OverlayZone::Get_Type(void) {

  return ZoneType;
}


// convert all points to line segments for easy intersection tests
void OverlayZone::Convert_PointsToLineSegs(void) {
  gt_Point_list *nextlist = Points;
  lineRec2d_t *linerec, *lr2;
  double pts[4];

  lr2 = NULL;

  if (nextlist == NULL) {
    fprintf(stderr, "Error: OverlayZone->Convert_PointsToLineSegs - nextlist is null!\n");
    return;
  }

  while (nextlist->next != NULL) {
    pts[0] = nextlist->x;
    pts[1] = nextlist->y;
    pts[2] = nextlist->next->x;
    pts[3] = nextlist->next->y;
    //printf("points: %f,%f   to    %f %f\n", pts[0], pts[1], pts[2], pts[3]);


    linerec = newLineRec(pts, 1, 0);
    linerec->next = lr2;
    lr2 = linerec;
    LineSegments.push_back(linerec);
    nextlist = nextlist->next;
  }
//  printf("lineseg size: %d\n", LineSegments.size());

  //  vector<lineRec2d_t *>::iterator lrit = LineSegments.begin();
  //while (lrit != LineSegments.end()) {
  lr2 = LineSegments[LineSegments.size() - 1];
  while (lr2 != NULL) {
//    printf("lr points: %f %f    %f %f\n", lr2->ls.endpt[ 0 ].x,  lr2->ls.endpt[ 0 ].y,  lr2->ls.endpt[ 1 ].x,  lr2->ls.endpt[ 1 ].y);
    lr2 = lr2->next;
  }


}
      


// Check by casting a ray from the given point and counting intersections with the boundry
bool OverlayZone::In_Zone(double x, double y, double z) {
  lineRec2d_t linerec;
  int numintersec = 0;
  point2d_t* p;

  setLineRec(x, y, 0.0, 0.0, &linerec);

  vector<lineRec2d_t *>::iterator lrit = LineSegments.begin();
  while(lrit != LineSegments.end()) {
    int suc = intersectTwoLinesegs(&linerec, *lrit, p);
    if (suc) {
      numintersec++;
    }
    lrit++;
  }
  
  if ((numintersec % 2) == 1) {
    return false;
  }

  return true;
}  
  

bool OverlayZone::Intersect_Zone(double x1, double y1, double z1, double x2, double y2, double z2) {
  lineRec2d_t linerec;
  point2d_t* p;
  
  setLineRec(x1, y1, x2, y2, &linerec);

  vector<lineRec2d_t *>::iterator lrit = LineSegments.begin();
  while(lrit != LineSegments.end()) {
    int suc = intersectTwoLinesegs(&linerec, *lrit, p);
    if (suc) {
      return true;
    }
    lrit++;
  }
  
  return false;
}


//----------------------------------------------- NoFlyZone -------------------------------------------------------------------------
NoFlyZone::NoFlyZone(char *name, gt_Point_list *points) : OverlayZone(name, points) {
  ZoneType = NOFLY_ZONE;
}

NoFlyZone::~NoFlyZone(void) {
}


//---------------------------------------------- NoCommZone ------------------------------------------------------------------------
NoCommZone::NoCommZone(char *name, gt_Point_list *points) : OverlayZone(name, points) {
  ZoneType = NOCOMM_ZONE;
}

NoCommZone::~NoCommZone(void) {
}


//---------------------------------------------- WeatherZone ------------------------------------------------------------------------
WeatherZone::WeatherZone(char *name, gt_Point_list *points) : OverlayZone(name, points) {
  ZoneType = NOCOMM_ZONE;
}

WeatherZone::~WeatherZone(void) {
}
