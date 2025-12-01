# ----------------------------------------------------------------------
# makefile for libhardware_drivers
#
# Douglas C. MacKenzie
#
# Copyright 1995 - 2006, Georgia Tech Research Corporation 
# Atlanta, Georgia  30332-0415
# ALL RIGHTS RESERVED, See file COPYRIGHT for details. 
#
# $Id: makefile,v 1.1.1.1 2006/07/12 13:37:56 endo Exp $
# ----------------------------------------------------------------------

include ../make.include

TARGET = libhardware_drivers.a

GPP = g++

RM = rm -f

AR = ar rcv

ARX = ar xv

RANLIB = ranlib

CFLAGS = -g -Wall -ansi -pedantic

LIBS = -L$(MLAB_HOME)/lib -L$(IPT_LIBS)

INCS = -I../include -I../mlab $(XINCS) -I../terrain -I../cnp -I../comm_behavior -I/usr/include/openmotif/

C_SRCS = robot_side_com.c print_overloads.c dock.c hclient.c hserver/ipt_names.c lmadjust.c lineutils.c

CC_SRCS = robot_cnp.cc

C_OBJS=$(C_SRCS:c=o)

CC_OBJS=$(CC_SRCS:cc=o)

# ----------------------------------------------------------------------
#                        COMPILATION RULES
# ----------------------------------------------------------------------
all:
	$(MAKE) $(TARGET)
	$(MAKE) all -C ./hserver
	$(MAKE) all -C ./laser

.c.o:
	$(GPP) $(CFLAGS) $(INCS) -c $*.c -o $*.o -DLINUX 

.cc.o:
	$(GPP) $(CFLAGS) $(INCS) -c $*.cc -o $*.o -DLINUX 

$(TARGET): $(C_OBJS) $(CC_OBJS)
	$(RM) $(TARGET)
	$(AR) $(TARGET) $(C_OBJS) $(CC_OBJS) 
	$(RANLIB) $(TARGET)

lineutils.o:
	$(GPP) $(CFLAGS) $(INCS) -c ../mlab/lineutils.c -o lineutils.o -DLINUX 

clean:
	$(RM) *.o *.bak *~
	$(MAKE) clean -C ./hserver
	$(MAKE) clean -C ./laser

veryclean: clean
	$(RM) *.a
	$(MAKE) veryclean -C ./hserver
	$(MAKE) veryclean -C ./laser

depend:	
	$(MAKE) depend -C hserver
	$(MAKE) depend -C laser
	

# ----------------------------------------------------------------------
#                          DEPENDENCIES
# ----------------------------------------------------------------------
# DO NOT DELETE

