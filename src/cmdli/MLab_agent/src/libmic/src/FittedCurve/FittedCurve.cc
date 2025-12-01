/**********************************************************************
 **                                                                  **
 **  FittedCurve.h                                                   **
 **                                                                  **
 **  Create a 3D curve with specified end points and tangents with   **
 **  the specified length.                                           **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-2002.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Alto, Michigan, USA                                             **
 **                                                                  **
 **********************************************************************/

/* $Id: FittedCurve.cc,v 1.1.1.1 2008/07/14 16:44:16 endo Exp $ */

/**********************************************************************
* $Log: FittedCurve.cc,v $
* Revision 1.1.1.1  2008/07/14 16:44:16  endo
* MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
*
* Revision 1.1.1.1  2006/07/20 17:17:46  endo
* MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
*
* Revision 1.1.1.1  2006/07/12 13:37:55  endo
* MissionLab 7.0
*
* Revision 1.1  2006/07/01 00:30:28  endo
* CMDLi from MARS 2020 migrated into AO-FNC repository.
*
* Revision 1.1.1.1  2006/06/29 20:42:07  endo
* cmdli local repository.
*
* Revision 1.15  2004/05/11 19:34:29  doug
* massive changes to integrate with USC and GaTech
*
* Revision 1.14  2003/03/26 22:58:47  doug
* moved libccl_code to the sara namespace
*
* Revision 1.13  2003/03/21 15:55:30  doug
* moved libccl_code to the sara namespace
*
* Revision 1.12  2003/03/10 16:31:34  doug
* FittedCurve is closer
*
* Revision 1.10  2003/03/03 13:44:23  doug
* seems to work
*
* Revision 1.9  2003/02/28 22:07:21  doug
* turn off some debug prints
*
* Revision 1.8  2002/09/30 14:34:54  doug
* *** empty log message ***
*
* Revision 1.7  2002/09/06 13:16:52  doug
* *** empty log message ***
*
* Revision 1.6  2002/08/29 17:06:06  doug
* *** empty log message ***
*
* Revision 1.5  2002/08/29 02:23:37  doug
* *** empty log message ***
*
* Revision 1.4  2002/08/27 18:58:09  doug
* *** empty log message ***
*
* Revision 1.3  2002/08/26 19:51:34  doug
* *** empty log message ***
*
* Revision 1.2  2002/08/23 19:07:56  doug
* *** empty log message ***
*
* Revision 1.1  2002/08/23 18:03:22  doug
* Initial revision
*
**********************************************************************/

#include "mic.h"
#include <math.h>
#include "FittedCurve.h"

