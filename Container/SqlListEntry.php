<?php
/*
 * SqlListEntry.php
 * Defines a class for objects representing database rows
 */
abstract class SqlListEntry extends ListEntry {
	const CONSTR_ID = 0;
	const CONSTR_NAME = 1;
	const CONSTR_FULL = 2;

	private $needsave = false;
	private $filledProperties = false;

	private static $SqlListEntry_commands = array(

	);

	/*
	 * Constructor: only does EH setup stuff.
	 */
	public function __construct($data, $code, &$parent) {
		$this->p =& $parent;
		// hack needed to satisfy ListEntry::add() for now
		$doAddNew = false;
		if($code === 'n') {
			$code = self::CONSTR_NAME;
			$data = $data['name'];
			$doAddNew = true;
		}
		
		switch($code) {
			case self::CONSTR_ID:
				$this->id = $data;
				break;
			case self::CONSTR_NAME:
				$this->name = $data;
				break;
			case self::CONSTR_FULL:
				$this->fillPropertiesOnce = true;
				$this->set($data);
				break;
			default:
				throw new EHException(
					'Invalid code for SqlListEntry constructor', 
					EHException::E_RECOVERABLE
				);
		}
		
		if($doAddNew) {
			$this->addNew();
		}

		// TODO: figure out how to get the commands all right
		// parent::__construct(array_merge($mcds, self::$SqlListEntry_commands));
	}
	
	/*
	 * Either return the object with name $name or create a new object.
	 */
	public static function withName(/* string */ $name) {
		$parent = static::parentClass();
		$parentObj = $parent::singleton();
		if($parentObj->has($name)) {
			return $parentObj->get($name);
		} else {
			$newName = $this->menu(array(
				'head' =>
					'There is no entry with the name ' . $name . '. Please '
					. 'enter a correct name, or type "n" to create a new ' 
					. 'entry.',
				'options' => array(
					'n' => 'Create a new entry',
				),
				'validfunction' => function($in) use($parentObj) {
					return $parentObj->has($in);
				},
				'process' => array(
					'n' => function(&$cmd) {
						$cmd = false;
						return false;
					},
				),
			));
			if($newName === false) {
				$obj = new static($name, self::CONSTR_NAME, $parentObj);
				$obj->addNew();
				$parentObj->addEntry($obj);
				return $obj;
			} else {
				return $parentObj->get($newName);
			}
		}
	}

	/*
	 * Either return the object with name $name or create a new object.
	 */
	public static function withId(/* int */ $id) {
		$parent = static::parentClass();
		$parentObj = $parent::singleton();
		if($parentObj->hasIdInMemory($id)) {
			return $parentObj->get($name);
		} else {
			$obj = new static($id, self::CONSTR_ID, $parentObj);
			$parentObj->addEntryWithId($obj);
			return $obj;
		}
	}

	/*
	 * Getter: fills properties if necessary.
	 */
	public function __get($key) {
		if(!property_exists($this, $key)) {
			throw new EHException(
				'Access to unknown property "' . $key . '" of class ' 
					. get_called_class(),
				EHException::E_RECOVERABLE);
		}
		if(!$this->filledProperties && $this->$key === NULL) {
			$this->fillProperties();
		}
		return $this->$key;
	}

	/*
	 * Setter: sets properties and sets dirty bit.
	 */
	public function __set($key, $value) {
		if(!property_exists($this, $key)) {
			throw new EHException(
				'Access to unknown property "' . $key . '" of class ' 
					. get_called_class(),
				EHException::E_RECOVERABLE);
		}
		$this->$key = $value;
		$this->needSave();
	}

	/*
	 * Isset: does property exist?
	 */
	public function __isset(/* string */ $key) {
		return property_exists($this, $key);
	}

	/*
	 * Unset: do we need this at all? Perhaps it can be useful in some way.
	 */
	public function __unset(/* string */ $key) {
		throw new EHException(
			'Attempt to unset property "' . $key . '" of class '
				. get_called_class(),
			EHException::E_RECOVERABLE);
	}

	/*
	 * Fill a new object. Default implementation merely asks the user; children
	 * may do more.
	 */
	protected function addNew() {
		// there will be nothing left to fill
		$this->fillPropertiesOnce = true;
		$out = $this->fillPropertiesManually();
		return $out;
	}
	
	/*
	 * Fill properties manually from user input.
	 */
	protected function fillPropertiesManually() {
		$fields = self::fields();
		$file = $this;
		echo 'Filling data manually for object' . PHP_EOL;
		$hasId = false;
		try {
			foreach($fields as $field) {
				$fname = $field->getName();
				if($this->$fname !== NULL) {
					continue;
				}
				$type = $field->getType();
				switch($type) {
					case SqlProperty::ID:
						$hasId = $fname;
						$this->$fname = NULL;
						break;					
					case SqlProperty::REFERENCE:
					case SqlProperty::STRING:
					case SqlProperty::INT:
					case SqlProperty::BOOL:
					case SqlProperty::CHILDREN:
					case SqlProperty::JOINT_REFERENCE:
					case SqlProperty::CUSTOM:
						$manualFiller = $field->getManualFiller();
						$this->$fname = $manualFiller($this, $this->p->table());
						break;
					case SqlProperty::TIMESTAMP:
						// let it be filled in automatically by MySQL
						break;
				}
			}
		} catch(StopException $e) {
			// ignore it
		}
		// insert into DB, record ID
		$result = Database::singleton()->insert(array(
			'into' => $this->p->table(),
			'values' => $this->toArray(),
		));
		if($hasId !== false) {
			$this->$hasId = $result;
		}
		// record success
		echo 'Added to database' . PHP_EOL;
		return true;	
	}

