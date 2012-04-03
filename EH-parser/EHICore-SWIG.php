<?php
/*
 * EHICore-SWIG.php
 * Jelle Zijlstra, January 2012
 *
 * PHP interface for the C++ EHI interpreter.
 */
@include_once(BPATH . "/EH-parser/ehphp.php");
if(!class_exists("EHI")) return;
define('IS_EHPHP', 0);
abstract class EHICore extends EHI implements EHICoreInterface {
	private $prompt;
	/* core commands */
	protected static $core_commands = array(

	);
	public function __construct() {
		parent::__construct();
	}
	public function setup_commandline($name, array $paras = array()) {
		// set up if necessary
		if($this->_cPtr === NULL)
			parent::__construct();
		$this->prompt = $name . '> ';
		try {
			$ret = $this->eh_interactive();
		} catch(StopException $e) {
			// don't do anything, just stop it
			$ret = NULL;
		}
		echo "Goodbye." . PHP_EOL;
		return $ret;
	}
	public function eh_getline() {
		$cmd = '';
		static $history = array();
		try {
			$cmd = $this->getline(array(
				'prompt' => $this->prompt,
				'lines' => $history,
				'includenewlines' => true,
			));
		} catch(StopException $e) {
			// make StopException here equivalent to typing 'quit'
			return "quit\n";
		}
		$history[] = $cmd;
		return $cmd;
	}
	public function exec_file(array $paras = array()) {
		$file = NULL;
		if(isset($paras[0])) {
			$file = $paras[0];
		} elseif(isset($paras['file'])) {
			$file = $paras['file'];
		}
		if($file === NULL) {
			return false;
		} else {
			return parent::exec_file_name($file);
		}
	}
	/* workarounds to prevent EH from blowing up when using the PHP implementation's features */
	public function setvar($var, $value) {
		return;
	}
}
