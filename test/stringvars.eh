#!/usr/bin/ehi
# Examples of string variables in interpreted EH
foo = 'test'
bar = 'test'
if foo == bar
	$echo 'It works!'
else
	$echo 'This should not be happening'
endif
# >= currently does a cast to integer, which won't work for either foo or bar. Therefore, EHI::to_int will return the fallback value 0 for both, and the test will evaluate to true.
if foo >= bar
	$echo 'There should have been several error messages on the lines before this one'
else
  $echo 'This is reasonable too'
endif
$echo foo
$echo bar
baz = foo + bar
$echo 'This should print "testtest":'
$echo baz
