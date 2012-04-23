<?php
/*
 * SqlProperty.php
 *
 * A class representing an individual property in a MySQL table.
 */
class SqlProperty {
	// name of the field
	private $name;
	public function getName() {
		return $this->name;
	}
	public function __toString() {
		return $this->name;
	}

	// function that, given a value for this field, returns a bool representing
	// whether the value is valid
	private $validator;
	public function getValidator() {
		return $this->validator;
	}

	// type of the field
	const INT = 0x1;
	const STRING = 0x2;
	const REFERENCE = 0x3; // a reference (id) to another object
	const ID = 0x5; // identifier of this entry
	const TIMESTAMP = 0x4;
	const BOOL = 0x6;
	const CUSTOM = 0x7; // custom type
	const CHILDREN = 0x8; // list of children
	private $type;
	public function getType() {
		return $this->type;
	}

	// for properties of type SqlProperty::REFERENCE, name of the class they
	// refer to
	private $referredClass = NULL;
	public function getReferredClass() {
		return $this->referredClass;
	}
	
	// for custom properties, function that fills the field
	private $creator = NULL;
	public function getCreator() {
		return $this->creator;
	}
	
	public function __construct($data) {
		if(isset($data['name'])) {
			$this->name = $data['name'];
		} else {
			throw new EHException('name not given in SqlProperty constructor');
		}
		if(isset($data['type'])) {
			$this->type = $data['type'];
		} else {
			// default type is string
			$this->type = self::STRING;
		}
		if(isset($data['referredClass'])) {
			$this->referredClass = $data['referredClass'];
		} elseif($this->type === self::REFERENCE) {
			throw new EHException(
				'referredClass not specified for SqlProperty of type REFERENCE');
		}
		if(isset($data['validator'])) {
			$this->validator = $data['validator'];
		} else {
			// default validator accepts anything of the right type
			switch($this->type) {
				case self::INT:
				case self::REFERENCE:
				case self::ID:
					$this->validator = function($in) {
						return is_int($in) || preg_match('/^\d+$/', $in);
					};
					break;
				case self::BOOL:
					$this->validator = function($in) {
						return is_bool($in);
					};
					break;
				case self::STRING:
					$this->validator = function($in) {
						return is_string($in);
					};
					break;
				case self::TIMESTAMP:
					$this->validator = function($in) {
						// TODO: check what MySQL actually returns here
						return true;
					};
					break;
				case self::CHILDREN:
					$this->validator = function($in) {
						return is_array($in);
					};
					break;
			}
		}
		if(isset($data['creator'])) {
			$this->creator = $data['creator'];
		} elseif($this->type === self::CUSTOM) {
			throw new EHException('creator not specified for SqlProperty of '
				. 'type CUSTOM');
		}
	}
}
