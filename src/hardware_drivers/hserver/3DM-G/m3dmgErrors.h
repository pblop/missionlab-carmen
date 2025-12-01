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
 * Tel: 802-862-6629 tel
 * Fax: 802-863-4093 fax
 *--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------
 * m3dmgErrors.h
 *
 * Definition of error codes.
 *--------------------------------------------------------------------------*/

#define M3D_COMM_OK 0 
#define M3D_OK 1

#define M3D_COMM_FAILED -1
#define M3D_COMM_INVALID_PORTNUM -2
#define M3D_COMM_WRITE_ERROR -3
#define M3D_COMM_READ_ERROR -4
#define M3D_COMM_RDLEN_ERROR -5
#define M3D_COMM_RDTIMEOUT_ERROR -6
#define M3D_CHECKSUM_ERROR -7
#define M3D_INVALID_DEVICENUM -8
#define M3D_EERPOM_DATA_ERROR -9
#define M3D_EERPOM_ADDR_ERROR -10
#define M3D_GYROSCALE_ERROR -11

#define LAST_ERROR -11    /* last error number. */

/*-------------- end of m3dmgErrors.h ----------------------*/
