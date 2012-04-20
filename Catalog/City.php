<?php
class City extends SqlListEntry {
	protected $id;
	
	protected $name;
	
	protected $state;
	
	protected $country;
	
	protected static $City_commands = array(
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
				'name' => 'state',
				'type' => SqlProperty::STRING)),
			new SqlProperty(array(
				'name' => 'country',
				'type' => SqlProperty::STRING)),
		);
	}
}
