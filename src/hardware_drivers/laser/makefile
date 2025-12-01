# ----------------------------------------------------------------------
# makefile
#
# Copyright 2005 - 2006, Georgia Tech Research Corporation 
# Atlanta, Georgia  30332-0415
# ALL RIGHTS RESERVED, See file COPYRIGHT for details. 
#
# $Id: makefile,v 1.1.1.1 2006/07/12 13:37:57 endo Exp $
# ----------------------------------------------------------------------

LN = ln -s
MV = mv
RM = -rm -rf
#CC = gcc -g -Wall
CC = g++
CFLAGS = -g -Wall -ansi -pedantic
INCS =
LIBS = -lm -lX11 -lXt -lpthread
LPATH = -L/usr/X11R6/lib

TARGETS = laserfit

SRCS =  main.c algorithm.c ezxwin.c tanfit.c laser_connection.c laser_protocol.c laser_io.c
OBJS =  $(SRCS:.c=.o)


LCOPTS = -fullwarn -wlint -woff 813,852,827,826,819 

.c.o:
	$(CC) $(CFLAGS) $(INCS) -c $*.c

all: $(TARGETS)

$(TARGETS):	$(OBJS) laser_protocol.h
	$(RM) $@
	$(CC) -o $@ $(CFLAGS) $(OBJS) $(LIBS) $(LPATH)

clean:
	$(RM) -f *.o *~ core a.out

veryclean: clean
	$(RM) -f $(TARGETS)

depend:	
	

