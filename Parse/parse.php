#!/usr/bin/php
<?php
require_once(__DIR__ . "/../Common/common.php");
require_once(BPATH . "/Parse/parser.php");
if(!isset($argv[1])) {
	exit(1);
}
switch($argv[1]) {
	case '-n': 
		$citetype = 'normal';
	case '-w':
		if(!isset($argv[2])) {
			throw new EHException('No text to parse', EHException::E_FATAL);
		}
		parse_wlist($argv[2]);
		break;
	case '-t':
		if(!isset($argv[2])) {
			throw new EHException('No text to parse', EHException::E_FATAL);
		}
		parse_wtext($argv[2]);
		break;
	default: 
		parse_paper($argv[1]); 
		break;
}
