# ----------------------------------------------------------------------
# makefile
#
# Written by Yoichir Endo
#
# Copyright 2002 - 2006, Georgia Tech Research Corporation 
# Atlanta, Georgia  30332-0415
# ALL RIGHTS RESERVED, See file COPYRIGHT for details. 
#
# $Id: Makefile,v 1.1.1.1 2008/07/14 16:44:17 endo Exp $
# ----------------------------------------------------------------------

# ----------------------------------------------------------------------
#                           FLAGS ETC.
# ----------------------------------------------------------------------

export SHELL = csh

TARGET_SO = libCommBehavior.so
TARGET_A = libCommBehavior.a

TEST_PROG = TestCommBehavior

GENERATE_PLAN = GeneratePlan

BEHAVIOR_LIB_DIRS = InternalizedPlan RecoveryBehavior PreservationBehavior

SRCS = CommBehavior.cpp CBehaviorCoordinator.cpp CJboxInterface.cpp \
       SensorData.cpp

TEST_SRCS = TestCommBehavior.cpp

INCLUDES = -I. -I./RecoveryBehavior -I./InternalizedPlan -I./PreservationBehavior -I../include

LDLIBS = -L./InternalizedPlan -lInternalizedPlan -L./RecoveryBehavior -lRecoveryBehavior\
 -L./PreservationBehavior -lPreservationBehavior

GPP = g++

GPPFLAGS = -Wall -g -fPIC -ansi -pedantic

OBJS = $(SRCS:cpp=o)

TEST_OBJS = $(TEST_SRCS:cpp=o)

RM = rm -f -v

AR = ar rcv

ARX = ar xv

RANLIB = ranlib

LD = g++

LDFLAGS = -shared

# ----------------------------------------------------------------------
#                        COMPILATION RULES
# ----------------------------------------------------------------------

all:
#	$(MAKE) $(TARGET_SO)
	$(MAKE) $(TARGET_A)
	$(MAKE) $(TEST_PROG)
	$(MAKE) $(GENERATE_PLAN)

.cpp.o:
	$(GPP) -c $(GPPFLAGS) $(INCLUDES) $*.cpp

$(TARGET_SO): $(OBJS)
	$(MAKE) behaviorlibs
	$(RM) $(TARGET_SO)
	$(LD) $(LDFLAGS) $(OBJS) $(LDLIBS) -o $(TARGET_SO)

$(TARGET_A): $(OBJS)
	$(MAKE) behaviorlibs
	$(RM) $(TARGET_A)
	$(ARX) ./InternalizedPlan/libInternalizedPlan.a
	$(ARX) ./RecoveryBehavior/libRecoveryBehavior.a
	$(ARX) ./PreservationBehavior/libPreservationBehavior.a
	$(AR) $(TARGET_A) *.o
	$(RANLIB) $(TARGET_A)

$(TEST_PROG): $(TEST_OBJS)
	$(GPP) -Wall -g -o $@ $(TEST_OBJS) -L./ -lCommBehavior

$(GENERATE_PLAN):
	$(MAKE) -C InternalizedPlan $(GENERATE_PLAN)

behaviorlibs:
	$(foreach DIR, $(BEHAVIOR_LIB_DIRS), && $(MAKE) -C $(DIR))

clean:
	$(foreach DIR, $(BEHAVIOR_LIB_DIRS), && $(MAKE) -i clean -C $(DIR))
	@ $(RM) *.o core *~ 

veryclean:
	$(foreach DIR, $(BEHAVIOR_LIB_DIRS), && $(MAKE) -i veryclean -C $(DIR))
	@ $(RM) $(TARGET_SO)
	@ $(RM) $(TARGET_A)
	@ $(RM) $(TEST_PROG)
	@ $(RM) *.o core *~ 

depend:
	$(foreach DIR, $(BEHAVIOR_LIB_DIRS), && $(MAKE) depend -C $(DIR))

nodepend:
	$(foreach DIR, $(BEHAVIOR_LIB_DIRS), && $(MAKE) nodepend -C $(DIR))

