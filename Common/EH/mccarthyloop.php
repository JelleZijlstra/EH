#!/usr/bin/php
<?php
// Implementation of the McCarthy function in PHP
function mccarthy($n) {
	if($n > 100)
		return $n - 10;
	else
		return mccarthy(mccarthy($n+11));
}
if($argc != 2) {
	echo 'Usage: ' . $argv[0] . 'n ' . PHP_EOL;
	exit(1);
}
for($i = 0; $i < $argv[1]; $i++)
	echo mccarthy($i) . PHP_EOL;
exit(0);
?>
