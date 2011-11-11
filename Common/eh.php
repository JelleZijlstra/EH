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
		$this->setupcommandline(__CLASS__);
	}
}
$fe = new FileExecuter();
$fe->execute("exec_file " . $arg);
exit(0);
?>
