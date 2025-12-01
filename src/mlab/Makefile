##########################################################################
#
# Makefile for MissionLab console program: mlab
#
# Jonathan M. Cameron
#
# Copyright 1995 - 2008 Georgia Tech Research Corporation 
# Atlanta, Georgia  30332-0415
# ALL RIGHTS RESERVED, See file COPYRIGHT for details. 
#
# NOTE: If any directory used in this makefile or src/make.include is
#	modified (such as IPTDIR, or the directories in XLIBS or
#	EXTRA_INCS), the dependencies in the bottom of makefile will
#	need to be updated by running "make depend".
#
##########################################################################

# $Id: Makefile,v 1.4 2008/07/16 19:10:17 endo Exp $

include ../make.include

# ----------------------------------------------------------------------
#			 COMPILER DEFAULTS
# ----------------------------------------------------------------------

MACHINE := $(shell uname -m)

ifeq ($(MACHINE), x86_64)
LIBDIR = /usr/lib64
else
LIBDIR = /usr/lib
endif

CC = g++

RM = rm -f -v

CFLAGS1 = -g $(MLAB_STATIC) -Wall -ansi -pedantic -Dlinux -fstack-protector-all
CFLAGS2 = -g $(MLAB_STATIC) -Wall -ansi -Dlinux -fstack-protector-all -I/usr/include/openmotif/

CFLAGS = $(CFLAGS1) -DMLAB_HOME=\"$(DESTDIR)\" -fpermissive -I/usr/include/openmotif/

ifeq ($(ENABLE_OPENGL),0)
	CFLAGS += -DNO_OPENGL
endif

# Depending on your system setup, you may have to modify INCS as follows:
# If g++ doesn't know where to look for the X/Xm includes, add $(XINCS)
# If g++ doesn't know where to find its own include files, add $(EXTRA_INCS)
INCS =	-I../include -I. $(IPT_INCS) $(XINCS) -I../cnp -I../terrain

# Include the first version below to debug the heap
# LDOPTS = -static /usr/lib/debug/malloc.o
#LDOPTS = -static

PTHREAD_DEFS = -D_POSIX_THREADS -D_POSIX_THREAD_SAFE_FUNCTIONS -D_REENTRANT 

#---------------------------------------------------------------------
# Modifications to include cmdli (uncomment following lines to include cmdli)
#  (Don't forget to run "make depend" after changing this.)
#---------------------------------------------------------------------
CMDLI_INCS = -I../include/cmdli
CMDLI_LIBS = -L../../lib -lmic_cmdli -lz

#---------------------------------------------------------------------
# Modifications to include Telop (uncomment following lines to include Telop)
#  (Don't forget to run "make depend" after changing this.)
# ----------------------------------------------------------------------
TELOP_INCS = -DTELOP
TELOP_LIBS = -L../../lib -ltelop
TELOP_DEPENDENCY  = ../telop/libtelop.a

COMPASS_LIBS = -L../../lib -lcompass
COMPASS_DEPENDENCY = ../compass/libcompass.a

# Add -lmcheck for memory check
LIBS = 	$(TELOP_LIBS) $(CMDLI_LIBS) $(IPT_LIBS) $(OTHER_LIBS) $(SPHIGS_LIBS) -L/usr/lib/openmotif \
	$(COMPASS_LIBS) -lload_rc -lutilities -levents -lcompass -L$(LIBDIR)/flex-2.5.4a/ -lfl \
	-lassistantDialog -lcbrclient -lcbrwizard -L/usr/X11R6/lib -lGLU -lGL -lXm -lXt -lXext -lX11 -lmlab3d -lcnp -lpthread

ifeq ($(ENABLE_OPENGL),1)
LIBS += -lGLw 
endif

# ----------------------------------------------------------------------
#			 COMPILATION RULES
# ----------------------------------------------------------------------
.cc.o:
	$(CC) $(CFLAGS) $(INCS) $(TELOP_INCS) $(CMDLI_INCS) $(PTHREAD_DEFS) -c $*.cc

.cpp.o:
	$(CC) $(CFLAGS) $(INCS) $(TELOP_INCS) $(CMDLI_INCS) $(PTHREAD_DEFS) -c $*.cpp

