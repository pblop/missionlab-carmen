# Make file for libmic/common 
#
# Written by: Douglas C. MacKenzie
#
# Copyright 1996-2002.  All Rights Reserved.                      
# Mobile Intelligence Corporation                                
# Livonia, Michigan, USA                                           
#                                                              
#********************************************************************

# $Id: makefile,v 1.1.1.1 2008/07/14 16:44:16 endo Exp $

# ---------------------------------------------------

# Get system wide defaults
include $(SUPPORT_DIR)/make.include
include ../../make.local

# ---------------------------------------------------

CC_SRCS =  $(wildcard *.cc)

ifeq ($(TARGET_OS),linux)
   LLIBS = -L../$(ADIR) -l$(ALIB) \
	-L$(LIBENV_OBJECTS_DIR)/lib/$(TARGET_OS) -lenv_objects \
	-L$(LIBCMDL_IO_DIR)/lib/$(TARGET_OS) -lcmdl_io \
	-L$(LIBOBJCOMMS_DIR)/lib/$(TARGET_OS) -lobjcomms \
	-L$(LIBOBJTHREADS_DIR)/lib/$(TARGET_OS) -lobjthreads \
	-L$(LIBMIC_DIR)/lib/$(TARGET_OS) -lmic -lpthread \
	-lpthread -lz -lcrypto

#	-L$(LIBRC_DIR)/lib/$(TARGET_OS) -lrc 
endif
ifeq ($(TARGET_OS),win32)
   LLIBS = /LIBPATH:../$(ADIR) lib$(ALIB).lib \
       /LIBPATH:$(LIBENV_OBJECTS_DIR)/lib/$(TARGET_OS) libenv_objects.lib \
       /LIBPATH:$(LIBRC_DIR)/lib/$(TARGET_OS) librc.lib \
       /LIBPATH:$(LIBCMDL_IO_DIR)/lib/$(TARGET_OS) libcmdl_io.lib \
       /LIBPATH:$(LIBOBJCOMMS_DIR)/lib/$(TARGET_OS) libobjcomms.lib \
       /LIBPATH:$(LIBOBJTHREADS_DIR)/lib/$(TARGET_OS) libobjthreads.lib \
       /LIBPATH:$(LIBMIC_DIR)/lib/$(TARGET_OS) libmic.lib 

#       /LIBPATH:$(LIBRC_DIR)/lib/$(TARGET_OS) librc.lib 
endif
# ---------------------------------------------------

INCLUDES = -I../../include -I$(LIBMIC_DIR)/include \
	   -I$(LIBRC_DIR)/include \
	   -I$(LIBOBJTHREADS_DIR)/include \
	   -I$(LIBENV_OBJECTS_DIR)/include \
	   -I$(LIBIPC_ROBOT2SIM_DIR)/include \
	   -I$(LIBIPC_ROBOT2CON_DIR)/include \
	   -I$(LIBIPC_SUPPORT_DIR)/include \
	   -I$(LIBCMDL_IO_DIR)/include \
	   -I$(LIBOBJCOMMS_DIR)/include \
	   -I$(LIBROBOT_DIR)/include

# ---------------------------------------------------

../$(ARCHIVE): $(OBJS)
	$(AR) $(ARFLAGS) $@ $^
	
all: ../$(ARCHIVE)

#test1: test1.cc $(OBJS)
#	$(CC) -o $@ $(CFLAGS) $(INCLUDES) $(OBJS) $(LDOPTS) $(LDFLAGS) $(LLIBS) $(LIBS)

test1$(BINEXT): test1$(OBJEXT)
	$(LD) $< $(LDOPTS) $(LDFLAGS) $(LLIBS) $(LIBS) $(EXENAMEFLAG)$@

operatorConsole$(BINEXT): operatorConsole$(OBJEXT)
	$(LD) $< $(LDOPTS) $(LDFLAGS) $(LLIBS) $(LIBS) $(EXENAMEFLAG)$@

doc: 
	doc++ -A -t -o FSAi.tex *.h
	latex FSAi.tex
	latex FSAi.tex
	dvips FSAi.dvi -o
	rm -f FSAi.dvi FSAi.tex FSAi.aux FSAi.log


# Get general, invariant targets
EXTRA_DELETES = operatorConsole
include $(SUPPORT_DIR)/make.targets

clean:
	rm -f *.o *.obj *.bak core core.* *.exe *.map *.lst *.ps *.pdb *.output *.idb *.pdb $(TESTS) $(LY_CC) $(LY_H) $(DATAS) $(EXTRA_DELETES)

