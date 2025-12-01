#!/bin/bash

OUT_FILE=/dev/null
ERROR_FILE=/dev/null

if test -z $1 
then
    echo "Usage: $0 <port> [Carmen Robot Name] [Carmen Map file]"
    exit 1
fi

if test -z $2 
then
    ROBOT_NAME=pioneer-I
else
    ROBOT_NAME=$2
fi

if test -z $2 
then
    MAP_FILE=/usr/data/deptoiaGIS.map.gz
else
    MAP_FILE=$3
fi

echo "Setting CENTRALHOST environment variable to 127.0.0.1:$1..."
export CENTRALHOST="127.0.0.1:$1"
echo "Done!"

echo "Starting IPC server..."
central -u -s -p$1 >$OUT_FILE 2>$ERROR_FILE &
sleep 1
echo "Done!"

echo "Starting param_daemon using robot $ROBOT_NAME and map $MAP_FILE..."
param_daemon -r $ROBOT_NAME $MAP_FILE >$OUT_FILE 2>$ERROR_FILE &
sleep 1
echo "Done!"

echo "Starting simulator..."
simulator >$OUT_FILE 2>$ERROR_FILE &
sleep 1
echo "Done!"

echo "Starting robot..."
robot >$OUT_FILE 2>$ERROR_FILE &
sleep 1
echo "Done!"

echo "Starting navigator..."
navigator >$OUT_FILE 2>$ERROR_FILE &
sleep 1
echo "Done!"

echo "Starting localize..."
localize >$OUT_FILE 2>$ERROR_FILE &
sleep 1
echo "Done!"

echo "Starting robotgui and navigatorgui..."
robotgui >$OUT_FILE 2>$ERROR_FILE &
navigatorgui >$OUT_FILE 2>$ERROR_FILE &
echo "Done!"

echo "Enjoy your CARMEN robot ;-)"
exit 0