.c.o:
	$(CC) $(CFLAGS) $(INCS) $(TELOP_INCS) $(CMDLI_INCS) $(PTHREAD_DEFS) -c $*.c

# ----------------------------------------------------------------------
#			    MAIN TARGETS
# ----------------------------------------------------------------------

COMMON_OBJS = console.o gt_command.o gt_console_db.o gt_measure.o gt_command_panel.o \
              gt_console_com.o simulation_server.o console_side_com.o \
              gt_world.o gt_create_world.o gt_map.o gt_scale.o draw.o \
              PickMapOverlay.o \
              lineutils.o waypoint.o lm_param_dialog.o \
              gt_console_windows.o mission_design.o \
              env_change_gram.o env_change_lex.o env_change.o FileOpenDlg.o \
              VectorFieldDlg.o VectorField.o gt_playback.o mlab_cbrclient.o \
              mlab_cnp.o gt_cmdli_panel.o jbox_mlab.o jbox_display.o overlay_zones.o \
			  draw_military.o bitmap_icon_displayer.o

ONLY_OVERLAY_OBJS = gt_overlay_yac.o gt_overlay_lex.o gt_load_overlay.o

ONLY_COMMAND_OBJS = gt_command_yac.o gt_command_lex.o gt_load_command.o

OVERLAY_OBJS = $(COMMON_OBJS) $(ONLY_OVERLAY_OBJS)

COMMAND_OBJS = $(COMMON_OBJS) $(ONLY_COMMAND_OBJS) $(ONLY_OVERLAY_OBJS)


all: mlab 


gt_overlay_lex.c: gt_overlay_lex.l gt_overlay_yac.h 
	flex -i -t -Podl_ gt_overlay_lex.l > gt_overlay_lex.c

gt_overlay_yac.c gt_overlay_yac.h: gt_overlay_yac.y gt_load_overlay.h \
				   gt_console_db.h
	bison -b gt_overlay_ -p odl_ -d -o gt_overlay_yac.c gt_overlay_yac.y

gt_overlay_yac.o: gt_overlay_yac.c
	$(CC) $(CFLAGS2) $(INCS) $(TELOP_INCS) $(PTHREAD_DEFS) -c gt_overlay_yac.c

gt_command_lex.c: gt_command_lex.l gt_command_yac.h
	flex -i -t -Pcmdl_ gt_command_lex.l > gt_command_lex.c

gt_command_yac.c gt_command_yac.h: gt_command_yac.y gt_load_command.h \
				   gt_console_db.h
	bison -b gt_command_ -p cmdl_ -d -o gt_command_yac.c gt_command_yac.y

gt_command_yac.o: gt_command_yac.c
	$(CC) $(CFLAGS2) $(INCS) $(TELOP_INCS) $(PTHREAD_DEFS) -c gt_command_yac.c

env_change_gram.c env_change_gram.h: env_change_gram.y
	bison -b env_change_gram -p env_change_ -d -o env_change_gram.c env_change_gram.y

env_change_lex.c: env_change_lex.l
	flex -i -t -Penv_change_ env_change_lex.l > env_change_lex.c

env_change_gram.o: env_change_gram.c
	$(CC) $(CFLAGS2) $(INCS) $(TELOP_INCS) $(PTHREAD_DEFS) -c env_change_gram.c

env_change_lex.o: env_change_lex.c

# The following dependency rules are necessary because parts of
# console.c, etc are ifdef'ed in/out depending on the TELOP flag.
# Whether the TELOP flag is given is specified in Makefile.
# Therefore, if Makefile changes, it is possible that these object 
# files need to be recompiled.

mlab: $(COMMAND_OBJS) version.c
		$(CC) $(CFLAGS) $(INCS) version.c \
		-DDATE="\"`date '+%T, %a %h %d, %Y'`\"" \
		-o $@ $(COMMAND_OBJS) $(LDOPTS) $(LIBS) 

install: all
	echo "No need to install, everything is linked..."

clean: 
	@ $(RM) *.o *.bak *~
	@ $(RM) *_lex.c *_yac.c *_yac.h env_change_gram.c env_change_gram.h

veryclean: clean
	@ $(RM) mlab

depend: gt_overlay_yac.h gt_command_yac.h
	

nodepend:
	

