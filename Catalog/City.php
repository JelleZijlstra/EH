<?php
class City extends SqlListEntry {
	protected $id;
	
	protected $name;
	
	protected $state;
	
	protected $country;
	
	protected static $City_commands = array(
	);
	
	public function fields() {
		return array('id', 'name', 'state', 'country');
	}
}
