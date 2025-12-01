/**********************************************************************
 **                                                                  **
 **  axis.cc                                                         **
 **                                                                  **
 **  Implement a axis-like environmental object.                     **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-1998.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Alto, Michigan, USA                                             **
 **                                                                  **
 **********************************************************************/

/* $Id: axis.cc,v 1.1.1.1 2008/07/14 16:44:16 endo Exp $ */

/**********************************************************************
* $Log: axis.cc,v $
* Revision 1.1.1.1  2008/07/14 16:44:16  endo
* MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
*
* Revision 1.1.1.1  2006/07/20 17:17:46  endo
* MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
*
* Revision 1.1.1.1  2006/07/12 13:37:54  endo
* MissionLab 7.0
*
* Revision 1.1  2006/07/01 00:21:03  endo
* CMDLi from MARS 2020 migrated into AO-FNC repository.
*
* Revision 1.1.1.1  2006/06/29 20:42:07  endo
* cmdli local repository.
*
* Revision 1.4  2004/05/11 19:34:39  doug
* massive changes to integrate with USC and GaTech
*
* Revision 1.3  2004/03/08 14:52:28  doug
* cross compiles on visual C++
*
* Revision 1.2  2003/11/26 15:21:47  doug
* back from GaTech
*
* Revision 1.1  2002/09/20 13:05:19  doug
* Initial revision
*
**********************************************************************/

#include "mic.h"
#include "Vector.h"
#include "axis.h"
#include <math.h>

namespace sara
{
/**********************************************************************/
// Constructor
axis::axis()
{
}

/**********************************************************************/
// Constructor
axis::axis(const string &name, const Vectors &verticies, const double width, const string &color) /* :
   polygon(build_axis_polygon(verticies, width), color, name) */
{
}

/**********************************************************************/
// destructor
axis::~axis()
{
}

/**********************************************************************/
/// build a string of polylines that will draw the axis
Vectors
axis::build_axis_polygon(const Vectors &verticies, const double width)
{
   Vector      beg, pt, end;
   Vector         a, al, au, b, bl, bu, cu, cl;
   double         theta_a, theta_b, phi;
   double         dx, dy, length;
   double         head_length, head_width;
   Vectors        rtn;
   Vectors        stack;

   /* error */
   if( verticies.size() < 2 )
   {
      stringstream out;
      out << "Error in axis::build_axis: wrong number of points: " << verticies.size() << endl;
      ERROR(out.str().c_str());
      return rtn;
   }

   beg = verticies[0];
   end = verticies[ verticies.size()-1 ];

   /* Draw the cap at the start */
   pt = verticies[1];
   theta_a = atan2(pt.y - beg.y, pt.x - beg.x);
   dx = width * sin(theta_a) / 2.0;
   dy = width * cos(theta_a) / 2.0;
   al.x = beg.x + dx;
   al.y = beg.y - dy;
   au.x = beg.x - dx;
   au.y = beg.y + dy;
   a.x = beg.x;
   a.y = beg.y;
   b.x = pt.x;
   b.y = pt.y;
   rtn.push_back(al);
   rtn.push_back(au);

   // Add the segments up to the end
   for (uint i=1; i<verticies.size()-1; i++) 
   {
      /* find the next set of intersection points */
      pt = verticies[i+1];
      theta_b = atan2(pt.y - b.y, pt.x - b.x);
      phi = (theta_a + theta_b + PI) / 2.0;
      length = width/(2.0*sin(phi - theta_b));
      dx = length * cos(phi);
      dy = length * sin(phi);

      bl.x = b.x - dx;
      bl.y = b.y - dy;

      bu.x = b.x + dx;
      bu.y = b.y + dy;

      /* Draw the edges for this segment */
      rtn.push_back(bu);
      stack.push_back(bl);

      /* update for the next round */
      theta_a = theta_b;
      au = bu;
      al = bl;
      a = b;
      b.x = pt.x;
      b.y = pt.y;
   }


   /* Draw the last segment and the arrow head */
   dx = end.x - a.x;
   dy = end.y - a.y;
   head_length = sqrt(dx*dx + dy*dy);
   if (head_length < width * 2.0)
      head_length = 0.5 * head_length;
   else
      head_length = width;
   dx = head_length * cos(theta_a);
   dy = head_length * sin(theta_a);
   b.x = end.x - dx;
   b.y = end.y - dy;
   dx = width * sin(theta_a) / 2.0;
   dy = width * cos(theta_a) / 2.0;
   bl.x = b.x + dx;
   bl.y = b.y - dy;
   bu.x = b.x - dx;
   bu.y = b.y + dy;
   head_width = width * 1.4;
   dx = head_width * sin(theta_a) / 2.0;
   dy = head_width * cos(theta_a) / 2.0;
   cl.x = b.x + dx;
   cl.y = b.y - dy;
   cu.x = b.x - dx;
   cu.y = b.y + dy;

   rtn.push_back(bu);
   rtn.push_back(cu);
   rtn.push_back(end);
   rtn.push_back(cl);
   rtn.push_back(bl);

   for (int i=(int)stack.size()-1; i>=0; i--) 
   {
      rtn.push_back(stack[i]);
   }

   return rtn;
}
/**********************************************************************/
}
