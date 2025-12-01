/**********************************************************************
 **                                                                  **
 **  environment.cc                                                  **
 **                                                                  **
 **  the main environment map file loader                            **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-1998.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Alto, Michigan, USA                                             **
 **                                                                  **
 **********************************************************************/

/* $Id: environment.cc,v 1.1.1.1 2008/07/14 16:44:16 endo Exp $ */

///////////////////////////////////////////////////////////////////////
// $Log: environment.cc,v $
// Revision 1.1.1.1  2008/07/14 16:44:16  endo
// MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
//
// Revision 1.1.1.1  2006/07/20 17:17:46  endo
// MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
//
// Revision 1.1.1.1  2006/07/12 13:37:54  endo
// MissionLab 7.0
//
// Revision 1.1  2006/07/01 00:21:03  endo
// CMDLi from MARS 2020 migrated into AO-FNC repository.
//
// Revision 1.1.1.1  2006/06/29 20:42:07  endo
// cmdli local repository.
//
// Revision 1.8  2004/11/12 21:58:54  doug
// fixed problem where offsets were not being inited to zero on creation, and sometimes would cause nan in Y value on PP locations
//
// Revision 1.7  2004/05/11 19:34:39  doug
// massive changes to integrate with USC and GaTech
//
// Revision 1.6  2004/03/08 14:52:28  doug
// cross compiles on visual C++
//
// Revision 1.5  2003/11/03 16:41:56  doug
// working on libcmdl
//
// Revision 1.4  2002/10/30 21:08:13  doug
// need to zero the width and height fields
//
// Revision 1.3  2002/09/20 13:05:19  doug
// *** empty log message ***
//
// Revision 1.2  1999/03/10 00:22:28  doug
// moved extra to here from env_object
//
// Revision 1.1  1999/03/08 23:37:42  doug
// Initial revision
//
//
///////////////////////////////////////////////////////////////////////

#include "mic.h"
#include "env_scanner.h"
#include "env_parser.h"
#include "environment.h"
#include <math.h>

namespace sara
{
/*********************************************************************/
// constructor
environment::environment() :
   origin_x(0),
   origin_y(0),
   origin_heading(0),
   width(0),
   height(0)
{
}

/*********************************************************************/
// Load the environment map
// Returns 0 if success
//        -1 if parse error, but some of the table is loaded
//        -2 if unable to open file
int 
environment::load_environment(const string &filename, 
                 bool no_path_search, 
					  bool verbose, 
                 bool debug)
{
   /************** initialize the parser ********************/
   theEnv = this;

   env_debug = debug;
   env_had_error = false;
   _env_filename = &filename;
   
   string fullname = filename;
   
#ifndef WIN32
   if( !no_path_search )
   {  
      if( !findfile(filename, fullname) )
         return(-2); 
   }
#endif

   env_in = fopen(fullname.c_str(),"r");
   if( env_in == NULL )
      return(-2); 
      
   if( verbose )
      INFORM("Loading environment file %s", fullname.c_str());

   // Pass on the verbose setting to the parser.
   env_verbose = verbose;

/************** load the input file ********************/

   env_parse();

/************** done ********************/

   fclose(env_in);

   if( env_had_error )
   {
      if( verbose )
         INFORM("\tDone, errors were encountered.");

      return -1;
   }

   if( verbose )
      INFORM("\tDone.");

   return 0;
}

// **********************************************************************
/// Add an object to the environment
bool 
environment::addObject(class env_object *obj)
{
   int index = objects.size();
   objects.push_back( obj );

   if( obj->is_labeled() )
   {
      strings strs = obj->get_label();
      if( strs.size() >= 1 )
      {
         int last = strs[0].length() - 1;
         if( last > 0 && strs[0][ last ] == '*' )
         {
            strs[0] = strs[0].substr(0, last);
            obj->set_Label( strs );
            obj->set_displayLabel(false);
         }
         objectsByName[ strs[0] ] = index;
      }
   }

   return true;
}

// **********************************************************************
/// Get a reference to the named object or return NULL if not found
/// The user must not delete the returned pointer
env_object *
environment::getObject(const string &name) const
{
   objectsByName_T::const_iterator it = objectsByName.find( name );
   if( it == objectsByName.end() )
   {
      return NULL;
   }

   env_object *obj = objects[ it->second ];
   if( !obj->is_labeled() )
   {
      ERROR("environment::getObject - objectsByName is out of sync with objects! Wrong index for '%s'", name.c_str());
      return NULL;
   }
   strings strs = obj->get_label();
   if( strs.size() != 1 || cmp_nocase(strs[0], name) != 0 )
   {
      ERROR("environment::getObject - objectsByName is out of sync with objects! Bad index for '%s'", name.c_str());
      return NULL;
   }

   return obj;
}

// **********************************************************************
// decimal to dd.mmsssssssss 
double 
environment::dec_to_dms(double dec) 
{
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

/**********************************************************************/
/* separate dd,mm,ss */
void 
environment::parsedeg(double dec_in,double *deg,double *min,double *sec) 
{
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

/**********************************************************************/
// distance from meridian 
void 
environment::meridiandist(double latrad, double *m) 
{
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

/**********************************************************************/
/// lat/lon latitude and longitude passed in as decimal degrees are converted to utm coordinates
void 
environment::convert_latlon_to_utm(double lat, double lon, double &utmx, double &utmy) 
{
  #define DEG_TO_RAD 0.017453293
  #define RAD_TO_DEG 57.29578

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
  if (longrad>=0.0) 
  {
    /* east Longitude */
    cenmerrad=floor(longrad/cst6);
    cenmerrad = cenmerrad * cst6 + cst3;
  }
  else 
  {
    /* West Longitude */
    cenmerrad = floor(longrad/cst6);
    cenmerrad = cenmerrad * cst6 - cst3;
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
  utmx = x + 500000.0;

  latorg = 0.0;
  meridiandist(latorg,&mo);
  temp10 = m - mo;
  temp7 = (5.0 - t + 9.0 * c + 4.0 * c * c) * ((a * a * a * a) / 24.0);
  temp8 = (61.0 - 58.0 * t + t * t + 600.0 * c - 330.0 * ecc2);
  temp9 = (temp11 * a) / 720.0;

  utmy = ko * (temp10 + n * lattan * ((a * a) / 2.0 + temp7 + temp8 * temp9));
}

/**********************************************************************/
}
