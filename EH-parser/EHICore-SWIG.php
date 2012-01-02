<?php
/*
 * EHICore-SWIG.php
 * Jelle Zijlstra, January 2012
 *
 * PHP interface for the C++ EHI interpreter.
 */
require_once(BPATH . "/EH-parser/ehphp.php");
if(!class_exists("EHI")) return;
abstract class EHICore extends EHI {
	public function __construct() {
		parent::__construct();
	}
	public function setup_commandline($name, $paras = array()) {
		$this->eh_interactive();
	}
	/* workarounds to prevent EH from blowing up when using the PHP implementation's features */
	public function setvar($var, $value) {
		return;
	}
}
