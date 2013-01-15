<?php
/*
 * EHICore-SWIG.php
 * Jelle Zijlstra, January 2012
 *
 * PHP interface for the C++ EHI interpreter.
 */
try {
	include_once("ehphp.php");
} catch(EHException $e) {
	echo $e;
	return;
}
if(extension_loaded("ehphp"))  {
	define('IS_EHPHP', 0);
	abstract class EHICore extends EHI implements EHICoreInterface {
		private $prompt;
		/* core commands */
		protected static $core_commands = array(

		);
		protected function __construct() {
			parent::__construct();
		}
		protected function fillThisPointer() {
			if($this->_cPtr === NULL) {
				parent::__construct();
			}
		}
		public function setup_commandline($name, array $paras = array()) {
			// set up if necessary
			$this->fillThisPointer();
			$cmd = "include '/Users/jellezijlstra/code/EH/lib/library.eh';";
			$this->global_parse_string($cmd);
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
					'autocompleter' => $this->getAutocompleter(),
				));
			} catch(EHException $e) {
				echo "Exception occurred in getline: ";
				echo $e;
				return '';
			} catch(StopException $e) {
				// make StopException here equivalent to typing '$quit'
				return "\$quit\n";
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
				return parent::parse_file($file);
			}
		}
		/* workarounds to prevent EH from blowing up when using the PHP implementation's features */
		public function setvar($var, $value) {
			return;
		}
	}
}
