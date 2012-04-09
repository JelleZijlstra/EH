<?php
/*
 * Interface for a ListEntry.
 */
abstract class ListEntry extends ExecuteHandler {
	// $p is not a property, because it is set with __get magic
	// protected $p = NULL;
	private $setup_execute = false;
	// array of variables that shouldn't get dynamically defined set commands
	private static $set_exclude = array('_cPtr', '_pData', 'current', 'config', 'bools', 'props', 'discardthis', 'setup_execute', 'commands', 'synonyms');
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
	);
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
	static function haspm($in) {}
	static function hasproperty($property) {}
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
				self::printvar($value);
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
			$prop = substr($name, 3);
			if(static::hasproperty($prop)) {
				$paras = $arguments[0];
				$new = isset($paras['new']) ? $paras['new'] :
					(isset($paras[0]) ? $paras[0] : false);
				if($new === false or $new === '') {
					if($this->$prop)
						echo 'Current value: ' . $this->$prop . PHP_EOL;
					$new = $this->getline('New value: ');
				}
				return $this->set(array($prop => $new));
			}
		}
		return NULL;
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
		$this->setup_commandline($label, array('undoable' => true));
	}
	
	/*
	 * Set properties.
	 */
	public function set(array $paras) {
	// default method; should be overridden by child classes with more precise needs
		foreach($paras as $field => $content) {
			if(self::hasproperty($field)) {
				if($this->$field === $content) continue;
				$this->$field = $content;
				$this->p->needsave();
			}
		}
	}

	/*
	 * Empty properties.
	 */
	public function setempty(array $paras) {
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
	abstract protected function listproperties();
}
