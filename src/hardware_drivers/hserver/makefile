# makefile for HServer
#
# William Halliburton
#
# Copyright 2000 - 2006, Georgia Tech Research Corporation
# Atlanta, Georgia  30332-0415
# ALL RIGHTS RESERVED, See file COPYRIGHT for details.

# $Id: makefile,v 1.1.1.1 2006/07/12 13:37:56 endo Exp $


include ../../make.include

CC = g++

#CFLAGS = -Wall -g -pedantic
CFLAGS = -g -Wno-long-long -fstack-protector-all `pkg-config --cflags libxml-2.0` -DNCURSES_INTERNALS
LDFLAGS = `pkg-config --libs libxml-2.0`

#Activate data log
#CFLAGS += -DLOG_POSE_DATA

CFLAGS+=-D_GNU_SOURCE -D_REENTRANT
LDFLAGS+=-lpthread -lrt -lstdc++

IPT_LIBS = $(MLAB_HOME)/lib

LIBS =  -L$(MLAB_HOME)/lib -L$(IPT_LIBS) -L./ -lpthread -lncurses -lpanel -lipt -lipc \
        -L./3DM-G -lm3dmg -L./PoseCalculator -lposecalc -L./PoseCalculator/Kalman -lkalman \
		-L./PoseCalculator/ParticleFilter -lpfilter $(LDFLAGS)

XLIBS = -L/usr/lib -lXt -lSM -lICE -lX11

INCS = -I$(MLAB_HOME)/include -I$(MLAB_HOME)/src/include -Ihserver_configuration -I. -I/usr/include/ncurses/

SRCS	= camera.c pioneer.c pioneer2.c amigobot.c roomba.c roomba560.c \
		laser.c terminal.c  Watchdog.c robot.c \
		message.c cognachrome.c ipt_handler.c jbox.c \
		RemoteControl.c fred.c statusbar.c video.c \
		xwindow.c apm_module.c nomad.c ndirect_hs.c robot_config.c \
		sensors.c report_level.c module.c console.c \
		gps.c gps_default.c gps_CARMEN.c compass.c compass_3DM-G.c gyro.c ipt_names.c hserver_configuration/HSRCFile.c \
		carmenRobot.c hserver_configuration/HServerConfig.c hserver_configuration/HServerConfigSection.c \
		hserver_configuration/RCFileReader.c hserver_configuration/HServerConfigurator.c \
		hserver_configuration/LabelToIdConversions.c LogManager.c webCam.c ipc_client.c

SRCS_MOBILITY = urban.c atrvjr.c gyro_dmu-vgx.c compass_kvh-c100.c

OBJS=$(SRCS:c=o)

OBJS_MOBILITY=$(SRCS_MOBILITY:c=o)

DEFINES = -D_POSIX_THREADS -D_POSIX_THREAD_SAFE_FUNCTIONS -D_REENTRANT  -DLINUX_OS `test -f /usr/include/linux/videodev.h && echo -DLINUX_VIDEODEV_PRESENT`

MOBILITY_INC = -I./mobility/tools/include -I./mobility/include/ -I.

MOBILITY = -Wall -Wno-unused -D_POSIX_THREADS -D_POSIX_THREAD_SAFE_FUNCTIONS \
			 -D_REENTRANT -DPOSIX -D__x86__ -D__linux__ -D__OSVERSION__=2 -g $(MOBILITY_INC)

MOBILITY_L = -L./mobility/lib/ -L./mobility/tools/lib -lmby -lidlmby -lomniDynamic2 -lomniORB2 -ltcpwrapGK -lomnithread

APM_SUPPORT = $(shell if (ls /usr/include/apm.h >& /dev/null) then echo 1; else echo 0; fi)

# ----------------------------------------------------------------------
#                        COMPILATION RULES
# ----------------------------------------------------------------------
.c.o:
	$(CC) $(CFLAGS) $(DEFINES) $(INCS) -c $*.c -o $*.o

.cpp.o:
	$(CC) $(CFLAGS) $(DEFINES) $(INCS) -c $*.cpp -o $*.o
	
all:
	$(MAKE) m3dmg
	$(MAKE) posecalc
	$(MAKE) hserver

hserver: tpuvis.yy.o $(OBJS) hserver_no_mobility.o ControlInterfaces_no_mobility.o
	$(CC) $(CFLAGS) $(DEFINES) -o hserver tpuvis.yy.o $(OBJS) hserver_no_mobility.o ControlInterfaces_no_mobility.o $(INCS) $(LIBS) $(XLIBS)

hserver_mobility: m3dmg posecalc tpuvis.yy.o hserver.o ControlInterfaces.o $(OBJS_MOBILITY) $(OBJS)
	$(CC) $(CFLAGS) -o hserver tpuvis.yy.o hserver.o ControlInterfaces.o $(OBJS_MOBILITY) $(OBJS) $(LIBS) $(XLIBS) $(MOBILITY_L)

m3dmg:
	$(MAKE) all -C ./3DM-G

posecalc:
	$(MAKE) all -C ./PoseCalculator
	
hserver.o: hserver.c hserver.h
	$(CC) $(CFLAGS) $(DEFINES) $(INCS) $(MOBILITY) -c hserver.c -o hserver.o

hserver_no_mobility.o: hserver.c hserver.h
	$(CC) $(CFLAGS) $(DEFINES) $(INCS) -DNO_MOBILITY -c hserver.c -o hserver_no_mobility.o

ControlInterfaces_no_mobility.o: ControlInterfaces.c ControlInterfaces.h
	$(CC) $(CFLAGS) $(DEFINES) $(INCS) -DNO_MOBILITY -c ControlInterfaces.c -o ControlInterfaces_no_mobility.o

ControlInterfaces.o: ControlInterfaces.c ControlInterfaces.h
	$(CC) $(CFLAGS) $(DEFINES) $(INCS) $(MOBILITY) -c ControlInterfaces.c -o ControlInterfaces.o

urban.o: urban.c urban.h
	$(CC) $(CFLAGS) $(DEFINES) $(INCS) $(MOBILITY) -c urban.c -o urban.o

atrvjr.o: atrvjr.c atrvjr.h
	$(CC) $(CFLAGS) $(DEFINES) $(INCS) $(MOBILITY) -c atrvjr.c -o atrvjr.o

gyro_dmu-vgx.o: gyro_dmu-vgx.c gyro_dmu-vgx.h
	$(CC) $(CFLAGS) $(DEFINES) $(INCS) $(MOBILITY) -c gyro_dmu-vgx.c -o gyro_dmu-vgx.o

compass_kvh-c100.o: compass_kvh-c100.c compass_kvh-c100.h
	$(CC) $(CFLAGS) $(DEFINES) $(INCS) $(MOBILITY) -c compass_kvh-c100.c -o compass_kvh-c100.o

tpuvis.yy.o: tpuvis.yy.c
	$(CC) $(CFLAGS) $(DEFINES) $(INCS) -c tpuvis.yy.c -o tpuvis.yy.o

tpuvis.yy.c:
	flex -otpuvis.yy.c tpuvis.yy

apm_module.o:
	$(CC) $(CFLAGS) $(DEFINES) -DAPM_SUPPORT=$(APM_SUPPORT) $(INCS) -c apm_module.c -o apm_module.o

clean:
	rm -f hserver *.o *.mo *.rpo tpuvis.yy.c core.*
	rm -f hserver_configuration/*.o hserver_configuration/*~
	$(MAKE) clean -C ./3DM-G
	$(MAKE) clean -C ./PoseCalculator

veryclean: clean

depend:
	

	
