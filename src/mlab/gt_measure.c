/**********************************************************************
 **                                                                  **
 **                           gt_measure.c                           **
 **                                                                  **
 **                                                                  **
 **  Written by:  Jonathan M. Cameron                                **
 **                                                                  **
 **  Copyright 1995, 1996, 1997 Georgia Tech Research Corporation    **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: gt_measure.c,v 1.1.1.1 2008/07/14 16:44:22 endo Exp $ */

extern "C" {
#include <stdio.h>
#include <malloc.h>
}

#include "console.h"

#define GT_MEASURE_C
#include "gt_measure.h"


/**********************************************************************
 **                                                                  **
 **                    implement global functions                    **
 **                                                                  **
 **********************************************************************/

/* decimal to dd.mmsssssssss */
double dec_to_dms(double dec) {
  double val,val2,val3,val4;
  char buf2[123],buf3[123],buf4[123],buf5[123],*res,buf4b[123];
  char buf[256];

  val = dec;
  sprintf(buf2,"%.9f",val);
  res=strstr(buf2,".");
  res[0]='\0';
  val2=atof(buf2);
  val=val-val2;
  val3=val/(1.0/60.0);
  sprintf(buf3,"%.7f",val3);
  res=strstr(buf3,".");
  res[0]='\0';
  val3=atof(buf3);
  val=val-(val3/60.0);
  val4=val/(1.0/3600.0);
  sprintf(buf4,"%.4f",val4);
  res=strstr(buf4,".");
  res[0]='\0';
  strcpy(buf4b,&res[1]);
  val4=atof(buf4);
  if (val4==60.0) {
    val4=0.0;
    strcpy(buf4,"00");
    val3=val3+1.0;
    sprintf(buf3,"%.0f",val3);
  }
  if (val3>=60.0) {
    val3=val3-60.0;;
    sprintf(buf3,"%.0f",val3);
    val2=val2+1.0;
    sprintf(buf2,"%.0f",val2);
  }
  if (buf4[0]=='-') {
    strcpy(buf5,&buf4[1]);
    strcpy(buf4,buf5);
  }
  if (buf3[0]=='-') {
    strcpy(buf5,&buf3[1]);
    strcpy(buf3,buf5);
  }
  if (strlen(buf4)<2) {
    sprintf(buf5,"0%s",buf4);
    strcpy(buf4,buf5);
  }
  if (strlen(buf3)<2) {
    sprintf(buf5,"0%s",buf3);
    strcpy(buf3,buf5);
  }
  sprintf(buf,"%s.%s%s%s",buf2,buf3,buf4,buf4b);
  return atof(buf);
}

/* separate dd,mm,ss */
void parsedeg(double dec_in,double *deg,double *min,double *sec) {
  char buf2[123],*res;
  double val4,latd,latm,lats,val2;
						   
  val4=dec_in;
  sprintf(buf2,"%.6f",dec_in);
  res=strstr(buf2,".");
  res[0]='\0';
  val2=atof(buf2);
  latd=val2;
  val4=val4-val2;
  val4=val4*100.0;
  sprintf(buf2,"%.6f",val4);
  res=strstr(buf2,".");
  res[0]='\0';
  val2=atof(buf2);
  latm=val2;
  val4=val4-val2;
  val4=val4*100.0;
  sprintf(buf2,"%.6f",val4);
  val2=atof(buf2);
  lats=val2;
  *deg=latd;
  *min=latm;
  *sec=lats;
}

/* distance from meridian */
void meridiandist(double latrad, double *m) {
  double latrad2,latrad4,latrad6,latsin2,latsin4,latsin6,latdecdeg;

  latrad2 = latrad * 2.0;
  latrad4 = latrad * 4.0;
  latrad6 = latrad * 6.0;
  latsin2 = sin(latrad2);
  latsin4 = sin(latrad4);
  latsin6 = sin(latrad6);
  latdecdeg = latrad * 180.0 / (4.0 * atan(1.0));
  *m = 111132.0894 * latdecdeg - 16216.94 * latsin2 + 17.21 * latsin4 - 0.02 * latsin6;
}

/* lat/lon latitude and longitude should be passed in as decimal degrees
 * they will be converted to utm coordinates
 */
