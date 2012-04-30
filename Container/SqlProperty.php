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
	
	// function that, given a value, returns a corrected version
	private $processor;
	public function getProcessor() {
		return $this->processor;
	}
	
	// function that fills a property
	private $manualFiller;
	public function getManualFiller() {
		return $this->manualFiller;	
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
	const JOINT_REFERENCE = 0x9; // list of references in a different table
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
	
	// table name for JOINT_REFERENCEs
	private $tableName = NULL;
	public function getTableName() {
		return $this->tableName;
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
						return is_bool($in) 
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
			}
		}
		if(isset($data['processor'])) {
			$this->processor = $data['processor'];
		} else {
			switch($this->type) {
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
					$referredClass = $this->referredClass;
					$this->processor = function($in) use($referredClass) {
						return $referredClass::withName($in);
					};
					break;
				case self::TIMESTAMP:
				case self::ID:
				case self::CHILDREN:
				case self::JOINT_REFERENCE:
				case self::CUSTOM:
					$this->processor = function($in) {
						return $in;
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
		if(isset($data['manualFiller'])) {
			$this->manualFiller = $data['manualFiller'];
		} else switch($this->type)
			case self::CHILDREN:
				$this->manualFiller = function(SqlListEntry $file, /* string */ $table) {
					if($file->id() === NULL) {
						throw new EHException("Unable to set children array");
					}
					$children = Database::singleton()->select(array(
						'from' => $table,
						'where' => array(
							'parent' => Database::escapeValue($file->id()),
						),
					));
					$out = array();
					$class = ucfirst($table);
					foreach($children as $child) {
						$out[] = $class::withId($in['id']);
					}
					return $out;
				};
				break;
			case self::JOINT_REFERENCE:
				$this->manualFiller = function(SqlListEntry $file, /* string */ $table) {
					$referred = array();
					while(true) {
						$cmd = $this->menu(array(
							'head' => 
								'Please provide entries for field ' . $name,
							'options' => array('q' => 'Stop adding entries'),
							'processcommand' => function(&$cmd, &$data) {
								
							},
						));
					}
				};
				break;
			case SqlProperty::REFERENCE:
			case SqlProperty::STRING:
			case SqlProperty::INT:
			case SqlProperty::BOOL:
				$this->manualFiller = function(SqlListEntry $file, $table) {
					return $file->menu(array(
						'head' => $fname,
						'headasprompt' => true,
						'options' => array(
							'e' => "Enter the file's command-line interface",
						),
						'validfunction' => $field->getValidator(),
						'process' => array(
							'e' => function() use($file) {
								$file->edit();
								return true;
							},
						),
						'processcommand' => function($in) use($field) {
							$out = $in;
							// enable having 'e' or 's' as field by typing '\e'
							if($in[0] === '\\') {
								$out = substr($in, 1);
							}
							$processor = $field->getProcessor();
							return $processor($out);
						},
					));				
				};
				break;
			default:
				$this->manualFiller = function() {
					throw new EHException("Use of unspecified manualFiller");
				};
				break;
		}
	}
}
