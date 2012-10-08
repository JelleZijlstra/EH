#!/bin/bash
# Sort the tests in the testfiles file
cp testfiles testfiles-tmp
sort testfiles-tmp | uniq > testfiles-sorted
cat -s testfiles-sorted > testfiles
rm testfiles-sorted testfiles-tmp
