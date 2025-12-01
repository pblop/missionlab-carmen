/**********************************************************************
 **                                                                  **
 **                               clipstruct.h                          **
 **                                                                  **
 **                                                                  **
 **                                                                  **
 **                                                                  **
 **  Written by:                                                     **
 **                                                                  **
 **  Copyright 2000, Georgia Tech Research Corporation               **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **********************************************************************/

/* $Id: clipstruct.h,v 1.1.1.1 2008/07/14 16:44:24 endo Exp $ */

#ifndef CLIPSTRUCT_H
#define CLIPSTRUCT_H

/*--------------------------------------------------------------------------

			CLIPSTRUCT.H

		Clipboard header file

-------------------------------------------------------------------------*/

/* RMS global section stuff */

#define DEFAULT_FILE_NAME ".dat"
#define SONAR_SCANS 5		/* Maximum number of raw sonar readings */
#define ENCODER_READINGS 10	/* Maximum number of raw encoder readings */
#define MAX_SCANS 1024		/* Maximum number of scans for sonk-map
				 * reports */
#define MAX_BASES 16		/* Maximum number of bases for sonkmap
				 * reports */

/* ASCID descriptor for global section names */
struct ascid
{
   short int       length;
   short int       info;
   char           *pointer;
};

/*-------------------------- DATA STRUCTURES --------------------------------*/

struct vision_frame
{				/* for 256 by 256 video image */
   char            plane[256][256];
   long            time;	/* time stamp */
};

struct ultrasonic
{				/* for sonar data as it is received from
				 * range_read */
   int             sensor[24];
   long            time;
};

struct encoder_data
{				/* for encoder data as received from
				 * getxy,drivevel, etc. */
   int             x;
   int             y;
   int             theta;
   int             velocity;
   int             rot_velocity;
   long            time;
};


struct line_result
{				/* Line finder data - to be deteremined */
   /* data here */
   long            time;	/* time stamp of original image */
};

/*--------------------------- REPORT STRUCTS ------------------------------*/
struct sonar_reading
{				/* for CMU sonkmap stuff */
   int             x;		/* in 16ths of foot */
   int             y;		/* in 16ths of foot */
   int             sonar_theta;	/* NOT WHEEL THETA in 512 ths */
   int             range;	/* in 16ths of foot */
   int             base_number;
};

/* for use in sonar_reading struct above */
struct base_for_reading
{				/* for displacements */
   int             x;		/* Global coordinates in 16ths of feet */
   int             y;		/* Global coordinates in 16ths of feet */
   int             sensor_0_angle;	/* in 512ths */
};


/*----------------------- DECLARATION FOR CLIPBOARD --------------------------*/

struct clip_board_t
{
   /* Data Region */
   struct
   {
      struct vision_frame frame[5];
      int             last_frame;
   }               vision_data;
   struct
   {
      struct ultrasonic scan[SONAR_SCANS];
      int             last_scan;
   }               ultrasonic_data;
   struct
   {
      struct encoder_data mark[ENCODER_READINGS];
      int             last_mark;
   }               encoder_data;

   /* REPORT REGION */
   struct
   {
      struct line_result line_report[5];
      int             last_report;
   }               line_reports;
   struct
   {
      struct sonar_reading raw_sonar[MAX_SCANS];
      int             current_sonar;	/* maintain as circular buffer */
      struct base_for_reading base[MAX_BASES];
      int             current_base;
   }               sonkmap_report;
};

extern struct clip_board_t clipboard;


#endif

/**********************************************************************
 * $Log: clipstruct.h,v $
 * Revision 1.1.1.1  2008/07/14 16:44:24  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/20 17:17:51  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:38:00  endo
 * MissionLab 7.0
 *
 * Revision 1.2  2006/05/14 05:57:38  endo
 * gcc-3.4 upgrade
 *
 **********************************************************************/
