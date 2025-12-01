#!/bin/bash

for ((cnt=1; cnt <= 250 ; cnt++))
do
	./randomizer.pl mine_land.txt mine.feature.data 900 600
	cp test.feature.data $cnt.feature.data
	./run_test.sh $cnt
done
		
