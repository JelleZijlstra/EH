<?php
/*
 * Property objects for both CsvContainerList and SqlContainerList objects.
 */
abstract class Property {
	/*
	 * name of the field
	 */
	private $name;
	public function getName() {
		return $this->name;
	}
	protected function nameDefault() {
		throw new EHException('name not given in SqlProperty constructor');	
	}
	public function __toString() {
		return $this->name;
	}

	/*
	 * type of the field
	 */
	const INT = 0x1;
	const STRING = 0x2;
	const REFERENCE = 0x3; // a reference (id) to another object
	const ID = 0x5; // identifier of this entry
	const TIMESTAMP = 0x4;
	const BOOL = 0x6;
	const CUSTOM = 0x7; // custom type
	const CHILDREN = 0x8; // list of children
	const JOINT_REFERENCE = 0x9; // list of references in a different table
	private $type;
	public function getType() {
		return $this->type;
	}
	protected function typeDefault() {
		$this->type = self::STRING;	
	}

	/*
	 * Validator: function that, given a value for this field, returns a bool 
	 * representing whether the value is valid.
	 */
	private $validator;
	public function getValidator() {
		return $this->validator;
	}
	protected function validatorDefault() {
		// default validator accepts anything of the right type
		switch($this->getType()) {
			case self::INT:
			case self::REFERENCE:
			case self::ID:
				$this->validator = function($in) {
					return is_int($in) || preg_match('/^\d+$/', $in);
				};
				break;
			case self::BOOL:
				$this->validator = function($in) {
					return is_bool($in)
						|| ($in === '')
						|| preg_match('/^(1|0|true|false)$/', $in);
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
			case self::JOINT_REFERENCE:
				$this->validator = function($in) {
					return is_array($in);
				};
				break;
			case self::CUSTOM:
				$this->validator = function() {
					return true;
				};
				break;
			default:
				throw new EHInvalidInputException($this->getType());
		}
	}
	
	/*
	 * Processor: function that, given a value, returns a corrected version.
	 */
	private $processor;
	public function getProcessor() {
		return $this->processor;
	}
	protected function processorDefault() {
		switch($this->getType()) {
			case self::ID:
			case self::INT:
				$this->processor = function($in) {
					return (int) $in;
				};
				break;
			case self::STRING:
				$this->processor = function($in) {
					return (string) $in;
				};
				break;
			case self::BOOL:
				$this->processor = function($in) {
					if($in === 'false' || $in === '0') {
						return false;
					} else {
						return (bool) $in;
					}
				};
				break;
			case self::REFERENCE:
				$referredClass = $this->getReferredClass();
				$this->processor = function($in) use($referredClass) {
					if($in instanceof $referredClass) {
						return $in;
					} elseif(is_string($in)) {
						return $referredClass::withName($in);
					} else {
						return NULL;
					}
				};
				break;
			case self::TIMESTAMP:
			case self::CHILDREN:
			case self::JOINT_REFERENCE:
			case self::CUSTOM:
				$this->processor = function($in) {
					return $in;
				};
				break;
		}
	}
	
	/*
	 * Constructor
	 */
	protected function set(array $data, $field) {
		if(isset($data[$field])) {
			$this->$field = $data[$field];
		} else {
			$this->{$field . 'Default'}();
		}
	}
	
	public function __construct(array $data) {
		$this->set($data, 'name');
		$this->set($data, 'type');
		$this->set($data, 'validator');
		$this->set($data, 'processor');
	}
}
