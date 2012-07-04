<?php
class Age extends SqlListEntry {
	protected $id;
	
	protected $parent;
	
	protected $name;
	
	protected $start;
	
	protected $end;
	
	protected static $Folder_commands = array(
	);
	
	protected static function fillFields() {
		return array(
			'id' => new SqlProperty(array(
				'name' => 'id',
				'type' => SqlProperty::ID)),
			'name' => new SqlProperty(array(
				'name' => 'name',
				'type' => SqlProperty::STRING)),
			'parent' => new SqlProperty(array(
				'name' => 'parent',
				'type' => SqlProperty::REFERENCE,
				'referredClass' => 'Age')),
			'start_time' => new SqlProperty(array(
				'name' => 'start_time',
				'type' => SqlProperty::INT)),
			'end_time' => new SqlProperty(array(
				'name' => 'end_time',
				'type' => SqlProperty::INT)),
		);
	}
}
