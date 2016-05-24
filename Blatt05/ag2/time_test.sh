#!/bin/bash

make all
for i in 1000 5000
do
	time make "PARAMS=$i $i" run
done