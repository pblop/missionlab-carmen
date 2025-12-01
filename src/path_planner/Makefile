##
## Makefile for the path_planner program
## Linux Version
##

MACHINE := $(shell uname -m)

ifeq ($(MACHINE), x86_64)
LIBDIR = /usr/lib64
else
LIBDIR = /usr/lib
endif

#CC = gcc -Wall -DSUN 
CC = g++
# generate profiling information for prof
#CC = gcc -Wall -DSUN -p

#CFLAGS = -g -I/u5/users/projects/libs
CFLAGS = -c -Wall -g -ansi -pedantic
#LDOPTIONS = -static
LOCAL_LIBRARIES =

SYS_LIBRARIES = -L$(LIBDIR)/termcap -L$(LIBDIR)/curses -lm -lcurses -ltermcap -lX11 -lXext
LDLIBS = -L/usr/X11R6/lib

SRCS =  main.c \
	map_editor.c map_display.c window.c vfc_util.c cart_lib.c map_bldr.c\
	map_glob.c nav.c generateCDL.c

OBJS = main.o \
       map_editor.o map_display.o window.o vfc_util.o cart_lib.o map_bldr.o\
       map_glob.o nav.o generateCDL.o

.KEEP_STATE:

 PROGRAM = path_planner

all:: path_planner

path_planner: $(OBJS) $(DEPLIBS)
	$(RM) $@
	$(CC) -o $@ $(OBJS) $(LDOPTIONS) $(LOCAL_LIBRARIES) $(LDLIBS) $(EXTRA_LOAD_FLAGS) $(SYS_LIBRARIES)

depend:
	@echo "Make depend is not necessary in this directory."

nodepend:
	@echo "No dependencies to remove."

clean::
	rm -f *.o

veryclean: clean
	rm -f path_planner
