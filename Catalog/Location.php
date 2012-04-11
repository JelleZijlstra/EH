<?php
class Location extends SqlListEntry {
	protected $id;
	
	protected $parent;
	
	protected $name;
	
	protected static $Location_commands = array(
	);
	
	public function fields() {
		return array('id', 'parent', 'name');
	}
}
