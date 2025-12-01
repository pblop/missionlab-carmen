# Makefile
#
# Copyright 2000, Georgia Tech Research Corporation 
# Atlanta, Georgia  30332-0415
# ALL RIGHTS RESERVED, See file ../COPYRIGHT for details. 
#
# $Id: makefile,v 1.1.1.1 2008/07/14 16:44:24 endo Exp $

export SHELL = csh

ROBOT_MODULES =	mlab-2.0-robot		\
		tmr_robot-1998

all:
	$(foreach dir, $(ROBOT_MODULES), && make all -C $(dir))

depend:
	$(foreach dir, $(ROBOT_MODULES), && $(MAKE) depend -C $(dir))

nodepend:
	@echo "No dependencies to remove"


clean: 
	$(foreach dir, $(ROBOT_MODULES), && $(MAKE) -i clean -C $(dir))

veryclean: 
	$(foreach dir, $(ROBOT_MODULES), && $(MAKE) -i veryclean -C $(dir))

checkout:
	$(foreach dir, $(ROBOT_MODULES), && $(MAKE) checkout -C $(dir))

######################################################################
# $Log: makefile,v $
# Revision 1.1.1.1  2008/07/14 16:44:24  endo
# MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
#
# Revision 1.2  2007/02/11 03:54:02  pulam
# Added nodepend condition to makefile to clear dependencies
#
# Revision 1.1.1.1  2006/07/20 17:17:51  endo
# MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
#
# Revision 1.1.1.1  2006/07/12 13:38:00  endo
# MissionLab 7.0
#
# Revision 1.1.1.1  2005/02/06 23:00:13  endo
# AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
#
# Revision 1.1  2000/08/01 19:16:44  endo
# Initial revision
#
#
#######################################################################
