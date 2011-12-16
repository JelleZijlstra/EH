#!/usr/bin/php
<?
require_once(__DIR__ . '/../Common/common.php');
// don't load catalog when we have an argument that doesn't need it
if(!in_array($argv[1], array('backup', 'revert', 'diff', 'findcode', 'test')))
	require_once(__DIR__ . '/load.php');
else
	require_once(__DIR__ . '/settings.php');
switch($argv[1]) {
	case '': case 'byfile': $csvlist->cli(); break; // perform by-file operations
	case 'check': $csvlist->check(); break; // check for new files, add them to catalog
	case 'format': // format catalog
		if($argv[2] === '-w')
			$csvlist->formatall(true); 
		else
			$csvlist->formatall();
		break;
	case 'adddata': // add data to catalog entries
	case 'edittitle': $csvlist->doall($argv[1]); break; // edit titles
	// find files with a certain key-value pair
	// usage: ./catalog.php find KEY VALUE
	case 'find':
		if($argv[3]) $csvlist->find($argv[2], $argv[3]);
		else $csvlist->find_input($argv[2]);
		break;
	case 'dupes': $csvlist->dups(); break;
	case 'stats': $csvlist->stats(); break;
	// list values
	case 'list':
		$paras = array();
		if($argv[3]) $paras['sort'] = $argv[3];
		if($n3 = in_array($argv[3], array('true', 'false')) or $n4 = $argv[4]) {
			$v = $n3 ? $n3 : $n4;
			$paras['isfunc'] = $v;
		}
		$csvlist->mlist($argv[2], $paras);
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
if($csvlist) $csvlist->saveifneeded();
?>
