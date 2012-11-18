#!/usr/bin/ehi
include '../lib/exception.eh'

# Arrow access to ranges
rangev = 1..42
echo(rangev->0)
echo(rangev->1)
# This is now illegal: ranges are immutable
rescue(() => (rangev->0 = 2))
rescue(() => (rangev->1 = 41))
echo(rangev->0)
echo(rangev->1)
# Errors
rangev->2 = 9000
echo(rangev->-1)
