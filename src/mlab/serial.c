/**********************************************************************
 **                                                                  **
 **                            serial.c                              **
 **                                                                  **
 **     Functions to open, close, read, and write to serial ports.   ** 
 **                                                                  **
 **  Written by:  Jonathan M. Cameron                                **
 **                                                                  **
 **  Copyright 1995, 1996, 1997 Georgia Tech Research Corporation    **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <sys/termio.h>
#include <sys/time.h>
#include <ctype.h>
#include <memory.h>
#include "serial.h"

#ifdef DEC
#define TTYLINE "/dev/tty00"
#else
#define TTYLINE "/dev/ttya"
#endif

static int      ECHO_TTY = 0;
static int      LOG_TTY = 0;
static int      ECHO_BINARY = 0;
static int      FIRST_TIME_OUT = 1;
static int      REST_TIME_OUT = 1;
static unsigned WRITE_DELAY = 0;

static FILE    *tty_log;



/*----------------------------------------------------------------------------
  
        read_chr: read a character from the serial line 
  
----------------------------------------------------------------------------*/
  
int  read_chr(int serial_port, unsigned char *chr, int time_out)
{
   int             count;
   int             rtn;

   for (count = 0; count < time_out; count++)
   {
      if ((rtn = read(serial_port, (char *)chr, 1)) == 1 && *chr != '\0')
      {
         if (ECHO_TTY)
         {
            if (ECHO_BINARY)
               fprintf(stderr, "%2.2X ", *chr);
            else if (isprint(*chr))
               putc(*chr, stderr);
            else
               putc('.', stderr); 

            if (LOG_TTY)
            {   
               if (ECHO_BINARY)
                  fprintf(tty_log, "%2.2X ", *chr);
               else if (isprint(*chr))
                  putc(*chr, tty_log);
               else
                  putc('.', tty_log);
            }
         }

         return 0;
      } 
      else if (rtn < 0)
      {
         perror("read_chr:read_chr - error on read of serial port");
         return 1;
      } 
   }

   return 1;   
}


/*----------------------------------------------------------------------------

        read_line: read characters from the serial line until timeout 
	or bufsize characters have been read.  then return.
         
----------------------------------------------------------------------------*/

int  read_line(int serial_port, unsigned char line[], int bufsize)
{     
   int             pos;
      

   if (ECHO_TTY)
   {  
      fputs("read>", stderr);
      if (LOG_TTY)
         fputs("read>", tty_log);
   }
      
   /* if got a character then try to read more */
   pos = 0;
   if (!read_chr(serial_port, &line[pos], FIRST_TIME_OUT))
   {
      for (pos = 1; pos < bufsize - 1; pos++)
      {
         /* if didn't get a character (timeout) then exit */
         if (read_chr(serial_port, &line[pos], REST_TIME_OUT))
            break;
      }  
   }
      
   /* stick a null on the end */
   line[pos] = '\0';

   if (ECHO_TTY)
   {  
      fputs("<\n", stderr);
      if (LOG_TTY)
         fputs("<\n", tty_log);
   }
   
   /* return the number of characters read */
   return pos;
}


/*----------------------------------------------------------------------------

        write_chr: write a character to the serial line 

----------------------------------------------------------------------------*/
        
int  write_chr(int serial_port, unsigned char chr)
{  
   /* echo if desired */
   if (ECHO_TTY)
   {
      if (ECHO_BINARY)
         fprintf(stderr, "%2.2X ", chr);
      else if (isprint(chr))
         putc(chr, stderr);
      else
         putc('.', stderr); 
         
      if (LOG_TTY)
      {     
         if (ECHO_BINARY)
            fprintf(tty_log, "%2.2X ", chr);
         else if (isprint(chr))
            putc(chr, tty_log);
         else  
            putc('.', tty_log);
      }     
   }
            
   /* write it */
   if (write(serial_port, (char *)&chr, 1) != 1)
   {              
      perror("write_chr:write_chr - error writing to arm");
      return 1;   
   }           
                  
   return 0; 
}        


/*----------------------------------------------------------------------------
      
        write_line: write a message to the serial line.
      
----------------------------------------------------------------------------*/
      
int  write_line(int serial_port, unsigned char line[], int length)
{     
   int             pos;
      
   if (ECHO_TTY)
   {
      fprintf(stderr, "write>");      
      if (LOG_TTY)
         fprintf(tty_log, "write>");
   }

   /* try to write the message */
   for (pos = 0; pos < length; pos++)
   {
      /* if get an error then exit */
      if (write_chr(serial_port, line[pos]))
         break;

      if (WRITE_DELAY && pos < length - 1)
      {
#ifdef DECstation
#else
         usleep(WRITE_DELAY);
#endif
      }
   }

   if (ECHO_TTY)
   {
      fputs("<\n", stderr);
      if (LOG_TTY)
         fputs("<\n", tty_log);
   }

   /* if was an error, then return error code */
   if (pos != length)
      return 1;

   return 0;
}


