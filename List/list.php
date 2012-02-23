#!/usr/bin/php
<?
require_once(__DIR__ . "/../Common/common.php");
require_once(BPATH . '/List/load.php');
if(!isset($argv[1])) {
	$taxonlist->cli();
} else {
	// execute arguments as command
	array_shift($argv);
	$taxonlist->execute(implode(' ', $argv));
}
