/**********************************************************************
 **                                                                  **
 **                              terminal.c                          **
 **                                                                  **
 **                                                                  **
 **  Written by:  William Halliburton                                **
 **                                                                  **
 **  Serial port terminal for HServer                                **
 **                                                                  **
 **  Copyright 2000 - 2006, Georgia Tech Research Corporation        **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: terminal.c,v 1.1.1.1 2006/07/12 13:37:56 endo Exp $ */
 
#include <termios.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <curses.h>
#include <panel.h>
#include <pthread.h>
#include <ctype.h>
#include "hserver.h"
#include "terminal.h"
#include "cognachrome.h"
#include "message.h"

WINDOW *termWindow,*termBordWindow;
PANEL *termPanel,*termBordPanel;

bool useTerminal,terminalConnected,displayTerm;

int terminalTextLine;

int termsizeX,termsizeY;

pthread_t terminal_reader;

int terminal_fd;


int terminalX,terminalY;

bool hex_mode = false;		/* Output in hex */
bool run_cog_mode = false;	/* Output cog stream */
bool log_mode = false;		/* Log to file */

bool echo_mode = true;

int cog_stream = 3;		/* Cognachrome stream */
int current_stream = 0;

char terminal_baud_str[10];
char terminal_status_str[TERM_COL-2];

char termStatus[100];

int term_total_recieve = 0;
int term_total_transmit = 0;

FILE *logfile;
bool logfile_open = false;
char logfilename[] = "./HServer-Terminal.log"; 

void setLogMode(bool log_mode){
  if (log_mode == true) {
    logfile = fopen(logfilename,"a");
    logfile_open = true;
    printTextWindow("Terminal logging started");
    return;
  } else {
    if (logfile_open) fclose(logfile);
    printTextWindow("Terminal logging stopped");
  }
}

void setStatusStr(){
    int s = 0;

    strcpy(terminal_status_str,"                              ");
    wmove(termBordWindow,termsizeY-2,1);
    waddstr(termBordWindow,terminal_status_str);
    if (hex_mode) {
	sprintf(&terminal_status_str[s],"hex ");
	s+=4;
    }
    if (run_cog_mode) {
	sprintf(&terminal_status_str[s],"cog[%2d] ",cog_stream);
	s+=9;
    }
    if (logfile_open) {
	sprintf(&terminal_status_str[s],"logging ");
	s+=8;
    }
    if (echo_mode){
	sprintf(&terminal_status_str[s],"echo ");
	s+=5;
    }
    wmove(termBordWindow,termsizeY-2,1);
    waddstr(termBordWindow,terminal_status_str);
    redrawWindows();
}

void terminalStatusBarUpdate(){
  static int c1 = 0;
  static int c2 = 0;
  char spin1,spin2;
  static int recieve = 0;
  static int transmit = 0;
  char buf[50];
  
  termStatus[0] = 0;

  strcat(termStatus,"Terminal: ");
  if (terminalConnected) strcat(termStatus,"Connected  baud: ");
  else {
    strcat(termStatus,"Not connected ");
      //printStatusLine(terminalTextLine,0,termStatus);
    return;
  }
  
  strcat(termStatus,terminal_baud_str);
  strcat(termStatus,"  ");

  if (recieve!=term_total_recieve) c1 = (c1+1)%4;
  switch(c1){
  case 0:
    spin1 = '-';
    break;
  case 1:
    spin1 = '\\';
    break;
  case 2:
    spin1 = '|';
    break;
  case 3:
    spin1 = '/';
    break;
  }
  if (transmit!=term_total_transmit) c2 = (c2+1)%4;
  switch(c2){
  case 0:
    spin2 = '-';
    break;
  case 1:
    spin2 = '\\';
    break;
  case 2:
    spin2 = '|';
    break;
  case 3:
    spin2 = '/';
    break;
  }
  recieve = term_total_recieve;;
  transmit = term_total_transmit;
  sprintf(buf," R %c %d  T %c %d         ",spin1,recieve,spin2,transmit);
  strcat(termStatus,buf);
    //printStatusLine(terminalTextLine,0,termStatus);
}

void terminalShow(){

  show_panel(termBordPanel);
  show_panel(termPanel);
  redrawWindows();
  terminalControl(terminal_fd);
  hide_panel(termPanel);
  hide_panel(termBordPanel);
  redrawWindows();
  move(0,0);
}

