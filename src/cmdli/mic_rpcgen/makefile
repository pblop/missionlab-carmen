# Makefile for the mic_rpcgeN CONTROLler 
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

include $(SUPPORT_DIR)/make.include
include make.local

# ---------------------------------------------------

all:
	$(MAKE) -C src all
	touch lib/$(TARGET_OS)/mic_rpcgen-dummy

test:
	$(MAKE) -C src test

depend:
	$(MAKE) -C src depend

configure:
	@if test ! -d bin ; then \
	   mkdir bin ;\
	   mkdir bin/$(TARGET_OS) ;\
	fi
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
	rm -rf bin
	$(MAKE) -C src veryclean

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
# Revision 1.1  2006/07/01 00:36:19  endo
# CMDLi from MARS 2020 migrated into AO-FNC repository.
#
# Revision 1.3  2006/06/30 23:41:32  endo
# *** empty log message ***
#
# Revision 1.2  2006/06/30 23:14:00  endo
# *** empty log message ***
#
# Revision 1.1.1.1  2006/06/29 20:42:07  endo
# cmdli local repository.
#
# Revision 1.1.1.1  2002/10/02 20:26:58  doug
# Initial import into cvs
#
#######################################################################