#######################################################################
# $Log: makefile,v $
# Revision 1.1.1.1  2008/07/14 16:44:16  endo
# MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
#
# Revision 1.1.1.1  2006/07/20 17:17:45  endo
# MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
#
# Revision 1.1.1.1  2006/07/12 13:37:54  endo
# MissionLab 7.0
#
# Revision 1.1  2006/07/01 00:14:58  endo
# CMDLi from MARS 2020 migrated into AO-FNC repository.
#
# Revision 1.1.1.1  2006/06/29 20:42:07  endo
# cmdli local repository.
#
# Revision 1.41  2004/10/07 21:42:38  doug
# fixed so cleans
#
# Revision 1.40  2004/10/07 21:39:05  doug
# chasing gui_extra problems
#
# Revision 1.39  2004/10/07 13:09:42  doug
# work in progress
#
# Revision 1.38  2004/10/05 22:15:39  doug
# working on binding points
#
# Revision 1.37  2004/10/01 14:21:51  doug
# glyphs look nicer, and FSAs display
#
# Revision 1.36  2004/09/23 21:52:08  doug
# getting robots to run again
#
# Revision 1.35  2004/09/21 14:51:49  doug
# add more debugging, remove BlockIsDone, and don't loop if pastBarrier.
#
# Revision 1.34  2004/08/16 13:36:22  doug
# cleanup
#
# Revision 1.33  2004/08/16 12:29:27  doug
# cleanup
#
# Revision 1.32  2004/08/13 20:56:20  doug
# three phase bombing works again
#
# Revision 1.31  2004/07/30 13:42:15  doug
# handles comm timeouts and resyncs
#
# Revision 1.30  2004/06/22 20:58:13  doug
# working on getting matcon to build
#
# Revision 1.29  2004/06/11 16:10:36  doug
# move to sara namespace
#
# Revision 1.28  2004/06/11 16:10:02  doug
# move to sara namespace
#
# Revision 1.27  2004/05/11 19:34:50  doug
# massive changes to integrate with USC and GaTech
#
# Revision 1.26  2004/04/30 17:36:01  doug
# works for UPenn
#
# Revision 1.25  2004/04/27 16:19:34  doug
# added support for multicast as unicast
#
# Revision 1.24  2004/04/14 15:37:09  doug
# fixed makefile for test1
#
# Revision 1.23  2004/04/13 22:29:43  doug
# switched to testing cmdli functions used by upenn
#
# Revision 1.22  2004/04/13 18:00:06  doug
# renamed test.cc to test1.cc to prevent it shadowing system test command
#
# Revision 1.21  2004/04/13 17:57:33  doug
# switch to using internal communications and not passing messages in and out
#
# Revision 1.20  2004/03/26 16:57:36  doug
# switched to using make.target
#
# Revision 1.19  2004/03/26 00:19:12  doug
# working on win32 port
#
# Revision 1.18  2004/03/22 18:42:16  doug
# coded block transitions
#
# Revision 1.17  2004/03/17 22:17:11  doug
# working on procedure calls
#
# Revision 1.16  2004/03/15 22:33:34  doug
# working on cutting over to ActionCommand
#
# Revision 1.15  2004/03/12 20:36:59  doug
# snap
#
# Revision 1.14  2004/03/09 22:01:04  doug
# promote Blocks to mission segments and procedures
#
# Revision 1.13  2004/03/08 14:52:35  doug
# cross compiles on visual C++
#
# Revision 1.12  2004/02/28 19:26:31  doug
# builds clean on Fedora (commented out a matcon call)
#
# Revision 1.11  2004/02/28 16:35:43  doug
# getting cmdli to work in sara
#
# Revision 1.10  2004/02/20 20:54:48  doug
# barriers work
#
# Revision 1.9  2004/02/20 14:39:36  doug
# cmdl interpreter runs a bit
#
# Revision 1.8  2003/12/10 17:23:57  doug
# working on tasker
#
# Revision 1.7  2003/11/26 22:51:04  doug
# working on state machines
#
# Revision 1.6  2003/11/26 15:21:57  doug
# back from GaTech
#
# Revision 1.5  2003/11/13 22:29:33  doug
# adding transitions
#
# Revision 1.4  2003/11/11 22:04:13  doug
# getting it to execute
#
# Revision 1.3  2003/11/10 22:48:33  doug
# working on cmdli
#
# Revision 1.2  2003/11/07 22:54:53  doug
# working on cmdl_io
#
# Revision 1.1.1.1  2003/11/07 20:27:33  doug
# initial import
#
# Revision 1.19  2003/11/03 22:43:47  doug
# adding defComposite support
#
# Revision 1.18  2003/10/22 20:34:19  doug
# still working on glyph_editor
#
# Revision 1.17  2003/10/07 21:02:45  doug
# working on open click
#
# Revision 1.16  2003/09/29 19:09:16  doug
# getting hooked into mouse events
#
# Revision 1.15  2003/08/25 22:01:58  doug
# builds against libccl_gui
#
# Revision 1.14  2003/08/22 21:23:57  doug
# Back from Benning
#
# Revision 1.13  2003/07/09 22:19:18  doug
# The robot now exits cleanly
#
# Revision 1.12  2003/06/30 22:30:46  doug
# Added altitude to robots
#
# Revision 1.11  2003/06/27 19:13:48  doug
# got tomahawk to display
#
# Revision 1.10  2003/04/25 22:13:39  doug
# still working on tcp support
#
# Revision 1.9  2003/04/07 12:44:47  doug
# moving to RedHat 9
#
# Revision 1.8  2003/03/21 15:55:37  doug
# moved libccl_code to the sara namespace
#
# Revision 1.7  2003/03/13 22:42:11  doug
# seems to work
#
# Revision 1.6  2003/03/03 13:44:30  doug
# seems to work
#
# Revision 1.5  2003/02/26 22:16:26  doug
# integrated with opcon
#
# Revision 1.4  2003/02/26 14:53:18  doug
# controller now runs with objcomms
#
# Revision 1.3  2002/10/04 20:36:53  doug
# make library name all lower case to match convention
#
# Revision 1.2  2002/10/03 15:34:41  doug
# fixed include in makefile
#
# Revision 1.1.1.1  2002/10/02 16:04:54  doug
# Initial import into cvs
#
#
#######################################################################



###########################################################################
# dependencies generated by makedepend

# DO NOT DELETE

