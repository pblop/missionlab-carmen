#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <pthread.h>
#include <values.h>
#include "data_structures.h"
#include "ezxwin.h"
#include "tanfit.h"

#include "laser_protocol.h"
#include "laser_connection.h"
#include "laser_io.h"


extern char gsInputFile[128];
int REAL_LASER_READ = 1; 
int COLLECT_LASER_DATA_TO_FILE = 1;
int COLLECT_LASER_DATA_TO_FILE_MANUAL = 0;

#define MAX_DIST_TO_DOORWAY 500
FILE *gpDataOutFp, *gpDataInFp, *gpFixedDataFp;
int giHserverFd;


int giPlayMode= PLAY_FORWARD;
int giPlayRate=1;
int giCurrentFrame=-1;  /* Number assigned to each laser reading */
double gdAvgHwayAng = MAXDOUBLE;
double gdLearningRate = 0.06;
double gdLastTheta=  MAXDOUBLE;

/* Global structure holds the current hallway information */
laser_hallway_reply_t gpHwayInfo       = LASER_HALLWAY_REPLY_INITIALIZER;

/* Global structrues to hold the current doorway information for left and
   right sides of the hallway. Either holds the information of the closest doorway */

laser_doorway_reply_t gpDwayRightPos   = LASER_DOORWAY_REPLY_INITIALIZER;
laser_doorway_reply_t gpDwayLeftPos    = LASER_DOORWAY_REPLY_INITIALIZER;
laser_doorway_reply_t gpDwayEitherPos  = LASER_DOORWAY_REPLY_INITIALIZER;

/* Mutex locks for the global information */
pthread_mutex_t gHwayStructMutex       = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t gDwayRightStructMutex  = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t gDwayLeftStructMutex   = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t gDwayEitherStructMutex = PTHREAD_MUTEX_INITIALIZER;

BOOL gbDoWrite = FALSE;
double gdLastHwayAng = MAXDOUBLE;
int giNumRejects = 0;

void initAlg(void)
{
  time_t timep;
  char *s, *sp, fname[256];

  time(&timep);
  sp = s =ctime(&timep);
  sp[19] = '\0';
  do {
	if (*s == ' ') *s = '_';
  } while (*s++);
  
  sprintf(fname, "dataout_%s", sp+4);
  if (COLLECT_LASER_DATA_TO_FILE)
	{
	  gpDataOutFp = fopen(fname,"wb");
	  fprintf(stdout, "Collecting binary data to file: %s\n", fname);
	}
  if (COLLECT_LASER_DATA_TO_FILE_MANUAL) {
    gpDataOutFp = fopen(fname,"wb");
    gpFixedDataFp = fopen("fdataout.txt", "w");
	fprintf(stdout, "Collecting manual data to  files(%s  and fdataout.txt)\n", fname);
  }
  if (!REAL_LASER_READ)
	{
	  gpDataInFp = fopen(gsInputFile,"rb");
	  fprintf(stdout, "Using binary input file %s\n", gsInputFile);
	} 

}

double meanerror(double lineseg[4], double x[], double y[], int ndata);

/* Function calculates the mean perpendicular distance from each datapoint 
   to the closer of the two walls, which are either side of the y-axis */
double computeHallwayMeanError(double x[], double y[], int ndata, double posWallX)
{
  double rx[NSCANS], ry[NSCANS], lx[NSCANS], ly[NSCANS], lineseg[4];
  double err_r, err_l, terr = 0;
  int nr = 0, nl = 0, i;
  char buf[1024];
  
  //Sort the data about the Y-axis
  for (i = 0; i < ndata; i++) {
    if (x[i] < 0.0) {
      lx[nl] = x[i];
      ly[nl] = y[i];
      nl++;
    }
    else {
      rx[nr] = x[i];
      ry[nr] = y[i];
      nr++;
    }
  }
  // Calculate the mean error to the right wall
  lineseg[0] =  posWallX;
  lineseg[1] = -8000;
  lineseg[2] =  posWallX;
  lineseg[3] =  8000;
  err_r = meanerror(lineseg, rx, ry, nr);

  // Calculate the mean error to the left wall
  lineseg[0] = -posWallX;
  lineseg[1] = -8000;
  lineseg[2] = -posWallX;
  lineseg[3] =  8000;  
  err_l = meanerror(lineseg, lx, ly, nl);
 
  // Average the two errors
  terr = ((err_r*nr) + (err_l*nl));
  terr /= ndata;
  
  if (GRAPHICAL_DISPLAY) {
    sprintf(buf, "Hallway mean error: %f", terr);
    ezXWinDrawText(width/2, 0, buf);
  }
  return terr;
}

