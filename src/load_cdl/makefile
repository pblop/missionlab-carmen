# makefile for load_cdl
#
# Douglas C. MacKenzie
#
# Copyright 1995 - 2006 Georgia Tech Research Corporation 
# Atlanta, Georgia  30332-0415
# ALL RIGHTS RESERVED, See file COPYRIGHT for details. 

# $Id: makefile,v 1.1.1.1 2008/07/14 16:44:22 endo Exp $

# ----------------------------------------------------------------------
#                           FLAGS ETC.
# ----------------------------------------------------------------------

include ../make.include

CC = g++

CFLAGS = -g -Wall -ansi -pedantic -DMLAB_HOME=\"$(DESTDIR)\" -fpermissive
CFLAGS2 = -g -Wall -ansi -DMLAB_HOME=\"$(DESTDIR)\"

INCS = -I../include -I../ipt/include -I../cfgedit $(XINCS) -I/usr/include/openmotif/

SRCS = cdl_gram.cc cdl_lex.cc load_cdl.cc parse_support.cc print_symbols.cc \
       symbol.cc symtab.cc apply.cc ExpandSubtrees.cc TwoPoints.cc TwoWorldPoints.cc

OBJS=$(SRCS:cc=o)

# ----------------------------------------------------------------------
#                        COMPILATION RULES
# ----------------------------------------------------------------------

.cc.o:
	$(CC) $(CFLAGS) $(INCS) -c $*.cc

all: libload_cdl.a

libload_cdl.a: $(OBJS)
	rm -f libload_cdl.a 
	ar rcv libload_cdl.a $(OBJS)
	ranlib libload_cdl.a


cdl_gram.cc cdl_gram.tab.h: cdl_gram.y
		bison -tvd cdl_gram.y
		mv cdl_gram.tab.c cdl_gram.cc

cdl_gram.o: cdl_gram.cc
	$(CC) $(CFLAGS2) $(INCS) -c cdl_gram.cc

cdl_lex.o	: cdl_defs.h cdl_lex.cc cdl_gram.y

cdl_lex.cc	: cdl_lex.l
		flex cdl_lex.l
		mv lex.yy.c cdl_lex.cc

clean:
	rm -f *.o *.bak *~
	rm -f cdl_lex.cc
	rm -f cdl_gram.tab.c
	rm -f cdl_gram.cc
	rm -f cdl_gram.output
	rm -f y.tab.h
	rm -f y.output
	rm -f cdl_gram.out

veryclean: clean
	rm -f cdl_gram.tab.h
	rm -f libload_cdl.a


depend:	cdl_gram.cc cdl_lex.cc
	

nodepend:
	

