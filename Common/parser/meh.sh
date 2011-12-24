#!/bin/bash
PARDIR='/Users/jellezijlstra/Dropbox/git/Common/parser/'
${PARDIR}ehc < $1.eh
gcc -gstabs ${PARDIR}tmp.s -o $1
