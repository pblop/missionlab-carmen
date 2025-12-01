# ----------------------------------------------------------------------
#
#  Makefile
#
#  Written by:  Yoichiro Endo
#
# Copyright 2005 - 2006, Georgia Tech Research Corporation 
# Atlanta, Georgia  30332-0415
# ALL RIGHTS RESERVED, See file COPYRIGHT for details. 
#
# $Id: Makefile,v 1.1.1.1 2008/07/14 16:44:24 endo Exp $
# ----------------------------------------------------------------------

# ----------------------------------------------------------------------
#                           FLAGS ETC.
# ----------------------------------------------------------------------

include ../make.include

TARGET = libmlab3d.a

GPP = g++

RM = rm -f -v

AR = ar rcv

RANLIB = ranlib

CFLAGS = -g -Wall -ansi -pedantic

INCS =  -I$(MLAB_HOME)/src/include -I/usr/include/openmotif/

SRCS = 3d_building.cpp \
	3d_obstacle.cpp \
	3drobot.cpp \
	obst_handler.cpp \
	pmouse.cpp \
	robothandler.cpp \
	model_3ds.cpp \
	terrainmap.cpp

OBJS=$(SRCS:cpp=o)

# ----------------------------------------------------------------------
#                        COMPILATION RULES
# ----------------------------------------------------------------------

.cpp.o:
	$(GPP) $(CFLAGS) $(INCS) -c $*.cpp

all: $(TARGET)

$(TARGET): $(OBJS)
	@ $(RM) $(TARGET) 
	@ $(AR) $(TARGET) $(OBJS)
	@ $(RANLIB) $(TARGET)

clean:
	rm -fv *.o *~

veryclean: clean
	$(RM) $(TARGET)

depend:

nodepend:
	

