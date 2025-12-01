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

CC_SRCS = objthreads.cc objthreads_implementation.cc barrier.cc \
	condition.cc condition_implementation.cc \
	mutex.cc mutex_implementation.cc semaphore.cc

ifeq ($(TARGET_OS),linux)
   CC_SRCS += posix_objthreads.cc posix_mutex.cc posix_condition.cc
   LLIBS = -L../$(ADIR) -l$(ALIB) -L$(LIBMIC_DIR)/lib/$(TARGET_OS) -lmic -lpthread
endif
ifeq ($(TARGET_OS),win32)
   CC_SRCS += win32_objthreads.cc win32_mutex.cc win32_condition.cc
   LLIBS = /LIBPATH:../$(ADIR) lib$(ALIB).lib \
	   /LIBPATH:$(LIBMIC_DIR)/lib/$(TARGET_OS) libmic.lib 
endif
ifeq ($(TARGET_OS),djgpp)
   CC_SRCS += djgpp_objthreads.cc djgpp_mutex.cc djgpp_condition.cc
   LLIBS = $(LIBDIRFLAG)../$(ADIR) -l$(ALIB) -L$(LIBMIC_DIR)/lib/$(TARGET_OS) \
	   -lmic -lpthread
endif

# ---------------------------------------------------

INCLUDES = -I. -I$(LIBMIC_DIR)/include

../$(ARCHIVE): $(OBJS)
	$(AR) $(ARFLAGS) $@ $^
	
all: ../$(ARCHIVE)

TESTS = test_synchronized$(BINEXT) 
#TESTS = test1$(BINEXT) test2$(BINEXT) \
	test_barrier$(BINEXT) \
	test_condition$(BINEXT) \
	test1_mutex$(BINEXT) test2_mutex$(BINEXT) \
	test_semaphore$(BINEXT)

test: $(TESTS)
	touch test

test_synchronized$(BINEXT): test_synchronized$(OBJEXT)
	$(LD) $< $(LDOPTS) $(LDFLAGS) $(LLIBS) $(LIBS) $(EXENAMEFLAG)$@
	$@

test1$(BINEXT): test1$(OBJEXT)
	$(LD) $< $(LDOPTS) $(LDFLAGS) $(LLIBS) $(LIBS) $(EXENAMEFLAG)$@
	$@

test2$(BINEXT): test2$(OBJEXT)
	$(LD) $< $(LDOPTS) $(LDFLAGS) $(LLIBS) $(LIBS) $(EXENAMEFLAG)$@
	$@

test_barrier$(BINEXT): test_barrier$(OBJEXT)
	$(LD) $< $(LDOPTS) $(LDFLAGS) $(LLIBS) $(LIBS) $(EXENAMEFLAG)$@
	$@

test_condition$(BINEXT): test_condition$(OBJEXT)
	$(LD) $< $(LDOPTS) $(LDFLAGS) $(LLIBS) $(LIBS) $(EXENAMEFLAG)$@
	$@

test1_mutex$(BINEXT): test1_mutex$(OBJEXT)
	$(LD) $< $(LDOPTS) $(LDFLAGS) $(LLIBS) $(LIBS) $(EXENAMEFLAG)$@
	$@

test2_mutex$(BINEXT): test2_mutex$(OBJEXT)
	$(LD) $< $(LDOPTS) $(LDFLAGS) $(LLIBS) $(LIBS) $(EXENAMEFLAG)$@
	echo "You need to run test2_mutex by hand because it deadlocks."

test_semaphore$(BINEXT): test_semaphore$(OBJEXT)
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
# Revision 1.1  2006/07/01 00:33:51  endo
# CMDLi from MARS 2020 migrated into AO-FNC repository.
#
# Revision 1.1.1.1  2006/06/29 20:42:07  endo
# cmdli local repository.
#
# Revision 1.37  2004/03/26 16:57:18  doug
# switched to using make.target
#
# Revision 1.36  2004/03/25 22:26:30  doug
# added a thread_yield method
#
# Revision 1.35  2004/03/24 22:11:11  doug
# fix depend
#
# Revision 1.34  2004/03/24 17:54:07  doug
# builds under win32
#
# Revision 1.33  2004/03/10 15:49:49  doug
# not yet working
#
# Revision 1.32  2004/03/08 14:52:16  doug
# cross compiles on visual C++
#
# Revision 1.31  2004/02/28 19:26:13  doug
# builds clean on Fedora (commented out a matcon call)
#
# Revision 1.30  2004/02/28 16:35:21  doug
# getting cmdli to work in sara
#
# Revision 1.29  2004/02/20 20:55:12  doug
# barriers work
#
# Revision 1.28  2003/12/10 17:23:35  doug
# working on tasker
#
# Revision 1.27  2003/11/26 22:50:40  doug
# working on state machines
#
# Revision 1.26  2003/11/26 15:21:36  doug
# back from GaTech
#
# Revision 1.25  2003/11/07 22:54:31  doug
# working on cmdl_io
#
# Revision 1.24  2003/11/07 20:29:07  doug
# added cmdli interpreter library
#
# Revision 1.23  2003/11/03 22:43:41  doug
# adding defComposite support
#
# Revision 1.22  2003/10/22 20:34:13  doug
# still working on glyph_editor
#
# Revision 1.21  2003/10/07 21:02:39  doug
# working on open click
#
# Revision 1.20  2003/09/29 19:09:10  doug
# getting hooked into mouse events
#
# Revision 1.19  2003/08/25 22:01:52  doug
# builds against libccl_gui
#
# Revision 1.18  2003/08/22 21:23:50  doug
# Back from Benning
#
# Revision 1.17  2003/07/11 22:07:51  doug
# Convert condition.wait to use a TimeOfDay timeout
#
# Revision 1.16  2003/06/30 22:30:40  doug
# Added altitude to robots
#
# Revision 1.15  2003/06/27 19:13:42  doug
# got tomahawk to display
#
# Revision 1.14  2003/04/25 22:13:34  doug
# still working on tcp support
#
# Revision 1.13  2003/04/24 20:56:51  doug
# still coding on tcp support
#
# Revision 1.12  2003/04/23 20:57:12  doug
# still working on tcp support
#
# Revision 1.11  2003/04/07 12:44:41  doug
# moving to RedHat 9
#
# Revision 1.10  2003/03/21 15:55:33  doug
# moved libccl_code to the sara namespace
#
# Revision 1.9  2003/03/13 22:42:07  doug
# seems to work
#
# Revision 1.8  2003/03/03 13:44:25  doug
# seems to work
#
# Revision 1.7  2003/02/26 22:16:21  doug
# integrated with opcon
#
# Revision 1.6  2003/02/26 14:56:29  doug
# controller now runs with objcomms
#
# Revision 1.5  2003/02/19 22:57:57  doug
# change thread_sleep to use TimeOfDay instead of msecs
#
# Revision 1.4  2002/12/31 20:13:20  doug
# added thread_sleep call
#
# Revision 1.3  2002/10/03 19:25:21  doug
# fixed test2_mutex run
#
# Revision 1.2  2002/10/03 19:24:17  doug
# moved all objthreads to a common directory
#
# Revision 1.1.1.1  2002/10/02 18:59:28  doug
# Initial import into cvs
#
#
#######################################################################



###########################################################################
# dependencies generated by makedepend

# DO NOT DELETE

