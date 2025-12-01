# Makefile to build library
#
# Written by: Douglas C. MacKenzie
#
# Copyright 1996-2003.  All Rights Reserved.                      
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

CC_SRCS = objcomms.cc objcomms_implementation.cc commMsg.cc \
	commHandle.cc rawMsg.cc ComAddr.cc PacketHeader.cc

ifeq ($(TARGET_OS),linux)
   CC_SRCS += posix_udp_objcomms.cc 
   LLIBS = -L../$(ADIR) -l$(ALIB) \
	   -L$(LIBOBJTHREADS_DIR)/lib/$(TARGET_OS) -lobjthreads  \
           -L$(LIBMIC_DIR)/lib/$(TARGET_OS) -lmic \
           -lpthread -lz -lcrypto
endif
ifeq ($(TARGET_OS),win32)
   CC_SRCS += posix_udp_objcomms.cc 
#   CC_SRCS += win32_objcomms.cc 
   LLIBS = /LIBPATH:../$(ADIR) lib$(ALIB).lib \
	   /LIBPATH:Z:$(LIBOBJTHREADS_DIR)/lib/$(TARGET_OS) libobjthreads.lib \
	   /LIBPATH:Z:$(LIBMIC_DIR)/lib/$(TARGET_OS) libmic.lib 
endif
ifeq ($(TARGET_OS),djgpp)
   CC_SRCS += djgpp_objcomms.cc
   LLIBS = $(LIBDIRFLAG)../$(ADIR) -l$(ALIB) -L$(LIBMIC_DIR)/lib/$(TARGET_OS) \
	   -lmic -lpthread
endif

# ---------------------------------------------------

INCLUDES = -I. -I$(LIBMIC_DIR)/include -I$(LIBOBJTHREADS_DIR)/include


../$(ARCHIVE): $(OBJS)
	$(AR) $(ARFLAGS) $@ $^
	
all: ../$(ARCHIVE)

test_streaming$(BINEXT): test_streaming$(OBJEXT)
	$(LD) $< $(LDOPTS) $(LDFLAGS) $(LLIBS) $(LIBS) -lz -lcrypto $(EXENAMEFLAG)$@
	$@

test1$(BINEXT): test1$(OBJEXT)
	$(LD) $< $(LDOPTS) $(LDFLAGS) $(LLIBS) $(LIBS) -lz -lcrypto $(EXENAMEFLAG)$@
	$@

test_writer$(BINEXT): test_writer$(OBJEXT)
	$(LD) $< $(LDOPTS) $(LDFLAGS) $(LLIBS) $(LIBS) $(EXENAMEFLAG)$@

explore_server$(BINEXT): explore_server$(OBJEXT)
	$(LD) $< $(LDOPTS) $(LDFLAGS) $(LLIBS) $(LIBS) -lz -lcrypto $(EXENAMEFLAG)$@
	$@

explore_client$(BINEXT): explore_client$(OBJEXT)
	$(LD) $< $(LDOPTS) $(LDFLAGS) $(LLIBS) $(LIBS) -lz -lcrypto $(EXENAMEFLAG)$@
	$@

test_reader$(BINEXT): test_reader$(OBJEXT)
	$(LD) $< $(LDOPTS) $(LDFLAGS) $(LLIBS) $(LIBS) $(EXENAMEFLAG)$@
#	$@

test2$(BINEXT): test2$(OBJEXT)
	$(LD) $< $(LDOPTS) $(LDFLAGS) $(LLIBS) $(LIBS) -lz -lcrypto $(EXENAMEFLAG)$@
	$@

test3$(BINEXT): test3$(OBJEXT)
	$(LD) $< $(LDOPTS) $(LDFLAGS) $(LLIBS) $(LIBS) -lz -lcrypto $(EXENAMEFLAG)$@
	$@

echo$(BINEXT): echo$(OBJEXT)
	$(LD) $< $(LDOPTS) $(LDFLAGS) $(LLIBS) $(LIBS) -lz -lcrypto $(EXENAMEFLAG)$@
	$@

