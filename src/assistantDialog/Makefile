# ---------------------------------------------------------------------
#
#                               Makefile
#
#
#
#  Written by:  Yoichiro Endo
#
#  Copyright 2003 Georgia Tech Research Corporation
#  Atlanta, Georgia  30332-0415
#  ALL RIGHTS RESERVED, See file COPYRIGHT for details.
#
# ----------------------------------------------------------------------
# $Id: Makefile,v 1.1.1.1 2008/07/14 16:44:14 endo Exp $
# ----------------------------------------------------------------------

include ../make.include

ASSISTANT = libassistantDialog.a

GPP = g++

RM = rm -f -v

CFLAGS = -g

INCS = -I. -I$(MLAB_HOME)/src/include $(XINCS) $(EXTRA_INCS) -I/usr/include/openmotif/

SRCS = assistantDialog.cc

OBJS=$(SRCS:cc=o)

# ----------------------------------------------------------------------
#                        COMPILATION RULES
# ----------------------------------------------------------------------
.cc.o:
	$(GPP) $(CFLAGS) $(INCS) -c $*.cc -o $*.o

all: $(ASSISTANT)

$(ASSISTANT): $(OBJS)
	@ $(RM) $(ASSISTANT) 
	@ ar rcv $(ASSISTANT) $(OBJS)
	ranlib $(ASSISTANT)

clean:
	@ $(RM) $(OBJS)

veryclean: clean
	@ $(RM) $(ASSISTANT)

depend:
	

nodepend:
	