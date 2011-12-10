#!/usr/bin/php
<?
require_once('../Common/common.php');
require_once(BPATH . '/UcuchaBot/Bot.php');
$bot = new Bot();
// remove script name
array_shift($argv);
// get command to be executed
$cmd = implode($argv, ' ');
if($cmd) {
	$bot->execute($cmd);
	$bot->execute('quit');
}
else {
	$bot->cli();
}
