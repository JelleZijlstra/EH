#!/bin/bash
# Based on http://stackoverflow.com/questions/242538/unix-shell-script-find-out-which-directory-the-script-file-resides
PARDIR=$(dirname $0)
if [ $PARDIR = '.' ]; then
	PARDIR=$(pwd)
fi
${PARDIR}/ehc < $1.eh
gcc -gstabs ${PARDIR}/tmp.s -o $1
