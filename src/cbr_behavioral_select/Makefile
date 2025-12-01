# Makefile for cbr_behavioral_select
#
# Michael Kaess
#
# Copyright 2003 - 2006 Georgia Tech Research Corporation 
# Atlanta, Georgia  30332-0415
# ALL RIGHTS RESERVED, See file COPYRIGHT for details. 
#
# $Id: Makefile,v 1.1.1.1 2008/07/14 16:44:14 endo Exp $

# ----------------------------------------------------------------------
#                           FLAGS ETC.
# ----------------------------------------------------------------------

include ../make.include

TARGET = libcbr_behavioral_select.a

CC = g++

CFLAGS = -g -Wall -ansi -pedantic

CBR_HOME = $(MLAB_HOME)/src/cbr_behavioral_select
ROBOT_INC = $(MLAB_HOME)/src/hardware_drivers
INCLUDES = -I$(MLAB_HOME)/src/include -I$(CBR_HOME) -I$(ROBOT_INC)

CBR_INCLUDES = -I$(MLAB_HOME)/src/include -I$(CBRMLABDIR) -Imath

OBJS = Interface.o Case.o Controller.o Featurescomp.o GlobFuncs.o ShortTermMemory.o Utils.o cbr_behavioral_select.o
MOBJS = GaussianRandom.o

DEPEND_SRCS = Case.cpp Controller.cpp Featurescomp.cpp gasdev.cpp GlobFuncs.cpp Interface.cpp cbr_behavioral_select.cc nrutil.cpp ran1.cpp ShortTermMemory.cpp Utils.cpp

RM = rm -f -v

AR = ar rcv

RANLIB = ranlib

# ----------------------------------------------------------------------
#                        COMPILATION RULES
# ----------------------------------------------------------------------

all:
	$(MAKE) $(TARGET)

.cc.o:
	$(CC) -c $(CFLAGS) $(INCLUDES) $*.cc -o $*.o

.cpp.o:
	$(CC) -c $(CFLAGS) $(CBR_INCLUDES) $*.cpp -o $*.o

$(TARGET): $(OBJS) $(MOBJS)
	$(RM) $(TARGET)
	$(AR) $(TARGET) $(OBJS) $(MOBJS)
	$(RANLIB) $(TARGET)

clean:
	@ $(RM) *.o core *~ 

veryclean: clean
	@ $(RM) $(TARGET)

depend:
	

nodepend:
	