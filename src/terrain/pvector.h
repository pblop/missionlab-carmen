/**********************************************************************
 **                                                                  **
 **                               pvector.h                          **
 **                                                                  **
 ** Written by: Patrick Daniel Ulam                                  **
 **                                                                  **
 ** Copyright 2006 Georgia Tech Research Corporation                 **
 ** Atlanta, Georgia  30332-0415                                     **
 ** ALL RIGHTS RESERVED, See file COPYRIGHT for details.             **
 **                                                                  **
 **********************************************************************/

/* $Id: pvector.h,v 1.1.1.1 2008/07/14 16:44:24 endo Exp $ */

#ifndef VECTOR3D_H
#define VECTOR3D_H

#include <iostream>
#include <cmath>

struct PPoint {
  float X;
  float Y;
  float Z;
};

class PVector {
 public:
  inline PVector(void) { X = Y = Z = 0; };
  inline PVector(float x, float y, float z)  { X = x; Y = y; Z = z; };
  inline PVector(const PPoint &p1, const PPoint &p2) { X = p2.X - p1.X; Y = p2.Y - p1.Y; Z = p2.Z - p1.Z; };
  inline PVector(float x1, float y1, float z1, float x2, float y2, float z2) { X = x1 - x2; Y = y1 - y2; Z = z1 - z2; };

  inline ~PVector(void) {};
  inline void Print(void) { std::cout << X << "," << Y << "," << Z << '\n'; };


  inline PVector operator+ (const PVector &v) const { return PVector(X + v.X, Y + v.Y, Z + v.Z); };
  inline PVector operator- (const PVector &v) const { return PVector(X - v.X, Y - v.Y, Z - v.Z); };
  inline PVector operator* (const float scale) const { return PVector(X * scale, Y * scale, Z * scale); };
  inline PVector operator/ (const float scale) const { return PVector(X / scale, Y / scale, Z / scale); };

  inline PVector &operator+= (const PVector &v) { X += v.X; Y += v.Y; Z += v.Z; return *this; };
  inline PVector &operator-= (const PVector &v) { X -= v.X; Y -= v.Y; Z -= v.Z; return *this; };
  inline PVector &operator*= (const float scale) { X *= scale; Y *= scale; Z *=scale; return *this; };
  inline PVector &operator/= (const float scale)  { X /= scale; Y /= scale; Z /=scale; return *this; };
  
  inline float Length(const PVector &v) const { return sqrt(X * X + Y * Y + Z * Z); };
  inline void Normalize(void) { float len = Length(*this); X /= len; Y /= len; Z /= len; };
  inline float Dot(const PVector &v) const { return (X * v.X) + (Y * v.Y) + (Z * v.Z); };
  inline PVector Cross(const PVector &v) const {  return PVector(((Y * v.Z) - (Z * v.Y)), ((Z * v.X) - (X * v.Z)), ((X * v.Y) - (Y * v.X))); };

  float X;
  float Y;
  float Z;
};


#endif
/**********************************************************************
 * $Log: pvector.h,v $
 * Revision 1.1.1.1  2008/07/14 16:44:24  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/20 17:17:52  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:38:01  endo
 * MissionLab 7.0
 *
 * Revision 1.3  2006/07/04 16:51:34  endo
 * Model_3ds added.
 *
 **********************************************************************/
