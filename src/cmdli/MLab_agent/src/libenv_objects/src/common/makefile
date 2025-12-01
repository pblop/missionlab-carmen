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

CC_SRCS = $(wildcard *.cc)
Y_SRCS = env_parser.y
L_SRCS = env_scanner.l
YFLAGS += -p env_
LFLAGS += -Penv_

# ---------------------------------------------------

INCLUDES = -I../../include -I$(LIBMIC_DIR)/include \
	-I$(LIBOBJTHREADS_DIR)/include \
	-I$(LIBIPC_SUPPORT_DIR)/include 


../$(ARCHIVE): $(OBJS)
	$(AR) $(ARFLAGS) $@ $^
	
all: ../$(ARCHIVE)

doc: mic.ps mic_exception.ps

# Get general, invariant targets
include $(SUPPORT_DIR)/make.targets

#######################################################################
# $Log: makefile,v $
# Revision 1.1.1.1  2008/07/14 16:44:16  endo
# MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
#
# Revision 1.1.1.1  2006/07/20 17:17:46  endo
# MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
#
# Revision 1.1.1.1  2006/07/12 13:37:54  endo
# MissionLab 7.0
#
# Revision 1.1  2006/07/01 00:21:04  endo
# CMDLi from MARS 2020 migrated into AO-FNC repository.
#
# Revision 1.1.1.1  2006/06/29 20:42:07  endo
# cmdli local repository.
#
# Revision 1.62  2004/03/26 16:57:29  doug
# switched to using make.target
#
# Revision 1.61  2004/03/26 00:19:05  doug
# working on win32 port
#
# Revision 1.60  2004/03/12 20:36:53  doug
# snap
#
# Revision 1.59  2004/03/08 14:52:28  doug
# cross compiles on visual C++
#
# Revision 1.58  2004/03/02 18:49:07  doug
# fixed ATRV
#
# Revision 1.57  2004/03/02 14:41:06  doug
# oops, fix the typo
#
# Revision 1.56  2004/03/01 23:37:48  doug
# working on real overlay
#
# Revision 1.55  2004/02/28 19:26:24  doug
# builds clean on Fedora (commented out a matcon call)
#
# Revision 1.54  2004/02/28 16:35:32  doug
# getting cmdli to work in sara
#
# Revision 1.53  2004/02/20 20:55:24  doug
# barriers work
#
# Revision 1.52  2003/12/10 17:23:46  doug
# working on tasker
#
# Revision 1.51  2003/11/26 22:50:51  doug
# working on state machines
#
# Revision 1.50  2003/11/26 15:21:47  doug
# back from GaTech
#
# Revision 1.49  2003/11/07 22:54:42  doug
# working on cmdl_io
#
# Revision 1.48  2003/11/07 20:29:18  doug
# added cmdli interpreter library
#
# Revision 1.47  2003/11/03 22:43:53  doug
# adding defComposite support
#
# Revision 1.46  2003/10/22 20:34:24  doug
# still working on glyph_editor
#
# Revision 1.45  2003/10/07 21:02:50  doug
# working on open click
#
# Revision 1.44  2003/09/29 19:09:22  doug
# getting hooked into mouse events
#
# Revision 1.43  2003/08/25 22:02:02  doug
# builds against libccl_gui
#
# Revision 1.42  2003/08/22 21:24:01  doug
# Back from Benning
#
# Revision 1.41  2003/07/02 21:49:28  doug
# Trying to get tomahawk mission to work
#
# Revision 1.40  2003/06/30 22:30:51  doug
# Added altitude to robots
#
# Revision 1.39  2003/06/27 19:13:52  doug
# got tomahawk to display
#
# Revision 1.38  2003/06/17 20:43:03  doug
# debugging broadcast problem
#
# Revision 1.37  2003/06/16 19:48:46  doug
# Adding Tomahawk
#
# Revision 1.36  2003/04/25 22:13:43  doug
# still working on tcp support
#
# Revision 1.35  2003/04/07 12:44:51  doug
# moving to RedHat 9
#
# Revision 1.34  2003/03/25 21:45:13  doug
# moved libccl_code to the sara namespace
#
# Revision 1.33  2003/03/21 15:55:43  doug
# moved libccl_code to the sara namespace
#
# Revision 1.32  2003/03/15 19:46:04  doug
# working on opcon
#
# Revision 1.31  2003/03/13 22:42:15  doug
# seems to work
#
# Revision 1.30  2003/03/03 13:44:34  doug
# seems to work
#
# Revision 1.29  2003/02/26 22:16:30  doug
# integrated with opcon
#
# Revision 1.28  2003/02/26 14:53:12  doug
# controller now runs with objcomms
#
# Revision 1.27  2002/12/23 22:23:05  doug
# added draw trails support
#
# Revision 1.26  2002/11/20 22:17:28  doug
# can assign loiter areas in 2d
#
# Revision 1.25  2002/11/15 22:39:26  doug
# 2d images work
#
# Revision 1.24  2002/11/15 14:03:17  doug
# renamed robot class to robot_vehicle to remove conflict with the hardware library
#
# Revision 1.23  2002/11/14 22:34:15  doug
# working on photo loading
#
# Revision 1.22  2002/11/14 19:44:03  doug
# F16 works
#
# Revision 1.21  2002/11/14 15:25:09  doug
# snapshot
#
# Revision 1.20  2002/11/13 18:47:09  doug
# added boundary
#
# Revision 1.19  2002/11/13 18:18:39  doug
# added passagePoint
#
# Revision 1.18  2002/11/13 18:12:50  doug
# added battlePosition
#
# Revision 1.17  2002/11/13 17:52:09  doug
# added AttackPosition
#
# Revision 1.16  2002/11/13 17:40:44  doug
# assemblyArea added
#
# Revision 1.15  2002/11/13 17:15:50  doug
# objectives work!
#
# Revision 1.14  2002/11/07 23:31:43  doug
# 2d circles are working
#
# Revision 1.13  2002/11/06 19:12:01  doug
# compiles!
#
# Revision 1.12  2002/11/05 21:57:47  doug
# gaining
#
# Revision 1.11  2002/11/05 17:23:53  doug
# compiles
#
# Revision 1.10  2002/11/05 17:05:46  doug
# builds again
#
# Revision 1.9  2002/11/04 22:34:34  doug
# snapshot
#
# Revision 1.8  2002/11/01 22:44:33  doug
# snapshot
#
# Revision 1.7  2002/11/01 21:49:41  doug
# working on it
#
# Revision 1.6  2002/10/30 21:08:13  doug
# need to zero the width and height fields
#
# Revision 1.5  2002/10/23 19:39:25  doug
# stringstreams changed from gcc 2.9 to 3.0
#
# Revision 1.4  2002/10/07 19:32:31  doug
# update
#
# Revision 1.3  2002/10/04 17:48:21  doug
# tweaking
#
# Revision 1.2  2002/10/03 15:32:29  doug
# fixed include in makefile
#
# Revision 1.1.1.1  2002/10/02 15:51:15  doug
# Initial import into cvs
#
#
#######################################################################



###########################################################################
# dependencies generated by makedepend

# DO NOT DELETE

