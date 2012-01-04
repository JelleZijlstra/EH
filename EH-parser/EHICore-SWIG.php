<?php
/*
 * EHICore-SWIG.php
 * Jelle Zijlstra, January 2012
 *
 * PHP interface for the C++ EHI interpreter.
 */
include_once(BPATH . "/EH-parser/ehphp.php");
if(!class_exists("EHI")) return;
abstract class EHICore extends EHI {
	private $prompt;
	public function __construct() {
		parent::__construct();
	}
	public function setup_commandline($name, $paras = array()) {
		$this->prompt = $name . '> ';
		$ret = $this->eh_interactive();
		echo "Goodbye." . PHP_EOL;
		return $ret;
	}
	public function eh_getline() {
		$cmd = '';
		static $history = array();
		$cmd = $this->getline(array(
			'prompt' => $this->prompt, 
			'lines' => $history,
		));
		$history[] = $cmd;
		return $cmd;
	}
	/* workarounds to prevent EH from blowing up when using the PHP implementation's features */
	public function setvar($var, $value) {
		return;
	}
}
