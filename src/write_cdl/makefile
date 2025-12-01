# makefile for write_cdl
#
# Douglas C. MacKenzie
#
# Copyright 1995, Georgia Tech Research Corporation 
# Atlanta, Georgia  30332-0415
# ALL RIGHTS RESERVED, See file COPYRIGHT for details. 

# $Id: makefile,v 1.1.1.1 2008/07/14 16:44:25 endo Exp $

include ../make.include

CC = gcc

CFLAGS = -g -Wall

INCS = -I../include -I../load_cdl -I../cfgedit -I../ipt/include $(XINCS) -I/usr/include/openmotif/

SRCS = cdl_codegen.cc prune_list.cc

OBJS=$(SRCS:cc=o)

# ----------------------------------------------------------------------
#                        COMPILATION RULES

.cc.o:
	gcc $(CFLAGS) $(INCS) -c $*.cc


all: libwrite_cdl.a

libwrite_cdl.a: $(OBJS)
	rm -f libwrite_cdl.a 
	ar rcv libwrite_cdl.a $(OBJS)
	ranlib libwrite_cdl.a


clean:
	rm -f *.o *.bak core

veryclean: clean
	rm -f libwrite_cdl.a


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


COUNT_SRCS = $(SRCS) write_cdl.h 

countlines:
	@echo "Line count for ./src/write_cdl:"
	@wc -l $(COUNT_SRCS)


depend:	
	

nodepend:
	


######################################################################
# $Log: makefile,v $
# Revision 1.1.1.1  2008/07/14 16:44:25  endo
# MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
#
# Revision 1.4  2007/02/11 03:54:02  pulam
# Added nodepend condition to makefile to clear dependencies
#
# Revision 1.3  2007/02/08 19:40:43  pulam
# Added handling of zones (no-fly, weather)
# Fixed up some of the icarus objectives to add turn limiting
#
# Revision 1.2  2006/08/29 13:41:58  endo
# Dependencies removed from makefile
#
# Revision 1.1.1.1  2006/07/20 17:17:52  endo
# MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
#
# Revision 1.1.1.1  2006/07/12 13:38:01  endo
# MissionLab 7.0
#
# Revision 1.2  2005/03/23 07:36:57  pulam
# *** empty log message ***
#
# Revision 1.1.1.1  2005/02/06 23:00:22  endo
# AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
#
# Revision 1.4  1999/12/16 22:49:59  endo
# rh-6.0 porting.
#
# Revision 1.3  1997/01/07  19:24:26  tucker
# added make all
#
# Revision 1.2  1996/10/01  13:00:51  doug
# got library writes working
#
# Revision 1.1  1996/09/25 19:12:07  doug
# Initial revision
#
######################################################################

