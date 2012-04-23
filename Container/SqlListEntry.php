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
		switch($code) {
			case self::CONSTR_ID:
				$this->id = $data;
				break;
			case self::CONSTR_NAME:
				$this->name = $data;
				break;
			case self::CONSTR_FULL:
				$this->setProperties($data);
				break;
			default:
				throw new EHException(
					'Invalid code for SqlListEntry constructor', 
					EHException::E_RECOVERABLE
				);
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
			$obj = new static($name, self::CONSTR_NAME, $parentObj);
			$obj->addNew();
			$parentObj->addEntry($obj);
			return $obj;
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
			$parentObj->addEntry($obj);
			return $obj;
		}
	}
	
	/*
	 * Return all the fields of the object that are filled from the DB, in the
	 * form of an array of SqlProperty objects.
	 */
	abstract protected function fields();
	
	final protected function fieldsAsStrings() {
		return array_map(function($in) {
			return $in->name;
		}, $fields);
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
		return $this->fillPropertiesManually();
	}
	
	/*
	 * Fill properties manually from user input.
	 */
	protected function fillPropertiesManually() {
		$fields = $this->fields();
		$file = $this;
		echo 'Filling data manually for object' . PHP_EOL;
		foreach($fields as $field) {
			$fname = $field->name();
			if($this->$fname !== NULL) {
				break;
			}
			$cmd = $this->menu(array(
				'head' => $fname,
				'headasprompt' => true,
				'options' => array(
					'e' => "Enter the file's command-line interface",
					's' => "Save the file now",
				),
				'validfunction' => $field->getValidator(),
				'process' => array(
					'e' => function() use($file) {
						$file->edit();
						return true;
					},
					's' => function(&$cmd) {
						$cmd = false;
						return false;
					},
				),
			));
			if($cmd === false) {
				return true;
			}
			$this->$fname = $cmd;
		}
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
			return $this->setProperties($vars[0]);
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
	private /* bool */ function setProperties(array $in) {
		$fields = $this->fieldsAsStrings();
		foreach($in as $key => $value) {
			if(substr($key, -3, 3) === '_id') {
				// Get the relevant names. We don't actually need to care about
				// case, because PHP class names are case-insensitive, but let's
				// be precise.
				$name = substr($key, 0, -3);
				if(!in_array($name, $fields, true)) {
					throw new EHException('Invalid data key ' . $key);
				}
				$id = (int) $value;

				if($id === 0) {
					$this->$name = NULL;
				} else {
					$className = ucfirst($name);
					$parentClass = $this->parentClass();
					$list = $parentClass::singleton();
					$this->$name = $list->fromId($id);
				}
			} elseif($key === 'parent') {
				$id = (int) $value;
				if($id === 0) {
					$this->parent = NULL;
				} else {
					$this->parent = $this->p->fromId($id);
				}
			} else {
				if(!in_array($key, $fields, true)) {
					throw new EHException('Invalid data key ' . $key);
				}
				$this->$key = $value;
			}
		}
		return true;
	}

	/*
	 * Set properties from an array returned by the database.
	 */
	private /* bool */ function setPropertiesFromSql(array $in) {
		$fields = $this->fields();
		foreach($fields as $field) {
			$name = $field->getName();
			switch($field->getType()) {
				case SqlProperty::INT:
				case SqlProperty::TIMESTAMP:
				case SqlProperty::STRING:
					$this->$name = $in[$name];
					break;
				case SqlProperty::BOOL:
					$this->$name = (bool) $in[$name];
					break;
				case SqlProperty::REFERENCE:
					$referredClass = $field->getReferredClass();
					$this->$name = $referredClass::fromId($in[$name . '_id']);
					break;
				case SqlProperty::ID:
					$this->id = $in['id'];
					break;
				case SqlProperty::CUSTOM:
					$creator = $field->getCreator();
					$this->$name = $creator($this->id);
					break;
				case SqlProperty::CHILDREN:
					if($this->id === NULL) {
						throw new EHException("Unable to set children array");
					}
					$children = Database::singleton()->select(array(
						'from' => $this->p->table(),
						'where' => array(
							'parent' => Database::escapeValue($this->id),
						),
					));
					$this->$name = array();
					foreach($children as $child) {
						$this->$name[] = self::fromId($in['id']);
					}
					break;
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
		if($this->needsave === false) {
			return true;
		}
		// avoid committing changes while some properties are not filled
		if($this->filledProperties === false) {
			$this->fillProperties();
		}
		// commit changes
	}
	/*
	 * Converts itself into an array corresponding to the object's DB 
	 * representation.
	 */
	public function toArray() {
		$this->fillProperties();
		$out = array();
		foreach($this->fieldsAsStrings() as $field) {
			$out[$fname] = $this->$fieldsAsStrings;
		}
		return $out;
	}

	/*
	 * Return an identifier to be used as an array key. We assume that these are
	 * the "name" and "id" properties of the object, but child classes may make
	 * a different decision.
	 */
	public function name() {
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
	 * __toString() defaults to returning the name.
	 */
	public function __toString() {
		return $this->name;
	}

	protected function listproperties() {
		return $this->fieldsAsStrings();
	}
	
	/*
	 * Return the name of the parent class
	 */
	private static function parentClass() {
		return get_called_class() . 'List';
	}
}
