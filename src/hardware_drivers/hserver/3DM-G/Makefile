# ----------------------------------------------------------------------
# cbrclient makefile
#
# Written by Yoichir Endo
#
# Copyright 2004 - 2006, Georgia Tech Research Corporation 
# Atlanta, Georgia  30332-0415
# ALL RIGHTS RESERVED, See file COPYRIGHT for details. 
#
# $Id: Makefile,v 1.1.1.1 2006/07/12 13:37:56 endo Exp $
# ----------------------------------------------------------------------

# ----------------------------------------------------------------------
#                           FLAGS ETC.
# ----------------------------------------------------------------------

TARGET = libm3dmg.a
TEST_PROG = m3dmg

SRCS = m3dmgAdapter.c m3dmgUtils.c m3dmgSerialLinux.c
TEST_SRCS =  m3dmg.c

INCLUDES = -I.

LDLIBS =

GPP = g++

GPPFLAGS = -Wall -g -DLINUX_OS -ansi -pedantic

OBJS = $(SRCS:c=o)
TEST_OBJS = $(TEST_SRCS:c=o)

RM = rm -f -v

AR = ar rcv

RANLIB = ranlib

# ----------------------------------------------------------------------
#                        COMPILATION RULES
# ----------------------------------------------------------------------

all:
	$(MAKE) $(TARGET)
	$(MAKE) $(TEST_PROG)

.c.o:
	$(GPP) -c $(GPPFLAGS) $(INCLUDES) $(PTHREAD_DEFS) $*.c

$(TEST_PROG): $(OBJS) $(TEST_OBJS)
	$(GPP) $(GPPFLAGS) -o $@ $(TEST_OBJS) -L./ -lm3dmg

$(TARGET): $(OBJS)
	$(RM) $(TARGET)
	$(AR) $(TARGET) $(OBJS)
	$(RANLIB) $(TARGET)

clean:
	@ $(RM) *.o core.* *~ 
	@ $(RM) $(TARGET) $(TEST_PROG)

veryclean: clean

depend:
	

