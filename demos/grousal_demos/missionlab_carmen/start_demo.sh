#!/bin/bash

OUT_FILE=/dev/null
ERROR_FILE=/dev/null
MISSION_NAME=missionlab_carmen

export IPTHOST=127.0.0.1

echo "Building robots..."
echo "This also can be done running \"cfgedit missionlab.cdl\" and clicking \"Compile\""
echo "If so, remember copying demo folder to an user writable place or run as root"
echo ""
echo "Compiling CDL file..."
rm -rf    /tmp/grousal_demos/${MISSION_NAME}/*                        >$OUT_FILE 2>$ERROR_FILE
mkdir -p  /tmp/grousal_demos/${MISSION_NAME}/tmp/                     >$OUT_FILE 2>$ERROR_FILE
cdl -p -o /tmp/grousal_demos/${MISSION_NAME}/tmp/ ${MISSION_NAME}.cdl >$OUT_FILE 2>$ERROR_FILE
echo "Done!"

echo "Compiling CNL files..."
cnl /tmp/grousal_demos/${MISSION_NAME}/tmp/${MISSION_NAME}Robot1.cnl >$OUT_FILE 2>$ERROR_FILE
cnl /tmp/grousal_demos/${MISSION_NAME}/tmp/${MISSION_NAME}Robot2.cnl >$OUT_FILE 2>$ERROR_FILE
cnl /tmp/grousal_demos/${MISSION_NAME}/tmp/${MISSION_NAME}Robot3.cnl >$OUT_FILE 2>$ERROR_FILE
echo "Done!"

echo "Building robot executables..."
gcc /tmp/grousal_demos/${MISSION_NAME}/tmp/${MISSION_NAME}Robot1.cc -g -o /tmp/grousal_demos/${MISSION_NAME}/${MISSION_NAME}Robot1 -lcnl -lhardware_drivers -lutilities -lipt -lipc -lstdc++ -lm -lqlearn -lcbr_behavioral_select -ldstar -lcnp -lCommBehavior -lmic_cmdli -lpthread -lz >$OUT_FILE 2>$ERROR_FILE
gcc /tmp/grousal_demos/${MISSION_NAME}/tmp/${MISSION_NAME}Robot2.cc -g -o /tmp/grousal_demos/${MISSION_NAME}/${MISSION_NAME}Robot2 -lcnl -lhardware_drivers -lutilities -lipt -lipc -lstdc++ -lm -lqlearn -lcbr_behavioral_select -ldstar -lcnp -lCommBehavior -lmic_cmdli -lpthread -lz >$OUT_FILE 2>$ERROR_FILE
gcc /tmp/grousal_demos/${MISSION_NAME}/tmp/${MISSION_NAME}Robot3.cc -g -o /tmp/grousal_demos/${MISSION_NAME}/${MISSION_NAME}Robot3 -lcnl -lhardware_drivers -lutilities -lipt -lipc -lstdc++ -lm -lqlearn -lcbr_behavioral_select -ldstar -lcnp -lCommBehavior -lmic_cmdli -lpthread -lz >$OUT_FILE 2>$ERROR_FILE
echo "Done!"

echo "Cleaning temp files..."
#rm -rf tmp
echo "Done!"

echo "Starting iptserver..."
iptserver -u -s >$OUT_FILE 2>$ERROR_FILE &
echo "Done!"

echo "Starting CARMEN robots..."
./startCarmenRobot.sh 3001
./startCarmenRobot.sh 3002
echo "Done!"

sleep 2

echo "Starting hserver consoles..."
gnome-terminal --tab --command "hserver -a -r robot1 -f"                               --title=ROBOT1 \
               --tab --command "hserver -a -r robot2 -s 127.0.0.1:3001 -q carmen1 -l carmen1 -g carmen1" --title=ROBOT2 \
               --tab --command "hserver -a -r robot3 -s 127.0.0.1:3002 -f         -l carmen2" --title=ROBOT3 >$OUT_FILE 2>$ERROR_FILE
sleep 14
echo "Done!"

echo "Starting mission..."
cp -f ${MISSION_NAME} /tmp/grousal_demos/${MISSION_NAME}/
cd /tmp/grousal_demos/${MISSION_NAME}/
mlab -r ${MISSION_NAME} & >$OUT_FILE 2>$ERROR_FILE
cd -

sleep 10
clear
echo "Mission started!"
echo "Launch \"./stop_demo.sh\" to stop the mission"
echo "Bye!"
