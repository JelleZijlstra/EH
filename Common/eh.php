#!/usr/bin/php
<?php
require_once(__DIR__ . "/../Common/common.php");
class EH extends ExecuteHandler {
	public static function singleton() {
		static $instance = NULL;
		if($instance === NULL) {
			$instance = new static();
		}
		return $instance;
	}
}

array_shift($argv);
$arg = implode(' ', $argv);
if($arg === '') {
	echo "Usage: eh <file>" . PHP_EOL;
	exit(1);
}
$fe = EH::singleton();
if($arg === '-i') {
	$fe->cli();
} else {
	$fe->exec_file($arg);
}
exit(0);
