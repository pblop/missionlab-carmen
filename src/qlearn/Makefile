# Qlearning
#
# Brian Ellenberger
#
# Copyright 1995, Georgia Tech Research Corporation 
# Atlanta, Georgia  30332-0415
# ALL RIGHTS RESERVED, See file COPYRIGHT for details. 

# ---------------------------------------------------

SRCS  = qlearn.cc

# ---------------------------------------------------

INCLUDES = -I/users/d/doug/include -I../include

CC = gcc
CFLAGS = -Wall -g -ansi -pedantic

OBJS=$(SRCS:cc=o) $(C_SRCS:c=o)

.cc.o:
	$(CC) -c $(CFLAGS) $(INCLUDES) $*.cc

.c.o:
	$(CC) -c $(CFLAGS) $(INCLUDES) $*.c

all: libqlearn.a 

libqlearn.a: $(OBJS)
	rm -f libqlearn.a
	ar rcv libqlearn.a $(OBJS)
	ranlib libqlearn.a

clean:
	rm -f *.o *.bak core

veryclean:
	rm -f *.o *.bak core

depend:
	@echo "Make depend is not necessary for this directory." 

nodepend:
	@echo "No dependencies to remove"