#define DEG_TO_RAD 0.017453293
#define RAD_TO_DEG 57.29578
void gt_convert_latlon_to_utm(double lat, double lon, double *utmx, 
			      double *utmy) 
{
  double ko,cst6,cst3,latsin,n,t,c,a,lattan,latcos,temp5,temp6,temp11,x;
  double temp7,temp8,temp9,temp10,mo,latorg,m,cenmerrad;
  double majoraxis,minoraxis,ecc,ecc2,latrad,longrad;
  double latd, latm, lats, longd, longm, longs;
  
  parsedeg(dec_to_dms(lat), &latd, &latm, &lats); 
  parsedeg(dec_to_dms(lon), &longd, &longm, &longs);
  latrad = (latd + latm / 60.0 + lats / 3600.0) * DEG_TO_RAD;
  longrad = (longd + longm / 60.0 + longs / 3600.0) * DEG_TO_RAD;

  ko = 0.9996;
  cst6 = 6.0 * DEG_TO_RAD;  /* zone width (6 deg) in radians */
  cst3 = 3.0 * DEG_TO_RAD;  /* zone edge to CM (3 deg) in radians */
  majoraxis = 6378206.4;
  minoraxis = 6356583.8;
  ecc = ((majoraxis * majoraxis) - (minoraxis * minoraxis)) / (majoraxis * majoraxis);
  ecc2 = ecc / (1.0 - ecc);

  /* Compute the Central Meridian (CenMerRad#) in radians */
  if (longrad>=0.0) {
    /* east Longitude */
    cenmerrad=floor(longrad/cst6);
    cenmerrad = cenmerrad * cst6 + cst3;
    /* cenmerdeg = floor(fabs(cenmerrad * RAD_TO_DEG));
     * sprintf(utmzone,"%.0f",(61.0-(183.0-cenmerdeg))/6.0);
     */
  }
  else {
    /* West Longitude */
    cenmerrad = floor(longrad/cst6);
    cenmerrad = cenmerrad * cst6 - cst3;
    /* cenmerdeg = floor(fabs(cenmerrad*RAD_TO_DEG));   
     * sprintf(utmzone,"%.0f",(183.0-cenmerdeg)/6.0);
     */
  }

  latsin = sin(latrad);
  n = majoraxis / sqrt(1.0 - ecc * latsin * latsin);
  lattan = tan(latrad);
  t = lattan * lattan;
  latcos = cos(latrad);
  c = ecc2 * latcos * latcos;
  a = latcos * (longrad - cenmerrad);
  meridiandist(latrad,&m);
  temp5 = 1.0 - t + c;
  temp6 = 5.0 - 18.0 * t + t * t + 72.0 * c - 58.0 * ecc2;
  temp11 = a * a * a * a * a;
  x = ko * n * (a + (temp5 * a * a * a) / 6.0 + (temp6 * temp11 / 120.0));
  *utmx = x + 500000.0;

  latorg = 0.0;
  meridiandist(latorg,&mo);
  temp10 = m - mo;
  temp7 = (5.0 - t + 9.0 * c + 4.0 * c * c) * ((a * a * a * a) / 24.0);
  temp8 = (61.0 - 58.0 * t + t * t + 600.0 * c - 330.0 * ecc2);
  temp9 = (temp11 * a) / 720.0;

  *utmy = ko * (temp10 + n * lattan * ((a * a) / 2.0 + temp7 + temp8 * temp9));
  /*
   * *scale = ko * (1.0 + (1.0 + ecc2 * latcos * latcos) * x * x / (2.0 * ko * ko * n * n));
   * c1 = -(sqrt(t));
   * c3 = (1.0 + 3.0 * c + 2.0 * c * c) / 3.0;
   * c5 = (2.0 - t) / 15.0;
   * *conrad = c1 * a * (1.0 + a * a * (c3 + c5 * a * a));
   * if (*conrad < 0.0) {
   * strcpy(csign,"-");
   * *conrad = fabs(*conrad);
   * }
   * else strcpy(csign,"+");
   */
  fprintf(stderr, "\nUTM coord (%lf, %lf)\n", *utmx, *utmy);
}

/**********************************************************************
 **        gt_center_of_measure        **
 ****************************************/

void gt_center_of_measure(gt_Measure *measure,
                          gt_Point *point)
{
    float x, y;
    int i;

    switch (measure->data_type) {

    case POINT:
        point->x = measure->data.point[0].x;
        point->y = measure->data.point[0].y;
        break;
      
    case POLYLINE:
    case REGION:
        x = 0.0;
        y = 0.0;
        for (i = 0; i < (int)(measure->data.num_points); i++) {
            x += measure->data.point[i].x;
            y += measure->data.point[i].y;
        }
        point->x = x / measure->data.num_points;
        point->y = y / measure->data.num_points;
        break;
      
    default:
        fprintf(stderr, "gt_center_of_measure: ");
        fprintf(stderr, "Error, data type %d not recognized\n", measure->type);
        point->x = 0.0;
        point->y = 0.0;
        break;
    }
}