/* Find the distance to a door on the line 'x = hwayX'. Only the closest door
   is discovered. The distance to the center of the doorway is returned
   if a door is found. Otherwise a large distance is returned. This function also
   has the side effect of storing the doorway in the global structure */
double findDoorOnHallwaySide(double x[], double y[], int ndata, double hwayX,
			     double robotX, double hwayTheta, double tx, double ty, rawLaserScan_t *rawScan,
			     laser_doorway_reply_t *dway, pthread_mutex_t *dwayMutex)
{
  double sdx, sdy, edx, edy, pts[4], tdist = 1e9;
  lineseg2d_t ls;
  
  pthread_mutex_lock(dwayMutex);
  /* Run the matched filter to determine if there is a door */
  if (doorMatchedFilter(x, y, ndata, hwayX, robotX, hwayTheta, rawScan, &sdx, &sdy, &edx, &edy)) {
    ls.x1 = pts[0] = sdx;
    ls.x2 = pts[1] = edx;
    ls.y1 = pts[2] = sdy;
    ls.y2 = pts[3] = edy;
    if (GRAPHICAL_DISPLAY) {
      // Draw the doorway
      ezXWinDrawLineSegments(&ls, 1, EZ_XWIN_GREEN);
      ezXWinFillCircles(&pts[0], &pts[2], 1.0, 2, EZ_XWIN_PLUM);
    }
    /* tdist is the ditance from the robot at (robotX, 0) to the 
       midpoint of the door */
    tdist = sqrt((robotX - (sdx+edx)/2.0)*(robotX - (sdx+edx)/2.0) +
		 ((sdy+edy)/2.0)*((sdy+edy)/2.0));

    // If the distance to the door is within the tolerance fill in the global structure
    if (tdist < MAX_DIST_TO_DOORWAY) {
      dway->isdoor = 1;
      dway->pt1x = sdx;
      dway->pt1y = sdy;
      dway->pt2x = edx;
      dway->pt2y = edy;
      dway->time_stamp++;
      /* This transformation moves the door from aligned space back into 
	 robot-centric coordinates */
      rotateAndTranslateLaserScan(&dway->pt1x, &dway->pt1y, 1, -hwayTheta, tx, ty);
      rotateAndTranslateLaserScan(&dway->pt2x, &dway->pt2y, 1, -hwayTheta, tx, ty);
      if (GRAPHICAL_DISPLAY) {
	// Draw the two points
	ezXWinFillCircles(&dway->pt1x, &dway->pt1y, 4.0, 1, EZ_XWIN_PLUM);
	ezXWinFillCircles(&dway->pt2x, &dway->pt2y, 4.0, 1, EZ_XWIN_PLUM);
      }
    }
    else dway->isdoor = 0;
  } else dway->isdoor = 0;

  pthread_mutex_unlock(dwayMutex);
  /*Recall tdist is the distance to the midpoint of the door. If there
    is no door then tdist is equal to one billion centimeters */ 
  return tdist;
}

/* Convert the raw data expressed in polar coordinates to cartesian coordinates */
int convertRawLaserScan2CartesianCoords(rawLaserScan_t *rawScan,  laserScan_t *scan)
{
  int i;
  double  angle, ainc;

  ainc = M_PI/NSCANS;
  angle = -M_PI/2.0;
  for (i = 0; i < NSCANS; i++) {
    /* Covert to Carthesian Coordinates */
    scan->x[i] = rawScan->dist[i]*cos(angle); 
    scan->y[i] = rawScan->dist[i]*sin(angle);
    angle += ainc;
  }
  return SUCCESS;
}

#define MAX_HWAY_ANG_DELTA (M_PI/3.0)
#define MAX_REJECTS 8


