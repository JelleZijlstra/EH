<?php
class Publisher extends SqlListEntry {
	protected $id;
	
	protected $name;
	
	protected $location;
	
	protected static $Publisher_commands = array(
	);
	
	public function fields() {
		return array('id', 'name', 'location');
	}
}
