<?php
class Folder extends SqlListEntry {
	protected $id;
	
	protected $parent;
	
	protected $name;
	
	protected static $Folder_commands = array(
	);
	
	public function fields() {
		return array(
			new SqlProperty(array(
				'name' => 'id',
				'type' => SqlProperty::ID)),
			new SqlProperty(array(
				'name' => 'parent',
				'type' => SqlProperty::REFERENCE,
				'referredClass' => 'Folder')),
			new SqlProperty(array(
				'name' => 'name',
				'type' => SqlProperty::STRING)),
		);
	}
}
