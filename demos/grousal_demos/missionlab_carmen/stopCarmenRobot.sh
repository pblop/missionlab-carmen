#!/bin/bash

if test -z $1 
then
    echo "Usage: $0 <port>"
    exit 1
fi

echo "Stopping robot at port $1..."
PID_CENTRAL=`ps -Af --sort=cmd | grep ".*central.*p$1$" | head -n 1 | awk '{ print $2 }'`
if test -z $PID_CENTRAL
then
    echo "There isn't a robot at port $1!"
    exit 1
fi

echo "Killing central server with PID: $PID_CENTRAL..."
kill -15 $PID_CENTRAL
sleep 2
kill -9 $PID_CENTRAL >/dev/null 2>/dev/null
echo "Done!"
exit 0

