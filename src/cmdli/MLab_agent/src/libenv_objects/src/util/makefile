# Makefile 
#
# Written by: Douglas C. MacKenzie
#
# Copyright 1996-2003.  All Rights Reserved.
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

# ----------------------------------------------------------------------

INCLUDES = -I../../include -I../common \
	-I$(LIBMIC_DIR)/include \
	-I$(LIBRC_DIR)/include \
	-I$(LIBOBJTHREADS_DIR)/include \
	-I$(LIBIPC_SUPPORT_DIR)/include \
	-I$(LIBENV_OBJECTS_DIR)/include

LLIBS = -L../$(ADIR) -l$(ALIB) \
	-L$(LIBIPC_SUPPORT_DIR)/lib/$(TARGET_OS) -lipc_support \
	-L$(LIBENV_OBJECTS_DIR)/lib/$(TARGET_OS) -lenv_objects \
	-L$(LIBRC_DIR)/lib/$(TARGET_OS) -lrc \
	-L$(LIBOBJTHREADS_DIR)/lib/$(TARGET_OS) -lobjthreads \
	-L$(LIBMIC_DIR)/lib/$(TARGET_OS) -lmic -lpthread


all: test_loader$(BINEXT)


test_loader$(BINEXT): test_loader$(OBJEXT)
	$(LD) $< $(LDOPTS) $(LDFLAGS) $(LLIBS) $(LIBS) $(EXENAMEFLAG)$@

# Get general, invariant targets
include $(SUPPORT_DIR)/make.targets

clean:
	rm -f *.o *.obj *.bak core core.* *.exe *.map *.lst *.ps *.pdb *.output *.idb *.pdb test_loader$(BINEXT) 

######################################################################


# DO NOT DELETE

