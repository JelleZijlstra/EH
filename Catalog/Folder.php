<?php
class Folder extends SqlListEntry {
	protected $id;
	
	protected $parent;
	
	protected $name;
	
	protected $children;
	
	public function path() {
		$this->fillProperties();
		if($this->parent === NULL) {
			$path = '';
		} else {
			$path =  $this->parent->path();
		}
		return $path . '/' . $this->name;
	}
	
	private function getChildByName($name) {
		$this->fillProperties();
		foreach($this->children as $child) {
			if($child->name() === $name) {
				return $child;
			}
		}
		return false;
	}
	
	// Adds a child folder of name $name, returns the Folder object
	private function addChild($name) {
		$obj = self::newWithData(array(
			'parent' => $this,
			'name' => $name,
		), $this->p);
		$this->children[] = $obj;
		return $obj;
	}
	
	/*
	 * Make sure that the path in $in exists as a chain of folders. Return the 
	 * tip of the folder tree.
	 */
	public function makeChildPath(array $in) {
		if(count($in) === 0) {
			return $this;
		} else {
			$folder = array_shift($in);
			$child = $this->getChildByName($folder);
			if($folder === false) {
				$child = $this->addChild($folder);
			}
			return $child->makeChildPath($in);
		}
	}

	public static function withArray(array $in) {
		assert(isset($in[0]));
		$f = array_shift($in);
		$parentFolder = Folder::withName($f);
		return $parentFolder->makeChildPath($in);
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