/*----------------------------------------------------------------------------

      flush_to: read and discard characters from the robot until 'chr' is
read
returns true if timeout occurs.

----------------------------------------------------------------------------*/

int  flush_to(int *fildes, char chr)
{
        char            buf;

        if (ECHO_TTY)
        {
                fprintf(stderr, "flushing to '%c' >", chr);
                if (LOG_TTY)
                        fprintf(tty_log, "flushing to '%c' >", chr);
        } 

        /* loop until timeout or get the character we are looking for */
         while (!read_chr(*fildes, &buf, 1))
        {
                if ((char) buf == chr)
                {
                        if (ECHO_TTY)
                        {
                                fputs("<\n", stderr);
                                if (LOG_TTY)
                                        fputs("<\n", tty_log);
                        }
                        return 0;
                }
        }       
        return 1;
}


/*----------------------------------------------------------------------------

        find_first_non: read and discard any 'chr' characters from the robot

----------------------------------------------------------------------------*/

void  find_first_non(int *fildes, char skip, char *next)
{
        if (ECHO_TTY)
        {
                fputs("read>", stderr);
                if (LOG_TTY)
                        fputs("read>", tty_log);
        }

        while (1)
        {
                if (!read_chr(*fildes, next, 1))
                {
                        if (*next != skip)
                        {
                                if (ECHO_TTY)
                                {
                                        fputs("<\n", stderr);
                                        if (LOG_TTY)
                                                fputs("<\n", tty_log);
                                }

                                return;
                        }
                }
        }
}



/*----------------------------------------------------------------------------

        open_serial: open a serial link on ttynum with 8 bits, no parity,
	1 stop bit, and given baud rate
	
----------------------------------------------------------------------------*/

int  open_serial(int ttynum, int baud)
{
   struct termio   termset;
   char            tty[80];
   char            msg[80];
   int		   serial_port;

  /* open the tty log if necessary */
  if (ECHO_TTY && LOG_TTY)
   {
    tty_log = fopen("tty.log", "w");
    if (tty_log == 0)
     {
      perror("Error opening tty.log");
      LOG_TTY = 0;
     }  
   }


   strcpy(tty, TTYLINE);
   tty[strlen(tty) - 1] += ttynum;
  
   /* open tty line */
   if ((serial_port = open(tty, O_RDWR)) < 0)
   {
      sprintf(msg, "open_compass - Error opening serial port >%s<", tty);
      perror(msg);
      return (-1);
   }
  
   /* zero array to start with */
   memset((char *) &termset, 0, sizeof(struct termio));
  
   /* set desired baud rate, 8 bits, no parity, 1 stop bit */
   termset.c_iflag = IGNBRK | IGNPAR;
   termset.c_oflag = 0;
   switch (baud)
    {
     case 300:
       termset.c_cflag = B300 | CS8 | CLOCAL | CREAD;
       break;
     case 600:
       termset.c_cflag = B600 | CS8 | CLOCAL | CREAD;
       break;
     case 1200:
       termset.c_cflag = B1200 | CS8 | CLOCAL | CREAD;
       break;
     case 1800:
       termset.c_cflag = B1800 | CS8 | CLOCAL | CREAD;
       break;
     case 2400:
       termset.c_cflag = B2400 | CS8 | CLOCAL | CREAD;
       break;
     case 4800:
       termset.c_cflag = B4800 | CS8 | CLOCAL | CREAD;
       break;
     case 9600:
       termset.c_cflag = B9600 | CS8 | CLOCAL | CREAD;
       break;
     case 19200:
       termset.c_cflag = B19200 | CS8 | CLOCAL | CREAD;
       break;
     case 38400:
       termset.c_cflag = B38400 | CS8 | CLOCAL | CREAD;
       break;
     default:
       sprintf(msg, "open_serial - Chosen baud rate not handled");
       break;
    }
   termset.c_cc[VMIN] = 0;
   termset.c_cc[VTIME] = 1;     /* 1/10 second timeout on reads */

   /* do the ioctl call to set the line up */
   if (ioctl(serial_port, TCSETA, (char *) &termset) < 0)
   {
      perror("\nrobio:open_compass - Error setting serial port for
compass");
      return (-1);
   }

   return serial_port;
}


/*----------------------------------------------------------------------------

        close serial: close the serial link 

----------------------------------------------------------------------------*/

int  close_serial(int  serial_port)
 {
  if (serial_port)
    close(serial_port);

  return 0;
 }

