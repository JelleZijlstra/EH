#!/usr/bin/ehi
include '../lib/exception.eh'

# Does stuff after a failed include call get printed?
rescue(() => include 'thisfiledoesnotexist')
echo 42
