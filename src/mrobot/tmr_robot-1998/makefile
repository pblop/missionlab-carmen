# makefile for robot program
#
# Copyright 2000 - 2006, Georgia Tech Research Corporation 
# Atlanta, Georgia  30332-0415
# ALL RIGHTS RESERVED, See file COPYRIGHT for details. 

# $Id: makefile,v 1.1.1.1 2008/07/14 16:44:24 endo Exp $

include ../../make.include


CNL = ../../../bin/cnl

GCC = gcc

CFLAGS = -g -Wall -D INC_TELOP

INCS = -I../../include $(IPT_INCS)

LIBS = -L../../../lib $(IPT_LIBS) -lhardware_drivers -lCommBehavior -lcnl -lcnp -lpthread -lipt -lipc -lstdc++ -lm -lutilities


CNLSRC = ../mlab-2.0-robot/commonnodes.cnl ../mlab-2.0-robot/executive.cnl ../mlab-2.0-robot/extract.cnl \
         ../mlab-2.0-robot/formnodes.cnl main.cnl ../mlab-2.0-robot/occnodes.cnl ../mlab-2.0-robot/put_status.cnl \
         ../mlab-2.0-robot/telop_nodes.cnl \
         main.cnl navnodes.cnl variables.cnl

all: tmr_robot-1998

tmr_robot-1998:	main.o ../../../lib/libcnl.a ../../../lib/libhardware_drivers.a
	gcc $(MLAB_STATIC) -o tmr_robot-1998 -g main.o  $(LIBS)


main.o:	main.cc
	gcc -c $(CFLAGS) $(INCS) main.cc 

main.cc: $(CNLSRC) ../../include/cnl.inc
	$(CNL) $(INCS) $(STYLE) main.cnl

depend:
	@echo "Make depend is not necessary for this directory."

clean:
	rm -f main.cc main.o core

veryclean: clean
	rm -f tmr_robot-1998

COUNT_SRCS = $(CNLSRC)

######################################################################
# $Log: makefile,v $
# Revision 1.1.1.1  2008/07/14 16:44:24  endo
# MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
#
# Revision 1.2  2006/09/20 18:35:07  nadeem
# Added the code to convert latitude/longitude to X/Y and vice-versa. A lot of files had to be touched to make sure that the initialisation values can flow across from mlab to the robot executables as well as to ensure that the functions were available for use on both sides of the code.
#
# Revision 1.1.1.1  2006/07/20 17:17:51  endo
# MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
#
# Revision 1.1.1.1  2006/07/12 13:38:01  endo
# MissionLab 7.0
#
# Revision 1.3  2006/06/16 22:02:10  endo
# Minor clean ups.
#
# Revision 1.2  2006/01/10 06:22:34  endo
# AO-FNC Type-I check-in.
#
# Revision 1.1.1.1  2005/02/06 23:00:13  endo
# AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
#
# Revision 1.1  2000/08/01 19:20:30  endo
# Initial revision
#
#######################################################################