void terminalDraw(){
    pthread_cleanup_push((void(*)(void*))pthread_mutex_unlock, (void *) &screenMutex);
    pthread_mutex_lock(&screenMutex);
    wmove(termBordWindow,0,0);
    wborder(termBordWindow,0,0,0,0,0,0,0,0);
    wmove(termBordWindow,termsizeY-3,0);
    waddch(termBordWindow,ACS_LTEE);
    whline(termBordWindow,ACS_HLINE,termsizeX-2);
    wmove(termBordWindow,termsizeY-3,termsizeX-1);
    waddch(termBordWindow,ACS_RTEE);
    wmove(termBordWindow,termsizeY-2,1);
    waddstr(termBordWindow,terminal_status_str);
    pthread_cleanup_pop(1);
}

void terminalPrint(char *str, int x, int y, bool scroll)
{
    pthread_cleanup_push((void(*)(void*))pthread_mutex_unlock, (void *) &screenMutex);
    pthread_mutex_lock(&screenMutex);
    if ((x==1)&&(y==termsizeY-6)&&scroll) {
	scroll(termWindow);
    }
    wmove(termWindow,y,x);
    waddnstr(termWindow,str,termsizeX-x-1);
    pthread_cleanup_pop(1);
    if (displayTerm) {
	redrawWindows();
    }
    if (log_mode) fprintf(logfile,"%s",str);
}

void terminalDisconnect(){
  pthread_cancel(terminal_reader);
  pthread_join(terminal_reader,NULL);
  printTextWindow("Terminal disconnected");
  terminalConnected = false;
  useTerminal = false;
  displayTerm = false;
  hide_panel(termPanel);
  hide_panel(termBordPanel);
  redrawWindows();
  return;
}

void terminalInitialize(){
    termsizeX = screenX-TERM_X-2;
    termsizeY = screenY-TERM_Y-2;
    termBordWindow = newwin(termsizeY,termsizeX,TERM_Y,TERM_X);
    termBordPanel = new_panel(termBordWindow);
    termWindow = newwin(termsizeY-4,termsizeX-2,TERM_Y+1,TERM_X+1);
    termPanel = new_panel(termWindow);
    scrollok(termWindow,TRUE);
    hide_panel(termPanel);
    hide_panel(termBordPanel);
    terminalDraw();
}

int terminalOpen(char *port,int baud_selection,int *fd){
  long baud;
  int rtn;

  switch(baud_selection){
  case 1:
    baud = B9600;
    strcpy(terminal_baud_str,"9.6k");
    break;
  case 2:
    baud = B19200;
    strcpy(terminal_baud_str,"19.2k");
    break;
  case 3:
    baud = B38400;
    strcpy(terminal_baud_str,"38.4k");
    break;
  case 4:
    baud = B57600;
    strcpy(terminal_baud_str,"57.6k");
    break;
  case 5:
    baud = B115200;
    strcpy(terminal_baud_str,"115.2k");
    break;
  case 6:
    baud = B230400;
    strcpy(terminal_baud_str,"230.4k");
    break;
  default:
    printTextWindow("Terminal Error unknown baud.");
    return(-1);
    break;
  }
  
  rtn = setupSerial(fd,port,baud,HS_REP_LEV_ERR,"Terminal",1);
  
  if (rtn) return(-1);
  
  terminalX = 1;
  terminalY = 1;
  setStatusStr();
  redrawWindows();
  return(0);
}

int terminalFilter(byte_t *buf){ 
  if (run_cog_mode) {		/* Needs min 2 */
    if (buf[0] == COG_STREAM_BREAK) {
      current_stream = buf[1];
      return(TERM_FILTER_SKIP2);
    }
    if (current_stream != cog_stream) return(TERM_FILTER_SKIP);
    return(TERM_FILTER_CHAR);
  }
  
  if (isprint(buf[0])) return(TERM_FILTER_CHAR);

  switch(buf[0]){
  case ASCII_LF:
    return(TERM_FILTER_LF);
    break;
  default:
    return(TERM_FILTER_BAD);
    break;
  }
}
 
