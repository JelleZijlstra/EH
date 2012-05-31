<?php
/*
 * Interface for a ListEntry.
 */
abstract class ListEntry extends ExecuteHandler {
	protected $p = NULL;
	private $setup_execute = false;
	// array of variables that shouldn't get dynamically defined set commands
	protected static $set_exclude = array(
		'_cPtr', '_pData', 'current', 'config', 'bools', 'props', 'discardthis', 
		'setup_execute', 'commands', 'synonyms'
	);
	protected static $ListEntry_commands = array(
		'inform' => array('name' => 'inform',
			'aka' => array('i'),
			'desc' => 'Give information about an entry',
			'arg' => 'None'),
		'setempty' => array('name' => 'setempty',
			'aka' => array('empty'),
			'desc' => 'Empty a property of the entry',
			'arg' => 'Property to be emptied'),
		'set' => array('name' => 'set',
			'aka' => array('setprops'),
			'desc' => 'Set a property of a file'),
		'getField' => array('name' => 'getField',
			'desc' => 'Get a particular field in a file'),
	);
	
	/*
	 * Return all the fields of the object that are filled from the DB, in the
	 * form of an array of SqlProperty objects.
	 */
	protected static $fields = array();
	
	/*
	 * PHP does not allow abstract static functions, but I don't know another 
	 * way to convey the requirement that subclasses must implement this method.
	abstract static protected function fillFields();
	 */

	private static function grabFields() {
		if(static::$fields === array()) {
			static::$fields = static::fillFields();
		}
	}
	
	final public static function fields() {
		static::grabFields();
		return static::$fields;
	}
	
	final public static /* SqlProperty */ function getFieldObject(/* string */ $field) {
		static::grabFields();
		// if the field does not exist, that is a programming error, and
		// throwing an exception is appropriate
		return static::$fields[$field];
	}
	
	final public static /* bool */ function hasField(/* string */ $field) {
		static::grabFields();
		return isset(static::$fields[$field]);
	}
	
	final protected static function fieldsAsStrings() {
		return array_map(function($in) {
			return $in->getName();
		}, self::fields());
	}

	final protected /* bool */ function validateProperty(/* string */ $property, /* mixed */ $value) {
		if(static::hasField($property)) {
			$validator = self::getFieldObject($property)->getValidator();
			return $validator($value);
		} else {
			return true;
		}
	}

	/*
	 * Constructor merely calls parent with commands.
	 * Commented out for now as it's not clear to me that we should enforce
	 * this constructor signature on children.
	 */
	public function __construct($commands) {
		parent::__construct(array_merge(self::$ListEntry_commands, $commands));
	}

	/*
	 * Set up the EH interface.
	 */
	protected function setup_eh_ListEntry() {
	// set up EH handler for a ListEntry
		$this->setup_ExecuteHandler(array_merge(
			self::$ListEntry_commands,
			static::${get_called_class() . '_commands'}
		));
		foreach($this->listproperties() as $property) {
			if(is_array($property)) {
				continue;
			}
			$this->addcommand(array(
				'name' => 'set' . $property,
				'aka' => $property,
				'desc' => 'Edit the field "' . $property . '"',
			));
		}
		$this->setup_execute = true;
	}

	/*
	 * Convert this object into an array.
	 */
	abstract public function toArray();

	/*
	 * Various stuff to determine properties and methods that exist. Probably
	 * needs harmonization.
	 */
	// TODO: determine what those should do here
	static function haspm($in) {
		// should probably also check for methods
		return property_exists(get_called_class(), $in);
	}
	static function hasproperty($property) {
		return property_exists(get_called_class(), $property);
	}
	static function hasmethodps($method) {}
	static function hasmethod($method) {}
	
	/*
	 * Give information about a ListEntry.
	 */
	protected static $inform_exclude = array(
		'synonyms', 
		'commands', 
		'setup_execute',
		'p',
		'_cPtr',
	);
	public function inform(array $paras = array()) {
		if($this->process_paras($paras, array(
			'name' => __FUNCTION__,
			'checklist' => array( /* No paras */ ),
		)) === PROCESS_PARAS_ERROR_FOUND) return false;
		foreach($this as $key => $value) {
			if(in_array($key, self::$inform_exclude, true)) {
				continue;
			}
			if(is_array($value)) {
				foreach($value as $akey => $prop) {
					if($prop and !is_array($prop) and !is_object($prop)) {
						echo $akey . ': ' . $prop . PHP_EOL;
					}
				}
			} elseif($value !== '' && $value !== NULL) {
				echo $key . ': ';
				Sanitizer::printVar($value);
				echo PHP_EOL;
			}
		}
	}
	
