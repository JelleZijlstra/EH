<?php
/*
 * Interface for the core of EHI. Implemented by the EHICore class.
 */
interface EHICoreInterface {
	/*
	 * Set the variable named $name in the current scope to $value.
	 */
	function setvar($name, $value);
	
	/*
	 * Execute the contents of a file.
	 */
	function exec_file(array $paras = array());
	
	/*
	 * Sets up a command line.
	 */
	function setup_commandline($prompt, array $paras = array());
}
