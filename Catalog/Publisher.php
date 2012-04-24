<?php
class Publisher extends SqlListEntry {
	protected $id;
	
	protected $name;
	
	protected $location;
	
	protected static $Publisher_commands = array(
	);
	
	public function fields() {
		return array(
			new SqlProperty(array(
				'name' => 'id',
				'type' => SqlProperty::ID)),
			new SqlProperty(array(
				'name' => 'name',
				'type' => SqlProperty::STRING)),
			new SqlProperty(array(
				'name' => 'cities',
				'type' => SqlProperty::JOINT_REFERENCE,
				'referredClass' => 'city',
				'tableName' => 'publisher_city')),
		);
	}
}
