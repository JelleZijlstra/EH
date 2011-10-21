#!/usr/bin/php
<?
require_once('../Common/common.php');
require_once(BPATH . '/UcuchaBot/Bot.php');
$bot = new Bot();
array_shift($argv);
$cmd = implode($argv, ' ');
if($cmd) {
	$bot->execute($cmd);
	$bot->execute('quit');
}
else
	$bot->cli();
