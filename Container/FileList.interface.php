<?php
/*
 * Interface for a FileList
 */
interface FileList {
	/*
	 * Constructor takes in commands to be passed to EH core.
	 *
	 * Constructor will also pre-populate the list of children.
	 */
	/* function __construct(array $commands = array()); */

	/*
	 * Destructor will save any data still in memory.
	 */
	function __destruct();

	/*
	 * Add some kind of ListEntry to the container.
	 */
	function add_entry(ListEntry $file, array $paras = array());

	/*
	 * Remove an entry.
	 */
	function remove_entry(/* string */ $file, array $paras = array());

	/*
	 * Move an entry.
	 */
	function move_entry(/* string */ $file, /* string */ $newName, array $paras = array());
	
	/*
	 * Whether we have a child of this name.
	 */
	function has(/* string */ $file);
	
	/*
	 * Get a particular child, or a field of that child.
	 */
	function get(/* string */ $file, /* string */ $field = NULL);

	/*
	 * Save anything necessary.
	 */
	function saveifneeded();
	
	/*
	 * Tell the object that a save is needed.
	 */
	function needsave();
	
	/*
	 * Calls a method on a child.
	 */
	function __call($func, $args);
	
	/*
	 * Calls the __invoke method of a child.
	 */
	function __invoke($file);

	/*
	 * Execute a method on all children.
	 */
	function doall(array $paras);

	/*
	 * Go through all children and check validity, make minor corrections, 
	 * etcetera.
	 */
	function formatall(array $paras);

	/*
	 * List all children.
	 */
	function listmembers(array $paras);
	
	/*
	 * Provide statistics.
	 */
	function stats(array $paras = array());
	
	/*
	 * Sort the children.
	 * TODO: should this be in the interface? Let's think about where sorting is
	 * needed and how to implement it.
	 */
	function sort(array $paras = array());
	
	/*
	 * Make a list of the possible values of a field.
	 */
	function mlist(array $paras);
	
	/*
	 * Queries.
	 */
	function bfind(array $paras);
	
	/*
	 * Simple wrapper for bfind.
	 */
	function find_cmd(array $paras);
	
	/*
	 * Some little-used stuff we're keeping for now.
	 */
	/* statistics */
	public function average($files, $field);
	public function stdev($files, $field);
	public function smallest($files, $field, array $paras = array());
	public function largest($files, $field, array $paras = array());
	public function getstats(array $paras);
	public function listz(array $paras);
	/* logging and backups */
	public function log($msg);
	public function backup(array $paras = array());
}
