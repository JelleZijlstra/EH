#!/bin/bash
# Add a file to the EH test suite
if [ ! $1 ]; then
	echo 'No input file given'
	exit
fi
PARDIR=$(dirname $0)
if [ $PARDIR = '.' ]; then
	PARDIR=$(pwd)
fi
infile=$1
exp=${infile/.eh/.expected}
echo "$infile" >> testfiles
echo "@$infile:" >> stderr.expected
/usr/bin/ehi $infile > $exp 2>> stderr.expected
git add $infile
git add $exp
