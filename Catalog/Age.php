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
		return array(
			new SqlProperty(array(
				'name' => 'id',
				'type' => SqlProperty::ID)),
			new SqlProperty(array(
				'name' => 'name',
				'type' => SqlProperty::STRING)),
			new SqlProperty(array(
				'name' => 'parent',
				'type' => SqlProperty::REFERENCE,
				'referredClass' => 'Age')),
			new SqlProperty(array(
				'name' => 'start_time',
				'type' => SqlProperty::INT)),
			new SqlProperty(array(
				'name' => 'end_time',
				'type' => SqlProperty::INT)),
	}
}
