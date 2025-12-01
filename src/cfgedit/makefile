# graphic designer (cfgedit) makefile
#
# Douglas C. MacKenzie
#
# Copyright 1995 - 2007 Georgia Tech Research Corporation 
# Atlanta, Georgia  30332-0415
# ALL RIGHTS RESERVED, See file COPYRIGHT for details. 

# $Id: makefile,v 1.1.1.1 2008/07/14 16:44:15 endo Exp $


# ----------------------------------------------------------------------
#                           FLAGS ETC.
# ----------------------------------------------------------------------

export SHELL = csh

include ../make.include

TARGET = cfgedit

SRCS  = binding.cc buildbitmaps.cc buildmenu.cc callbacks.cc \
	circle.cc configuration.cc \
	error.cc fsa.cc glyph.cc menu.cc page.cc \
	popups.cc transition.cc make.cc run.cc cim_management.cc sgig_management.cc \
    edit.cc slot_record.cc verify.cc library_manager.cc toolbar.cc \
	so_movement.cc DrawArrow.cc EditParms.cc \
	PickAgent.cc ConstructPrintName.cc PuDataType.cc \
	cfg_apply.cc help.cc HaveParameters.cc no_make_needed.cc \
	renumber_robots.cc  PushupParms.cc analyze.cc import_symbol.cc \
	mission_expert.cc cfgedit_cbrclient.cc mission_spec_wizard.cc icarus_wizard.cc \
	bams_wizard.cc launch_wizard.cc

INCLUDES = -I. -I../load_cdl -I../load_rc -I../hardware_drivers -I../include \
	   -I../write_cdl -I../ipt/include -I../MPPC -I/usr/include/openmotif/

LDLIBS = -L../load_cdl -lload_cdl -L../write_cdl -lwrite_cdl -L../load_rc -L/usr/lib/openmotif -lload_rc \
	 -L../../lib -lassistantDialog -lutilities -levents -lcbrclient -lcbrwizard -lacdl_plus \
     -L/usr/X11R6/lib -lMPPC -lXm -lXt -lXext -lX11 -lm -lstdc++ -lc -lpthread -lmic_cmdli

LDOPTIONS = $(MLAB_STATIC)

CC = g++

CFLAGS = -Wall -g -ansi -pedantic -DMLAB_HOME=\"$(DESTDIR)\" -fpermissive

DATE_STRING = $(shell echo `date '+%s'`)

PTHREAD_DEFS = -D_POSIX_THREADS -D_POSIX_THREAD_SAFE_FUNCTIONS -D_REENTRANT 

OBJS = $(SRCS:cc=o) $(C_SRCS:c=o)

# ----------------------------------------------------------------------
#                        COMPILATION RULES
# ----------------------------------------------------------------------

all:
	$(MAKE) $(TARGET)

.cc.o:
	$(CC) -c $(CFLAGS) $(INCLUDES) $(PTHREAD_DEFS) $*.cc

.c.o:
	$(CC) -c $(CFLAGS) $(INCLUDES) $(PTHREAD_DEFS) $*.c

$(TARGET): $(OBJS) $(DEPLIBS) version.cc
	$(CC) $(CFLAGS) version.cc -DDATE=$(DATE_STRING) \
	     -o $@ $(OBJS) $(LDOPTIONS) $(LDLIBS)

clean:
	rm -f *.o *.bak *~

veryclean: clean
	rm -f cfgedit 


depend:
	

nodepend:
	