int terminalRead(int fd){
  byte_t buf[TERMINAL_BUF_SIZE];
  int cnum,i,j;
  char outstr[TERMINAL_BUF_SIZE];
  // ENDO - gcc 3.4
  //char c,sp;
  char c;
  int sp;
  int hex_size = 18;
  int fnum;

  cnum = read(fd,buf,TERMINAL_BUF_SIZE);
  if (cnum == -1) {
    return(0);
  }
  if (cnum == 0) return(0);
  buf[cnum]=0;
  if (hex_mode) {
    for(i=0;i<cnum;){
      sp = 0;
      for(j=0;j<hex_size;j++){
	if (i+j>cnum-1) sprintf(&outstr[sp],"   ");
	else sprintf(&outstr[sp],"%2x ",buf[i+j]);
	sp += 3;
      }
      for(j=0;j<hex_size;j++){
	if (i+j>cnum-1) sprintf(&outstr[sp]," ");
        else {
	  fnum = terminalFilter(&buf[i+j]);
	  if (fnum==TERM_FILTER_CHAR) c=buf[i+j];
	  if (fnum==TERM_FILTER_SKIP) c=' ';
	  if (fnum==TERM_FILTER_BAD)  c='^';
	  sprintf(&outstr[sp],"%c",c);
	  sp += 1;
	}
      }
      terminalX = 1;
      if (terminalY == termsizeY-6) {
      } else {
	terminalY += 1;
      }
      terminalPrint(outstr,terminalX,terminalY);
      i += hex_size;
    }
  } else {
    sp = 0;
    for(i=0;i<cnum;i++) {
      fnum = terminalFilter(&buf[i]);
      switch(fnum){
      case TERM_FILTER_LF:
	outstr[sp] = 0;
	terminalPrint(outstr,terminalX,terminalY);
	terminalX = 1;
	if (terminalY < termsizeY-5) terminalY += 1;
	if (log_mode) fprintf(logfile,"\n");
	sp = 0;
	break;
      case TERM_FILTER_CHAR:
	outstr[sp++] = buf[i];
	break;
      case TERM_FILTER_SKIP2:
	i++;
	break;
      }
    }
    outstr[sp] = 0;
    if (sp) {
      terminalPrint(outstr,terminalX,terminalY);
      terminalX += sp;
    }
  }
  return(cnum);
}

void readCogStream(){
  int c;
  int stream;
  char buf[2];

  c = messageGetChar(EnMessageFilterType_COG,EnMessageType_COG,EnMessageErrType_COG);
  if (c=='x') return;
  stream = c - '0';
  switch(stream){
  case 0:
    cog_stream = 0;
    break;
  case 1:
    cog_stream = 1;
    break;
  case 2:
    cog_stream = 2;
    break;
  case 3:
    cog_stream = 3;
    break;
  case 4:
    cog_stream = 28;
    break;
  case 5:
    cog_stream = 29;
    break;
  case 6:
    cog_stream = 30;
    break;
  }
  buf[0]=0xfe;
  buf[1]=cog_stream;
  write(terminal_fd,buf,2);
}
  
int calcSaphiraChksum(byte_t *ptr)
{
  int n;
  int c=0;
  n=*(ptr++);
  n-=2;
  while(n>1){
    c+=(*(ptr)<<8) | *(ptr+1);
    c=c&0xffff;
    n -= 2;
    ptr += 2;
  }
  if (n>0) c=c^(int)*(ptr++);
  return(c);
}

void sendSaphiraPacket(int fd,char *str,int len){
  byte_t pack[255];
  int i,chk;
#define SAPHIRA_VISIONIO_COM  0x64
#define SAPHIRA_VISIONIOpac   0x80
#define SAPHIRA_ARGSTR 0x2B


  pack[0] = 0xfa;
  pack[1] = 0xfb;
  pack[2] = 5+len;
  pack[3] = SAPHIRA_VISIONIO_COM;
  pack[4] = SAPHIRA_ARGSTR;
  pack[5] = len;
  for(i=0;i<len;i++){
    pack[6+i] = str[i];
  }
  chk = calcSaphiraChksum(&pack[2]);
  pack[6+len] = (chk&0x0000ff00)>>8;
  pack[7+len] = chk&0x000000ff;
  write(fd,pack,pack[2]+3);
}

