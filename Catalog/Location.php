<?php
class Location extends SqlListEntry {
	protected $id;
	
	protected $parent;
	
	protected $name;
	
	protected static $Location_commands = array(
	);
	
	protected static function fillFields() {
		return array(
			'id' => new SqlProperty(array(
				'name' => 'id',
				'type' => SqlProperty::ID)),
			'parent' => new SqlProperty(array(
				'name' => 'parent',
				'type' => SqlProperty::REFERENCE,
				'referredClass' => 'Folder')),
			'name' => new SqlProperty(array(
				'name' => 'name',
				'type' => SqlProperty::STRING)),
		);
	}
}
