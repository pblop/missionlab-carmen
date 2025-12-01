#########################################################################
#
# Makefile for telop
#
# Yoichiro Endo
#
# Copyright 2002 - 2006 Georgia Tech Research Corporation 
# Atlanta, Georgia  30332-0415
# ALL RIGHTS RESERVED, See file COPYRIGHT for details. 
#########################################################################

# $Id: Makefile,v 1.1.1.1 2008/07/14 16:44:24 endo Exp $

include ../make.include

TARGET = libtelop.a

SRCS = telop_window.c sound_simulation_window.c personality_window.c \
       motivational_vector_window.c UxXt.c UxMethod.c telop.c \
       sound_simulation.c motivational_vector.c adjust.c telop_joystick.c \
       sound_joystick.c assert.c telop_hw_joystick.c status_window.c

OBJS = $(SRCS:c=o)

INCLUDES = -I. -I$(MLAB_HOME)/src/include -I../mlab -I/usr/include/openmotif/

GPP = g++
GPPFLAGS = -Wall -g -ansi -pedantic -DMLAB_HOME=\"$(DESTDIR)\" -fpermissive
GPPDEFS = -DXT_CODE

all:
	$(MAKE) $(TARGET)

$(TARGET): $(OBJS)
	@echo Building $(TARGET)
	rm -f $(TARGET)
	ar rcv $(TARGET) $(OBJS)
	ranlib $(TARGET) 


depend:

nodepend:


clean:
	rm -f *.o *.bak *~

veryclean: clean
	rm -f *.a

.c.o:
#	$(GPP) -c $(GPPFLAGS) $(GPPDEFS) $(INCLUDES) $(LDLIBS) $*.c
	$(GPP) -c $(GPPFLAGS) $(GPPDEFS) $(INCLUDES) $*.c

