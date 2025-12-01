#!/bin/bash

./clean
zap iptserver
./run-cbrserver &
sleep 1
iptserver &
cfgedit -a -c config-ao-fnc -M $1.feature.data
zap cbrserver


