#CARMEN_HOME = ../../../
include ../../Makefile.conf

MODULE_NAME = "9000PRO"
MODULE_COMMENT = "Publishes images from a Logitech 9000 pro camera"

SOURCES = 9000pro.cpp 9000pro_lib.cpp
TARGETS = lib9000pro.a 

all: 

lib9000pro.a: 9000pro.o 9000pro_lib.o

9000pro.o: 9000pro.cpp

9000pro_lib.o: 9000pro_lib.cpp

# rules

include ../../Makefile.rules
