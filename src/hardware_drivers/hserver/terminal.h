/**********************************************************************
 **                                                                  **
 **                              terminal.h                          **
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

/* $Id: terminal.h,v 1.1.1.1 2006/07/12 13:37:56 endo Exp $ */
 
#ifndef TERMINAL_H
#define TERMINAL_H

#include <curses.h>
#include <panel.h>
#include <pthread.h>

#define ASCII_NUL   0x00
#define ASCII_SOH   0x01
#define ASCII_STX   0x02
#define ASCII_ETX   0x03
#define ASCII_EOT   0x04
#define ASCII_ENQ   0x05
#define ASCII_ACK   0x06
#define ASCII_BEL   0x07
#define ASCII_BS    0x08
#define ASCII_HT    0x09
#define ASCII_LF    0x0A
#define ASCII_VT    0x0B
#define ASCII_FF    0x0C
#define ASCII_CR    0x0D
#define ASCII_SO    0x0E
#define ASCII_SI    0x0F
#define ASCII_DLE   0x10
#define ASCII_DC1   0x11
#define ASCII_DC2   0x12
#define ASCII_DC3   0x13
#define ASCII_DC4   0x14
#define ASCII_NAK   0x15
#define ASCII_SYN   0x16
#define ASCII_ETB   0x17
#define ASCII_CAN   0x18
#define ASCII_EM    0x19
#define ASCII_SUB   0x1A
#define ASCII_ESC   0x1B
#define ASCII_FS    0x1C
#define ASCII_GS    0x1D
#define ASCII_RS    0x1E
#define ASCII_US    0x1F
#define ASCII_SP    0x20

#define ASCII_START 0x20

#define TERMINAL_BUF_SIZE 1024

#define TERM_COMMAND_EXIT 0x01

#define TERM_FILTER_LF   0x01
#define TERM_FILTER_CHAR   0x02
#define TERM_FILTER_SKIP   0x03
#define TERM_FILTER_SKIP2  0x04
#define TERM_FILTER_BAD    0x05

#define TERM_X                5
#define TERM_Y                5
#define TERM_COL              80
#define TERM_ROW              50

int terminalClose();
int terminalOpen(char *port,int baud_selection,int *fd);
void terminalInitialize();
int terminalRead(int fd);
void terminalControl(int fd);
void terminalDisconnect();
void terminalPrint(char *str,int x,int y,bool scroll=true);
void terminalDraw();
void terminalShow();
int terminalConnect(int terminal_port,int baud);
void terminalReconnect();

extern char terminal_baud_str[10];
extern char terminal_status_str[TERM_COL-2];

extern int terminal_fd;

extern WINDOW *termWindow,*termBordWindow;
extern PANEL *termPanel,*termBordPanel;
extern bool terminalConnected,useTerminal,displayTerm;
extern pthread_t terminal_reader;

extern int terminalTextLine;

extern char termStatus[100];

#endif

/**********************************************************************
# $Log: terminal.h,v $
# Revision 1.1.1.1  2006/07/12 13:37:56  endo
# MissionLab 7.0
#
# Revision 1.2  2006/06/05 21:35:52  endo
# PoseCalculator from MARA 2020 migrated
#
# Revision 1.1.1.1  2005/02/06 22:59:42  endo
# AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
#
# Revision 1.3  2000/09/19 03:51:29  endo
# RCS log added.
#
#
#**********************************************************************/
