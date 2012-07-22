#!/bin/bash
if [ ! $1 ]; then
	echo 'No input file given'
	exit
fi
/usr/bin/ehc $1.eh
gcc $1.s -o $1