test_tcp_server$(BINEXT): test_tcp_server$(OBJEXT)
	$(LD) $< $(LDOPTS) $(LDFLAGS) $(LLIBS) $(LIBS) -lz -lcrypto $(EXENAMEFLAG)$@

test_tcp_client$(BINEXT): test_tcp_client$(OBJEXT)
	$(LD) $< $(LDOPTS) $(LDFLAGS) $(LLIBS) $(LIBS) -lz -lcrypto $(EXENAMEFLAG)$@

datatype_test_writer$(BINEXT): datatype_test_writer$(OBJEXT)
	$(LD) $< $(LDOPTS) $(LDFLAGS) $(LLIBS) $(LIBS) $(EXENAMEFLAG)$@

datatype_test_reader$(BINEXT): datatype_test_reader$(OBJEXT)
	$(LD) $< $(LDOPTS) $(LDFLAGS) $(LLIBS) $(LIBS) $(EXENAMEFLAG)$@

doc: objcomms.ps 

# Get general, invariant targets
EXTRA_DELETES = test_reader test_writer
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
# Revision 1.1  2006/07/01 00:31:55  endo
# CMDLi from MARS 2020 migrated into AO-FNC repository.
#
# Revision 1.1.1.1  2006/06/29 20:42:07  endo
# cmdli local repository.
#
# Revision 1.70  2004/11/08 19:36:51  doug
# redid header read/write so packs fields, instead of just copying over the header, which is 4 bytes per field, even for a char.
#
# Revision 1.69  2004/08/20 21:45:39  doug
# working on bud_monitor
#
# Revision 1.68  2004/08/20 17:29:42  doug
# added bud packets
#
# Revision 1.67  2004/08/13 20:56:17  doug
# three phase bombing works again
#
# Revision 1.66  2004/08/12 16:09:54  doug
# correct selection of multicast port, and allow passing port as part of multicast address using colon syntax in open method.
#
# Revision 1.65  2004/04/14 17:40:15  doug
# fixed endian issues
#
# Revision 1.64  2004/04/12 21:21:54  doug
# udp unicast, udp multicast, and tcp streams all work on linux and win32!
#
# Revision 1.63  2004/04/12 21:20:20  doug
# udp unicast, udp multicast, and tcp streams all work on linux and win32!
#
# Revision 1.62  2004/04/07 12:57:46  doug
# fix typo
#
# Revision 1.61  2004/04/05 22:19:22  doug
# multicasting works on linux.  Trying to build test_reader.exe on win32
#
# Revision 1.60  2004/03/26 22:30:55  doug
# making progress on win32 build
#
# Revision 1.59  2004/03/26 21:42:30  doug
# no encryption for win32
#
# Revision 1.58  2004/03/26 21:25:08  doug
# finally compiles under win32
#
# Revision 1.57  2004/03/26 16:57:19  doug
# switched to using make.target
#
# Revision 1.56  2004/03/26 00:18:58  doug
# working on win32 port
#
# Revision 1.55  2004/03/24 22:15:37  doug
# working on win32 port
#
# Revision 1.54  2004/03/12 20:36:43  doug
# snap
#
# Revision 1.53  2004/03/08 14:52:17  doug
# cross compiles on visual C++
#
# Revision 1.52  2004/02/28 19:26:14  doug
# builds clean on Fedora (commented out a matcon call)
#
# Revision 1.51  2004/02/28 16:35:22  doug
# getting cmdli to work in sara
#
# Revision 1.50  2004/02/20 20:55:13  doug
# barriers work
#
# Revision 1.49  2003/12/10 17:23:36  doug
# working on tasker
#
# Revision 1.48  2003/11/26 22:50:41  doug
# working on state machines
#
# Revision 1.47  2003/11/26 15:21:37  doug
# back from GaTech
#
# Revision 1.46  2003/11/07 22:54:32  doug
# working on cmdl_io
#
# Revision 1.45  2003/11/07 20:29:08  doug
# added cmdli interpreter library
#
# Revision 1.44  2003/11/03 22:43:42  doug
# adding defComposite support
#
# Revision 1.43  2003/10/22 20:34:14  doug
# still working on glyph_editor
#
# Revision 1.42  2003/10/07 21:02:40  doug
# working on open click
#
# Revision 1.41  2003/09/29 19:09:11  doug
# getting hooked into mouse events
#
# Revision 1.40  2003/08/25 22:01:53  doug
# builds against libccl_gui
#
# Revision 1.39  2003/08/22 21:23:52  doug
# Back from Benning
#
# Revision 1.38  2003/07/10 16:48:53  doug
# test harness for commMsg in and out functions
#
# Revision 1.37  2003/07/03 16:32:23  doug
# Fixed a nasty race condition where we would read the header on a UDP packet and the packet would get overwritten before we went back and read the body, leaving the header and the body not matching.
#
# Revision 1.36  2003/06/30 22:30:41  doug
# Added altitude to robots
#
# Revision 1.35  2003/06/27 19:13:43  doug
# got tomahawk to display
#
# Revision 1.34  2003/06/18 21:46:59  doug
# debugging the debug prints
#
# Revision 1.33  2003/06/16 21:44:10  doug
# need to clear isDirected when broadcast
#
# Revision 1.32  2003/04/28 21:44:15  doug
# tried using out of band data to break the select call, but doesn't work.  Will next try creating a pipe between the reader and writer threads to break the select.
#
# Revision 1.31  2003/04/25 22:13:11  doug
# fixed handling of dropped tcp connections and changed to use non-padding encryptBuffer calls on the packet header.
#
# Revision 1.30  2003/04/24 20:56:51  doug
# still coding on tcp support
#
# Revision 1.29  2003/04/23 20:57:13  doug
# still working on tcp support
#
# Revision 1.28  2003/04/22 23:51:46  doug
# fixed compile errors
#
# Revision 1.27  2003/04/21 17:43:27  doug
# working on adding tcp support to objcomms to handle large data streams point to point
#
# Revision 1.26  2003/04/17 20:33:05  doug
# Adding support for tcp connections
#
# Revision 1.25  2003/04/11 18:46:55  doug
# added export capabilities
#
# Revision 1.24  2003/04/07 12:44:42  doug
# moving to RedHat 9
#
# Revision 1.23  2003/03/21 15:55:33  doug
# moved libccl_code to the sara namespace
#
# Revision 1.22  2003/03/16 03:24:01  doug
# moved libccl_code to the sara namespace
#
# Revision 1.21  2003/03/13 22:42:07  doug
# seems to work
#
# Revision 1.20  2003/03/03 13:44:26  doug
# seems to work
#
# Revision 1.19  2003/02/27 22:43:01  doug
# seems to work
#
# Revision 1.18  2003/02/26 22:16:22  doug
# integrated with opcon
#
# Revision 1.17  2003/02/26 14:55:48  doug
# controller now runs with objcomms
#
# Revision 1.16  2003/02/14 21:47:13  doug
# links again
#
# Revision 1.15  2003/02/13 22:13:18  doug
# Added AckLead
#
# Revision 1.14  2003/02/13 16:17:00  doug
# waitFor seems to work
#
# Revision 1.13  2003/02/10 22:39:54  doug
# tested out well
#
# Revision 1.12  2003/02/06 20:29:30  doug
# trying to get the packet loss down
#
# Revision 1.11  2003/02/04 21:51:34  doug
# It sends and receives packets now
#
# Revision 1.10  2003/02/04 18:23:07  doug
# switched to sending senderID in every packet header
#
# Revision 1.9  2003/01/31 19:30:13  doug
# test1 builds and links!
#
# Revision 1.8  2003/01/31 15:02:24  doug
# It compiles again
#
# Revision 1.7  2003/01/31 00:47:26  doug
# compiles!
#
# Revision 1.6  2003/01/29 22:19:56  doug
# snapshot
#
# Revision 1.5  2003/01/28 22:01:10  doug
# snapshot
#
# Revision 1.4  2003/01/27 23:35:35  doug
# snapshot
#
# Revision 1.3  2003/01/24 22:40:03  doug
# getting closer
#
# Revision 1.2  2003/01/23 22:39:12  doug
# making progress
#
# Revision 1.1  2003/01/22 22:10:50  doug
# snapshot
#
#######################################################################



###########################################################################
# dependencies generated by makedepend

# DO NOT DELETE

