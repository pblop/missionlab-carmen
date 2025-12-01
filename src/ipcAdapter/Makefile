include ../Makefile.conf

CFLAGS += -Wall -Wstrict-prototypes -Wnested-externs -Wmissing-prototypes -Wmissing-declarations -O3 -DREDHAT_52 -DREDHAT_6 -DREDHAT_71 -g -DTHREADED  -fstack-protector-all
IFLAGS += 
LFLAGS += -lpthread

ifdef LOUD_IPC
CFLAGS += -DDEFAULT_OPTIONS
endif

ifeq ($(PROCESSOR),x86_64)
CFLAGS += -fPIC
endif

ifndef NO_TCPD
CFLAGS += -DACCESS_CONTROL
LFLAGS = -lnsl -lwrap
endif

ifdef BIG_IPC_BUFFERS
CFLAGS += -DBIG_IPC_BUFFERS
endif

MODULE_NAME = ipcAdapter
MODULE_COMMENT = Adaptador para que MissionLab use IPC en vez de IPT de forma transparente

SOURCES = ipt.cc ../ipc/ipc.c ../ipc/queryResponse.c ../ipc/marshall.c ../ipc/timer.c ../ipc/list.c ../ipc/behaviors.c \
	../ipc/tcModError.c ../ipc/datamsg.c ../ipc/formatters.c ../ipc/hash.c ../ipc/idtable.c ../ipc/key.c \
	../ipc/primFmttrs.c ../ipc/reg.c ../ipc/sendMsg.c ../ipc/tcaMem.c ../ipc/tcaRef.c ../ipc/comModule.c ../ipc/com.c \
	../ipc/globalM.c ../ipc/globalMUtil.c ../ipc/strList.c ../ipc/modLogging.c ../ipc/modVar.c ../ipc/resMod.c \
	../ipc/parseFmttrs.c ../ipc/lex.c ../ipc/printData.c ../ipc/multiThread.c
	
PUBLIC_INCLUDES = ipt/ipt.h ipt/message.h ipt/connection.h ipt/callbacks.h ipt/spec.h
PUBLIC_LIBRARIES = libipt.a
PUBLIC_BINARIES = 
MAN_PAGES =

TARGETS = libipt.a

# rules

libipt.a:	ipt.o ../ipc/ipc.o ../ipc/queryResponse.o ../ipc/marshall.o ../ipc/timer.o ../ipc/list.o ../ipc/behaviors.o \
	../ipc/tcModError.o ../ipc/datamsg.o ../ipc/formatters.o ../ipc/hash.o ../ipc/idtable.o ../ipc/key.o \
	../ipc/primFmttrs.o ../ipc/reg.o ../ipc/sendMsg.o ../ipc/tcaMem.o ../ipc/tcaRef.o ../ipc/comModule.o ../ipc/com.o \
	../ipc/globalM.o ../ipc/globalMUtil.o ../ipc/strList.o ../ipc/modLogging.o ../ipc/modVar.o ../ipc/resMod.o \
	../ipc/parseFmttrs.o ../ipc/lex.o ../ipc/printData.o ../ipc/multiThread.o

include ../MakefileMissionlab.rules
