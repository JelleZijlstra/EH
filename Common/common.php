<?
/*
 * common.php
 * Jelle Zijlstra, March 2011
 * Contains functions used in several of my programs
 *
 * to use:
require_once(__DIR__ . "../Common/common.php");
 */
/* notes: regexes to cleanup PHP code syntax
s/"([a-zA-Z\s\.]+)"/'\1'/
s/\s+(?=\r)//
s/\r\s+(?={)/ /
*/
define('BPATH', __DIR__ . '/..');
// show all errors
error_reporting(E_ALL | E_STRICT);
ini_set("display_errors", '1');
// try to get the C++ EHI loaded
//require_once(BPATH . "/EH-parser/EHICore-C++.php");
// if we failed, use pure-PHP solutions
if(extension_loaded("ehphp"))
	define('EHI_LOADED', 1);
else {
	require_once("EHICore.php");
	define('EHI_LOADED', 0);
}

// set encoding
if(mb_internal_encoding('UTF-8') === false) {
	echo "Unable to set encoding" . PHP_EOL;
	exit(1);
}
function logwrite($in) {
// writes something to the log file; requires $logfile to be set to a file
	global $log, $logfile;
	if(!$logfile)
		mydie("Call to " . __FUNCTION__ . " without a set logfile");
	if(!$log)
		$log = fopen($logfile, "a");
	// write array as CSV
	if(is_array($in))
		fputcsv($log, $in);
	else
		fwrite($log, $in);
}
$eclog = BPATH . '/Misc/log';
function exec_catch($cmd, $debug = false) {
// makes command and catches output; returns TRUE if successful (i.e., nothing in STDERR), FALSE if unsuccessful
	global $eclog;
	if($debug) echo $cmd . PHP_EOL;
	// this is what keeps creating the file '1'; not sure yet how to fix
	$tmp = shell_exec("$cmd 2>$eclog");
	if($debug) echo $tmp . PHP_EOL;
	$result = shell_exec('echo $?');
	if($debug) echo $result . PHP_EOL;
	return (trim($result) == 0);
}
function getinput($limit = 500) {
	global $stdin;
	if(!$stdin) $stdin = fopen('php://stdin', 'r');
	return trim(fgets($stdin, $limit));
}
function getinput_label($label, $limit = 500) {
	echo $label . ': ';
	return getinput($limit);
}
function escape_shell($in) {
// escapes a string for use as a shell argument
	$in = escapeshellcmd($in);
	$in = preg_replace("/ /", "\ ", $in);
	return $in;
}
function escape_regex($in) {
	return str_replace(
		array("/",  "[",  "]",  "|",  ".",  "*",  "?",  "(",  ")",  "+",  "{",  "}",  "^",  "$"),
		array("\/", "\[", "\]", "\|", "\.", "\*", "\?", "\(", "\)", "\+", "\{", "\}", "\^", "\$"),
		$in);
}
function mydie($message) {
	global $debug;
	if($debug) debug_print_backtrace();
	echo $message . PHP_EOL;
	die(1);
}
function revert($list) {
// reverts to most recently edited file in Backup folder
// $list: name of file to be reverted to
	exec_catch('cp Backup/' . getbackup($list) . ' ' . $list);
}
function diff($list) {
	$listp = preg_replace('/[^\/]+$/', '', $list);
	echo shell_exec('diff ' . $list . ' ' . $listp . 'Backup/' . getbackup($list));
}
function getbackup($list) {
// gets most recent backup file
	$listp = preg_replace('/[^\/]+$/', '', $list);
	$backuplist = shell_exec('ls -lt ' . $listp . 'Backup');
	$backuplist = preg_split("/\n/", $backuplist);
	foreach($backuplist as &$file)
		$file = explode(' ', $file);
	return array_pop($backuplist[1]);
}
function findcode($pattern) {
	echo shell_exec('grep -n \'' . $pattern . '\' *.php');
}
function makemenu($in, $head = 'MENU') {
	echo $head . PHP_EOL;
	foreach($in as $cmd => $desc) {
		echo "-'" . $cmd . "': " . $desc . PHP_EOL;
	}
}
function wikify($in) {
// wikifies text (i.e., <i> => '')
	return preg_replace(array("/'/", "/<\/?i>/", "/(?<!')<nowiki>'<\/nowiki>(?!')/"), array("<nowiki>'</nowiki>", "''", "'"), $in);
}
/* small utilities */
function trimplus($in) {
// trims all cruft from a field
	return trim(preg_replace("/[\.;\(]$/", "", trim($in)));
}
function trimdoi($in) {
// similar, for DOIs specifically
	return trim(preg_replace("/([\.;\(]$|^[:]|^doi:)/", "", trim($in)));
}
function simplify($in) {
	return strtolower(preg_replace('/[^a-zA-Z0-9]|<\/?i>/u', '', $in));
}
function convertfile($in, $out) {
	static $started;
	if(!$started) {
		exec_catch('/Applications/OpenOffice.org.app/Contents/MacOS/soffice -headless -accept="socket,host=127.0.0.1,port=8100;urp;" -nofirststartwizard &');
		$started = true;
	}
	exec_catch('java -jar /Users/jellezijlstra/Documents/Programs/jodconverter-2.2.2/lib/jodconverter-cli-2.2.2.jar ' . $in . ' ' . $out);
	return true;
}
/* some stuff standard PHP mb is missing */
function mb_ucfirst($in) {
    $in[0] = mb_strtoupper($in[0]);
    return $in;
}
function mb_lcfirst($in) {
	$in[0] = mb_strtolower($in[1]);
	return $in;
}
function mb_str_split($in) {
	return preg_split('/(?<!^)(?!$)/u', $in);
}
function mw_normalize($in) {
// normalize MediaWiki link text
	return preg_replace(
		array(
			'/^WP(?=:)/u', // full namespace prefix
			'/[\s_]+/u', //whitespace
			'/(?<=Wikipedia:) /u', // no space after NS colon
		),
		array(
			'Wikipedia',
			' ',
			'',
		),
		$in
	);
}
function maketemplate($name, $paras) {
	$out = '{{' . $name;
	if(!is_array($paras)) return false;
	foreach($paras as $key => $value) {
		$out .= '|' . $key . '=' . $value;
	}
	$out .= '}}';
	return $out;
}
if(!function_exists('cal_days_in_month')) {
	function cal_days_in_month($calendar, $month, $year) {
		// this actually ignores leap years, calendar differences, etcetera
		switch($month) {
			case 1: case 3: case 5: case 7: case 8: case 10: case 12: return 31;
			case 2: return 28;
			case 4: case 6: case 9: case 11: return 30;
		}
	}
}
?>