	/*
	 * Edit an entry.
	 */
	public function edit($paras = array()) {
		return $this->cli($paras);
	}
	
	/*
	 * Log something.
	 */
	public function log($msg, $writefull = true) {
		$this->p->log($msg . ' (file ' . $this->name . ')' . PHP_EOL);
		if($writefull) {
			$this->p->log($this->toArray());
		}
	}
	
	/*
	 * Implements settitle-like commands.
	 */
	public function __call($name, $arguments) {
		// allow setting properties
		if(substr($name, 0, 3) === 'set') {
			$paras = $arguments[0];
			$paras['field'] = substr($name, 3);
			return $this->setProperty($paras);
		}
		throw new EHException("Call to non-existent method " . $name);
	}
	
	public /* bool */ function setProperty(array $paras) {
		if($this->process_paras($paras, array(
			'name' => __FUNCTION__,
			'synonyms' => array(
				0 => 'new',
			),
			'checklist' => array(
				'field' => 'Field to set',
				'new' => 'Value to set to',
			),
			'default' => array(
				'new' => false,
			),
			'errorifempty' => array(
				'field',
			),
			'checkparas' => array(
				'field' => function($in) {
					return static::hasproperty($in);
				},
			),
		)) === PROCESS_PARAS_ERROR_FOUND) return false;
		if($paras['new'] === false) {
			$value = $this->{$paras['field']};
			echo 'Current value: ' 
				. Sanitizer::varToString($value) . PHP_EOL;
			$paras['new'] = $this->getline(array(
				'initialtext' => $value,
				'prompt' => 'New value: ',
			));
		}
		return $this->set(array($paras['field'] => $paras['new']));
	}
	
	/*
	 * Sets up a CLI. Similar and perhaps redundant to edit().
	 */
	public function cli(array $paras = array()) {
	// edit information associated with an entry
		if(!$this->setup_execute) {
			$this->setup_eh_ListEntry();
			$this->setup_execute = true;
		}
		if($this->name !== NULL) {
			$label = $this->name;
		} else {
			$label = get_called_class();
		}
		$this->setup_commandline($label);
	}

	/*
	 * Set properties.
	 */
	public /* bool */ function set(array $paras) {
	// default method; should be overridden by child classes with more precise 
	// needs
		foreach($paras as $field => $content) {
			if(self::hasproperty($field)) {
				if($this->$field !== $content) {
					$this->$field = $content;
					$this->p->needsave();
				}
			}
		}
		return true;
	}

	/*
	 * Empty properties.
	 */
	public /* bool */ function setempty(array $paras) {
	// sets field to empty by calling set()
		if($this->process_paras($paras, array(
			'name' => __FUNCTION__,
			'synonyms' => array(0 => 'field'),
			'checklist' => array('field' => 'Field to be emptied'),
			'errorifempty' => array('field'),
		)) === PROCESS_PARAS_ERROR_FOUND) return false;
		return $this->set(array($paras['field'] => NULL));
	}

	/*
	 * Resolve a redirect. Default implementation simply returns name.
	 */
	public /* string */ function resolve_redirect() {
		return $this->name;
	}

	/*
	 * Return array of all the object's properties.
	 */
	abstract protected /* array */ function listproperties();

	/*
	 * Return a particular field.
	 */
	public /* mixed */ function getField(array $paras) {
		$class = get_called_class();
		if($this->process_paras($paras, array(
			'name' => __FUNCTION__,
			'checklist' => array(
				'field' => 'Field to get',
			),
			'errorifempty' => array('field'),
			'checkparas' => array(
				'field' => function($in) use($class) {
					return $class::hasproperty($in);
				}
			),
		)) === PROCESS_PARAS_ERROR_FOUND) return false;
		return $this->{$paras['field']};
	}
}
