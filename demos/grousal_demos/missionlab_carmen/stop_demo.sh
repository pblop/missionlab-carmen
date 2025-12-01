#!/bin/bash

OUT_FILE=/dev/null
ERROR_FILE=/dev/null

killall central          >$OUT_FILE 2>$ERROR_FILE
killall iptserver        >$OUT_FILE 2>$ERROR_FILE
killall hserver          >$OUT_FILE 2>$ERROR_FILE
killall mlab             >$OUT_FILE 2>$ERROR_FILE
killall missionlab_carmenRobot1 >$OUT_FILE 2>$ERROR_FILE
killall missionlab_carmenRobot2 >$OUT_FILE 2>$ERROR_FILE
killall missionlab_carmenRobot3 >$OUT_FILE 2>$ERROR_FILE
sleep 1
sleep 1
killall -s SIGKILL central          >$OUT_FILE 2>$ERROR_FILE
killall -s SIGKILL iptserver        >$OUT_FILE 2>$ERROR_FILE
killall -s SIGKILL hserver          >$OUT_FILE 2>$ERROR_FILE
killall -s SIGKILL mlab             >$OUT_FILE 2>$ERROR_FILE
killall -s SIGKILL missionlab_carmenRobot1 >$OUT_FILE 2>$ERROR_FILE
killall -s SIGKILL missionlab_carmenRobot2 >$OUT_FILE 2>$ERROR_FILE
killall -s SIGKILL missionlab_carmenRobot3 >$OUT_FILE 2>$ERROR_FILE
clear
echo "Mission stopped!"
echo "Thank you for trying this demo :-)"

