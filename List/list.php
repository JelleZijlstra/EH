#!/usr/bin/php
<?
require_once(__DIR__ . "/../Common/common.php");
if(!isset($argv[1])) {
	TaxonList::singleton()->cli();
} else {
	// execute arguments as command
	array_shift($argv);
	TaxonList::singleton()->execute(implode(' ', $argv));
}
