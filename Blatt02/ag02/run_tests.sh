#!/bin/bash

for arg in 10 100 1000 10000 100000 1000000 10000000
do
	./mergesort -n $arg -t 2
done