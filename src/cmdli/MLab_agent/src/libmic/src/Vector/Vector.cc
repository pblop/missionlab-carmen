/**********************************************************************
 **                                                                  **
 **  Vector.cc                                                       **
 **                                                                  **
 **  3D Vector class                                                 **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-1998.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Alto, Michigan, USA                                             **
 **                                                                  **
 **********************************************************************/

/* $Id: Vector.cc,v 1.1.1.1 2008/07/14 16:44:16 endo Exp $ */

/**********************************************************************
* $Log: Vector.cc,v $
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
* Revision 1.22  2004/10/25 22:58:11  doug
* working on mars2020 integrated demo
*
* Revision 1.21  2004/05/11 19:34:29  doug
* massive changes to integrate with USC and GaTech
*
* Revision 1.20  2004/03/24 15:11:52  doug
* builds under win32
*
* Revision 1.19  2004/02/20 14:39:30  doug
* cmdl interpreter runs a bit
*
* Revision 1.18  2003/03/10 16:31:34  doug
* FittedCurve is closer
*
* Revision 1.17  2003/03/03 13:44:23  doug
* seems to work
*
* Revision 1.16  2002/09/20 20:22:53  doug
* added vectors
*
* Revision 1.15  2002/06/25 21:32:42  doug
* added output stream capabilities
*
* Revision 1.14  2002/06/25 21:13:33  doug
* added dot product and cross product (3d)
*
* Revision 1.13  2002/06/25 20:48:04  doug
* added make_unit
*
* Revision 1.12  2002/06/20 20:55:02  doug
* Added more match
*
* Revision 1.11  2002/06/20 18:34:31  doug
* add subtraction
*
* Revision 1.10  2002/06/20 17:33:34  doug
* added subtraction -= operator
*
* Revision 1.9  1999/03/11 18:43:58  doug
* Added set_length_2d
*
* Revision 1.8  1998/03/19 15:11:17  doug
* added multiply and add
*
* Revision 1.7  1997/12/30 17:30:02  doug
* docified
*
* Revision 1.6  1997/12/10 11:59:07  doug
* *** empty log message ***
*
* Revision 1.5  1997/12/09 15:59:33  doug
* *** empty log message ***
*
* Revision 1.4  1997/11/25 11:23:55  doug
* Changed rotate functions to be in radians and added
* rotateA_degrees variants
*
* Revision 1.3  1997/03/26 09:20:01  doug
* *** empty log message ***
*
* Revision 1.2  1997/03/19 04:13:37  doug
* *** empty log message ***
*
* Revision 1.1  1997/02/28 09:29:12  doug
* Initial revision
*
*
**********************************************************************/

#include "mic.h"
#include <math.h>
#include "Vector.h"

