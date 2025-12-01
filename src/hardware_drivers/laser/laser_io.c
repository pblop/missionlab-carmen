#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "data_structures.h"
#include "laser_io.h"

extern int giCurrentFrame;  /* Number assigned to each laser reading */




void writeDataToFile(FILE *fp, rawLaserScan_t *rawScan)
{
  int i;
  fprintf(fp, "%f %f %f %ld %ld ", rawScan->x, 
	  rawScan->y, rawScan->t, rawScan->tstamp.tv_sec, rawScan->tstamp.tv_usec);
  for (i = 0; i < NSCANS; i++) {
    fprintf(fp, "%u ", rawScan->dist[i]);
  }
  fprintf(fp, "\n");
}
void writeFixedDataToFile(FILE *fp, laserScan_t *lscan, double x, double t)
{
  int i;

  fprintf(fp, "%f %f ", x, t);
  for (i = 0; i < NSCANS; i++) {
    fprintf(fp, "%f %f", lscan->x[i], lscan->y[i]);
  }
  fprintf(fp, "\n");
}



int writeDataToBinaryFile(FILE *fp, rawLaserScan_t *rawScan)
{
  /* Save the timestap information */
  if(!fwrite(rawScan, sizeof(rawLaserScan_t), 1, fp))  return FAILURE;         
  return SUCCESS;    
}


int readSeekRawLaserDataFromBinaryFile(FILE *fp, int offset, int isGlobal, rawLaserScan_t *rawScan)
{
  int new_frame;
  if (fp == NULL)    return FAILURE;
  
  
  if(isGlobal==GLOBAL_PLAY_MODE) {
	if( fseek(fp, offset*sizeof(rawLaserScan_t), SEEK_SET) == -1) return FAILURE;
	new_frame = offset;
  }
  else {
	if(fseek(fp, offset*sizeof(rawLaserScan_t), SEEK_CUR) == -1)  return FAILURE;

	new_frame = giCurrentFrame + (offset+1);
  }
  
  
  /* Read the timestap information */
  if(!fread(rawScan, sizeof(rawLaserScan_t), 1, fp))  return FAILURE;    
  
  giCurrentFrame = new_frame;
  //printf("Current Record = %d\n", giCurrentFrame);
  return SUCCESS;
}



/* Function reads one laser reading from an ASCII file. The format
   here assumes one reading per line in unsigned int notation */
int readRawLaserDataFromAsciiFile(FILE *fp, rawLaserScan_t *rawScan)
{
  
  char buf[32840], *s;
  int i;
  static int num_records = 0;

  if (fp == NULL)
  {
    return FAILURE;
  }

  s = fgets(buf, 4095, fp);
  
  if (!s) {
    fprintf(stdout, "\nnum readings %d\n", num_records);
    return FAILURE;
  }

  buf[strlen(buf) - 1] = '\0';


  if (!(s = strtok(buf, " "))) return FAILURE;
  rawScan->x = atof(s); 
  if (!(s = strtok(NULL, " "))) return FAILURE;
  rawScan->y = atof(s); 
  if (!(s = strtok(NULL, " "))) return FAILURE;
  rawScan->t = atof(s); 
  if (!(s = strtok(NULL, " "))) return FAILURE;
  rawScan->tstamp.tv_sec = atol(s); 
  if (!(s = strtok(NULL, " "))) return FAILURE;
  rawScan->tstamp.tv_usec = atol(s); 

  i = 0;
  while ((s = strtok(NULL, " "))) {
    rawScan->dist[i] = (unsigned int) atoi(s);
    if (rawScan->dist[i] > MAX_VALID_READING) {
      rawScan->dist[i] = MAX_VALID_READING;
    } 
    i++;
  }

  num_records++;
  
  return SUCCESS;
}
