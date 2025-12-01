# makefile for cnl
#
# Written by Douglas C. MacKenzie
#
# Copyright 1995 - 2006 Georgia Tech Research Corporation 
# Atlanta, Georgia  30332-0415
# ALL RIGHTS RESERVED, See file COPYRIGHT for details. 

# $Id: makefile,v 1.1.1.1 2008/07/14 16:44:17 endo Exp $

# ----------------------------------------------------------------------
#                           FLAGS ETC.
# ----------------------------------------------------------------------

include ../make.include

MACHINE := $(shell uname -m)

ifeq ($(MACHINE), x86_64)
LIBDIR = /usr/lib64
else
LIBDIR = /usr/lib
endif

CC = g++

CFLAGS = -g -Wall -ansi -pedantic
CFLAGS2 = -g -Wall -ansi

O_FILES = cnl_gram.o symtab.o parse.o cnl_lex.o scan.o codegen.o

INCS = -I$(MLAB_HOME)/src/include

LIBS = 

# ----------------------------------------------------------------------
#                        COMPILATION RULES
# ----------------------------------------------------------------------

.c.o:
	$(CC) $(CFLAGS) $(INCS) -c $*.c

all:	cnl

cnl	: $(O_FILES) version.c
	$(CC) $(CFLAGS) version.c -DDATE="\"`date '+%T, %a %h %d, %Y'`\"" \
		-o cnl $(O_FILES) -L$(LIBDIR)/flex-2.5.4a -lfl -lc $(LIBS)

c.o	: defs.h

cnl_gram.tab.h:	cnl_gram.c

cnl_gram.c: cnl_gram.y
		bison -tvd cnl_gram.y
		mv cnl_gram.tab.c cnl_gram.c

cnl_gram.o: cnl_gram.c
	$(CC) $(CFLAGS2) $(INCS) -c cnl_gram.c

cnl_lex.o: defs.h cnl_lex.c cnl_gram.y

cnl_lex.c: cnl_lex.l
		flex -B cnl_lex.l
		mv lex.yy.c cnl_lex.c

clean:
	rm -f *.o
	rm -f *.bak
	rm -f cnl_lex.c
	rm -f cnl_gram.tab.c
	rm -f cnl_gram.c
	rm -f cnl_gram.output
	rm -f cnl_gram.tab.h
	rm -f y.tab.h
	rm -f y.output
	rm -f cnl_gram.out

veryclean: clean
	rm -f cnl


checkin:
	@for f in `rlog -L -R RCS/*` ;\
		do ci -u $$f;\
	done

checkout: 
	@for f in `cd RCS;ls * | sed s/,v//g` ;\
		do if (test ! -f $$f) ; \
			then co $$f; \
		fi; \
	done

COUNT_SRCS = cnl_gram.y cnl_lex.l codegen.c defs.h parse.c scan.c symtab.c \
             version.c version.h

countlines:
	@echo "Line count for ./src/cnl:"
	@wc -l $(COUNT_SRCS)


DEPEND_SRCS = cnl_gram.y cnl_lex.l codegen.c parse.c scan.c symtab.c version.c

depend: cnl_gram.c cnl_gram.tab.h cnl_lex.c
	

nodepend:
	