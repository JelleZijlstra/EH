<?php
class Age extends SqlListEntry {
	protected $id;
	
	protected $parent;
	
	protected $name;
	
	protected $start;
	
	protected $end;
	
	protected static $Folder_commands = array(
	);
	
	public function fields() {
		return array('id', 'parent', 'name', 'start', 'end');
	}
}
