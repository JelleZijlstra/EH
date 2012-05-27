#!/usr/bin/php
<?php
require_once(__DIR__ . '/../Common/common.php');
if(!isset($argv[1])) {
	CsvArticleList::singleton()->cli();
} else {
	// execute arguments as command
	array_shift($argv);
	CsvArticleList::singleton()->execute(implode(' ', $argv));
}
