<?php
class Author extends SqlListEntry {
	protected $id;
	
	protected $first_names;
	
	protected $name;
	
	protected $suffix;
	
	protected static $Author_commands = array(
	);
	
	public function initials() {
		if(strpos($this->first_names, ' ') !== false) {
			return array_map(function($name) {
				return $name[0] . '.';
			}, explode(' ', $this->first_names));
		} else {
			return $this->first_names;
		}
	}
	
	public function toArray() {
		return array($this->initials(), $this->name, $this->suffix);
	}
	
	public static function withArray(array $in) {
		// TODO: get Author object corresponding to this array
	}
	
	protected static function fillFields() {
		return array(
			'id' => new SqlProperty(array(
				'name' => 'id',
				'type' => SqlProperty::ID)),
			'first_names' => new SqlProperty(array(
				'name' => 'first_names',
				'type' => SqlProperty::STRING,
				'validator' => function($in) {
					return ($in === '') 
						|| preg_match('/^(\w+( \w+)*|(\w\w?\.)+)$/', $in);
				})),
			'name' => new SqlProperty(array(
				'name' => 'name',
				'type' => SqlProperty::STRING)),
			'suffix' => new SqlProperty(array(
				'name' => 'suffix',
				'type' => SqlProperty::STRING
				'validator' => function($in) {
					return ($in === 'Jr.') 
						|| ($in === 'Sr.')
						|| preg_match('/^[IV]+$/', $in);
				})),
		);
	}
}
