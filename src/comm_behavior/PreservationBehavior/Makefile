# ----------------------------------------------------------------------
# Makefile
#
# Written by Yoichir Endo
#
# Copyright 2004 - 2006 Georgia Tech Research Corporation 
# Atlanta, Georgia  30332-0415
# ALL RIGHTS RESERVED, See file COPYRIGHT for details. 
#
# $Id: Makefile,v 1.1.1.1 2008/07/14 16:44:17 endo Exp $
# ----------------------------------------------------------------------

# ----------------------------------------------------------------------
#                           FLAGS ETC.
# ----------------------------------------------------------------------

TARGET = libPreservationBehavior.a

SRCS = ValueBasedCommPres.cpp vbcp.cpp
SRCS2 = lineutils.c

INCLUDES = -I. -I../ -I../../mlab -I../../include -I../../terrain -I/usr/include/openmotif

LDLIBS =

GPP = g++

GPPFLAGS = -Wall -g -ansi -pedantic

OBJS = $(SRCS:cpp=o)
OBJS2 = $(SRCS2:c=o)

RM = rm -f -v

AR = ar rcv

RANLIB = ranlib

# ----------------------------------------------------------------------
#                        COMPILATION RULES
# ----------------------------------------------------------------------

all:
	$(MAKE) $(TARGET)

.cpp.o:
	$(GPP) -c $(GPPFLAGS) $(INCLUDES) $*.cpp

.c.o:
	$(GPP) -c $(GPPFLAGS) $(INCLUDES) $*.c

$(TARGET): $(OBJS) $(OBJS2)
	$(RM) $(TARGET)
	$(AR) $(TARGET) $(OBJS) $(OBJS2)
	$(RANLIB) $(TARGET)

lineutils.o:
	$(GPP) $(GPPFLAGS) $(INCLUDES) -c ../../mlab/lineutils.c -o lineutils.o -DLINUX 

clean:
	@ $(RM) *.o core *~ 

veryclean: clean
	@ $(RM) $(TARGET)

depend:
	

nodepend:
	