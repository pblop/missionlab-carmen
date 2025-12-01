# ----------------------------------------------------------------------
# Makefile
#
# Written by Yoichir Endo
#
# Copyright 2004, Georgia Tech Research Corporation 
# Atlanta, Georgia  30332-0415
# ALL RIGHTS RESERVED, See file COPYRIGHT for details. 
#
# $Id: Makefile,v 1.1.1.1 2006/07/12 13:37:57 endo Exp $
# ----------------------------------------------------------------------

# ----------------------------------------------------------------------
#                           FLAGS ETC.
# ----------------------------------------------------------------------

include ../../../make.include

TARGET = libposecalc.a

SRCS = Kalman.c MaximumConfidence.c ParticleFilter.c PCCoordinatorPickFilter.c \
       PCModuleFilters.c PCModules.c PCSensorFuser.c PCSensoryDataBus.c \
       PCSituationalContextEnvironment.c PCSituationalContextSensory.c \
       PCUtility.c PoseCalculator.c

INCLUDES = -I. -I../ -I$(MLAB_HOME)/src/include -I./Kalman -I./ParticleFilter -I/usr/include/ncurses

GPP = g++

GPPFLAGS = -Wall -g -DLINUX_OS -ansi -pedantic

#Activate data log
#GPPFLAGS += -DLOG_POSE_DATA

OBJS = $(SRCS:c=o)

RM = rm -f -v

AR = ar rcv

RANLIB = ranlib

PTHREAD_DEFS = -D_POSIX_THREADS -D_POSIX_THREAD_SAFE_FUNCTIONS -D_REENTRANT 

# ----------------------------------------------------------------------
#                        COMPILATION RULES
# ----------------------------------------------------------------------

all:
	$(MAKE) kalman
	$(MAKE) pfilter
	$(MAKE) $(TARGET)

.c.o:
	$(GPP) -c $(GPPFLAGS) $(INCLUDES) $(PTHREAD_DEFS) $*.c

$(TARGET): $(OBJS)
	$(RM) $(TARGET)
	$(AR) $(TARGET) $(OBJS)
	$(RANLIB) $(TARGET)

kalman:
	$(MAKE) all -C ./Kalman

pfilter:
	$(MAKE) all -C ./ParticleFilter

clean:
	$(MAKE) clean -C ./Kalman
	$(MAKE) clean -C ./ParticleFilter
	@ $(RM) *.o core.* *~ 
	@ $(RM) $(TARGET)

veryclean: clean

depend:




