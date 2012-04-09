<?php
/*
 * Model to represent an individual journal.
 */
class Journal extends SqlListEntry {
	// ID of the journal in the database
	protected $id;
	// name of the journal
	protected $name;
	// ISSN
	protected $issn;
	// whether it's a journal without page numbers
	protected $nopagenumber;
		
	protected static $Journal_commands = array(
	
	);

	public function fields() {
		return array('id', 'name', 'issn', 'nopagenumber');
	}
}
