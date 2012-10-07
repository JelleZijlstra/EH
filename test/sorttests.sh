#!/bin/bash
cp testfiles testfiles-tmp
sort testfiles-tmp | uniq > testfiles-sorted
cat -s testfiles-sorted > testfiles
