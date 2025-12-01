# ----------------------------------------------------------------------
# makefile for load_rc
#
# Douglas C. MacKenzie
#
# Copyright 1995 - 2006 Georgia Tech Research Corporation 
# Atlanta, Georgia  30332-0415
# ALL RIGHTS RESERVED, See file COPYRIGHT for details. 
#
# $Id: makefile,v 1.1.1.1 2008/07/14 16:44:22 endo Exp $
# ----------------------------------------------------------------------

# ----------------------------------------------------------------------
#                           FLAGS ETC.
# ----------------------------------------------------------------------

include ../make.include

CC = g++

CFLAGS = -g -Wall -ansi -pedantic -DMLAB_HOME=\"$(DESTDIR)\"
CFLAGS2 = -g -Wall -ansi -DMLAB_HOME=\"$(DESTDIR)\"

INCS = -I../include

SRCS = rc_gram.cc rc_lex.cc load_rc.cc

OBJS=$(SRCS:cc=o)

# ----------------------------------------------------------------------
#                        COMPILATION RULES
# ----------------------------------------------------------------------

all: libload_rc.a

.cc.o:
	$(CC) $(CFLAGS) $(INCS) -c $*.cc

libload_rc.a: $(OBJS)
	rm -f libload_rc.a 
	ar rcv libload_rc.a $(OBJS)
	ranlib libload_rc.a

rc_gram.cc rc_gram.tab.h: rc_gram.y
		bison -tvd -p rc_ rc_gram.y
		mv rc_gram.tab.c rc_gram.cc

rc_gram.o: rc_gram.cc
	$(CC) $(CFLAGS2) $(INCS) -c rc_gram.cc

rc_lex.o	: rc_defs.h rc_lex.cc rc_gram.y

rc_lex.cc	: rc_lex.l
		flex -Prc_ rc_lex.l
		mv lex.rc_.c rc_lex.cc

clean:
	rm -f *.o *.bak core
	rm -f rc_lex.cc
	rm -f rc_gram.tab.c
	rm -f rc_gram.cc
	rm -f rc_gram.output
	rm -f y.tab.h
	rm -f y.output
	rm -f rc_gram.out

veryclean: clean
	rm -f rc_gram.tab.h
	rm -f libload_rc.a

depend:	rc_gram.cc rc_lex.cc
	

nodepend:
	

