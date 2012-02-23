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
require_once(BPATH . "/EH-parser/EHICore-SWIG.php");
// if we failed, use pure-PHP solutions
if(extension_loaded("ehphp"))
	define('EHI_LOADED', 1);
else {
	require_once("EHICore.php");
	define('EHI_LOADED', 0);
}
// load exceptions
require_once(BPATH . '/Common/EHException.php');

// set encoding
if(mb_internal_encoding('UTF-8') === false) {
	echo "Unable to set encoding" . PHP_EOL;
	exit(1);
}
$eclog = BPATH . '/Misc/log';
function exec_catch($cmd, $debug = false) {
// makes command and catches output; returns TRUE if successful (i.e., return status = 0), FALSE if unsuccessful
	global $eclog;
	if($debug) echo $cmd . PHP_EOL;
	$tmp = shell_exec("$cmd 2>$eclog");
	if($debug) echo $tmp . PHP_EOL;
	$result = shell_exec('echo $?');
	if($debug) echo $result . PHP_EOL;
	return (trim($result) == 0);
}
function getinput($limit = 500) {
	return trim(fgets(STDIN, $limit));
}
function escape_shell($in) {
// escapes a string for use as a shell argument
	$in = escapeshellcmd($in);
	$in = preg_replace("/ /", "\ ", $in);
	return $in;
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
function simplify($in) {
	return strtolower(preg_replace('/[^a-zA-Z0-9]|<\/?i>/u', '', $in));
}
function convertfile($in, $out) {
	static $started;
	if(!$started) {
		shell_exec('/Applications/OpenOffice.org.app/Contents/MacOS/soffice -headless \'-accept="socket,host=127.0.0.1,port=8100;urp;"\' -nofirststartwizard &');
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
