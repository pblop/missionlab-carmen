# makefile for robot program
#
# Copyright 1995 - 2006, Georgia Tech Research Corporation 
# Atlanta, Georgia  30332-0415
# ALL RIGHTS RESERVED, See file COPYRIGHT for details. 

# $Id: makefile,v 1.1.1.1 2008/07/14 16:44:24 endo Exp $

include ../../make.include


TARGET = mlab-2.0-robot

CNL = ../../../bin/cnl

GCC = gcc

CFLAGS = -g -Wall -D INC_TELOP

INCS = -I../../include $(IPT_INCS)

LIBS = -L../../../lib $(IPT_LIBS) -lhardware_drivers -lCommBehavior -lcnl -lcnp -lpthread -lipt -lipc -lstdc++ -lm -lutilities


CNLSRC = commonnodes.cnl executive.cnl extract.cnl \
         formnodes.cnl main.cnl navnodes.cnl occnodes.cnl put_status.cnl \
         telop_nodes.cnl variables.cnl

all: $(TARGET)

$(TARGET):	main.o ../../../lib/libcnl.a ../../../lib/libhardware_drivers.a
	$(GCC) $(MLAB_STATIC) -o $@ -g main.o $(LIBS)

main.o:	main.cc
	$(GCC) -c $(CFLAGS) $(INCS) main.cc 

main.cc: $(CNLSRC) ../../include/cnl.inc
	$(CNL) $(INCS) $(STYLE) main.cnl

depend:
	@echo "Make depend is not necessary for this directory."

nodepend:
	@echo "No dependencies to remove"

clean:
	rm -f main.cc main.o core

veryclean: clean
	rm -f $(TARGET)

COUNT_SRCS = $(CNLSRC)

######################################################################
# $Log: makefile,v $
# Revision 1.1.1.1  2008/07/14 16:44:24  endo
# MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
#
# Revision 1.3  2007/02/11 03:54:02  pulam
# Added nodepend condition to makefile to clear dependencies
#
# Revision 1.2  2006/09/20 18:35:05  nadeem
# Added the code to convert latitude/longitude to X/Y and vice-versa. A lot of files had to be touched to make sure that the initialisation values can flow across from mlab to the robot executables as well as to ensure that the functions were available for use on both sides of the code.
#
# Revision 1.1.1.1  2006/07/20 17:17:51  endo
# MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
#
# Revision 1.1.1.1  2006/07/12 13:38:00  endo
# MissionLab 7.0
#
# Revision 1.6  2006/07/07 04:33:39  endo
# robot -> mlab-2.0-robot
#
# Revision 1.5  2006/06/16 22:02:05  endo
# Minor clean ups.
#
# Revision 1.4  2006/05/15 06:06:39  endo
# gcc-3.4 upgrade
#
# Revision 1.3  2006/05/15 06:03:26  endo
# gcc-3.4 upgrade
#
# Revision 1.2  2006/01/10 06:22:26  endo
# AO-FNC Type-I check-in.
#
# Revision 1.1.1.1  2005/02/06 23:00:14  endo
# AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
#
# Revision 1.43  2000/08/01 19:17:48  endo
# Directory changed from src/robot to src/mlab-2.0-robot.
#
# Revision 1.42  2000/06/13 16:52:46  endo
# -lipt added.
#
# Revision 1.41  1999/12/16 23:19:08  mjcramer
# rh6 port
#
# Revision 1.40  1999/05/25 17:29:43  dj
# organized and updated build structure
#
# Revision 1.39  1997/01/07  19:58:21  tucker
# added make all
#
# Revision 1.38  1996/12/20  05:06:04  tucker
# MLAB_STATIC
#
# Revision 1.37  1996/06/01  21:54:55  doug
# *** empty log message ***
#
# Revision 1.36  1996/04/30  20:47:15  tucker
# trb
#
# Revision 1.35  1996/02/21  15:43:53  doug
# fixedup so compiles with current libcnl
#
# Revision 1.34  1995/10/03  21:27:37  doug
# fix some stuff that got changed in the libraries
#
# Revision 1.33  1995/07/13  22:12:25  jmc
# Moved things around.
#
# Revision 1.32  1995/06/30  15:16:14  doug
# *** empty log message ***
#
# Revision 1.31  1995/06/29  20:12:30  jmc
# Added countlines target.
#
# Revision 1.30  1995/06/29  14:02:49  jmc
# Cleaned up CNLSRC.  Added RCS log string.
#######################################################################
