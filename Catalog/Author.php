<?php
class Author extends SqlListEntry {
	protected $id;
	
	protected $firstnames;
	
	protected $name;
	
	protected static $Author_commands = array(
	);
	
	protected static function fillFields() {
		return array(
			'id' => new SqlProperty(array(
				'name' => 'id',
				'type' => SqlProperty::ID)),
			'firstnames' => new SqlProperty(array(
				'name' => 'firstnames',
				'type' => SqlProperty::STRING)),
			'name' => new SqlProperty(array(
				'name' => 'name',
				'type' => SqlProperty::STRING)),
		);
	}
}
