<?php
class City extends SqlListEntry {
	protected $id;
	
	protected $name;
	
	protected $state;
	
	protected $country;
	
	protected static $City_commands = array(
	);
	
	protected static function fillFields() {
		return array(
			'id' => new SqlProperty(array(
				'name' => 'id',
				'type' => SqlProperty::ID)),
			'name' => new SqlProperty(array(
				'name' => 'name',
				'type' => SqlProperty::STRING)),
			'state' => new SqlProperty(array(
				'name' => 'state',
				'type' => SqlProperty::STRING)),
			'country' => new SqlProperty(array(
				'name' => 'country',
				'type' => SqlProperty::STRING)),
		);
	}
}
