#ifndef OVERLAY_ZONES_H
#define OVERLAY_ZONES_H

#include <vector>
#include <stdio.h>	//Old compilation error: stderr and fprintf not defined
#include "gt_std.h"
#include "lineutils.h"

enum OverlayZone_Type {
  NO_ZONE,
  NOFLY_ZONE,
  WEATHER_ZONE,
  NOCOMM_ZONE
};
  


class OverlayZone {
 public:
  OverlayZone(char *name, gt_Point_list *points);
  ~OverlayZone(void);
  
  OverlayZone_Type Get_Type(void);
  lineRec2d_t *Get_LineList(void);
  bool In_Zone(double x, double y, double z);
  bool Intersect_Zone(double x1, double y1, double z1, double x2, double y2, double z2);

 protected:
  void Convert_PointsToLineSegs(void);

  char *Name;
  OverlayZone_Type ZoneType;
  gt_Point_list *Points;
  std::vector<lineRec2d_t *>LineSegments;
};


class NoFlyZone : public OverlayZone {

 public:
  NoFlyZone(char *name, gt_Point_list *points);
  ~NoFlyZone(void);

 protected:
};

class NoCommZone : public OverlayZone {

 public:
  NoCommZone(char *name, gt_Point_list *points);
  ~NoCommZone(void);

 protected:
};

class WeatherZone : public OverlayZone {

 public:
  WeatherZone(char *name, gt_Point_list *points);
  ~WeatherZone(void);

 protected:
};


#endif
