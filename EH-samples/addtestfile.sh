#!/bin/bash
# Add a file to the EH test suite
for infile in $@; do
	exp=${infile/.eh/.expected}
	echo "$infile" >> testfiles
	/usr/bin/ehi $infile &> $exp
	git add $infile
	git add $exp
done
