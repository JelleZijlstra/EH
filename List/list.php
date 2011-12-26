#!/usr/bin/php
<?
require_once(__DIR__ . "/../Common/common.php");
if(!isset($argv[1])) {
	require_once(BPATH . '/List/load.php');
	$taxonlist->cli();
}
else switch($argv[1]) {
	// edit data
	case 'edit':
		require_once('load.php');
		$taxonlist->cli(); break;
	case 'backup':
		$date = new DateTime();
		exec_catch('cp ' . __DIR__ . '/data/list.csv' . ' ' . BPATH . '/List/Backup/list.csv.' . $date->format('YmdHis'));
		break;
	case 'revert': revert($list); break;
	// edit all comments
	case 'wikipublish':
		require_once('TaxonList.php');
		TaxonList::wikipublish(); break;
	// die
	default: mydie("Invalid argument");
}
?>