namespace sara
{
uint newPointIndex = 0;

/// The extension/compression spring constant
static const double springK = 10.0;

/// The torsional spring constants
static const double springT =  5.0;   // local (5.0 worked)
static const double springT2 = 3.0;   // right (3.0 worked)
static const double springT3 = 3.0;   // left  (3.0 worked)

/// The number of cycles to loop in smoothing
//static const uint smoothCycles = 200;
static const uint smoothCycles = 1000;

double maxMovementFactor = 1.00;
double friction = 20.0;
bool debugFlag = false;

// *********************************************************************
/// Create the FittedCurve object
FittedCurve::FittedCurve()
{
}

// *********************************************************************
/// Destroy the FittedCurve
FittedCurve::~FittedCurve()
{
}

// *********************************************************************
/// Create a new FittedCurve
void 
FittedCurve::initCurve(const uint numSegments, const double segLength, const Vector &point1, const Vector &point2, const Vector &heading1, const Vector &heading2)
{
   // Remember our step size
	




// $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$

// GIANT HACK!!!!
// The segment length always runs about 15% long, so just start out smaller for now
//   segmentLength = segLength;
   segmentLength = segLength * 0.90;

// $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
   





   // create a vector for one step along the initial heading
   Vector dv = heading1.unit() * segmentLength;

   // create a fake point one step before the real first step (to simplify the smoothing alg)
   Vector p0 = point1 - dv;
   
   // create a fake point one step after the real last step (to simplify the smoothing alg)
   Vector p3 = point2 + (heading2.unit() * segmentLength);

   // Add the first two fixed points
   curve.push_back(waypoint(p0));
   curve.push_back(waypoint(point1));

   // loop for 1 less than the number of segments, 
   // adding an intermediate point during each loop
   // The intermediate points lie on a straight line from p1, along the initial heading
/* 
   double dx = (point2.x - point1.x) / numSegments;
   double dy = (point2.y - point1.y) / numSegments;
   Vector dv(dx, dy, 0.0);
*/
   Vector p = point1;
   for(uint i=1; i<numSegments; i++)
   {
      p += dv;
      curve.push_back(p);
   }   

   // Add the last two fixed points
   curve.push_back(waypoint(point2));
   curve.push_back(waypoint(p3));

   // Pick some smoothing criteria
   smoothnessBound = segLength * 0.05;  // 5 percent
   maxSmoothingLoops = 10;

   // Save some info for later
   finalPoint = point2;

/*
cerr << "\"Initial points\"" << endl;
   for(int i=0; i<(int)curve.size(); i++)
   {
      cerr << curve[i].position.x << "," << curve[i].position.y << "," << curve[i].position.z << endl;
   }
   cerr << endl;

cerr << endl << "\"forces\"" << endl;
*/
   // smooth the curve.
   smoothCurve();
/*
cerr << "\"After Smooth\"" << endl;
   for(int i=0; i<(int)curve.size(); i++)
   {
      cerr << curve[i].position.x << "," << curve[i].position.y << "," << curve[i].position.z << endl;
   }
   cerr << endl;

exit(0);
*/
}

// *********************************************************************
/// Specify the actual location and heading, and get the next point along the curve
Vector 
FittedCurve::computeNextPoint(const Vector &actualPoint, const Vector &actualHeading)
{
   // Fix up the Z value: just linear change in altitude along the path
   if( curve.size() <= 4 )
   {
//      cerr << "FittedCurve::computeNextPoint - called with curve size=" << curve.size() << endl;
      return finalPoint;
   }

   // Throw away p0 and p1.
   curve.pop_front();
   curve.pop_front();

   // update our actual position
   curve[0] = actualPoint;

   // Compute a new p0 based on the actual heading.
   Vector p0 = actualPoint - (actualHeading.unit() * segmentLength);

   // Add it the the front of the list
   curve.push_front(p0);

//cerr << "FittedCurve::computeNextPoint - before smooth, curve.size() = " << curve.size() << endl;

   // Re-smooth the curve.
   smoothCurve();

//cerr << "FittedCurve::computeNextPoint - after smooth, curve.size() = " << curve.size() << endl;
   Vector rtn = curve[2].position;

//cerr << "FittedCurve::computeNextPoint - 3" << endl;

   // Fix up the Z value: just linear change in altitude along the path
   if( curve.size() <= 4 )
   {
      rtn.z = finalPoint.z;
   }
   else
   {
      double dz = (finalPoint.z - actualPoint.z) / (curve.size() - 4);
      rtn.z = actualPoint.z + dz;
   }

   // Return the next waypoint to move towards.
   // Recall the first 2 (and last 2) points are fixed, used to smooth the curve.
   return rtn;
}

// *********************************************************************
/// Specify the actual location and heading, and get the next point along the curve
/// also returns the length of the path after this leg just returned
Vector 
FittedCurve::computeNextPoint(const Vector &actualPoint, const Vector &actualHeading, double &remainingLength)
{
   Vector rtn = computeNextPoint(actualPoint, actualHeading);
   
   // Compute the length of the remaining path
   double lng = 0;
   for(uint i=3; i<curve.size()-1; i++)
   {
      Vector diff = curve[i+1].position - curve[i].position;
      lng += diff.length();
   }
   remainingLength = lng;

   return rtn;
}

// *********************************************************************
/// Compute the forces acting on a node using a spring model
void
FittedCurve::computeForces()
{
   double maxDist = segmentLength * maxMovementFactor;
   int maxpoint = (int)curve.size() - 1;

   // zero the force bucket for the initial one
   curve[maxpoint].forces.clear();
   curve[maxpoint - 1].forces.clear();

   // Compute tension/compression forces trying to restore the length of the spring
   // work from the right back to the left since the greatest disturbance is on the right
   for(int i=maxpoint-2; i>1; i--)
   {
      Vector leftLink = curve[i-1].position - curve[i].position;
      Vector left2Link = curve[i-2].position - curve[i].position;
      Vector rightLink = curve[i+1].position - curve[i].position;
      Vector right2Link = curve[i+2].position - curve[i].position;

      double leftLength  = leftLink.length();
      double rightLength = rightLink.length();
      Vector diff = rightLink.unit() + leftLink.unit();

      double leftForce  = springK * (leftLength - segmentLength);
      double rightForce = springK * (rightLength - segmentLength);

      // compute the "pressure" that the point is under 
//      double pressure = leftLink.length() + rightLink.length();

      // compute our torque
      Vector torque = diff * (-1 * springT /* * pressure */ / 2.0);

if( debugFlag )
{
   cerr << endl << "point " << i << endl;
   cerr << " leftLink=" << leftLink << " " << " rightLink=" << rightLink << endl;
   cerr << " dleftLength=" << leftLength-segmentLength << " drightLength=" << rightLength-segmentLength << "  ";

   Vector vl = leftLink + rightLink;
   cerr << " linear=" << vl.x << "," << vl.y << "," << vl.z << endl;
   cerr << " diff=" << diff.x << "," << diff.y << "," << diff.z << " ";
   cerr << " torque=" << torque.x << "," << torque.y << "," << torque.z << " ";
}

      // compute the tangential vector as a vector pointing from this point to the 
      // center of a quadrateral created with our left and right link.
      Vector tangential = (leftLink + rightLink) / 2.0 * springT;

      // compute the tangential vector as a vector pointing from this point to the 
      // center of a quadrateral created with our left2 and right2 link.
//      Vector tangential2 = (left2Link + right2Link) / 2.0 * springT2;
//cerr << " left2Link=" << left2Link << " " << " right2Link=" << right2Link << " tangential2 =" << tangential2 << endl;

      // compute our co-linear forces
      leftLink.set_length(leftForce);
      rightLink.set_length(rightForce);
      Vector colinear = leftLink + rightLink;

      // add the forces together
      curve[i].forces = colinear + tangential;

//      if( i == 2 )
      if( i < maxpoint/2 )
      {
         // Same for the left, but only on the left most point:
         // compute where we would be if (i+1) -> (i+2) was reversed to point at us.
         Vector dvL = curve[i-1].position - curve[i-2].position;

//cerr << " dvL=" << dvL << " " << " i-1 pos=" << curve[i-1].position << " i-2 pos=" << curve[i-2].position << endl;
	 
         // fixup the length in case there is a large discontinuity
         dvL.set_length( segmentLength );
//cerr << " dvL new length=" << dvL;
         Vector desiredL = curve[i-1].position + dvL;
//cerr << " desiredL =" << desiredL;
         Vector tangentialL = (desiredL - curve[i].position) * springT3;
//cerr << " tangentialL =" << tangentialL << " current=" << curve[i].position << endl;

         curve[i].forces += tangentialL;
      }
//      else if( i == maxpoint-2 )
      {
         // compute where we would be if (i+1) -> (i+2) was reversed to point at us.
         Vector dvR = curve[i+1].position - curve[i+2].position;
         // fixup the length in case there is a large discontinuity
         dvR.set_length( segmentLength );
         Vector desiredR = curve[i+1].position + dvR;
         Vector tangentialR = (desiredR - curve[i].position) * springT2;

         curve[i].forces += tangentialR;
      }


      // Add in a tangential force trying to make the left and right links co-linear
/* 
      Vector left = curve[i].position - curve[i-1].position;
      Vector full = curve[i+1].position - curve[i-1].position;
      double lng = full.dotProduct(left);
cerr << "left=" << left << " full=" << full << " lng=" << lng << endl;
      full.set_length( left.dotProduct(full) );
      full += curve[i-1].position;
*/
//      Vector tangential = (full - curve[i].position) * springT;
//cerr << " point " << i << " current loc=" << curve[i].position /*<< " desiredLoc=" << full*/ << " tangential=" << tangential << endl;

      // init the left force bucket with our transferred torque (we are moving left)
//      curve[i-1].forces = torque * leftLength;

      // add the torque to our right waypoint (we are moving left)
//      curve[i+1].forces += torque * rightLength;

if( debugFlag )
{
   Vector leftTorque = torque * leftLength;
   cerr << " left torque force=" << leftTorque.x << "," << leftTorque.y << "," << leftTorque.z << " ";

   cerr << " force=" << curve[i].forces.x << "," << curve[i].forces.y << "," << curve[i].forces.z << endl;

   Vector rightTorque = torque * rightLength;
   cerr << " right torque force=" << rightTorque.x << "," << rightTorque.y << "," << rightTorque.z << " " << endl;
   cerr << "Total force on point " << i+1 << " = " << curve[i+1].forces.x << "," << curve[i+1].forces.y << "," << curve[i+1].forces.z << " " << endl;
}

      Vector delta = curve[i].forces / friction;
      if( delta.length() > maxDist )
      {
         delta.set_length(maxDist);
      }

      if( debugFlag )
      {
         cerr << "Moving point " << i << " by " << delta.x << "," << delta.y << "," << delta.z << " " << endl;
      }

      curve[i].position += delta;
   }
}

// *********************************************************************
/// Compute a smoothness score for the curve
/// We mostly care about a smooth curve, reward constant curvature
double 
FittedCurve::score(const uint newPointIndex, const Vector &newPoint)
{
   // Save the current value of the point
   Vector saved = curve[newPointIndex].position;
   curve[newPointIndex].position = newPoint;

   // Compute the averate curvature for the path
   double curvature = 0;
   int numPoints = 0;
   double newCurve = 0;
   for(int i=1; i<(int)curve.size()-1; i++)
   {
      Vector df0 = curve[i].position - curve[i-1].position;
      Vector df1 = curve[i+1].position - curve[i].position;

      Vector d2f = df1 - df0;

      double d = df0.length();
      Vector n = df0.crossProduct(d2f);
      double c = sign(n.z) * n.length() / (d*d*d);

      curvature += c;
      numPoints ++;

      if( i == (int)newPointIndex )
         newCurve = c;
   }

   if( numPoints > 0 )
      curvature /= numPoints;

   // Restore the value of the point
   curve[newPointIndex].position = saved;

   // Return the curvature error for the new point
   return fabs(curvature - newCurve);
}

// *********************************************************************
/// Compute a smoothness score for 5 points.
/// We mostly care about a smooth curve, reward constant curvature
double 
FittedCurve::score(const Vector &p0, const Vector &p1, const Vector &p2, const Vector &p3, const Vector &p4)
{
   Vector df0 = p1 - p0;
   Vector df1 = p2 - p1;
   Vector df2 = p3 - p2;
   Vector df3 = p4 - p3;
//cout << "df0=" << df0 << " df1=" << df1 << " df2=" << df2 << " df3=" << df3 << endl;
/*
   Vector d2fA = df1 - df0;
   Vector d2fB = df2 - df1;
   Vector d2fC = df3 - df2;
//cout << "d2fA=" << d2fA << " d2fB=" << d2fB << " d2fC=" << d2fC << endl;

   double d = df0.length();
   Vector n = df0.crossProduct(d2fA);
   double c0 = sign(n.z) * n.length() / (d*d*d);
//cout << "n0=" << n << " c0=" << c0;

   d = df1.length();
   n = df1.crossProduct(d2fB);
   double c1 = sign(n.z) * n.length() / (d*d*d);
//cout << "n1=" << n << " c1=" << c1;

   d = df2.length();
   n = df2.crossProduct(d2fC);
   double c2 = sign(n.z) * n.length() / (d*d*d);
//cout << "n2=" << n << " c2=" << c2 << endl;

   double smoothnessErr = (fabs(c0 - c1) + fabs(c1 - c2) + fabs(c0 - c2));
*/
   double smoothnessErr = score(newPointIndex, p2);
/*
   Vector p0_to_1 = p1 - p0;
   Vector p1_to_2 = p2 - p1;
   Vector p2_to_3 = p3 - p2;
   Vector p3_to_4 = p4 - p3;

   Vector d1 = p0_to_1.crossProduct(p1_to_2);
   Vector d2 = p1_to_2.crossProduct(p2_to_3);
   Vector d3 = p2_to_3.crossProduct(p3_to_4);

//cout << "d1=" << d1 << " d2=" << d2 << "d3=" << d3 << endl;

   Vector ddA = d2 - d1;
   Vector ddB = d3 - d1;
   Vector ddC = d3 - d2;
   double smoothnessErr = ddA.length() + ddB.length() + ddC.length();
*/
   double lengthErr = fabs(df1.length() - segmentLength) +
                      fabs(df2.length() - segmentLength);


   // score is the change in angles and square of length error
   double score = sqr(smoothnessErr) + sqr(lengthErr);

//cout << "smoothnessErr =" << smoothnessErr << " lengthErr=" << lengthErr << " score=" << score << endl;

   return score;
}

// *********************************************************************
/// Compute where to move the center point (p2) given 5 points
/// p0 and p4 are used to smooth the curve.
///    p0------p1     p2     p3-----p4
// Copies the result into p2 and returns the distance the point moved.
double 
FittedCurve::placeMidPoint(const Vector &p0, const Vector &p1, Vector &p2, const Vector &p3, const Vector &p4, const double gain)
{
   Vector originalP2 = p2;

//cout << endl << "placeMidPoint  p0=" << p0 << " p1=" << p1 << " p2=" << p2 << " p3=" << p3 << "p4=" << p4 << endl;
//   double curScore = score(p0, p1, p2, p3, p4);

/*
   // Decide if the left is pushing or pulling
   Vector p2_to_1 = p1 - p2;
   // positive is pulling
   double left = p2_to_1.length() - segmentLength;

   // Decide if the right is pushing or pulling
   Vector p2_to_3 = p3 - p2;
   // positive is pulling
   double right = p2_to_3.length() - segmentLength;

//cout << "left=" << left << " right=" << right << endl;

   // Is there a significant spring force?
   if( fabs(left) + fabs(right) > segmentLength * 0.1 )
   {
      // sum the forces to get a movement vector
      Vector movement = (p2_to_1 * left + p2_to_3 * right) / 2.0;
      double lng = movement.length();

//cout << "movement=" << movement << " movement.length()=" << lng << endl;

      // check for sigularity: all three points on a line, but pushing.
      if( left < EPS_ZERO && right < EPS_ZERO && lng <= 0.1 )
      {
         // Randomly pick a direction and move the point off the axis.
         double dx = segmentLength * 0.10 * ((double)random() / RAND_MAX);
         double dy = segmentLength * 0.10 * ((double)random() / RAND_MAX);

         movement = Vector(dx, dy, 0.0);
         p2 += movement * 0.20; // slow it down a bit

//         double newScore = score(p0, p1, p2, p3, p4);
//cout << "random spring movement: curScore=" << curScore << " newScore=" << newScore << endl;
      }
      else
      {
         Vector newp2 = p2 + movement * 0.20; // slow it down a bit
         double newScore = score(p0, p1, newp2, p3, p4);
         if( newScore + EPS_ZERO < curScore )
         {
//cout << "standard spring movement: curScore=" << curScore << " newScore=" << newScore << endl;
            p2 = newp2;
         }
//	 else
//	 {
//cout << "NO spring movement" << endl;
//	 }
      }
   }

   // Now jiggle it a bit.
   curScore = score(p0, p1, p2, p3, p4);
   for(int loop=0; loop<10; loop++)
   {
      // bounce the point around (positive and negative)
      double dx = 2 * ((double)random() / RAND_MAX) - 1.0;
      double dy = 2 * ((double)random() / RAND_MAX) - 1.0;
      dx *= segmentLength * gain;
      dy *= segmentLength * gain;

//cout << "gain=" << gain << " dx=" << dx << " dy=" << dy << " dz=" << dz << endl;

      Vector newp2( p2.x + dx,  p2.y + dy,  0.0);
      double newScore = score(p0, p1, newp2, p3, p4);

//cout << "jiggle check: p2=" << p2 << "  newp2=" << newp2 << " curScore=" << curScore << " newScore=" << newScore << endl;

      if( newScore < curScore )
      {
//cout << "jiggled it: p2=" << p2 << "  newp2=" << newp2 << " curScore=" << curScore << " newScore=" << newScore << endl;
         p2 = newp2;
         break;
      }
   }
*/







/*
cout << endl << "p0=" << p0 << " p1=" << p1 << " p2=" << p2 << " p3=" << p3 << "p4=" << p4 << endl;

   double curScore = score(p0, p1, p2, p3, p4);

   // Compute the input and output trajectories
   Vector p0_to_1 = p1 - p0;
   Vector p3_to_4_t = p4 - p3;

cout << "p0_to_1=" << p0_to_1 << " p3_to_4_t=" <<  p3_to_4_t << endl;

   // make them unit vectors
   Vector A(p0_to_1.unit());
   Vector B(p3_to_4_t.unit());

   // Compute the cross product to get the rotation axis 
   Vector d1 = A.crossProduct(B);
   double cost = A.dotProduct(B);
   double sint = d1.length();
   double theta = asin( sint );
   if( cost < 0 )
      theta = PI - theta;

cout << "cost=" << cost << " sint=" << sint << " theta=" << RADIANS_TO_DEGREES(theta) << endl;

cout << "d1=" << d1 << " A=" << A << endl;

   // Rotate the input trajectory by 1/3 of the angle (three steps from in to out)
   A.rotateAroundAxis(d1, theta/3.0);

cout << "A rotated around d1 by 1/3 theta=" << A << endl;

   // Make A a bit closer to the desired length
   double diff = segmentLength - A.length();
   diff *= 0.25; // 25%
   double lng = A.length() + diff;
   A = A.unit() * lng;

   // Compute the new point location
   Vector newp2 = p1 + A;

cout << "newp2=" << newp2 << endl;

   double newScore = score(p0, p1, newp2, p3, p4);

cout << "curScore=" << curScore << " newScore=" << newScore << endl;

   if( newScore < curScore )
   {
      p2 = newp2;
   }
   else
   {
      // Overshoot, go part way.
      Vector delta = (newp2 - p2) * 0.1;
      p2 += delta;
   }

newScore = score(p0, p1, p2, p3, p4);
cout << "final Score=" << newScore << endl;
*/
/*
   if( newScore < curScore )
   {
      p2 = newp2;
   }
   else
   {
      // Overshoot, go part way.
      Vector delta = (newp2 - p2) * 0.1;
      newp2 = p2 + delta;

      double newScore = score(p0, p1, newp2, p3, p4);

cout << "10% newp2=" << newp2 << " 10% newp2 score=" << newScore << endl;
      p2 = newp2;
   }
*/
/*
   // bounce the point around
   double dx = 2 * gain * ((double)random() / RAND_MAX) - gain;
   double dy = 2 * gain * ((double)random() / RAND_MAX) - gain;
   double dz = 2 * gain * ((double)random() / RAND_MAX) - gain;

   Vector newp2( p2.x + dx,  p2.y + dy,  p2.z + dz);
   double newScore = score(p0, p1, newp2, p3, p4);
   if( newScore > curScore )
   {
      newp2 = Vector( p2.x + dx,  p2.y,  p2.z);
      newScore = score(p0, p1, newp2, p3, p4);
   }

   if( newScore > curScore )
   {
      newp2 = Vector( p2.x,  p2.y + dy,  p2.z);
      newScore = score(p0, p1, newp2, p3, p4);
   }

   if( newScore > curScore )
   {
      newp2 = Vector( p2.x,  p2.y,  p2.z + dz);
      newScore = score(p0, p1, newp2, p3, p4);
   }

cout << "p2=" << p2 << "  newp2=" << newp2 << " curScore=" << curScore << " newScore=" << newScore << endl;

   double rtn = curScore;
   if( newScore < curScore )
   {
      p2 = newp2;
      rtn = newScore;
   }

   // return the final score for this point.
   return rtn;
*/

   // translate p3 so p1 is at the orgin
   Vector t3 = p3 - p1; 

   // Is it out of reach or just barely touches?
   if( t3.length()+EPS_ZERO >= 2*segmentLength )
   {
      // Yup, just pick the mid point.
      Vector v = p1 + ((p3 - p1) / 2.0);

      // compute how far the point moved
      Vector tmp = v - p2;

      // update the point location
      p2 = v;

      // return the distance moved
      return tmp.length();
   }

   // Rotate about Z so p3 is on X axis to make the math easier
   double length = t3.length();
   double cost = t3.x / length;
   double sint = t3.y / length;
   t3.y = 0;
   t3.x = length;

   // The intersection is a circle in the XZ plane
   
   // For 2D case, there are two points with +-Y
   // The new X point will be half way between the two points
   double newX = t3.x / 2.0;
   double newY = sqrt( sqr(segmentLength) - sqr(newX) );
   double newZ = 0.0;

   // Pick the best point
   Vector p2a(newX, newY, newZ);
   Vector p2b(newX, -1 * newY, newZ);

   // Translate back to normal coordinate frame.
   p2a += p1;
   p2b += p1;

   // Rotate back to normal coordinate frame.
   Vector p;
   p.x = cost * p2a.x +  -sint * p2a.y;
   p.y = sint * p2a.x +  cost  * p2a.y;
   p.z = p2a.z;
   p2a = p;

   p.x = cost * p2b.x +  -sint * p2b.y;
   p.y = sint * p2b.x +  cost  * p2b.y;
   p.z = p2b.z;
   p2b = p;

   // Score the two choices (in 2D case) and compare to current placement.
   double scorep2 = score(p0, p1, p2, p3, p4);
   double scorep2a = score(p0, p1, p2a, p3, p4);
   double scorep2b = score(p0, p1, p2b, p3, p4);

   // Select the lowest score from the three placements.
   // Copy the result into p2 and return the distance the point moved.
   if( scorep2 < scorep2a )
   {
      if( scorep2 < scorep2b )
      {
         // p2 = p2;
      }
      else
      {
         p2 = p2b;
      }
   }
   else
   {
      if( scorep2a < scorep2b )
      {
         p2 = p2a;
      }
      else
      {
         p2 = p2b;
      }
   }

//double finalScore = score(p0, p1, p2, p3, p4);
//cout << endl << "placeMidPoint done: score=" << finalScore << " p0=" << p0 << " p1=" << p1 << " p2=" << p2 << " p3=" << p3 << "p4=" << p4 << endl;

   Vector movement = p2 - originalP2;
   return fabs(movement.length());
}

// *********************************************************************
/// Compute the curve using a relaxation process
void 
FittedCurve::smoothCurve()
{
//cout << "FittedCurve::smoothCurve - starting to smooth curve " << endl << *this;

   // loop a few times so stuff can jiggle around
//   for(uint i=0; i<curve.size(); i++)
   for(uint i=0; i<smoothCycles; i++)
   {
      computeForces();
/*
   // compute an error bound
 
   // first 2 points and last 2 points are fixed.
   double lng = 0;
   for(int i=1; i<(int)curve.size() - 2; i++)
   {
      Vector v(curve[i+1].position - curve[i].position);
      lng += v.length();
   }

   double desired = segmentLength * (curve.size() - 3);
   cout << "Done: Length= " << lng << " desired=" << desired << " Error=" << fabs((lng-desired) / desired) * 100 << endl;
*/
   }

/*
   double gain=segmentLength * 0.10;

//   for(int loops=0; loops<maxSmoothingLoops; loops++)
   gain = 0.20;
   for(int loops=0; loops<200; loops++)
   {
      // first 2 points and last 2 points are fixed.
      double movement = 0;
      for(int i=2; i<(int)curve.size() - 2; i++)
      {
newPointIndex = i;
         movement += fabs( placeMidPoint(curve[i-2], curve[i-1], curve[i], curve[i+1], curve[i+2], gain) );
      }

      // compute the average distance the points moved
      movement /= curve.size() - 4;

//cout << "FittedCurve::smoothCurve - average movement = " << movement << " Current curve = " << endl << *this << endl << endl;

      // Is it good enough
      if( movement <= smoothnessBound )
      {
//cout << "Good enough: smoothnessBound=" << smoothnessBound << endl;
         break;
      }

      if( loops == 100 )
         gain = 0.10;
      else if( loops == 50 )
         gain = 0.05;
   }
*/

}

// *********************************************************************
// test harness
/*
void
FittedCurve::test()
{

   // straight line
   {
      Vector p0(0,0,0);
      Vector p1(1,0,0);
      Vector p2(2,0,0);
      Vector p3(3,0,0);
      Vector p4(4,0,0);
      segmentLength  = 1.0;

      double rtn = score(p0, p1, p2, p3, p4);
      cout << "straight line: score=" << rtn << endl;
   }

   // reverse straight line
   {
      Vector p0(4,0,0);
      Vector p1(3,0,0);
      Vector p2(2,0,0);
      Vector p3(1,0,0);
      Vector p4(0,0,0);
      segmentLength  = 1.0;

      double rtn = score(p0, p1, p2, p3, p4);
      cout << "reverse straight line: score=" << rtn << endl;
   }

   // 45 degree straight line
   {
      Vector p0(0,0,0);
      Vector p1(0.707106,0.707106,0);
      Vector p2(1.414213,1.414213,0);
      Vector p3(2.121320,2.121320,0);
      Vector p4(2.828426,2.828426,0);
      segmentLength  = 1.0;

      double rtn = score(p0, p1, p2, p3, p4);
      cout << "45 degree straight line: score=" << rtn << endl;
   }

   // half Circle
   {
      Vector p0(-1,0,0);
      Vector p1(-0.707106,0.707106,0);
      Vector p2(0,1,0);
      Vector p3(0.707106,0.707106,0);
      Vector p4(1,0,0);
      segmentLength  = 0.765366;

      double rtn = score(p0, p1, p2, p3, p4);
      cout << "half circle : score=" << rtn << endl;
   }
}
*/
// *********************************************************************
// Stream out the value
ostream & operator << (ostream & out, FittedCurve &c)
{
   // first and last points are fake.
   out << '[' << c.curve[0].position << "]" << endl;
   for(int i=1; i<(int)c.curve.size() - 1; i++)
   {
      out << c.curve[i].position << endl;
   }
   out << '[' << c.curve[c.curve.size()-1].position << "]" << endl;
   out << endl;

   for(int i=2; i<(int)c.curve.size() - 2; i++)
   {
newPointIndex = i;
      out << "Score at p" << i << "=" << 
         c.score(c.curve[i-2].position, c.curve[i-1].position, c.curve[i].position, c.curve[i+1].position, c.curve[i+2].position)
	 << endl;
   }
   out << endl;

   return out;
}

// *********************************************************************
}
