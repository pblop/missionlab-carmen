# Make file for libmic/common 
#
# Written by: Douglas C. MacKenzie
#
# Copyright 1996-2002.  All Rights Reserved.                      
# Mobile Intelligence Corporation                                
# Livonia, Michigan, USA                                           
#                                                              
#********************************************************************

# $Id: makefile,v 1.1.1.1 2008/07/14 16:44:15 endo Exp $

# ---------------------------------------------------

# Get system wide defaults
include $(SUPPORT_DIR)/make.include
#include ../../make.local

# ---------------------------------------------------

DIRS = 	../libenv_objects \
	../libcmdli \
	../libcmdl_io \
	../libobjcomms \
	../libobjthreads \
	../librc \
	../libipc_support \
	../mic_rpcgen \
	../libipc_robot2sim \
	../libipc_robot2con \
	../libmic

all:
	rm -rf lib include tmp doc src
#	rm -f cmdli.tz
	mkdir lib
	mkdir include
	mkdir include/cmdli
	mkdir tmp
	mkdir doc
	mkdir src
	@for i in $(DIRS) ;\
		do cp -rf $$i src/. ;\
	done
	@for i in src/* ;\
		do cd $$i ;\
		make veryclean ;\
		cd .. ;\
	done
	cp -rf ../support src/.
	mkdir src/MLab_agent
	cp makefile src/MLab_agent/.
	@for i in $(DIRS) ;\
		do cp $$i/include/* include/cmdli/ ;\
	done
	@for i in $(DIRS) ;\
		do cp $$i/lib/linux/* tmp/. ;\
	done
	@for i in $(DIRS) ;\
		do if test -e $$i/src/common/*.doc ;\
		   then cp -f $$i/src/common/*.doc doc/. ;\
		   fi ;\
	done
	@(cd tmp; for i in `ls *.a` ;\
		do ar x $$i ;\
	done; rm -f *.a)
	@(cd tmp; ar r libmic_cmdli.a *.o; mv libmic_cmdli.a ../lib/.; cd ..; rm -rf tmp)
#	tar czf cmdli.tz lib include doc src

clean:
#	rm -rf lib include tmp doc cmdli.tz
	rm -rf lib include tmp doc

veryclean: clean

configure:
	echo ""

depend:
	echo ""

# Get general, invariant targets
#include $(SUPPORT_DIR)/make.targets

#######################################################################
# $Log: makefile,v $
# Revision 1.1.1.1  2008/07/14 16:44:15  endo
# MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
#
# Revision 1.1.1.1  2006/07/20 17:17:45  endo
# MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
#
# Revision 1.1.1.1  2006/07/12 13:37:54  endo
# MissionLab 7.0
#
# Revision 1.3  2006/07/04 16:54:39  endo
# CMDLI.cnl compiling error fixed.
#
# Revision 1.2  2006/07/02 17:46:00  endo
# cmdli configure improved.
#
# Revision 1.1  2006/07/01 00:36:58  endo
# CMDLi from MARS 2020 migrated into AO-FNC repository.
#
# Revision 1.4  2006/06/30 23:41:46  endo
# *** empty log message ***
#
# Revision 1.3  2006/06/30 23:14:10  endo
# *** empty log message ***
#
# Revision 1.2  2006/06/30 22:49:03  endo
# Compiler error fix.
#
# Revision 1.1.1.1  2006/06/29 20:42:07  endo
# cmdli local repository.
#
# Revision 1.6  2004/10/30 00:30:00  doug
# Sending out to MARS2020
#
# Revision 1.5  2004/09/17 15:57:49  doug
# added docs
#
# Revision 1.4  2004/09/17 00:21:08  doug
# string_editor displays the label
#
# Revision 1.3  2004/07/30 13:51:32  doug
# snap
#
# Revision 1.2  2004/06/11 21:11:58  doug
# added configure rule
#
# Revision 1.1  2004/05/11 19:42:34  doug
# create a library for mlab
#
#######################################################################


