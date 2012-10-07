#!/bin/bash
# Add a file to the EH test suite

# Find the executer
executer=/usr/bin/ehi
if [ -z "$executer" ]; then
	echo 'Invalid executer'
	exit
fi
if [[ "x$1" != 'x--replace' ]]; then
	replacing=false
else
	replacing=true
fi

for infile in $@; do
	[[ $infile = '--replace' ]] && continue
	name=${infile/.eh/}
	stderr=tmp/$name.stderr
	stdout=tmp/$name.stdout
	$executer $infile > $stdout 2> $stderr
	exp=$name.expected
	echo '%%stdout%%' > $exp
	cat $stdout >> $exp
	echo '%%stderr%%' >> $exp
	cat $stderr >> $exp

	if [[ ! $replacing ]]; then
		echo "$infile" >> testfiles
		git add $infile
		git add $exp
	fi
done
