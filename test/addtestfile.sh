#!/bin/bash
# Add a file to the EH test suite

# Find the executer
if [[ "$EXECUTER" == "" ]]; then
	EXECUTER=/usr/bin/ehi
fi

if [ -z "$EXECUTER" ]; then
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
	name=$(echo $infile | sed -e s/\\.[a-z]*//)
	stderr=tmp/$name.stderr
	stdout=tmp/$name.stdout
	$EXECUTER $infile > $stdout 2> $stderr
	exp=$name.expected
	echo '%%stdout%%' > $exp
	cat $stdout >> $exp
	echo '%%stderr%%' >> $exp
	cat $stderr >> $exp

	if ! $replacing ; then
		echo "$infile" >> testfiles
		git add $infile
		git add $exp
	fi
done
