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
	const TIMESTAMP = 0x4;
	private $type;
	public function getType() {
		return $this->type;
	}

	// for properties of type SqlProperty::REFERENCE, name of the class they
	// refer to
	private $referredClass;
	public function getReferredClass() {
		return $this->referredClass;
	}
	
	public function __construct($data) {
		if(isset($data['name'])) {
			$this->name = $data['name'];
		} else {
			throw new EHException('name not given in SqlProperty constructor');
		}
		if(isset($data['validator'])) {
			$this->validator = $data['validator'];
		} else {
			// default validator accepts anything
			$this->validator = function($in) {
				return true;
			};
		}
		if(isset($data['type'])) {
			$this->type = $data['type'];
		} else {
			// default type is string
			$this->type = self::STRING;
		}
		if(isset($data['referredClass'])) {
			$this->referredClass = $data['referredClass'];
		} else {
			if($this->type === self::REFERENCE) {
				throw new EHException(
					'referredClass not specified for SqlProperty of type REFERENCE');
			} else {
				$this->referredClass = NULL;
			}
		}
	}
}