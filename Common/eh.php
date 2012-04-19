#!/usr/bin/php
<?php
require_once(__DIR__ . "/../Common/common.php");
array_shift($argv);
$arg = implode(' ', $argv);
if($arg === '') {
	echo "Usage: eh <file>" . PHP_EOL;
	exit(1);
}
$fe = new ExecuteHandler();
if($arg === '-i') {
	$fe->cli();
} else {
	$fe->exec_file($arg);
}
exit(0);
