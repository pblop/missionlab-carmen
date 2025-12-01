# Makefile for the MIC library
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

include $(SUPPORT_DIR)/make.include
include make.local

# ---------------------------------------------------

all:
	$(MAKE) -C src all

test:
	$(MAKE) -C src test

depend:
	$(MAKE) -C src depend

configure:
	@if test ! -d include ; then \
	   mkdir include;\
	   cd include;\
	   for X in $(SUBS) ; do \
	     ln -s ../src/$$X/*.h . ; done ;\
	fi
	@if test ! -d lib ; then \
	   mkdir lib ;\
	fi
	@if test ! -d lib/$(TARGET_OS) ; then \
	   mkdir lib/$(TARGET_OS) ;\
	fi

clean: 
	$(MAKE) -C src clean

veryclean: 
	rm -rf lib
	rm -rf include
	$(MAKE) -C src veryclean

#######################################################################
# $Log: makefile,v $
# Revision 1.1.1.1  2008/07/14 16:44:16  endo
# MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
#
# Revision 1.1.1.1  2006/07/20 17:17:46  endo
# MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
#
# Revision 1.1.1.1  2006/07/12 13:37:55  endo
# MissionLab 7.0
#
# Revision 1.1  2006/07/01 00:30:28  endo
# CMDLi from MARS 2020 migrated into AO-FNC repository.
#
# Revision 1.1.1.1  2006/06/29 20:42:07  endo
# cmdli local repository.
#
# Revision 1.5  2004/03/25 22:36:51  doug
# fixed configure
#
# Revision 1.3  2004/03/24 15:11:52  doug
# builds under win32
#
# Revision 1.2  2004/03/24 14:20:00  doug
# zlib compiles
#
# Revision 1.1  2002/10/01 20:15:41  doug
# Added makefiles
#
#
#######################################################################
