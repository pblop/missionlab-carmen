#ifndef VECTOR_H_
#define VECTOR_H_
/**********************************************************************
 **                                                                  **
 **  Vector.h                                                        **
 **                                                                  **
 **  3D vector class                                                 **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-1998.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Alto, Michigan, USA                                             **
 **                                                                  **
 **********************************************************************/

/* $Id: Vector.h,v 1.1.1.1 2008/07/14 16:44:16 endo Exp $ */

/**********************************************************************
* $Log: Vector.h,v $
* Revision 1.1.1.1  2008/07/14 16:44:16  endo
* MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
*
* Revision 1.1.1.1  2006/07/20 17:17:46  endo
* MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
*
* Revision 1.1.1.1  2006/07/12 13:37:55  endo
* MissionLab 7.0
*
* Revision 1.1  2006/07/01 00:30:29  endo
* CMDLi from MARS 2020 migrated into AO-FNC repository.
*
* Revision 1.1.1.1  2006/06/29 20:42:07  endo
* cmdli local repository.
*
* Revision 1.22  2004/05/11 19:34:29  doug
* massive changes to integrate with USC and GaTech
*
* Revision 1.21  2004/02/20 14:39:30  doug
* cmdl interpreter runs a bit
*
* Revision 1.20  2003/03/10 16:31:34  doug
* FittedCurve is closer
*
* Revision 1.19  2003/03/03 13:44:23  doug
* seems to work
*
* Revision 1.18  2002/12/23 22:17:04  doug
* added Vectorq as a type
*
* Revision 1.17  2002/09/20 20:22:53  doug
* added vectors
*
* Revision 1.16  2002/06/25 21:32:42  doug
* added output stream capabilities
*
* Revision 1.15  2002/06/25 21:13:33  doug
* added dot product and cross product (3d)
*
* Revision 1.14  2002/06/25 20:48:04  doug
* added make_unit
*
* Revision 1.13  2002/06/20 20:55:02  doug
* Added more match
*
* Revision 1.12  2002/06/20 18:34:31  doug
* add subtraction
*
* Revision 1.11  2002/06/20 17:33:34  doug
* added subtraction -= operator
*
* Revision 1.10  1999/03/11 18:43:58  doug
* Added set_length_2d
*
* Revision 1.9  1998/03/19 15:11:17  doug
* added multiply and add
*
* Revision 1.8  1997/12/30 17:30:02  doug
* docified
*
* Revision 1.7  1997/12/30 17:19:35  doug
* setup for doc++
*
* Revision 1.6  1997/12/09 15:59:33  doug
* *** empty log message ***
*
* Revision 1.5  1997/11/25 11:25:09  doug
* Fixed typo
*
* Revision 1.4  1997/11/25 11:23:55  doug
* Changed rotate functions to be in radians and added
* rotateA_degrees variants
*
* Revision 1.3  1997/03/26 09:20:01  doug
* *** empty log message ***
*
* Revision 1.2  1997/03/19 04:13:37  doug
* Added clear
*
* Revision 1.1  1997/02/28 09:29:12  doug
* Initial revision
*
**********************************************************************/

#include <deque>

namespace sara
{

class Vector
{
public:
   /**@name Public Data */
   //@{
   /// The internal representation is 3 doubles.
   double x,y,z;
   //@}

   /**@name Public Member Functions */
   //@{
   /// Create a <0,0,0> vector
   Vector() :
      x(0),
      y(0),
      z(0)
   {};

   /// Create a <X,Y,0> vector
   Vector(double X, double Y) :
      x(X),
      y(Y),
      z(0)
   {};

   /// Create a <X,Y,Z> vector
   Vector(double X, double Y, double Z) :
      x(X),
      y(Y),
      z(Z)
   {};

   /// Create from another vector
   Vector(const Vector &v);

   /// Destroy the vector
   ~Vector()
   {
   }

   /// Rotate the vector R radians about the X axis
   void rotateX(double R);

   /// Rotate the vector D degrees about the X axis
   void rotateX_degrees(double D);

   /// Rotate the vector R radians about the Y axis
   void rotateY(double R);