/**********************************************************************
 **        gt_new_point_measure        **
 ****************************************/

gt_Measure *gt_new_point_measure(gt_Measure_type type,
				 float x, float y, float width)
{
   gt_Measure *measure = (gt_Measure *)malloc(sizeof(gt_Measure));

   /* Copy in the appropriate data */
   if (measure) {
      measure->type = type;
      measure->data_type = POINT;
      measure->data.num_points = 1;
      measure->data.point[0].x = x;
      measure->data.point[0].y = y;
      measure->draw_label = TRUE;
      measure->width = width;
      }
   else {
      fprintf(stderr, "Error: Ran of out memory creating %s POINT measure.",
	      gt_measure_type_name[type]);
      }

   return measure;
}



/**********************************************************************
 **     gt_new_multipoint_measure      **
 ****************************************/

gt_Measure *gt_new_multipoint_measure(gt_Measure_type type,
				      gt_Measure_data_type data_type,
				      float width,
				      gt_Point_list *points,
				      unsigned *size,
				      double height)
{
   gt_Measure *measure;
   gt_Point_list *pt;
   int num_points, i;

   /* first, figure out how many points we're dealing with */
   num_points = 0;
   pt = points;
   while (pt) {
      num_points++;
      pt = pt->next;
      }

   /* allocate the measure */
   *size = sizeof(gt_Measure) + (num_points-1)*sizeof(gt_Multipoint);
   if ((measure = (gt_Measure *)malloc(*size)) == NULL) {
      fprintf(stderr, "Error: Ran of out memory creating %s multipoint measure.",
	      gt_measure_type_name[type]);
      return NULL;
      }

   /* copy in the data */
   measure->type = type;
   measure->data_type = data_type;
   measure->draw_label = TRUE;
   measure->width = width;
   measure->data.num_points = num_points;
   pt = points;
   for (i=0; i<num_points; i++) {
      measure->data.point[i].x = pt->x;
      measure->data.point[i].y = pt->y;
      pt = pt->next;
      }

   return measure;
}




/**********************************************************************
 * $Log: gt_measure.c,v $
 * Revision 1.1.1.1  2008/07/14 16:44:22  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/20 17:17:50  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:37:59  endo
 * MissionLab 7.0
 *
 * Revision 1.3  2006/05/15 01:50:11  endo
 * gcc-3.4 upgrade
 *
 * Revision 1.2  2005/03/23 07:36:55  pulam
 * *** empty log message ***
 *
 * Revision 1.1.1.1  2005/02/06 23:00:10  endo
 * AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
 *
 * Revision 1.15  1999/08/30 21:43:38  jdiaz
 * added conversion routines for utm
 *
 * Revision 1.14  1997/02/12 05:42:50  zchen
 * *** empty log message ***
 *
 * Revision 1.13  1995/04/03  20:24:57  jmc
 * Added copyright notice.
 *
 * Revision 1.12  1995/02/14  22:16:23  jmc
 * Added initialization of new draw_label field in measure creation
 * routines.
 *
 * Revision 1.11  1994/10/31  16:34:22  jmc
 * Changed warn_userf message for out-of-memory conditions to
 * fprintf(stderr,...) commands since it is unlikely that warn_userf
 * would work with no memory.
 *
 * Revision 1.10  1994/09/02  20:01:19  jmc
 * Fixed a minor #include ordering problem.
 *
 * Revision 1.9  1994/08/04  18:54:22  jmc
 * Modified gt_new_point_measure and gt_new_multipoint_measure to
 * deal with the new component, width.  Switched to warn_userf.
 *
 * Revision 1.8  1994/08/04  14:18:48  jmc
 * Modified includes because prototypes for warn_user... were
 * moved to console.h.
 *
 * Revision 1.7  1994/07/26  16:11:56  jmc
 * Changed explicit "extern void warn_user" to include "gt_sim.h".
 *
 * Revision 1.6  1994/07/26  15:50:53  jmc
 * Converted error messages to use error dialog box.
 *
 * Revision 1.5  1994/07/19  16:59:42  jmc
 * Added #define GT_MEASURE_C to allow header file to compile strings into
 * the object file for this file and not make copies everywhere.  Put definitions
 * of strings back into gt_measure.h.
 *
 * Revision 1.4  1994/07/19  16:31:58  jmc
 * Moved char *gt_measure_type_name[] from the header file to here
 * to avoid duplication in object modules.
 *
 * Revision 1.3  1994/07/12  19:26:14  jmc
 * Tweaked RCS automatic identification strings
 *
 * Revision 1.2  1994/07/12  17:57:06  jmc
 * Added RCS automatic identification strings
 **********************************************************************/
