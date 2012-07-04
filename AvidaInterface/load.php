#!/usr/bin/php
<?php
require_once(__DIR__ . "/../Common/common.php");
require_once(BPATH . "/Common/ExecuteHandler.php");
require_once(BPATH . "/AvidaInterface/AvidaInterface.php");

$ai = new AvidaInterface();
if(isset($argv[1]))
	$ai->exec_file($argv[1]);
else
	$ai->cli();
