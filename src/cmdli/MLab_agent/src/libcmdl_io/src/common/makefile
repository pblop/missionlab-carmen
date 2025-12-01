# Makefile 
#
# Written by: Douglas C. MacKenzie
#
# Copyright 1996-2003.  All Rights Reserved.
# Mobile Intelligence Corporation
# Livonia, Michigan, USA
#
#********************************************************************

# $Id: makefile,v 1.1.1.1 2008/07/14 16:44:15 endo Exp $

# ---------------------------------------------------

# Get system wide defaults
include $(SUPPORT_DIR)/make.include
include ../../make.local

# ---------------------------------------------------

CC_SRCS = $(wildcard *.cc)
Y_SRCS = cmdl_parser.y
L_SRCS = cmdl_scanner.l

# ----------------------------------------------------------------------
LFLAGS += -d -i -Psara_cmdl_
YFLAGS += -d -p sara_cmdl_ -b cmdl_parser

# ----------------------------------------------------------------------

INCLUDES = -I. \
	-I$(LIBMIC_DIR)/include \
	-I$(LIBRC_DIR)/include \
	-I$(LIBOBJTHREADS_DIR)/include \
	-I$(LIBIPC_SUPPORT_DIR)/include \
	-I$(LIBENV_OBJECTS_DIR)/include

LLIBS = -L../$(ADIR) -l$(ALIB) \
	-L$(LIBIPC_SUPPORT_DIR)/lib/$(TARGET_OS) -lipc_support \
	-L$(LIBENV_OBJECTS_DIR)/lib/$(TARGET_OS) -lenv_objects \
	-L$(LIBOBJTHREADS_DIR)/lib/$(TARGET_OS) -lobjthreads \
	-L$(LIBMIC_DIR)/lib/$(TARGET_OS) -lmic -lpthread

all: ../$(ARCHIVE)($(OBJS))

test$(BINEXT): test$(OBJEXT)
	$(LD) $< $(LDOPTS) $(LDFLAGS) $(LLIBS) $(LIBS) $(EXENAMEFLAG)$@

# Get general, invariant targets
include $(SUPPORT_DIR)/make.targets

######################################################################


# DO NOT DELETE

