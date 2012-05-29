<?php
require_once(BPATH . '/Catalog/settings.php');

class Folder extends SqlListEntry {
	protected $id;
	
	protected $parent;
	
	protected $name;
	
	protected $children;
	
	public function path() {
		$this->fillProperties();
		if($this->parent === NULL) {
			$path = LIBRARY;
		} else {
			$path =  $this->parent->path();
		}
		return $path . '/' . $this->name;
	}
	
	protected static $Folder_commands = array(
	);
	
	protected static function fillFields() {
		return array(
			'id' => new SqlProperty(array(
				'name' => 'id',
				'type' => SqlProperty::ID)),
			'parent' => new SqlProperty(array(
				'name' => 'parent',
				'type' => SqlProperty::REFERENCE,
				'referredClass' => 'Folder')),
			'name' => new SqlProperty(array(
				'name' => 'name',
				'type' => SqlProperty::STRING)),
			'children' => new SqlProperty(array(
				'name' => 'children',
				'type' => SqlProperty::CHILDREN)),
		);
	}
}
