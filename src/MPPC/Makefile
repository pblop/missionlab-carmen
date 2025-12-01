# ----------------------------------------------------------------------
# Makefile
#
# Written by Alan Wagner
#
# Copyright 2006 - 2007, Georgia Tech Research Corporation 
# Atlanta, Georgia  30332-0415
# ALL RIGHTS RESERVED, See file COPYRIGHT for details. 
#
# $Id: Makefile,v 1.1.1.1 2008/07/14 16:44:14 endo Exp $
# ----------------------------------------------------------------------

# ----------------------------------------------------------------------
#                           FLAGS ETC.
# ----------------------------------------------------------------------

include ../make.include

TARGET = libMPPC.a

SRCS = GT_MPPC_client.cpp acdl_to_xml_converter.cpp xml_to_acdl_converter.cpp

INCLUDES = -I$(MLAB_HOME)/src/include

LDLIBS = 

LIBS = 

GPP = g++

GPPFLAGS = -Wall -g

OBJS = $(SRCS:cpp=o)

RM = rm -f -v

AR = ar rcv

RANLIB = ranlib

# ----------------------------------------------------------------------
#                        COMPILATION RULES
# ----------------------------------------------------------------------

all: libMPPC.a

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
	

