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

TARGET = libkalman.a

SRCS = ekf_compasssensormodel.cpp ekf_gpsthetasensormodel.cpp \
       ekf_gyrosensormodel.cpp ekf_localize.cpp ekf_locateprocessmodel.cpp \
       ekf_xysensormodel.cpp extend_kalman_filter.cpp kalman_filter.cpp \
       kf_processmodel.cpp kf_sensormodel.cpp mxn_matrix.cpp

INCLUDES = -I. -I../../

LDLIBS =

GPP = g++

GPPFLAGS = -Wall -g -DLINUX_OS -ansi -pedantic

OBJS = $(SRCS:cpp=o)

RM = rm -f -v

AR = ar rcv

RANLIB = ranlib

PTHREAD_DEFS = -D_POSIX_THREADS -D_POSIX_THREAD_SAFE_FUNCTIONS -D_REENTRANT 

# ----------------------------------------------------------------------
#                        COMPILATION RULES
# ----------------------------------------------------------------------

all:
	$(MAKE) $(TARGET)

.cpp.o:
	$(GPP) -c $(GPPFLAGS) $(INCLUDES) $(PTHREAD_DEFS) $*.cpp

$(TARGET): $(OBJS)
	$(RM) $(TARGET)
	$(AR) $(TARGET) $(OBJS)
	$(RANLIB) $(TARGET)

clean:
	@ $(RM) *.o core.* *~ 
	@ $(RM) $(TARGET)

veryclean: clean

depend:
	



