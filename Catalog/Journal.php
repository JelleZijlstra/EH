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

	static protected function fillFields() {
		return array(
			'id' => new SqlProperty(array(
				'name' => 'id',
				'type' => SqlProperty::ID)),
			'name' => new SqlProperty(array(
				'name' => 'name',
				'type' => SqlProperty::STRING)),
			'issn' => new SqlProperty(array(
				'name' => 'issn',
				'validator' => function($in) {
					return preg_match('/^\d{4}-\d{4}$/', $in);
				},
				'type' => SqlProperty::STRING)),
			'nopagenumber' => new SqlProperty(array(
				'name' => 'nopagenumber',
				'type' => SqlProperty::BOOL)),
		);
	}
}