int terminalCommand(int command,int fd){
    char ascii_esc = 0x1B;

    switch(command){
    case 'Q':
    case 'x':
	hide_panel(termPanel);
	hide_panel(termBordPanel);
	redrawWindows();
	return(TERM_COMMAND_EXIT);
	break;
    case 'd':
	terminalDisconnect();
	return(TERM_COMMAND_EXIT);
    case 'e':
	terminalX = 0;
	terminalY = 0;
	werase(termWindow);
	wrefresh(termWindow);
	break;
    case 'k':
	echo_mode = !echo_mode;
	break;
    case 'h':
	hex_mode = !hex_mode;
	setStatusStr();
	break;
    case 'C':
	run_cog_mode = !run_cog_mode;
	setStatusStr();
    case 'c':
	if (run_cog_mode) {
	    readCogStream();
	    setStatusStr();
	}
	break;
    case 'p':
	sendSaphiraPacket(terminal_fd,"q",1);
	break;
    case 'r':
	refreshScreen();
	break;
    case 'l':
	log_mode = !log_mode;
	setLogMode(log_mode);
	terminalStatusBarUpdate();
	break;
    case ASCII_ESC:
	write(fd,&ascii_esc,1);
	term_total_transmit++;
	terminalStatusBarUpdate();
	break;
    }
    return(0);
}

void terminalControl(int fd){
  int c;
  int done,rtn;
  char charStr[2];

  charStr[1] = 0;
  done = 0;

  curs_set(1);
  do {
    c = getch();
    switch(c){
    case ASCII_ESC:
      c = getch();
      rtn = terminalCommand(c,fd);
      if (rtn) return;
      break;
    default:
      write(fd,&c,1);
      if (echo_mode) {
	  charStr[0] = c;
	  terminalPrint(charStr,terminalX,terminalY);
	  terminalX++;
      }
      term_total_transmit++;
      terminalStatusBarUpdate();
      break;
    }
  } while (!done);
  curs_set(0);
  return;
}

void terminal_reader_thread(void){
  int rtn;
  while(1){
    rtn = terminalRead(terminal_fd);
    term_total_recieve += rtn;
    terminalStatusBarUpdate();
  }
}
  
int terminalConnect(int terminal_port,int baud){
  char terminal_port_str[16];
  sprintf(terminal_port_str,"/dev/ttyS%d",terminal_port);
  if (!terminalOpen(terminal_port_str,baud,&terminal_fd)) {
    terminalConnected = true;
    displayTerm = true;
  }
  else {
    terminalConnected = false;
    displayTerm = false;
    return(-1);
  }
  pthread_create(&terminal_reader,NULL,(void * (*)(void *))*terminal_reader_thread,NULL);  
  return(0);
}
  
void terminalReconnect(){
  int c;
  int port,baud,rtn;

  c = messageGetChar(EnMessageFilterType_DIGIT,EnMessageType_TERMINAL,EnMessageErrType_PORT);
  if (c=='x') return;
  port = c - '0';
  c = messageGetChar(EnMessageFilterType_BAUD,EnMessageType_TERMINAL_BAUD,EnMessageErrType_BAUD);
  if (c=='x') return;
  baud = c - '0';
  rtn = terminalConnect(port,baud);
  useTerminal = true;
  if (displayTerm) {
    werase(termWindow);
    show_panel(termBordPanel);
    show_panel(termPanel);
    redrawWindows();  
    terminalControl(terminal_fd);
    hide_panel(termPanel);
    hide_panel(termBordPanel);
    displayTerm = false;
  } 
  redrawWindows();
}

/**********************************************************************
# $Log: terminal.c,v $
# Revision 1.1.1.1  2006/07/12 13:37:56  endo
# MissionLab 7.0
#
# Revision 1.3  2006/06/05 21:35:52  endo
# PoseCalculator from MARA 2020 migrated
#
# Revision 1.2  2006/05/15 03:06:53  endo
# gcc-3.4 upgrade
#
# Revision 1.1.1.1  2005/02/06 22:59:42  endo
# AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
#
# Revision 1.5  2003/04/06 13:32:37  endo
# gcc 3.1.1
#
# Revision 1.4  2000/12/12 23:21:36  blee
# Changed #defines for the user interface to enumerated types.
#
# Revision 1.3  2000/09/19 03:51:29  endo
# RCS log added.
#
#
#**********************************************************************/
