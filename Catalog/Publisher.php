<?php
class Publisher extends SqlListEntry {
	protected $id;
	
	protected $name;
	
	protected $location;
	
	protected static $Publisher_commands = array(
	);
	
	protected static function fillFields() {
		return array(
			'id' => new SqlProperty(array(
				'name' => 'id',
				'type' => SqlProperty::ID)),
			'name' => new SqlProperty(array(
				'name' => 'name',
				'type' => SqlProperty::STRING)),
			'cities' => new SqlProperty(array(
				'name' => 'cities',
				'type' => SqlProperty::JOINT_REFERENCE,
				'referredClass' => 'city',
				'tableName' => 'publisher_city')),
		);
	}
}
