#!/usr/bin/ehi
# Test defining commands by manipulating the cmdtable directly.

commands->'test' = printvar

$test -abde -cfgh=42 --test --foo='bar' --"foox"="barx" --"bam"=[3, 4]

$test 'foo' 'bar'
