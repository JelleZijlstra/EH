<?php
class Author extends SqlListEntry {
	protected $id;
	
	protected $firstnames;
	
	protected $name;
	
	protected static $Author_commands = array(
	);
	
	public function fields() {
		return array(
			new SqlProperty(array(
				'name' => 'id',
				'type' => SqlProperty::ID)),
			new SqlProperty(array(
				'name' => 'firstnames',
				'type' => SqlProperty::STRING)),
			new SqlProperty(array(
				'name' => 'name',
				'type' => SqlProperty::STRING)),
		);
	}
}
