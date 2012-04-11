<?php
class Author extends SqlListEntry {
	protected $id;
	
	protected $firstnames;
	
	protected $name;
	
	protected static $Author_commands = array(
	);
	
	public function fields() {
		return array('id', 'firstnames', 'name');
	}
}