	/*
	 * Function to fill properties through a DB query.
	 */
	private $fillPropertiesOnce = false;
	protected function fillProperties() {
		// no need to call this function more than once
		if($this->fillPropertiesOnce) {
			return true;
		}
		$this->fillPropertiesOnce = true;
		
		// get existing data
		$id = $this->id();
		if($id !== NULL) {
			$vars = Database::singleton()->select(array(
				'from' => $this->p->table(),
				'where' => array('id' => $id),
			));
		} else {
			$name = $this->name();
			if($name !== NULL) {
				$vars = Database::singleton()->select(array(
					'from' => $this->p->table(),
					'where' => array('name' => $name),
				));
			} else {
				throw new EHException("Unable to fill properties",
					EHException::E_RECOVERABLE);
			}
		}
		if(count($vars) === 1) {
			return $this->set($vars[0]);
		} else {
			throw new EHException(
				"Multiple instances of id " . $id . " detected in table "
					. $this->p->table(),
				EHException::E_RECOVERABLE
			);
		}
	}

	/*
	 * Set properties from an array.
	 */
	public /* bool */ function set(array $paras) {
	// default method; should be overridden by child classes with more precise 
	// needs
		foreach($paras as $key => $value) {
			if(substr($key, -3, 3) === '_id') {
				$key = substr($key, 0, -3);
			}
			$fieldObject = self::getFieldObject($key);
			if($fieldObject === NULL) {
				$this->setFakeProperty($key, $value);
				continue;
			}
			$processor = $fieldObject->getProcessor();
			$value = $processor($value);
			$validator = $fieldObject->getValidator();
			if(!$validator($value)) {
				echo 'set: error: invalid value '
					. Sanitizer::varToString($value) . ' for field '
					. Sanitizer::varToString($key);
				continue;
			}
			$this->needSave();
			// TODO: check whether any types need more things to do
			$this->$field = $content;
		}
		return true;
	}
	protected function setFakeProperty($field, $value) {
		// Subclasses may allow "setting" more properties
		throw new EHInvalidInputException($field);
	}

	/*
	 * Set properties from an array returned by the database.
	 */
	private /* bool */ function setPropertiesFromSql(array $in) {
		$fields = self::fields();
		foreach($fields as $field) {
			$name = $field->getName();
			$processor = $field->getProcessor();
			if(isset($in[$name])) {
				$value = $processor($in[$name]);
			}
			switch($field->getType()) {
				case SqlProperty::INT:
				case SqlProperty::TIMESTAMP:
				case SqlProperty::STRING:
				case SqlProperty::BOOL:
				case SqlProperty::REFERENCE:
				case SqlProperty::ID:
					$this->$name = $value;
					break;
				case SqlProperty::CHILDREN:
				case SqlProperty::CUSTOM:
					$creator = $field->getAutomatedFiller();
					$this->$name = $creator($this->id);
					break;
				case SqlProperty::JOINT_REFERENCE:
					if($this->id === NULL) {
						throw new EHException(
							"Unable to set JOINT_REFERENCE property");
					}
					$otherClass = $field->getReferredClass();
					$thisClassId = get_called_class() . '_id';
					$otherClassId = strtolower($otherClass) . '_id';
					$entries = Database::singleton()->select(array(
						'fields' => array($otherClassId),
						'from' => $field->getTableName(),
						'where' => array(
							$thisClassId => Database::escapeValue($this->id),
						),
					));
					$out = array();
					foreach($entries as $entry) {
						$out[] = $otherClass::withId($entry[$otherClassId]);
					}
					$this->$name = $out;
					break;
				default:
					throw new EHException(
						'Unrecognized type: ' . $field->getType());
			}
		}
		return true;
	}

	/*
	 * Saving changes in the properties of the object.
	 */
	public function needSave() {
		$this->needsave = true;
	}

	public function save() {
		// Commits changes in simple properties, which are cached in memory.
		// Changes in complicated structures like JOINT_REFERENCEs should be 
		// sent to the DB immediately.
		if($this->needsave === false) {
			return true;
		}
		// avoid committing changes while some properties are not filled
		$this->fillProperties();
		Database::singleton()->insert(array(
			'into' => $this->p->table(),
			'values' => $this->toArray(),
			'replace' => true,
		));
	}
	/*
	 * Converts itself into an array corresponding to the object's DB 
	 * representation.
	 */
	public function toArray() {
		$this->fillProperties();
		$fields = array();
		foreach(self::fields() as $field) {
			$name = $field->getName();
			switch($field->getType()) {
				case SqlProperty::INT:
				case SqlProperty::TIMESTAMP:
				case SqlProperty::STRING:
				case SqlProperty::BOOL:
				case SqlProperty::ID:
					if($this->$name !== NULL) {
						$fields[$name] = $this->$name;
					}
					break;
				case SqlProperty::REFERENCE:
					$fields[$name] = $this->$name->id();
					break;
				case SqlProperty::CUSTOM:
				case SqlProperty::JOINT_REFERENCE:
					// ignored
					break;
			}
		}
		return $fields;
	}

	/*
	 * Return an identifier to be used as an array key. We assume that these are
	 * the "name" and "id" properties of the object, but child classes may make
	 * a different decision.
	 */
	public function name() {
		$this->fillProperties();
		return $this->name;
	}
	public function id() {
		return $this->id;
	}

	/* 
	 * Destructor makes sure changes are committed.
	 */
	public function __destruct() {
		$this->save();
	}

	/*
	 * __toString() shall return the name.
	 */
	final public function __toString() {
		return $this->name();
	}

	protected function listproperties() {
		return self::fieldsAsStrings();
	}
	
	/*
	 * Return the name of the parent class
	 */
	private static function parentClass() {
		return get_called_class() . 'List';
	}
}
