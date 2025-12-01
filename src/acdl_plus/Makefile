# ----------------------------------------------------------------------
# acdl_plus makefile
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

TARGET = libacdl_plus.a

SRCS = acdl_plus.cc

INCLUDES = -I. -I$(MLAB_HOME)/src/include -I../load_cdl -I../write_cdl -I../cfgedit -I../MPPC -I../cmdli/MLab_agent/include/cmdli -I/usr/include/openmotif

LDLIBS = -L$(MLAB_HOME)/lib -lwrite_cdl

GPP = g++

GPPFLAGS = -Wall -g -ansi -pedantic

OBJS = $(SRCS:cc=o)

RM = rm -f -v

AR = ar rcv

RANLIB = ranlib

#PTHREAD_DEFS = -D_POSIX_THREADS -D_POSIX_THREAD_SAFE_FUNCTIONS -D_REENTRANT 
PTHREAD_DEFS =

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
	