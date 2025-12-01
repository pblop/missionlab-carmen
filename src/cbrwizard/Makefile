# ----------------------------------------------------------------------
# cbrclient makefile
#
# Written by Yoichir Endo
#
# Copyright 2006 Georgia Tech Research Corporation 
# Atlanta, Georgia  30332-0415
# ALL RIGHTS RESERVED, See file COPYRIGHT for details. 
#
# $Id: Makefile,v 1.1.1.1 2008/07/14 16:44:14 endo Exp $
# ----------------------------------------------------------------------

# ----------------------------------------------------------------------
#                           FLAGS ETC.
# ----------------------------------------------------------------------

include ../make.include

TARGET = libcbrwizard.a

SRCS = cbrwizard.cc

INCLUDES = -I. -I../. -I$(MLAB_HOME)/src/include $(XINCS) $(EXTRA_INCS) -I/usr/include/openmotif/

LDLIBS =

GPP = g++

GPPFLAGS = -Wall -g -ansi -pedantic

OBJS = $(SRCS:cc=o)

RM = rm -f -v

AR = ar rcv

RANLIB = ranlib

PTHREAD_DEFS = -D_POSIX_THREADS -D_POSIX_THREAD_SAFE_FUNCTIONS -D_REENTRANT 

# ----------------------------------------------------------------------
#                        COMPILATION RULES
# ----------------------------------------------------------------------

all:
	$(MAKE) $(TARGET)

.cc.o:
	$(GPP) -c $(GPPFLAGS) $(INCLUDES) $(PTHREAD_DEFS) $*.cc

$(TARGET): $(OBJS)
	$(RM) $(TARGET)
	$(AR) $(TARGET) $(OBJS)
	$(RANLIB) $(TARGET)

clean:
	@ $(RM) *.o core *~ 

veryclean: clean
	@ $(RM) $(TARGET)

depend:
	

nodepend:
	