# Make file for libmic/common 
#
# Written by: Douglas C. MacKenzie
#
# Copyright 1996-2002.  All Rights Reserved.                      
# Mobile Intelligence Corporation                                
# Livonia, Michigan, USA                                           
#                                                              
#********************************************************************

# $Id: makefile,v 1.1.1.1 2008/07/14 16:44:17 endo Exp $

# ---------------------------------------------------

# Get system wide defaults
include $(SUPPORT_DIR)/make.include
include ../../make.local

# ---------------------------------------------------

L_SRCS = rc_scanner.l
LFLAGS += -Psara_rc_
Y_SRCS = rc_parser.y
YFLAGS += -p sara_rc_
CC_SRCS = resources.cc

ifeq ($(TARGET_OS),linux)
   LLIBS= -L../$(ADIR) -l$(ALIB) -L$(LIBMIC_DIR)/lib/$(TARGET_OS) -lmic 
endif
ifeq ($(TARGET_OS),win32)
   LLIBS = /LIBPATH:../$(ADIR) lib$(ALIB).lib \
	   /LIBPATH:$(LIBMIC_DIR)/lib/$(TARGET_OS) libmic.lib 
endif
ifeq ($(TARGET_OS),djgpp)
   LLIBS= -L../$(ADIR) -l$(ALIB) -L$(LIBMIC_DIR)/lib/$(TARGET_OS) -lmic 
endif

# ----------------------------------------------------------------------

INCLUDES = -I../../include -I$(LIBMIC_DIR)/include

../$(ARCHIVE): $(OBJS)
	$(AR) $(ARFLAGS) $@ $^
	
all: ../$(ARCHIVE)

ifeq ($(TARGET_OS),win32)
# For win32 we need to run sed on the output of lex.
rc_scanner.cc: rc_scanner.l
	$(LEX) $(LFLAGS) -o$*.tmp $*.l
	source clean_lex.sh < $*.tmp > $*.cc
	rm -f $*.tmp

# For win32 we need to define MSDOS for the bison parser code.
rc_parser.obj: rc_parser.cc
	$(CC) -c -DMSDOS $(CFLAGS) $(INCLUDES) rc_parser.cc
endif

test: test1$(BINEXT) 
	touch test

test1$(BINEXT): test1$(OBJEXT)
	$(LD) $< $(LDOPTS) $(LDFLAGS) $(LLIBS) $(LIBS) $(EXENAMEFLAG)$@
	$@

doc: mic.ps mic_exception.ps

# Get general, invariant targets
include $(SUPPORT_DIR)/make.targets

#######################################################################
# $Log: makefile,v $
# Revision 1.1.1.1  2008/07/14 16:44:17  endo
# MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
#
# Revision 1.1.1.1  2006/07/20 17:17:46  endo
# MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
#
# Revision 1.1.1.1  2006/07/12 13:37:55  endo
# MissionLab 7.0
#
# Revision 1.1  2006/07/01 00:34:51  endo
# CMDLi from MARS 2020 migrated into AO-FNC repository.
#
# Revision 1.1.1.1  2006/06/29 20:42:07  endo
# cmdli local repository.
#
# Revision 1.36  2004/11/12 21:59:51  doug
# renamed yacc/bision stuff so doesn't conflict with missionlab
#
# Revision 1.35  2004/03/26 16:57:15  doug
# switched to using make.target
#
# Revision 1.34  2004/03/25 20:34:46  doug
# fix depend flags
#
# Revision 1.33  2004/03/24 22:15:36  doug
# working on win32 port
#
# Revision 1.32  2004/03/12 20:36:41  doug
# snap
#
# Revision 1.31  2004/03/08 14:52:14  doug
# cross compiles on visual C++
#
# Revision 1.30  2004/02/28 19:26:11  doug
# builds clean on Fedora (commented out a matcon call)
#
# Revision 1.29  2004/02/28 16:35:19  doug
# getting cmdli to work in sara
#
# Revision 1.28  2004/02/20 20:55:10  doug
# barriers work
#
# Revision 1.27  2003/12/10 17:23:33  doug
# working on tasker
#
# Revision 1.26  2003/11/26 22:50:38  doug
# working on state machines
#
# Revision 1.25  2003/11/26 15:21:35  doug
# back from GaTech
#
# Revision 1.24  2003/11/07 22:54:29  doug
# working on cmdl_io
#
# Revision 1.23  2003/11/07 20:29:05  doug
# added cmdli interpreter library
#
# Revision 1.22  2003/11/03 22:43:39  doug
# adding defComposite support
#
# Revision 1.21  2003/10/22 20:34:11  doug
# still working on glyph_editor
#
# Revision 1.20  2003/10/07 21:02:36  doug
# working on open click
#
# Revision 1.19  2003/09/29 19:09:07  doug
# getting hooked into mouse events
#
# Revision 1.18  2003/09/16 16:44:12  doug
# added get_string and set_string helper functions
#
# Revision 1.17  2003/08/25 22:01:50  doug
# builds against libccl_gui
#
# Revision 1.16  2003/08/22 21:23:48  doug
# Back from Benning
#
# Revision 1.15  2003/07/12 01:59:40  doug
# Much work to get tomahawk mission to work.  Added task lead capability to opcon and -o flag to disable it in controller
#
# Revision 1.14  2003/06/30 22:30:38  doug
# Added altitude to robots
#
# Revision 1.13  2003/06/27 19:13:40  doug
# got tomahawk to display
#
# Revision 1.12  2003/04/25 22:13:32  doug
# still working on tcp support
#
# Revision 1.11  2003/04/24 20:56:49  doug
# still coding on tcp support
#
# Revision 1.10  2003/04/23 20:57:11  doug
# still working on tcp support
#
# Revision 1.9  2003/04/07 12:44:39  doug
# moving to RedHat 9
#
# Revision 1.8  2003/03/21 15:55:31  doug
# moved libccl_code to the sara namespace
#
# Revision 1.7  2003/03/13 22:42:06  doug
# seems to work
#
# Revision 1.6  2003/03/03 13:44:24  doug
# seems to work
#
# Revision 1.5  2003/02/26 22:16:20  doug
# integrated with opcon
#
# Revision 1.4  2002/12/23 20:21:37  doug
# added get/set int and set_double.  Also changed get_double to use reference instead of pointer
#
# Revision 1.3  2002/10/23 20:08:39  doug
# fix deprecated code
#
# Revision 1.2  2002/10/10 20:22:10  doug
# Added save capabilities to the resource library
#
# Revision 1.1.1.1  2002/10/02 19:19:18  doug
# Initial import into cvs
#
#
#######################################################################



###########################################################################
# dependencies generated by makedepend

# DO NOT DELETE

