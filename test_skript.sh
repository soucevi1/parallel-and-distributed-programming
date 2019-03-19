#!/bin/bash

for i in {1..12}
do
	echo "Test $i"
	(cmake-build-debug/MI_PDP -f ./data_in/poi"$i".txt) > out-"$i" 2>&1
done
