#!/bin/bash
# A script that takes C code from the command line and compiles it.
input=$@
if [ -z "$input" ]
then
	echo "Please input code"
	exit
fi
echo "$input"
echo "$input" > "/tmp/curr.c"
gcc -o /tmp/curr /tmp/curr.c
/tmp/curr
echo $?
