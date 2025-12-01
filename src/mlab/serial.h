/**********************************************************************
 **                                                                  **
 **                            serial.h                              **
 **                                                                  **
 **  Written by:  Jonathan M. Cameron                                **
 **                                                                  **
 **  Copyright 1995, 1996, 1997 Georgia Tech Research Corporation    **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

#ifndef SERIAL_INC
#define SERIAL_INC
/*----------------------------------------------------------------------------

	serial.h

	Functions to open, close, read, and write to serial ports.

----------------------------------------------------------------------------*/


extern int  read_chr(int serial_port, unsigned char *chr, int time_out);
	/* reads a character from the serial line.  returns 0 if successful. */

extern int  read_line(int serial_port, unsigned char line[], 
                             int bufsize);
	/* reads characters until timeout or bufsize characters have been
  	   read.  returns number of characters read. */

extern int  write_chr(int serial_port, unsigned char chr);
	/* write a character to the serial line.  returns 0 if successful. */

extern int  write_line(int serial_port, unsigned char line[], 
                              int length);
	/* write a message to the serial line.  returns 0 if successful. */

extern int  flush_to(int *fildes, char chr);
	/* read and discard characters until 'chr' is read.  returns 0
	   if character is found.  returns 1 if a timeout occurs. */

extern void  find_first_non(int *fildes, char skip, char *next);
	/* read and discard and 'chr' characters */

extern int  open_serial(int ttynum, int baud);
	/* open a serial link on ttynum with 8 bits, no parity, 1 stop bit,
	   and given baud rate.  returns id of serial link */

extern int  close_serial(int  serial_port);
	/* close the serial link */

#endif
