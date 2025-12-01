# Makefile for all of MissionLab 
#
# Copyright 1995 - 2006, Georgia Tech Research Corporation 
# Atlanta, Georgia  30332-0415
# ALL RIGHTS RESERVED, See file ../COPYRIGHT for details. 

export SHELL = csh

include make.include

# Makefile in this directory list should contain targets for all, depend,
# clean, veryclean.

MODULES = cnl \
		telop \
		utility_functions \
		EventLogParser \
		assistantDialog \
		cnp \
		cmdli \
		comm_behavior \
		libcnl \
		load_cdl \
		load_rc \
		write_cdl \
		cdl \
		cbrwizard \
		cbrplanner \
		MPPC \
	    acdl_plus \
		cfgedit \
		compass \
		terrain \
		mlab \
		path_planner \
		hardware_drivers \
		qlearn \
		cbr_behavioral_select \
		dstar \
		mrobot

ifdef DESTDIR
	INSTALL = "YES"
else
	DESTDIR = /usr
endif

ifeq ($(USE_IPC),0)
	PREPARE_MESSAGE_SERVER=prepare_iptserver
	BUILD_MESSAGE_SERVER=build_iptserver
else
	PREPARE_MESSAGE_SERVER=prepare_ipcserver
	BUILD_MESSAGE_SERVER=build_ipcserver
endif

all:
	-rm -rf ../include/ipt
	-rm -rf include/ipt
	-rm -rf ../lib/libipt.a
	$(MAKE) $(PREPARE_MESSAGE_SERVER)

ifdef INSTALL
	-cp -LRTf ../src/ ../tmp_src/ 
	-cp -LRTf ../demos/ ../tmp_demos/ 
	-cp -LRTf ../docs/ ../tmp_docs/
endif

	$(MAKE) $(BUILD_MESSAGE_SERVER)
	@echo "\nBuilding MissionLab...\n"
	$(foreach dir, $(MODULES), && make all -C $(dir))

prepare_iptserver:
	-ln -s ../ipt/include/ipt include/ipt
	-ln -s ../src/ipt/include/ipt ../include/ipt
	-ln -s  ../src/ipt/lib/libipt.a ../lib/libipt.a

build_iptserver:
	@echo "\n...building the IPT libraries...\n"
	cd ipt/src/communications/ipt; ./configure -unix LINUX -vx "" -install LINUX -main LINUX
	cd ipt && rm -rf bin && mkdir bin
	$(MAKE) install -C ipt/src/communications/ipt
	make all -C ipc
	rm -rf ipc/*.o
	ln -s -f ../src/ipt/bin/iptserver ../bin/iptserver
	
prepare_ipcserver:
	-ln -s ../ipcAdapter/ipt include/ipt
	-mkdir ../include/ipt

build_ipcserver:
	@echo "\n...building the IPC libraries...\n"
	make all -C ipc
	rm -rf ipc/*.o
	make all -C ipcAdapter
	ln -s -f ../bin/central ../bin/iptserver

install:
ifdef INSTALL
	-mkdir -p "$(DESTDIR)/usr"
	-cp -LRTf "../tmp_src/" "$(DESTDIR)/usr/src/"
	-cp -LRTf "../tmp_demos/" "$(DESTDIR)/usr/demos/"
	-cp -LRTf "../tmp_docs/" "$(DESTDIR)/usr/docs/"
	-cp -LRTf "../bin/" "$(DESTDIR)/usr/bin/"
	-cp -LRTf "../lib/" "$(DESTDIR)/usr/lib/"
	-cp -LRTf "../include/" "$(DESTDIR)/usr/include/"
	-cp -LRTf "../data/" "$(DESTDIR)/usr/data/"
	-rm -rf "../tmp_src"
	-rm -rf "../tmp_demos"
	-rm -rf "../tmp_docs"
	-rm -rf "$(DESTDIR)/usr/demos/mast_demos"
	-rm -f  "$(DESTDIR)/usr/src/make.include"
	-mv "../src/make.include.default" "$(DESTDIR)/usr/src/make.include"
	-mkdir -p "$(DESTDIR)/etc"
	-cp "../src/global/carmen-std.ini" "$(DESTDIR)/etc/carmen.ini"
endif
	
depend:
	$(foreach dir, $(MODULES), && $(MAKE) depend -C $(dir))

nodepend:
	$(foreach dir, $(MODULES), && $(MAKE) nodepend -C $(dir))


clean: 
	$(foreach dir, $(MODULES), && $(MAKE) -i clean -C $(dir))
	cd ipt && rm -rf bin
	cd ipt/src/communications/ipt && rm -rf LINUX
	-rm -rf ../tmp_src
	-rm -rf ../tmp_demos
	-rm -rf ../tmp_docs

veryclean: clean
	$(foreach dir, $(MODULES), && $(MAKE) -i veryclean -C $(dir))
	cd ipt/src/communications/ipt && rm -rf LINUX
	cd ipt/lib && rm -rf LINUX
	cd ipt && rm -rf bin