using namespace std;
namespace sara
{

/*********************************************************************/
/// Create from another vector
Vector::Vector(const Vector &v) :
   x(v.x),
   y(v.y),
   z(v.z)
{
   /* empty */
}

/*********************************************************************/
/// Rotate the vector R radians about the X axis
void
Vector::rotateX(double R)
{
   double c = cos(R);
   double s = sin(R);
   double newy = y*c - z*s;
   z = y*s + z*c;
   y = newy;
}

/*********************************************************************/
/// Rotate the vector D degrees about the X axis
void
Vector::rotateX_degrees(double D)
{
   rotateX( DEGREES_TO_RADIANS(D) );
}

/*********************************************************************/
/// Rotate the vector R radians about the Y axis
void
Vector::rotateY(double R)
{
   double c = cos(R);
   double s = sin(R);
   double newx = x*c + z*s;
   z = -1 * x*s + z*c;
   x = newx;
}

/*********************************************************************/
/// Rotate the vector R degrees about the Y axis
void
Vector::rotateY_degrees(double D)
{
   rotateY(DEGREES_TO_RADIANS(D) );
}

/*********************************************************************/
/// Rotate the vector R radians about the Z axis
void
Vector::rotateZ(double R)
{
   double c = cos(R);
   double s = sin(R);
   double newx = x*c - y*s;
   y = x*s + y*c;
   x = newx;
}

/*********************************************************************/
/// Rotate the vector R degrees about the Z axis
void
Vector::rotateZ_degrees(double D)
{
   rotateZ(DEGREES_TO_RADIANS(D) );
}

/*********************************************************************/
/// Compute the length of the X,Y components of the vector
double 
Vector::length_2d()
{
   double distance_squared = x*x + y*y;

   // Handle special case where distance is very small
   if( distance_squared <= 0.000001 )
      return 0.0;

   // Compute the distance.
   return sqrt(distance_squared);
}

/*********************************************************************/
/// Set the length of the vector to the desired_length
void 
Vector::set_length(const double desired_length)
{
   double len = length();

   // If we will get overflow, do something smarter.
   if( len < EPS_ZERO )
   {
      x = desired_length;
      y = 0.0;
      z = 0.0;
   }
   else
   {
      x *= desired_length / len;
      y *= desired_length / len;
      z *= desired_length / len;
   }
}

/*********************************************************************/
/// Set the 2d length of the X,Y components of the vector to LENGTH
void 
Vector::set_length_2d(const double desired_length)
{
   double len = length_2d();

   // If we will get overflow, do something smarter.
   if( len < EPS_ZERO )
   {
      x = desired_length;
      y = 0.0;
   }
   else
   {
      x *= desired_length / len;
      y *= desired_length / len;
   }

   z = 0.0;
}

/*********************************************************************/
/// Compute the length of the vector
double 
Vector::length()
{
   double distance_squared = x*x + y*y + z*z;

   // Handle special case where distance is very small
   if( distance_squared <= 0.000001 )
      return 0.0;

   // Compute the distance.
   return sqrt(distance_squared);
}

/*********************************************************************/
// Compute the 2d angle (in radians) about Z with +X being 0 and +Y being Pi/2 as expected.
double 
Vector::thetaZ()
{
   double theta(0.0);

   if( length() > EPS_ZERO )
      theta = atan2(y, x);

   return theta;
}

/*********************************************************************/
// Compute the 2d angle (in degrees) about Z with +X being 0 and +Y being Pi/2 as expected.
double 
Vector::thetaZ_degrees()
{
   return RADIANS_TO_DEGREES( thetaZ() );
}

/*********************************************************************/
/// Turn the 2d vector (X,Y) into a unit vector
void 
Vector::make_unit_2d()
{
   double len = length_2d();

   // If we will get overflow, do something smarter.
   if( len < EPS_ZERO )
   {
      x = 1.0;
      y = 0.0;
   }
   else
   {
      x /= len;
      y /= len;
   }

   z = 0.0;
}

/*********************************************************************/
/// Turn the vector into a unit vector
void 
Vector::make_unit()
{
   double len = length();

   // If we will get overflow, do something smarter.
   if( len < EPS_ZERO )
   {
      x = 1.0;
      y = 0.0;
      z = 0.0;
   }
   else
   {
      x /= len;
      y /= len;
      z /= len;
   }
}

// ********************************************************************
/// Returns a unit vector along this vector
Vector 
Vector::unit() const
{
   Vector v = *this;
   v.make_unit();
   return v;
}

// ********************************************************************
/// Compute the dot product of THIS . V
double 
Vector::dotProduct(const Vector &v) const
{
   double rtn = x*v.x + y*v.y + z*v.z;
   return rtn;   
}

// ********************************************************************
/// Compute the cross product of THIS x V
Vector 
Vector::crossProduct(const Vector &v) const
{
   Vector rtn(y*v.z - z*v.y,
	      z*v.x - x*v.z,
	      x*v.y - y*v.x);
   return rtn;
}

// ********************************************************************
/// Compute the angle swept out when the specified vector is rotated 
/// around the Z axis and onto this vector.  
/// Note this vector and v must both lie in the XY plane!
/// Returns theta in radians
double 
Vector::dThetaZ(const Vector &v) const
{
   Vector v1( unit() );
   Vector v2( v.unit() );
   Vector d = v2.crossProduct(v1);
   double x = v2.dotProduct(v1);
   double y = sign(d.z) * d.length();
//cerr << "v1= " << v1 << " v2=" << v2 << " x=" << x << " y=" << y << endl;
   return atan2(y,x);
}

/*********************************************************************/
/// Rotate this point around the specified Axis by the angle theta (radians)
/// Positive angles are anticlockwise looking down the axis towards the origin.
/// Assume right hand coordinate system.
void 
Vector::rotateAroundAxis(const Vector &Axis, const double theta)
{
   double costheta = cos(theta);
   double sintheta = sin(theta);

   return rotateAroundAxis(Axis, sintheta, costheta);
}

/// Rotate this point around the specified Axis by the sin and cos of theta
/// Positive angles are anticlockwise looking down the axis towards the origin.
/// Assume right hand coordinate system.
void 
Vector::rotateAroundAxis(const Vector &Axis, const double sintheta, const double costheta)
{
   Vector q(0.0,0.0,0.0);

   Vector r = Axis.unit();

   q.x += (costheta + (1 - costheta) * r.x * r.x) * x;
   q.x += ((1 - costheta) * r.x * r.y - r.z * sintheta) * y;
   q.x += ((1 - costheta) * r.x * r.z + r.y * sintheta) * z;

   q.y += ((1 - costheta) * r.x * r.y + r.z * sintheta) * x;
   q.y += (costheta + (1 - costheta) * r.y * r.y) * y;
   q.y += ((1 - costheta) * r.y * r.z - r.x * sintheta) * z;

   q.z += ((1 - costheta) * r.x * r.z - r.y * sintheta) * x;
   q.z += ((1 - costheta) * r.y * r.z + r.x * sintheta) * y;
   q.z += (costheta + (1 - costheta) * r.z * r.z) * z;

   *this = q;
}

/*********************************************************************/
/// Create a 2d unit vector (X,Y) pointing in the R radians direction.
void 
Vector::make_unit_along_dir(double R)
{
   x = cos(R);
   y = sin(R);
   z = 0;
}

/*********************************************************************/
/// Create a 2d unit vector (X,Y) pointing in the D degrees direction.
void 
Vector::make_unit_along_dir_degrees(double D)
{
   make_unit_along_dir(DEGREES_TO_RADIANS(D));
}

/*********************************************************************/
/// Multiply the vector by a scalar value (supports: V * d)
Vector 
operator*(const Vector &v, const double d)
{
   Vector rtn( v.x * d, 
	       v.y * d, 
	       v.z * d);
   return rtn;
}

/*********************************************************************/
/// Multiply the vector by a scalar value (supports: V *= d)
Vector &
Vector::operator*=(const double d)
{
   x *= d;
   y *= d;
   z *= d;

   return *this;
}

/*********************************************************************/
/// Divide the vector by a scalar value (supports: V /= d)
Vector &
Vector::operator/=(const double d)
{
   x /= d;
   y /= d;
   z /= d;

   return *this;
}

/*********************************************************************/
/// Support multiplication by a Vector (form: V1 *= V2)
Vector &
Vector::operator*=(const Vector &v)
{
   x *= v.x;
   y *= v.y;
   z *= v.z;

   return *this;
}

/*********************************************************************/
/// Support addition by a Vector (form: V1 += V2)
Vector &
Vector::operator+=(const Vector &v)
{
   x += v.x;
   y += v.y;
   z += v.z;

   return *this;
}

/*********************************************************************/
/// Support subtraction by a Vector (form: V1 -= V2)
Vector &
Vector::operator-=(const Vector &v)
{
   x -= v.x;
   y -= v.y;
   z -= v.z;

   return *this;
}

/*********************************************************************/
/// Support division by a scalar (form: V / D)
Vector
operator/(const Vector &v, const double d)
{
   Vector rtn( v );
   rtn /= d;
   return rtn;
}

/*********************************************************************/
/// Support addition by a Vector (form: V1 + V2)
Vector 
operator+(const Vector &v1, const Vector &v2)
{
   Vector rtn( v1 );
   rtn += v2;
   return rtn;
}

	 
/*********************************************************************/
/// Support subtraction by a Vector (form: V1 - V2)
Vector 
operator-(const Vector &v1, const Vector &v2)
{
   Vector rtn( v1 );
   rtn -= v2;
   return rtn;
}

	 
/*********************************************************************/
/// Support multiplication by a Vector (form: V1 * V2)
Vector 
operator*(const Vector &v1, const Vector &v2)
{
   Vector rtn( v1 );
   rtn *= v2;
   return rtn;
}

// *********************************************************************
// Stream out the value
ostream & operator << (ostream & out, const Vector &v)
{
//char hack[2048];
//sprintf(hack, "<%.20lf,%.20lf,%.20lf>", v.x, v.y, v.z);
//out << hack;
   out << std::setprecision(FloatPrecision)
       << "<" << v.x << "," << v.y << "," << v.z << ">";
   return out;
}

// *********************************************************************
/// Compute the center of the polygon
Vector PolygonCenter(const Vectors &v)
{
   Vector center;
   int points = v.size();
   for(int i=0; i<points; i++)
   {
      center += v[i];
   }

   if( points > 1 )
      center /= points;

   return center;
}

// *********************************************************************
/// Create a vector from a stream
Vector *
Vector::load(const string &in)
{
   stringstream ist(in);

   // Legal strings: "<111.111, 222.222, 333.333>"
   //                "<111.111, 222.222>"

   double x, y, z(0.0);
   char comma, bracket, ch;

   // Single location
   bracket = '.';
   comma = '.';
   do
   {
      ist >> bracket;
   } while( !ist.eof() && !ist.fail() && isspace(bracket) );
   if( ist.eof() || ist.fail() || bracket != '<' )
      return NULL;

   // get the X
   ist >> x;
   if( ist.eof() || ist.fail() )
      return NULL;

   // get past the comma
   do
   {
      ist >> comma;
   } while( !ist.eof() && !ist.fail() && isspace(comma) );
   if( ist.eof() || ist.fail() || comma != ',' )
      return NULL;

   // get the Y
   ist >> y;
   if( ist.eof() || ist.fail() )
      return NULL;

   // Get either the comma or the final bracket
   do
   {
      ist >> ch;
   } while( !ist.eof() && !ist.fail() && isspace(ch) );
   if(ist.eof() || ist.fail() || ch != ',' && ch != '>')
      return NULL;

   // get the Z
   if( ch == ',' )
   {
      ist >> z;
      if( ist.eof() || ist.fail() )
         return NULL;

      // Get the final bracket
      do
      {
         ist >> ch;
      } while( !ist.eof() && !ist.fail() && isspace(ch) );
      if( ist.eof() || ist.fail() || ch != '>')
         return NULL;
   }

   // good one.
   return new Vector(x,y,z);
}

}
// *********************************************************************
