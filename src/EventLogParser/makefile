# makefile for EventLogParser
#
# Douglas C. MacKenzie
#
# Copyright 1995, Georgia Tech Research Corporation 
# Atlanta, Georgia  30332-0415
# ALL RIGHTS RESERVED, See file COPYRIGHT for details. 

# $Id: makefile,v 1.1.1.1 2008/07/14 16:44:14 endo Exp $

include ../make.include

CC = gcc

CFLAGS = -g -Wall -anci -pedantic

INCS = -I../include $(XINCS) -I/usr/include/openmotif/

SRCS = el_gram.cc el_lex.cc main.cc 
LIBSRCS = EventLogging.cc

OBJS=$(SRCS:cc=o)
LIBOBJS=$(LIBSRCS:cc=o)

# ----------------------------------------------------------------------

INCLUDES = -I../include

LDLIBS = -L../../lib -lutilities -lstdc++ -lm -lc 

# Include this to add the sun malloc debugger
# LDOPTIONS = /usr/lib/debug/malloc.o

# Include this to add the electric fence malloc debugger
# LDOPTIONS = -static -L/users/d/doug/lib -lefence

LDOPTIONS = $(MLAB_STATIC)

CC = gcc
CFLAGS = -Wall -g

.cc.o:
	g++ -c $(CFLAGS) $(INCS) $*.cc

.c.o:
	$(CC) -c $(CFLAGS) $(INCS) $*.c

all: EventLogParser libevents.a

EventLogParser: $(OBJS) $(DEPLIBS) version.cc
	rm -f $@
	g++ $(CFLAGS) version.cc -DDATE="\"`date '+%T, %a %h %d, %Y'`\"" \
             -o $@ $(OBJS) $(LDOPTIONS) $(LDLIBS)

el_gram.cc el_gram.tab.h: el_gram.y
	bison -tvd -p el_ el_gram.y
	mv el_gram.tab.c el_gram.cc

el_lex.o        : defs.h el_lex.cc el_gram.y

el_lex.cc       : el_lex.l
	flex -Pel_ el_lex.l
	mv lex.el_.c el_lex.cc

libevents.a: $(LIBOBJS)
	rm -f libevents.a
	ar rcv libevents.a $(LIBOBJS)
	ranlib libevents.a

clean:
	rm -f *.o *~ core *.bak
	rm -f el_lex.cc
	rm -f el_gram.tab.c
	rm -f el_gram.cc
	rm -f el_gram.output
	rm -f y.tab.h
	rm -f y.output
	rm -f el_gram.out
	rm -f el_gram.tab.h

veryclean: clean
	rm -f EventLogParser
	rm -f libevents.a

checkin:
	@for f in `rlog -L -R RCS/*` ;\
		do ci -u $$f;\
		done

checkout: 
	@for f in `cd RCS;ls * | sed s/,v//g` ;\
		do if (test ! -f $$f) ; \
		      then co $$f; \
		   fi; \
		done

depend:	el_gram.cc el_lex.cc
	

nodepend:
	


######################################################################
# $Log: makefile,v $
# Revision 1.1.1.1  2008/07/14 16:44:14  endo
# MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
#
# Revision 1.3  2007/02/11 03:54:01  pulam
# Added nodepend condition to makefile to clear dependencies
#
# Revision 1.2  2007/02/08 19:40:41  pulam
# Added handling of zones (no-fly, weather)
# Fixed up some of the icarus objectives to add turn limiting
#
# Revision 1.1.1.1  2006/07/20 17:17:45  endo
# MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
#
# Revision 1.1.1.1  2006/07/12 13:37:53  endo
# MissionLab 7.0
#
# Revision 1.3  2006/05/12 22:51:56  endo
# g++-3.4 upgrade.
#
# Revision 1.2  2005/03/23 07:35:47  pulam
# *** empty log message ***
#
# Revision 1.1.1.1  2005/02/06 22:59:27  endo
# AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
#
# Revision 1.8  2002/01/13 01:52:11  endo
# The event logging functions were organized with the object orient style.
#
# Revision 1.7  2001/12/23 20:34:49  endo
# RH 7.1 porting.
#
# Revision 1.6  1999/12/16 23:02:39  endo
# rh-6.0 porting.
#
# Revision 1.5  1996/12/20  04:50:51  tucker
# added MLAB_STATIC
#
# Revision 1.4  1996/05/05  17:59:39  doug
# *** empty log message ***
#
# Revision 1.3  1996/04/13  21:54:42  doug
# *** empty log message ***
#
# Revision 1.2  1996/03/05  22:26:15  doug
# *** empty log message ***
#
# Revision 1.1  1996/03/05  22:25:19  doug
# Initial revision
#
######################################################################

