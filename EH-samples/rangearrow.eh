#!/usr/bin/ehi
# Arrow access to ranges
rangev := 1..42
echo $rangev->0
echo $rangev->1
rangev->0 := 2
rangev->1 := 41
echo $rangev->0
echo $rangev->1
# Errors
rangev->2 := 9000
echo $rangev->(-1)
