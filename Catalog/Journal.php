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
		static $fields = array(
			new SqlProperty(array(
				'name' => 'id',
				'type' => SqlProperty::ID)),
			new SqlProperty(array(
				'name' => 'name',
				'type' => SqlProperty::STRING)),
			new SqlProperty(array(
				'name' => 'issn',
				'validator' => function($in) {
					return preg_match('/^\d{4}-\d{4}$/', $in);
				},
				'type' => SqlProperty::STRING)),
			new SqlProperty(array(
				'name' => 'nopagenumber',
				'type' => SqlProperty::BOOL)),
		);
		return $fields;
	}
}