/*** HANDLE THE PLAYBACK/FORWARD CAPABILITY OF THE LASER DATA WHEN READ FROM A FILE ***/
/*** giPlayRate and Mode are set in main.c                                          ***/
int handlePlayBack(rawLaserScan_t *rawScan)
{ 

 	switch(giPlayMode) 
	  {
	  case PLAY_FORWARD:
		if (!readSeekRawLaserDataFromBinaryFile(gpDataInFp, giPlayRate-1, 
                   RELATIVE_PLAY_MODE, rawScan)) return FAILURE;
		break;
	  case PLAY_SINGLE_FORWARD_FRAME:
		  if (!readSeekRawLaserDataFromBinaryFile(gpDataInFp, 0, RELATIVE_PLAY_MODE, rawScan))
			return FAILURE;
		  giPlayMode= PLAY_PAUSE;
		  break;
	  case PLAY_REVERSE:

		  if (!readSeekRawLaserDataFromBinaryFile(gpDataInFp, -(giPlayRate+1), 
				   RELATIVE_PLAY_MODE, rawScan))  return FAILURE;
		  break;
	  case PLAY_SINGLE_REVERSE_FRAME:
		  if (!readSeekRawLaserDataFromBinaryFile(gpDataInFp, -2, RELATIVE_PLAY_MODE, rawScan))
			return FAILURE;
		  giPlayMode= PLAY_PAUSE;
		  break;
	  case PLAY_PAUSE:
		//if (!readSeekRawLaserDataFromBinaryFile(gpDataInFp, -1, RELATIVE_PLAY_MODE, rawScan))
			return FAILURE;
		  break;

	  case PLAY_FROM_START:
		   if (!readSeekRawLaserDataFromBinaryFile(gpDataInFp, 0, GLOBAL_PLAY_MODE, rawScan))
			return FAILURE;
		   giPlayMode= PLAY_FORWARD;
		  break; 
	  default:
		printf("handlePlayBack: Unknown PlayMode %d! Exiting!\n",giPlayMode);
		exit(-1);
		break;
	  }
	return SUCCESS;
}

