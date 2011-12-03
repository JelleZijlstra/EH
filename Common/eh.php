#!/usr/bin/php
<?php
require_once(__DIR__ . "/../Common/Common.php");
require_once(BPATH . '/Common/ExecuteHandler.php');
array_shift($argv);
$arg = implode(' ', $argv);
if(!$arg) {
	echo "Usage: eh <file>" . PHP_EOL;
	exit(1);
}
class FileExecuter extends ExecuteHandler {
	public function __construct() {
		parent::__construct(array());
	}
	public function cli() {
		$this->setup_commandline(__CLASS__);
	}
}
$fe = new FileExecuter();
if($arg === '-i')
	$fe->cli();
else
	$fe->execute("exec_file " . $arg);
exit(0);
?>
