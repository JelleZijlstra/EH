<?php
class Folder extends SqlListEntry {
	protected $id;
	
	protected $parent;
	
	protected $name;
	
	protected static $Folder_commands = array(
	);
	
	public function fields() {
		return array('id', 'parent', 'name');
	}
}