void mainAlg()
{
  tanfitRec_t sv;
  double phi, tx, ty, y, hwidth;
  double robotOffestFromCenter;
  double rdoor_dist, ldoor_dist, perc;
  rawLaserScan_t rawScan;
  laserScan_t laserScan;
  lineseg2d_t centerln = {0, 0, 0, height}, robotHeading;
  lineseg2d_t xaxis = {-width/2.0, 0, width/2.0, 0};
  int otime;
  char buf[1024];
 
  


  // Here we gather a raw scan either from an ASCII file or the real laser.
  if (REAL_LASER_READ) {
    if (!getNextLaserReading(giHserverFd, &rawScan)) return;
	giCurrentFrame++;
  }
  else {
	if(!handlePlayBack(&rawScan)) return;
  }
 

  if (GRAPHICAL_DISPLAY)  
    ezXWinFillRectangle(0, 0, width, height, EZ_XWIN_WHITE);

  // If so desired here we perform data logging of the raw scan
  if (COLLECT_LASER_DATA_TO_FILE)
    writeDataToBinaryFile(gpDataOutFp, &rawScan);

  // We convert to cartesian coordinates
  convertRawLaserScan2CartesianCoords(&rawScan,  &laserScan);
  /* The angle of the hallway and the points that lie along it are
     stored in the sv structure */
  computeAllTanfits(laserScan.x, laserScan.y, NSCANS, &sv);
  winnerTakeAllTanfit(&sv, &phi);
  calculateCenterLineRho(&sv, &phi, &tx, &ty, &hwidth, &robotOffestFromCenter);

  if (gdLastHwayAng != MAXDOUBLE && fabs(phi - gdLastHwayAng) > MAX_HWAY_ANG_DELTA && giNumRejects < MAX_REJECTS) {
    giNumRejects++;
    return;
  }
  gdLastHwayAng = phi;
  giNumRejects = 0;

  //printf("phi %f\n", phi*180/M_PI);

  if (GRAPHICAL_DISPLAY)
    // Draw the raw data
    ezXWinFillCircles(laserScan.x, laserScan.y, 1, NSCANS, EZ_XWIN_BLUE);

  /* Translate the datapoints so that the hallway centerline passes through the origin.
     Then rotate so that the hallway centerline is vertical. The hallway centerline
     has now become the y-axis. */
  translateAndRotateLaserScan(laserScan.x, laserScan.y, NSCANS, -phi, -tx, -ty);
  if (GRAPHICAL_DISPLAY)
    // Draw the fixed data
    ezXWinFillCircles(laserScan.x, laserScan.y, 1, NSCANS, EZ_XWIN_BLACK);

  // Perform the same transformation to those points that fall along the hallway
  translateAndRotateLaserScan(sv.winningX, sv.winningY, sv.winningNum, -phi, -tx, -ty);
  if (GRAPHICAL_DISPLAY)
    // And draw them in red
    ezXWinFillCircles(sv.winningX, sv.winningY, 1, sv.winningNum, EZ_XWIN_RED);


  /* Find the distance to doors on both sides of the hallway. Only the closest door
     on each side is discovered. The distance to the center of the doorway is returned
     if a door is found. Otherwise a large distance is returned. This function also
     has the side effect of storing the doorway in the global structure */
  rdoor_dist = findDoorOnHallwaySide(laserScan.x, laserScan.y, NSCANS, hwidth/2.0,
				     robotOffestFromCenter, -phi, tx, ty, &rawScan,
				     &gpDwayRightPos, &gDwayRightStructMutex);
  ldoor_dist = findDoorOnHallwaySide(laserScan.x, laserScan.y, NSCANS, -hwidth/2.0,
				     robotOffestFromCenter, -phi, tx, ty, &rawScan,
				     &gpDwayLeftPos, &gDwayLeftStructMutex);
  
  y = 0;
  if (GRAPHICAL_DISPLAY) {
    // The robot lies along the x-axis, draw him
    ezXWinFillCircles(&robotOffestFromCenter, &y, 8, 1, EZ_XWIN_GREEN);
  // Draw the hallway centerline which is the y-axis  
    ezXWinDrawLineSegments(&centerln, 1, EZ_XWIN_RED);
    // Draw the x-axis
    ezXWinDrawLineSegments(&xaxis, 1, EZ_XWIN_RED);
    
    // Use the hallway angle to display the robot's heading
    robotHeading.x1 = robotOffestFromCenter;
    robotHeading.y1 = 0;
    robotHeading.x2 = cos(-phi)*20 + robotOffestFromCenter;
    robotHeading.y2 = sin(-phi)*20;
    ezXWinDrawLineSegments(&robotHeading, 1, EZ_XWIN_GREEN);
  }

  pthread_mutex_lock(&gHwayStructMutex);
  gpHwayInfo.width = hwidth;
  gpHwayInfo.forwardptx = 0;
  gpHwayInfo.forwardpty = 100000; 
  gpHwayInfo.backwardptx = 0;
  gpHwayInfo.backwardpty = -100000;
  gpHwayInfo.time_stamp++;
  /* The hallway centerline lies along the y-axis after we have stablized the data. The
     inverse transformation will bring the hallway endpoints back into ego-centric coordinates */
  rotateAndTranslateLaserScan(&gpHwayInfo.forwardptx, &gpHwayInfo.forwardpty, 1, phi, tx, ty);
  rotateAndTranslateLaserScan(&gpHwayInfo.backwardptx, &gpHwayInfo.backwardpty, 1, phi, tx, ty);
  pthread_mutex_unlock(&gHwayStructMutex);

  // Compute the mean error and display to the screen
  computeHallwayMeanError(laserScan.x, laserScan.y, NSCANS, hwidth/2.0);
  perc = (double)sv.winningNum/361.0;
  if (GRAPHICAL_DISPLAY){
    sprintf(buf, "Winning Percentage %f", perc);
    ezXWinDrawText(width/2, 25, buf);

	// Display the frame count
	sprintf(buf,"frame: %d", giCurrentFrame);
	ezXWinDrawText(width/2, 50, buf);
	
	sprintf(buf,"phi: %f", phi);
	ezXWinDrawText(width/2, 75, buf);

	sprintf(buf,"theta: %2.2f, x: %4.2f, y: %4.2f", rawScan.t, rawScan.x, rawScan.y);
	ezXWinDrawText(width/2, 100, buf);
	
  }


  pthread_mutex_lock(&gDwayRightStructMutex);
  pthread_mutex_lock(&gDwayLeftStructMutex);
  pthread_mutex_lock(&gDwayEitherStructMutex);

  // Fill gpDwayEitherPos with the closer of the two doorways
  if (rdoor_dist < ldoor_dist) {
    otime = gpDwayEitherPos.time_stamp;
    memcpy(&gpDwayEitherPos, &gpDwayRightPos, sizeof(laser_doorway_reply_t)); 
    gpDwayEitherPos.time_stamp = otime + 1;
  }
  else {
    otime = gpDwayEitherPos.time_stamp;
    memcpy(&gpDwayEitherPos, &gpDwayLeftPos, sizeof(laser_doorway_reply_t)); 
    gpDwayEitherPos.time_stamp = otime + 1;
  }
  /* perc is the hallway mean error if it is below acceptable levels
     then our doorway detection cannot provide reliable answers. */
  if (perc < 0.12) {
    gpDwayRightPos.isdoor = 0;
    gpDwayLeftPos.isdoor = 0;
    gpDwayEitherPos.isdoor = 0;
  }
  pthread_mutex_unlock(&gDwayRightStructMutex);
  pthread_mutex_unlock(&gDwayLeftStructMutex);
  pthread_mutex_unlock(&gDwayEitherStructMutex);
  

  /* Allow for manual data collection file. A mouse click sets 
     gbDoWrite to TRUE. Here we write and reset the flag */
  if (COLLECT_LASER_DATA_TO_FILE_MANUAL) {
    if (gbDoWrite) {
      writeDataToBinaryFile(gpDataOutFp, &rawScan);
      writeFixedDataToFile(gpFixedDataFp, &laserScan, robotOffestFromCenter, -phi);
      gbDoWrite = FALSE;
    }
  }
  
  //drawMap();
}  




