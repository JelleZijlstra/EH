#!/usr/bin/php
<?
require_once(__DIR__ . '/../Common/common.php');
require_once(BPATH . '/Catalog/load.php');
if(!isset($argv[1])) {
	$csvlist->cli();
} else {
	// execute arguments as command
	array_shift($argv);
	$csvlist->execute(implode(' ', $argv));
}
