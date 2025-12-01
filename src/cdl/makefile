# makefile for cdl
#
# Douglas C. MacKenzie
#
# Copyright 1995, Georgia Tech Research Corporation 
# Atlanta, Georgia  30332-0415
# ALL RIGHTS RESERVED, See file COPYRIGHT for details. 

# $Id: makefile,v 1.1.1.1 2008/07/14 16:44:14 endo Exp $

include ../make.include

# ---------------------------------------------------  

CC = g++

CFLAGS = -g -Wall -ansi -pedantic -fstack-protector-all

INCS = -I$(MLAB_HOME)/src/include -I../load_cdl -I../load_rc -I$(MLAB_HOME)/src/ipt/include

#SRCS = main.cc mrpl_codegen.cc cnl_codegen.cc common_codegen.cc 
SRCS = main.cc sausages_codegen.cc cnl_codegen.cc common_codegen.cc 

OBJS=$(SRCS:cc=o)

LDFLAGS = -L/usr/X11R6/lib -lXt -lXm -lstdc++ -L/usr/lib/openmotif/

LDOPTIONS = $(MLAB_STATIC)

# Include this to add the sun malloc debugger
#LDOPTIONS = /usr/lib/debug/malloc.o

# Include this to add the electric fence malloc debugger
#LDOPTIONS = -static -L/users/d/doug/lib -lefence

# ----------------------------------------------------------------------
#                        COMPILATION RULES

.cc.o:
	$(CC) $(CFLAGS) $(INCS) -c $*.cc


all: cdl

cdl: $(OBJS)
	$(CC) -o cdl $(CFLAGS) version.cc \
		-DDATE="\"`date '+%T, %a %h %d, %Y'`\"" \
		$(OBJS) $(LDOPTIONS) \
		-L../load_cdl -lload_cdl -L../load_rc -lload_rc -L../../lib -lutilities $(LDFLAGS)

../load_cdl/cdl_gram.tab.h:
	cd ../load_cdl; make cdl_gram.tab.h

clean:
	rm -f *.o *.bak *~

veryclean: clean
	rm -f cdl


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


COUNT_SRCS = cnl_codegen.cc codegens.h main.cc mrpl_codegen.cc \
             version.cc version.h common_codegen.cc 

countlines:
	@echo "Line count for ./src/cdl:"
	@wc -l $(COUNT_SRCS)


depend:	
	

nodepend:
	

