/**********************************************************************
 **                                                                  **
 **  testEndian.c                                                    **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-2003.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Livonia, Michigan, USA                                          **
 **                                                                  **
 **********************************************************************/

/* $Id: testEndian.c,v 1.1.1.1 2008/07/14 16:44:17 endo Exp $ */

///////////////////////////////////////////////////////////////////////
// $Log: testEndian.c,v $
// Revision 1.1.1.1  2008/07/14 16:44:17  endo
// MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
//
// Revision 1.1.1.1  2006/07/20 17:17:46  endo
// MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
//
// Revision 1.1.1.1  2006/07/12 13:37:55  endo
// MissionLab 7.0
//
// Revision 1.1  2006/07/01 00:31:55  endo
// CMDLi from MARS 2020 migrated into AO-FNC repository.
//
// Revision 1.1.1.1  2006/06/29 20:42:07  endo
// cmdli local repository.
//
// Revision 1.1  2004/04/14 19:02:12  doug
// tests the endian stuff
//
///////////////////////////////////////////////////////////////////////

// ********************************************************************

/* From /usr/include/endian.h
   Definitions for byte order, according to significance of bytes,
   from low addresses to high addresses.  The value is what you get by
   putting '4' in the most significant byte, '3' in the second most
   significant byte, '2' in the second least significant byte, and '1'
   in the least significant byte, and then writing down one digit for
   each byte, starting with the byte at the lowest address at the left,
   and proceeding to the byte with the highest address at the right.  */

unsigned long testLong;
unsigned long MIC_BIG_ENDIAN = 0x04030201;
unsigned long MIC_LITTLE_ENDIAN = 0x01020304;

#include <stdio.h>
#include <endian.h>

// ********************************************************************
/// Constructor
int main(int argc, char **argv)
{ 
   unsigned char *p = (unsigned char *)&testLong;
   p[0] = 0x04;
   p[1] = 0x03;
   p[2] = 0x02;
   p[3] = 0x01;

fprintf(stderr,"testLong = %lx\n", testLong);
fprintf(stderr,"MIC_BIG_ENDIAN = %lx\n", MIC_BIG_ENDIAN);
fprintf(stderr,"MIC_LITTLE_ENDIAN = %lx\n", MIC_LITTLE_ENDIAN);
fprintf(stderr,"BYTE_ORDER = %d\n", BYTE_ORDER);
fprintf(stderr,"BIG_ENDIAN = %d\n", BIG_ENDIAN);
fprintf(stderr,"LITTLE_ENDIAN = %d\n", LITTLE_ENDIAN);
}

