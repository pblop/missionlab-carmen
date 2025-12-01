# Makefile for the library
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
include ../make.local

# ---------------------------------------------------

all:
	@for X in $(SUBS) ; do \
		$(MAKE) -C $$X all ; done

test:
	@for X in $(SUBS) ; do \
		$(MAKE) -C $$X test ; done

depend:
	@for X in $(SUBS) ; do \
		$(MAKE) -C $$X depend ; done

clean: 
	@for X in $(SUBS) ; do \
		$(MAKE) -C $$X clean ; done

veryclean: 
	rm -rf lib
	rm -rf include
	@for X in $(SUBS) ; do \
		$(MAKE) -C $$X veryclean ; done

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
# Revision 1.1.1.1  2003/11/07 20:27:33  doug
# initial import
#
#######################################################################
