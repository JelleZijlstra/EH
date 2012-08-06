#!/usr/bin/ehi
include '../lib/exception.eh'

# Does stuff after a failed include call get printed?
rescue func: -> (include 'thisfiledoesnotexist')
echo 42
