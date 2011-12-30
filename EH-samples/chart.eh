#!/usr/bin/ehi
# Not yet compatible with ehi because of use of the put operator
// CS50 Problem Set 1: chart
echo 'M spotting M:'
$ mm = getinput:
echo 'M spotting F:'
$ mf = getinput:
echo 'F spotting M:'
$ fm = getinput:
echo 'F spotting F:'
$ ff = getinput:

echo
echo 'Who is spotting Whom'
echo

echo 'M spotting M'
for $mm
	put '#'
endfor
echo

echo 'M spotting F'
for $mf
	put '#'
endfor
echo

echo 'F spotting M'
for $fm
	put '#'
endfor
echo

echo 'F spotting F'
for $ff
	put '#'
endfor
echo
