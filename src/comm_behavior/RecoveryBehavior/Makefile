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

TARGET = libRecoveryBehavior.a

SRCS = CRecovery.cpp CRecoveryBehaviors.cpp CWaypointstack.cpp

INCLUDES = -I. -I../

LDLIBS =

GPP = g++

GPPFLAGS = -Wall -g -ansi -pedantic

OBJS = $(SRCS:cpp=o)

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
	