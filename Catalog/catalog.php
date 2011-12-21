#!/usr/bin/php
<?
require_once(__DIR__ . '/../Common/common.php');
// don't load catalog when we have an argument that doesn't need it
if(!isset($argv[1]) or !in_array($argv[1], array('backup', 'revert', 'diff', 'findcode', 'test')))
	require_once(BPATH . '/Catalog/load.php');
else
	require_once(BPATH . '/Catalog/settings.php');
if(!isset($argv[1]))
	$csvlist->cli();
else switch($argv[1]) {
	case 'byfile': 
		// enter main CLI
		$csvlist->cli(); 
		break;
	case 'backup': // backup catalog
		catalog_backup();
		break;
	case 'revert': revert($catalog); break;
	case 'diff': diff($catalog); break;
	case 'findcode': findcode($argv[2]); break;
	case 'test': break;
	default:
		// execute arguments as command
		array_shift($argv);
		$csvlist->execute(implode(' ', $argv));
		break;
}
?>
