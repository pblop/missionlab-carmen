# utilities function library
#
# Douglas C. MacKenzie
#
# Copyright 1995 - 2006 Georgia Tech Research Corporation 
# Atlanta, Georgia  30332-0415
# ALL RIGHTS RESERVED, See file COPYRIGHT for details. 

# $Id: makefile,v 1.1.1.1 2008/07/14 16:44:25 endo Exp $

# ----------------------------------------------------------------------
#                           FLAGS ETC.
# ----------------------------------------------------------------------

include ../make.include

SRCS  = copyfile.cc strdupcat.cc extract_directories.cc find_file.cc \
	file_date.cc build_dir_list.cc strip_crs.cc file_utils.cc string_utils.cc \
	x_utils.cc sllist.cc convert_scale.cc

INCLUDES = -I$(MLAB_HOME)/src/include -I/usr/include/openmotif/

CC = g++
CFLAGS = -Wall -g -Dlinux -ansi -pedantic -DMLAB_HOME=\"$(DESTDIR)\"

OBJS=$(SRCS:cc=o) $(C_SRCS:c=o)

.cc.o:
	$(CC) -c $(CFLAGS) $(INCLUDES) $*.cc

.c.o:
	$(CC) -c $(CFLAGS) $(INCLUDES) $*.c

# ----------------------------------------------------------------------
#                        COMPILATION RULES
# ----------------------------------------------------------------------

all: libutilities.a

libutilities.a: $(OBJS)
	rm -f libutilities.a
	ar rcv libutilities.a $(OBJS)
	ranlib libutilities.a

clean:
	rm -f *.o *.bak core

veryclean: clean
	rm -f libutilities.a 

depend:
	

nodepend:
	

