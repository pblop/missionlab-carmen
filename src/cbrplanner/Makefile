# ----------------------------------------------------------------------
# cbrserver makefile
#
# Written by Yoichir Endo
#
# Copyright 2003, Georgia Tech Research Corporation 
# Atlanta, Georgia  30332-0415
# ALL RIGHTS RESERVED, See file COPYRIGHT for details. 
#
# $Id: Makefile,v 1.1.1.1 2008/07/14 16:44:14 endo Exp $
# ----------------------------------------------------------------------

# ----------------------------------------------------------------------
#                           FLAGS ETC.
# ----------------------------------------------------------------------

include ../make.include

TARGET = cbrserver
LIBCBRCLIENT = libcbrclient.a

SRCS = main.cc cbrplanner.cc communicator.cc domain_manager.cc \
       memory_manager.cc planner.cc windows.cc keyboard_reader.cc \
       debugger.cc repair_plan.cc mission_memory_manager.cc \
       repair_memory_manager.cc repair_plan_solution.cc

INCLUDES = -I. -I$(MLAB_HOME)/src/include $(XINCS) $(EXTRA_INCS) -I../cnp -I/usr/include/ncurses/

LDLIBS = -lpthread -lncurses -lpanel -L$(MLAB_HOME)/lib -lcnp -lutilities 

DATE_STRING = $(shell echo `date '+%s'`)

GPP = g++

GPPFLAGS = -Wall -g -ansi -pedantic -DDATE=$(DATE_STRING) -D_POSIX_THREADS -D_POSIX_THREAD_SAFE_FUNCTIONS -D_REENTRANT -DNCURSES_INTERNALS

OBJS = $(SRCS:cc=o)

RM = rm -f -v

# ----------------------------------------------------------------------
#                        COMPILATION RULES
# ----------------------------------------------------------------------
all:
	$(MAKE) $(TARGET)
	$(MAKE) $(LIBCBRCLIENT)

.cc.o:
	$(GPP) -c $(GPPFLAGS) $(INCLUDES) $*.cc

$(TARGET): $(OBJS)
	$(GPP) $(GPPFLAGS) $(INCLUDES) -o $@ $(OBJS) $(LDLIBS)

$(LIBCBRCLIENT):
	$(MAKE) all -C cbrclient

clean:
	$(MAKE) clean -C cbrclient
	@ $(RM) *.o core *~ 

veryclean: clean
	$(MAKE) veryclean -C cbrclient
	@ $(RM) $(TARGET)

depend:
	$(MAKE) depend -C cbrclient

nodepend:
	$(MAKE) nodepend -C cbrclient

