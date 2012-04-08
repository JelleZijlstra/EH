<?php
/*
 * Interface for a ListEntry.
 */
interface ListEntry {
	/*
	 * Constructor merely calls parent with commands.
	 * Commented out for now as it's not clear to me that we should enforce
	 * this constructor signature on children.
	 */
	/* function __construct($commands); */

	/*
	 * Convert this object into an array.
	 */
	function toarray();

	/* OVERLOADING */
	function __set($property, $value);
	function __get($property);
	function __isset($property);
	function __unset($property);
	
	/*
	 * Various stuff to determine properties and methods that exist. Probably
	 * needs harmonization.
	 */
	static function haspm($in);
	static function hasproperty($property);
	static function hasmethodps($method);
	static function hasmethod($method);
	
	/*
	 * Give information about a ListEntry.
	 */
	function inform();
	
	/*
	 * Edit an entry.
	 */
	function edit($paras = array());
	
	/*
	 * Log something.
	 */
	function log($msg, $writefull = true);
	
	/*
	 * Implements settitle-like commands.
	 */
	function __call($name, $arguments);

	/*
	 * Sets up a CLI. Similar and perhaps redundant to edit().
	 */
	function cli(array $paras = array());
	
	/*
	 * Set properties.
	 */
	function set(array $paras);
	
	/*
	 * Empty properties.
	 */
	function setempty(array $paras);
	
	/*
	 * Resolve a redirect.
	 */
	/* string */ function resolve_redirect();
}
