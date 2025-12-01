# makefile for libcnl
#
# Douglas C. MacKenzie
#
# Copyright 1995 - 1997, 1999 - 2003, Georgia Tech Research Corporation
# Atlanta, Georgia  30332-0415
# ALL RIGHTS RESERVED, See file COPYRIGHT for details.

# $Id: makefile,v 1.1.1.1 2006/07/12 13:37:59 endo Exp $

# No se compilan:
# AVOID_OBSTACLES_MEM.cnl (ObstacleReadings no definido)
# CNP_IS_TASK_CANCELED.cnl (exec_cnp_is_task_canceled no definido)
# CNP_IS_TASK_COMPLETE.cnl (exec_cnp_is_task_complete no definido)
# SEGUIR_LIDER-DSTAR.cnl (varias variables no declaradas)
# GET_CAMERA_POSITION.cnl (exec_get_camera_position no definido)

include ../make.include

.SUFFIXES: .cnl .cc $(.SUFFIXES)

CC = g++

CNLBIN = $(MLAB_HOME)/bin

CFLAGS = -g -Wall -fstack-protector-all

INCLUDES = -I../include $(IPT_INCS)


SRCS = ADD_OBJECT.cnl                 GET_PRY.cnl \
AERIAL_AVOID_GROUND.cnl               GET_ROBOT_STATE.cnl \
AERIAL_AVOID_OBSTACLES.cnl            GET_XY.cnl \
AERIAL_AVOID_ROBOT.cnl                GET_YAW.cnl \
AERIAL_GLOBAL_TO_EGOCENTRIC.cnl       GLOBAL_TO_EGOCENTRIC.cnl \
AERIAL_MAINTAIN_ALTITUDE.cnl          GUIAR.cnl \
AERIAL_MOVE_TO_GOAL.cnl               GUIAR_ROBOT.cnl \
AERIAL_NOISE.cnl                      HALT.cnl \
AERIAL_TELOP.cnl                      HAS_STOPPED.cnl \
ALERT.cnl                             HAS_TURNED.cnl \
AND.cnl                               HEAD_REL_UNIT_V.cnl \
ASPIRAR560.cnl                        IN_HALLWAY.cnl \
ASPIRAR.cnl                           IN_ROOM.cnl \
AVOID_OBJECTS.cnl                     INTERCEPT.cnl \
AVOID_OBSTACLES.cnl                   IR_A_ROOMBA.cnl \
IR_POSICION_OBJETIVO.cnl \
AVOID_PAST.cnl                        IR_POSICON_ANT_LIDER.cnl \
AVOID_ROBOT.cnl                       IS_A_FROBOT.cnl \
AVOID_STATIC_OBSTACLES.cnl            IS_AN_OBJECT.cnl \
CBR_AGENT.cnl                         IS_AN_OBJECT_MOTIVATED.cnl \
CBR_GET_CASE.cnl                      IS_AT_GOAL.cnl \
CHANGE_MOTIVATIONAL_VECTOR.cnl        ISFACING.cnl \
CHANGE_ROBOT_COLOR.cnl                IS_TARGET_UNTRACKABLE.cnl \
CLOSEST_OBJECT.cnl                    IS_VALID_OBJECT.cnl \
CLOSEST_OBSTACLE.cnl                  LAST_BUTTON_PRESS.cnl \
CMDLI.cnl                             LAST_KEY_PRESS.cnl \
CNP_BID_ON_TASK.cnl                   LEAVE_ROOM.cnl \
CNP_GOT_A_TASK.cnl                    LOOK_TO.cnl \
CNP_INJECT_TASK.cnl                   MAINTAIN_FORM.cnl \
CNP_IS_AUCTION_ENDED.cnl              MAKE_UNIT_VECTOR.cnl \
CNP_IS_AUCTION_FAILED.cnl             MAP_LOCATION.cnl \
MARK_DOORWAY.cnl \
CNP_IS_TASK_CHANGED.cnl               MARK_OBJECT.cnl \
MDL_coordinator.cnl \
CNP_IS_TASK_LOST.cnl                  MNAV_OBSTACLES.cnl \
CNP_IS_TASK_WON.cnl                   MNAV_TO_GOAL.cnl \
CNP_NOTIFY_TASK_COMPLETED.cnl         MOTIVATIONAL_UPDATE.cnl \
CNP_NOTIFY_WON_TASK_STARTED.cnl       MOTIVATIONS.cnl \
CNP_SAVE_TARGET_LOCATION.cnl          MOVE_AHEAD.cnl \
CNP_SAVE_TARGET_VEHICLE_TYPE.cnl      MOVED_DISTANCE.cnl \
CNP_SAVE_TARGET_VELOCITY.cnl          MOVE_FORWARD.cnl \
CNP_TRACK.cnl                         MOVE_ROBOT.cnl \
COMMUNICATION_SENSITIVE_BEHAVIOR.cnl  MOVE_TO.cnl \
COMPUTE_A.cnl                         MOVE_TO_FREE_SPACE.cnl \
COMPUTE_B.cnl                         MOVE_TO_GOAL.cnl \
COMPUTE_F.cnl                         MOVE_TO_GOAL_DSTAR.cnl \
COMPUTE_FORM_POSITION.cnl             MOVE_TO_GUARDED.cnl \
COMPUTE_G.cnl                         MRV2_DRIVE.cnl \
COMPUTE_REP.cnl                       MUST_FOLLOW.cnl \
COMPUTE_T.cnl                         MUST_LEAVE.cnl \
COMPUTE_U.cnl                         NODES.cnl \
COMPUTE_ULIM.cnl                      NOISE.cnl \
COMPUTE_UNIT_CENTER.cnl               NOOP.cnl \
CONSTANT.cnl                          NOT.cnl \
COOP.cnl                              NOTIFY.cnl \
COUNT_OBJECTS.cnl                     NOTIFY_GOALS.cnl \
CURRENT_TARGET.cnl                    NUEVA_TAREA.cnl \
DATABASE.cnl                          NULL_MS.cnl \
DATABASE_VEC.cnl                      OBJECT_LOCATION.cnl \
DEBUG_PROC.cnl                        OR.cnl \
DEBUG_SHOW_INPUT.cnl                  PANTILT.cnl \
DEBUG_SHOW_INPUT_PRY.cnl              PICKUP_OBJECT.cnl \
DETECT_ALTERNATE_HALLWAY.cnl          POS_ANT_LIDER_REL_LOC.cnl \
DETECT_CENTER_OF_MASS.cnl             PROBE.cnl \
DETECT_FORM_HEADING.cnl               PROBE_MOVABLE.cnl \
DETECT_FORM_POSITION.cnl              PROBE_OBJECT.cnl \
DETECT_GOAL_WITH_SHAFT_ENCODERS.cnl   PROGRESS.cnl \
DETECT_IN_POSITION.cnl                PROJECTED_GOAL.cnl \
DETECT_LEADER_CENTER.cnl              PUT_ROBOT_STATE.cnl \
DETECT_OBJECTS.cnl                    RAW_ULTRASONICS.cnl \
DETECT_OBSTACLES.cnl                  REMOVE_ROBOTS.cnl \
DETECT_ROBOTS.cnl                     RESET_WORLD.cnl \
DETECT_SOUND.cnl                      SCALE_VECTOR.cnl \
DETECT_SOUND_MOTIVATED.cnl            SEGUIR_LIDER.cnl \
DETECT_UNIT_CENTER.cnl \
DETECT_UNIT_CENTER_GLOBAL.cnl         SENSE_LOCATION.cnl \
DOCK.cnl                              SET_CAMERA_TRACKING_MODE.cnl \
DOORWAY_ULTRASOUND.cnl                SET_CID.cnl \
DRIVE.cnl                             SET_MOBILITY_TYPE.cnl \
DRIVE_ROOMBA.cnl                      SET_TARGET.cnl \
DRIVE_W_SPIN.cnl                      SETXY.cnl \
DROP_IN_BASKET.cnl                    SETXYTHETA.cnl \
DROP_OBJECT.cnl                       SHAFTENCODERS.cnl \
ECHO_OBJECTS.cnl                      SKIP_GOAL.cnl \
ENTER_ALTERNATE_HALLWAY.cnl           SLUAV_TRANSPORT.cnl \
ENTER_DOORWAY.cnl                     SOUND.cnl \
ENTER_ROOM.cnl                        SPIN.cnl \
ENTRAR_HABITACION.cnl                 STAY_ON_PATH.cnl \
EQUAL_INT.cnl                         STRING_UTILITY.cnl \
ESTOY_OBJETIVO.cnl                    SUB_MISSION.cnl \
ESTOY_POS_ANT_LIDER.cnl               SUBTRACT_VECTORS.cnl \
EXTRACT_GOAL.cnl                      SUM.cnl \
FILTER_OBJECT_CLASSES.cnl             SUM_WEIGHTED_VECTORS.cnl \
FILTER_OBJECTS_BY_CLASS.cnl           SWIRL_STATIC_OBSTACLES.cnl \
FILTER_OBJECTS.cnl                    TALK.cnl \
FIN_ASPIRAR.cnl                       TELOP.cnl \
FIRST_TIME.cnl                        TERMINATE_MISSION.cnl \
FREE_US_PATH.cnl                      TRIGGER_FLAGS.cnl \
ULTRASONICS.cnl \
GET_COMPASS_HEADING.cnl               VECTOR_COMPASS_DIFFERENTIAL.cnl \
GET_HALLWAY_INFO.cnl                  V_IS_ZERO.cnl \
GET_HEADING.cnl                       WAIT.cnl \
GET_LOCATION.cnl                      GROUSAL.cnl                  CARMEN.cnl


# NOTE: If you add a cnl file to this list, be sure and include a
#       corresponding dependency line below

evitar_obstaculos.o:
	$(CC) -c $(CFLAGS) $(INCLUDES) evitar_obstaculos.c

OBJS=$(SRCS:cnl=o) losfunctions.o evitar_obstaculos.o

# init_dummy.o
CFILES=$(SRCS:cnl=cc)




.cnl.cc:
	$(CNLBIN)/cnl -c $(INCLUDES) $<

.cc.o:
	$(CC) -c $(CFLAGS) $(INCLUDES) $*.cc

.c.o:
	$(CC) -c $(CFLAGS) $(INCLUDES) $*.c


libcnl.a:	$(OBJS)
	rm -f libcnl.a
	ar rcv libcnl.a $(OBJS)
	ranlib libcnl.a

libevitarObstaculos.a: evitar_obstaculos.o
	rm -f libevitarObstaculos.a
	ar rcv libevitarObstaculos.a evitar_obstaculos.o
	ranlib libevitarObstaculos.a

all: libcnl.a libevitarObstaculos.a

clean:
	rm -f $(CFILES)
	rm -f *.o core

depend:
	@echo "Make depend is not necessary for this directory."

veryclean: clean
	rm -f *.a

