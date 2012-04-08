<?php
/*
 * SqlListEntry.php
 * Defines a class for objects representing database rows
 */
require_once(BPATH . '/Container/ListEntry.interface.php');

abstract class SqlListEntry extends ListEntry {
	static const CONSTR_ID = 0;
	static const CONSTR_NAME = 1;
	static const CONSTR_FULL = 2;
	
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
				$fields = $this->fields();
				foreach($data as $key => $value) {
					if(!in_array($key, $fields, true)) {
						throw new EHException(
							'Invalid data key ' . $key
						);
					}
					$this->$key = $value;
				}
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
	 * Return all the fields of the object that are filled from the DB.
	 */
	abstract protected function fields();
	
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
			foreach($vars[0] as $key => $value) {
				$this->$key = $value;
			}
			return true;
		} else {
			throw new EHException(
				"Multiple instances of id " . $id . " detected in table "
					. $this->p->table(),
				EHException::E_RECOVERABLE
			);
		}
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
		foreach($this->fields() as $field) {
			$out[$field] = $this->$field;
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

}
