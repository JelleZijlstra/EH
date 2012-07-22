#!/bin/bash
# Add a file to the EH test suite

# Find the executer
executer=/usr/bin/ehi
if [ -z "$executer" ]; then
	echo 'Invalid executer'
	exit
fi
for infile in $@; do
	exp=${infile/.eh/.expected}
	echo "$infile" >> testfiles
	$executer $infile &> $exp
	git add $infile
	git add $exp
done