   /// Rotate the vector D degrees about the Y axis
   void rotateY_degrees(double D);

   /// Rotate the vector R radians about the Z axis
   void rotateZ(double R);

   /// Rotate the vector D degrees about the Z axis
   void rotateZ_degrees(double D);

   /// Compute the 2d angle (in radians) about Z with +X being 0 and +Y being Pi/2 as expected.
   double thetaZ();

   /// Compute the 2d angle (in degrees) about Z with +X being 0 and +Y being Pi/2 as expected.
   double thetaZ_degrees();

   /// Compute the 3d length of the vector
   double length();

   /// Compute the 2d length of the X,Y components of the vector
   double length_2d();

   /// Set the length of the vector to desired_length
   void set_length(const double desired_length);

   /// Set the 2d length of the X,Y components of the vector to LENGTH
   void set_length_2d(const double length);

   /// Set the vector to <0,0,0>
   void clear() {x = 0; y = 0; z = 0;}

   /// Turn the vector into a 2d unit vector
   void make_unit_2d();

   /// Turn the vector into a unit vector
   void make_unit();

   /// Returns a unit vector along this vector
   Vector unit() const;

   /// Make a 2d unit vector in the R radian direction
   void make_unit_along_dir(double R);

   /// Make a 2d unit vector in the D degrees direction
   void make_unit_along_dir_degrees(double D);

   /// Compute the dot product of THIS . V
   /// If the vectors are unit vectors, the dot product
   /// is the cos of the angle between the vectors
   double dotProduct(const Vector &v) const;

   /// Compute the cross product of THIS x V
   /// If the vectors are unit vectors, the length of the
   /// cross product vector is the sin of the angle between
   /// the original vectors.
   Vector crossProduct(const Vector &v) const;

   /// Compute the angle swept out when the specified vector is rotated 
   /// around the Z axis and onto this vector.  
   /// Note this vector and v must both lie in the XY plane!
   /// Returns theta in radians
   double dThetaZ(const Vector &v) const;

   /// Rotate this point around the specified Axis by the angle theta (radians)
   /// Positive angles are anticlockwise looking down the axis towards the origin.
   /// Assume right hand coordinate system.
   void rotateAroundAxis(const Vector &Axis, const double angle);

   /// Rotate this point around the specified Axis by the sin and cos of theta
   /// Positive angles are anticlockwise looking down the axis towards the origin.
   /// Assume right hand coordinate system.
   void rotateAroundAxis(const Vector &Axis, const double sintheta, const double costheta);

   /// Support multiplication by a scalar (form: V * D)
   friend Vector operator*(const Vector &v, const double d);

   /// Support multiplication by a scalar (form: V *= D)
   Vector &operator*=(const double d);

   /// Support division by a scalar (form: V /= D)
   Vector &operator/=(const double d);

   /// Support multiplication by a Vector (form: V1 * V2)
   friend Vector operator*(const Vector &v1, const Vector &v2);

   /// Support subtraction by a Vector (form: V1 - V2)
   friend Vector operator-(const Vector &v1, const Vector &v2);

   /// Support subtraction by a scalar (form: V - D)
   friend Vector operator-(const Vector &v, const double d);

   /// Support division by a scalar (form: V / D)
   friend Vector operator/(const Vector &v, const double d);

   /// Support multiplication by a Vector (form: V1 *= V2)
   Vector &operator*=(const Vector &v);

   /// Support addition by a Vector (form: V1 * V2)
   friend Vector operator+(const Vector &v1, const Vector &v2);

   /// Support addition by a Vector (form: V1 *= V2)
   Vector &operator+=(const Vector &v);

   /// Support subtraction by a Vector (form: V1 -= V2)
   Vector &operator-=(const Vector &v);

   /// Stream out the value
   friend std::ostream & operator << (std::ostream & out, const Vector &v);

   /// Create a vector from a stream
   static Vector *load(const std::string &in);
   //@}

private:

};

/// Convience type
typedef std::vector<Vector> Vectors;
typedef std::deque<Vector> Vectorq;
typedef std::vector<Vectors> Vectorss;

/// Compute the center of the polygon
Vector PolygonCenter(const Vectors &v);

}

#endif
