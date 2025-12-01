#########################################################################
#
#       Makefile for compass
#
#       MAKEFILE FOR COMPASS LIBRARY
#
#       MAIN            is the .c file containing your main() function 
#       INTERFACES      is a list of the generated C code files
#       APP_OBJS        is a (possibly empty) list of the object code
#                       files that form the non-interface portion of
#                       your application
#
#       In the first three statements, the variables on the right 
#	of the equal sign will be replaced with their corresponding
#       values when the makefile is automatically generated.
#
#       This template is used for makefiles which do not reference 
#       the Ux runtime library.
#
#  	$Date: 2008/07/14 16:44:17 $ 		$Revision: 1.1.1.1 $
#########################################################################

# $Id: Makefile,v 1.1.1.1 2008/07/14 16:44:17 endo Exp $

include ../make.include

INTERFACES	= compass_win.c

LANGUAGE        = ANSI C
APPL_OBJS       = UxXt.o UxMethod.o
LIBRARY         = libcompass.a


UX_LIBPATH	= $(UX_DIR)/lib
X_LIBS		= -lXm -lXt -lX11

X_CFLAGS        = -I$(XINCDIR)
X_LIBPATH       = -L$(XLIBDIR)

# For Centerline K&R cc
# KR_CC         = clcc -traditional
# KR_CFLAGS     =
# For sun4 K&R cc
KR_CC           = cc
KR_CFLAGS       = -D_NO_PROTO
# For GNU K&R cc
# KR_CC         = gcc
# KR_CFLAGS     = -traditional -D_NO_PROTO
# For Centerline ansi cc
# ANSI_CC         = clcc
# ANSI_CFLAGS     = 
# For GNU ansi cc
ANSI_CC         = gcc
ANSI_CFLAGS     = -ansi -Dsparc -g
# For Centerline c++
# CPLUS_CC        = CC
# CPLUS_CFLAGS    = 
# For GNU c++
CPLUS_CC        = gcc
CPLUS_CFLAGS    = -xc++
CFLAGS		= -DXT_CODE $(X_CFLAGS) -I../include -I/usr/include/openmotif/
LIBPATH         = $(X_LIBPATH)
LIBS		= $(X_LIBS) -lm


OBJS = $(INTERFACES:.c=.o) $(APPL_OBJS)

 
$(LIBRARY): $(OBJS)
	@echo Building Library $(LIBRARY)
	rm -f $(LIBRARY)
	ar rcv $(LIBRARY) $(OBJS)
	ranlib $(LIBRARY) 
	@echo "Done"

all: library

library: $(LIBRARY)

depend:
	@echo "Make depend is not necessary for this directory."

nodepend:
	@echo "No dependencies to remove"

clean:
	rm -f *.o *.bak core

veryclean: clean
	rm -f *.a

checkout:
	@echo "Make checkout is not necessary for this directory."

.SUFFIXES: .o .c .c

#.c.o:
#	@echo Compiling $< [$(LANGUAGE)] [XT-CODE]
#	$(CC) -c $(CFLAGS) $< -o $@
.c.o:
	@echo Compiling $< [$(LANGUAGE)] [XT-CODE]
	$(CC) -c $(CFLAGS) $< -o $@



CC = \
@`if [ "$(LANGUAGE)" = "C++" ]; then echo $(CPLUS_CC) $(CPLUS_CFLAGS);fi` \
`if [ "$(LANGUAGE)" = "ANSI C" ]; then echo $(ANSI_CC) $(ANSI_CFLAGS); fi`\
`if [ "$(LANGUAGE)" = "KR-C" ]; then echo $(KR_CC) $(KR_CFLAGS); fi`

LD = \
@`if [ "$(LANGUAGE)" = "C++" ]; then echo $(CPLUS_CC);fi` \
`if [ "$(LANGUAGE)" = "ANSI C" ]; then echo $(ANSI_CC); fi`\
`if [ "$(LANGUAGE)" = "KR-C" ]; then echo $(KR_CC); fi`

UxXt.h:
	co -f UxXt.h

compass_win.o:	UxXt.h

UxXt.o: UxXt.h

UxMethod.o: UxXt.h

######################################################################
# $Log: Makefile,v $
# Revision 1.1.1.1  2008/07/14 16:44:17  endo
# MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
#
# Revision 1.2  2007/02/11 03:57:45  pulam
# Added nodepend build condition.
#
# Revision 1.1.1.1  2006/07/20 17:17:47  endo
# MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
#
# Revision 1.1.1.1  2006/07/12 13:37:55  endo
# MissionLab 7.0
#
# Revision 1.1.1.1  2005/02/06 23:00:08  endo
# AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
#
# Revision 1.14  2003/04/06 13:12:54  endo
# gcc 3.1.1
#
# Revision 1.13  1999/12/16 23:34:02  mjcramer
# rh6 porting
#
# Revision 1.11  1999/05/25  20:10:17  endo
# This makefile was modified from telop.
#
# Revision 1.13  1996/12/20 05:47:09  tucker
# added make all
#
# Revision 1.12  1995/07/13  22:09:43  jmc
# Added checkin and checkout.
#
# Revision 1.11  1995/06/29  20:13:24  jmc
# Added countlines target.
#
# Revision 1.10  1995/06/23  20:35:47  jmc
# Another hack to accomodate new src/make.include file.
#
# Revision 1.9  1995/06/23  20:19:28  jmc
# Modified to accomodate the new src/make.include file.
#
# Revision 1.8  1995/06/15  18:36:45  jmc
# Modified to include make.include for various directories, etc.
# Minor cleanup.
#
# Revision 1.7  1995/06/05  19:01:38  kali
# added the interface personality_window to be compiled
#
# Revision 1.6  1995/05/09  17:42:31  kali
# *** empty log message ***
#
# Revision 1.5  1995/04/14  18:56:57  jmc
# Added RCS id and log strings.
######################################################################
