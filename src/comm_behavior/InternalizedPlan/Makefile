# ----------------------------------------------------------------------
# Makefile
#
# Written by Yoichir Endo
#
# Copyright 2004 - 2006, Georgia Tech Research Corporation 
# Atlanta, Georgia  30332-0415
# ALL RIGHTS RESERVED, See file COPYRIGHT for details. 
#
# $Id: Makefile,v 1.1.1.1 2008/07/14 16:44:17 endo Exp $
# ----------------------------------------------------------------------

# ----------------------------------------------------------------------
#                           FLAGS ETC.
# ----------------------------------------------------------------------

TARGET_LIB = libInternalizedPlan.a

TARGET_BIN = GeneratePlan

SRCS = CCalculateMapCosts.cpp CCoordinate.cpp CFeatureVector.cpp \
       CInternalizedPlan.cpp CLoadMap.cpp CMap.cpp CMoveToElement.cpp \
       GeneratePlan.cpp CPlan.cpp CReconBuildingElement.cpp 

INCLUDES = -I. -I../ 

LDLIBS =

LIBS = libInternalizedPlan.a

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
	$(MAKE) $(TARGET_LIB)

.cpp.o:
	$(GPP) -c $(GPPFLAGS) $(INCLUDES) $*.cpp

$(TARGET_LIB): $(OBJS)
	$(RM) $(TARGET_LIB)
	$(AR) $(TARGET_LIB) $(OBJS)
	$(RANLIB) $(TARGET_LIB)

$(TARGET_BIN): GeneratePlan.o
	$(GPP) $(GPPFLAGS)	$(INCLUDES) -o $@ *.o ../SensorData.o ../CJboxInterface.o

clean:
	@ $(RM) *.o core *~ 

veryclean: clean
	@ $(RM) $(TARGET_LIB)
	@ $(RM) $(TARGET_BIN)

depend:
	

nodepend:
	