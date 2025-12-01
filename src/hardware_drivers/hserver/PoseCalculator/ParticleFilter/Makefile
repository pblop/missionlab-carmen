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

TARGET = libpfilter.a

SRCS = com.c linpack.c matt_lib.c pfilter.c randlib.c random_numbers.c	

INCLUDES = -I. -I../../

LDLIBS =

GPP = g++

GPPFLAGS = -Wall -Wno-unused -Wno-deprecated -g -DLINUX_OS -ansi -pedantic

OBJS = $(SRCS:c=o)

RM = rm -f -v

AR = ar rcv

RANLIB = ranlib

PTHREAD_DEFS = -D_POSIX_THREADS -D_POSIX_THREAD_SAFE_FUNCTIONS -D_REENTRANT 

# ----------------------------------------------------------------------
#                        COMPILATION RULES
# ----------------------------------------------------------------------

all:
	$(MAKE) $(TARGET)

.c.o:
	$(GPP) -c $(GPPFLAGS) $(INCLUDES) $(PTHREAD_DEFS) $*.c

$(TARGET): $(OBJS)
	$(RM) $(TARGET)
	$(AR) $(TARGET) $(OBJS)
	$(RANLIB) $(TARGET)

clean:
	@ $(RM) *.o core.* *~ 
	@ $(RM) $(TARGET)

veryclean: clean

depend:




