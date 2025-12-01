# ----------------------------------------------------------------------
# dstar makefile
#
# Written by Ananth Ranganathan
#
# Copyright 2003, Georgia Tech Research Corporation 
# Atlanta, Georgia  30332-0415
# ALL RIGHTS RESERVED, See file COPYRIGHT for details. 
#
# $Id: Makefile,v 1.1.1.1 2008/07/14 16:44:17 endo Exp $
# ----------------------------------------------------------------------

SRCS  = pq.cpp utility.cpp graph.cpp dstar.cpp 

# ----------------------------------------------------------------------

INCLUDES = -I../include

CC = g++

CFLAGS = -Wall -ansi -pedantic

OBJS=$(SRCS:cpp=o)

.cpp.o:
	$(CC) $(CFLAGS) -c $(INCLUDES) $*.cpp

all: libdstar.a 

libdstar.a: $(OBJS)
	rm -f libdstar.a
	ar rcv libdstar.a $(OBJS)
	ranlib libdstar.a

clean:
	rm -f *.o *.bak *~

veryclean: clean
	rm -f libdstar.a

depend:
	

nodepend:
	
