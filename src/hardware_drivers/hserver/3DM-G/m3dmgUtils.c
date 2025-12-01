/*--------------------------------------------------------------------------
 *
 * 3DM-G Interface Software
 *
 * (c) 2003 Microstrain, Inc.
 * All rights reserved.
 *
 * www.microstrain.com
 * 310 Hurricane Lane, Suite 4
 * Williston, VT 05495 USA
 * Tel: 802-862-6629
 * Fax: 802-863-4093
 *--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------
 * m3dmgUtils.c
 *
 * Miscellaneous untility functions used by the 3DM-G Adapter and interface.
 *--------------------------------------------------------------------------*/

#include <stdio.h>
#include "m3dmgUtils.h"
#include "m3dmgErrors.h"

/* error explanations */
char *ERROR_TEXT[] = {
    /* 0 */ "Undefined error",
    /* 1 */ "General failure",
    /* 2 */ "Invalid port number",
    /* 3 */ "Port write error",
    /* 4 */ "Port read error",
    /* 5 */ "Port read length error",
    /* 6 */ "Port read timout error",
    /* 7 */ "Checksum error",
    /* 8 */ "Invalid device Number",
    /* 9 */ "EEPROM data error",
    /* 10 */ "EEPROM address error",
    /* 11 */ "Unable to read gyro scale from EEPROM"
};

/*--------------------------------------------------------------------------
 * convert2int
 * Convert two adjacent bytes to an integer.
 *
 * parameters:  buffer : pointer to first of two buffer bytes.
 * returns:     the converted value.
 *--------------------------------------------------------------------------*/

int convert2int(char* buffer) {
	int x = (buffer[0]&LSB_MASK)*256 + (buffer[1]&LSB_MASK);
	return x;
}

/*--------------------------------------------------------------------------
 * convert2short
 * Convert two adjacent bytes to a short.
 *
 * parameters:  buffer : pointer to first of two buffer bytes.
 * returns:     the converted value.
 *--------------------------------------------------------------------------*/

short convert2short(char* buffer) {
	short x = (buffer[0]&LSB_MASK)*256 + (buffer[1]&LSB_MASK);
	return x;
}

/*--------------------------------------------------------------------------
 * calcChecksum
 * Calculate checksum on a received data buffer.
 *
 * Note: The last two bytes, which contain the received checksum,
 *       are not included in the calculation.
 *
 * parameters:  buffer : pointer to the start of the received buffer.
 *              length - the length (in chars) of the buffer.
 *
 * returns:     the calculated checksum.
 *--------------------------------------------------------------------------*/

int calcChecksum( char* buffer, int length) {
	int CHECKSUM_MASK = 0xFFFF;
	int checkSum, i;

	if (length<4)
		return -1;

	checkSum = buffer[0] & LSB_MASK;
	for (i=1; i<length-2; i = i+2) {
		checkSum += convert2int(&buffer[i]);
	}
	return(checkSum & CHECKSUM_MASK);
}

/*--------------------------------------------------------------------------
 * explainError
 * Provide a text explanation for an error code.
 *
 * parameters:  errornum : error number
 *
 * returns:     a string (char *) corresponding to the error number.
 *--------------------------------------------------------------------------*/

char * explainError(int errornum) {
    if (errornum < LAST_ERROR || errornum > 0)
        return ERROR_TEXT[0];
    else
        return ERROR_TEXT[-1*errornum];
}

/*-------------- end of m3dmgUtils.c ----------------------*/
