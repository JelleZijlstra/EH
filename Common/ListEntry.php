<?php
/*
 * ListEntry.php
 *
 * Defines a PHP class that can be used to implement objects that are part of
 * some kind of collection (implemented as a FileList). This class mostly
 * contains methods that enable saving the collection into and retrieving it
 * from a CSV file, as well as editing its properties at runtime.
 */
abstract class ListEntry extends ExecuteHandler {
	protected static $parentlist;
	protected static $arrays_to_check;
	public $props; // properties that are not otherwise specified
	public function __construct($commands) {
		parent::__construct(array_merge(self::$ListEntry_commands, $commands));
	}
	protected function setup_eh_ListEntry() {
	// set up EH handler for a ListEntry
		$this->setup_ExecuteHandler(array_merge(
			self::$ListEntry_commands,
			static::${get_called_class() . '_commands'}
		));
		foreach($this->listproperties() as $property) {
			if(is_array($property)) continue;
			$this->addcommand(array(
				'name' => 'set' . $property,
				'aka' => $property,
				'desc' => 'Edit the field "' . $property . '"',
				'arg' => 'Optionally, new content of field',
				'execute' => 'callmethod',
			));
		}
		$this->setup_execute = true;
	}
	abstract public function toarray();
	private $setup_execute;
	protected static $ListEntry_commands = array(
		'inform' => array('name' => 'inform',
			'aka' => array('i'),
			'desc' => 'Give information about an entry',
			'arg' => 'None'),
		'setempty' => array('name' => 'setempty',
			'aka' => array('empty'),
			'desc' => 'Empty a property of the entry',
			'arg' => 'Property to be emptied'),
	);
	// array of variables that shouldn't get dynamically defined set commands
	private static $set_exclude = array('_cPtr', '_pData', 'current', 'config', 'bools', 'props', 'discardthis', 'setup_execute', 'commands', 'synonyms');
	protected function getarray($var, $paras = array()) {
	// helper for toarray(), to prepare array variables for storage
		if(!is_array($this->$var)) return NULL;
		$out = array();
		foreach($this->$var as $key => &$value) {
			if($value)
				$out[$key] = $value;
		}
		if(isset($paras['func']))
			$func = $paras['func'];
		else
			$func = 'json_encode';
		return (count($out) > 0) ? $func($this->$var) : NULL;
	}
	protected function warn($text, $field) {
		echo 'Warning (entry: ' . $this->name . '): ' . $text . ' in field ' . $field . ': ' . $this->$field . PHP_EOL;
	}
	/* OVERLOADING */
	public function __set($property, $value) {
		if($property === 'p') {
			global ${static::$parentlist};
			if(${static::$parentlist})
				$this->p = ${static::$parentlist};
		}
		$arr = static::findarray($property);
		if($arr and $arr !== 'props')
			$this->{$arr}[$property] = $value;
		else
			$this->props[$property] = $value;
	}
	public function __get($property) {
		if($property === 'p') {
			global ${static::$parentlist};
			if(${static::$parentlist}) {
				$this->p = ${static::$parentlist};
				return ${static::$parentlist};
			}
			else {
				trigger_error('Undefined p property', E_USER_NOTICE);
				debug_print_backtrace(DEBUG_BACKTRACE_IGNORE_ARGS);
				var_dump(static::$parentlist);
				return new StdClass();
			}
		}
		switch($arr = $this->findarray_dyn($property)) {
			case false:
				return NULL;
			default:
				if(isset($this->{$arr}[$property]))
					return $this->{$arr}[$property];
				else
					return NULL;
		}
	}
	public function __isset($property) {
		switch($arr = $this->findarray_dyn($property)) {
			case false: return false;
			default: return isset($this->{$arr}[$property]);
		}
	}
	public function __unset($property) {
		switch($arr = $this->findarray_dyn($property)) {
			case false:
				return;
			default:
				if(!isset($this->{$arr}[$property])) return;
				unset($this->{$arr}[$property]);
		}
	}
	static public function haspm($in) {
	// Call self::hasproperty() and self::hasmethod() together, as we often 
	// want in calling code.
		return self::hasproperty($in) or self::hasmethodps($in);
	}
	static public function hasproperty($property) {
		if(!$property) {
			return false;
		}
		if(property_exists(get_called_class(), $property)) {
			return true;
		}
		return (static::findarray($property) !== false);
	}
	static public function hasmethodps($method) {
		// common way of referring to class methods in bfind queries
		// Ultimately, this should only allow a subset of methods; we don't
		// really want `bfind --'remove()'=true`.
		if(substr($method, -2) !== '()') {
			return false;
		}
		$mname = substr($method, 0, -2);
		return method_exists(get_called_class(), $mname);
	}
	static public function hasmethod($method) {
		// Like hasmethodps, but without the parentheses
		return method_exists(get_called_class(), $method);
	}
	protected function findarray_dyn($property) {
		$out = self::findarray($property);
		if($out)
			return $out;
		else if(is_array($this->props) and array_key_exists($property, $this->props))
			return 'props';
		else
			return false;
	}
	static protected function findarray($property) {
	// used in overloading methods
		foreach(static::$arrays_to_check as $array) {
			if(in_array($property, static::${'n_' . $array}))
				return $array;
		}
		return false;
	}
	protected static $inform_exclude = array(
		'synonyms', 
		'commands', 
		'setup_execute',
	);
	public function inform() {
	// provide information for an entry
		foreach($this as $key => $value) {
			if(in_array($key, static::$inform_exclude, true))
				continue;
			if(is_array($value)) {
				foreach($value as $akey => $prop)
					if($prop and !is_array($prop) and !is_object($prop))
						echo $akey . ': ' . $prop . PHP_EOL;
			}
			else if(is_string($value) or is_double($value) or is_int($value)) {
				if($value)
					echo $key . ': ' . $value . PHP_EOL;
			}
			else if(is_bool($value)) {
				echo $key . ': ';
				echo $value ? 'true' : 'false';
				echo PHP_EOL;
			}
		}
	}
	public function edit($paras = array()) {
		return $this->cli($paras);
	}
	public function log($msg, $writefull = true) {
		logwrite($msg . ' (file ' . $this->name . '):' . PHP_EOL);
		if($writefull) logwrite($this->toarray());
	}
	protected function listproperties() {
	// list the user-visible properties of the object
		$vars = get_object_vars($this);
		foreach($vars as $key => $var) {
			$out[] = $key;
		}
		$vars = get_class_vars(get_called_class());
		foreach($vars as $key => $var) {
			if(substr($key, 0, 2) === 'n_') {
				$varname = substr($key, 2);
				if(property_exists($this, $varname) and is_array($var)) {
					foreach($var as $ikey)
						$out[] = $ikey;
				}
			}
		}
		// ditch stuff we don't want as a dynamic property
		if(isset(static::$set_exclude_child) and is_array(static::$set_exclude_child))
			$out = array_diff($out, self::$set_exclude, static::$set_exclude_child);
		else
			$out = array_diff($out, self::$set_exclude);
		return $out;
	}
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
	public function cli(array $paras = array()) {
	// edit information associated with an entry
		if(!$this->setup_execute) {
			$this->setup_eh_ListEntry();
			$this->setup_execute = true;
		}
		$this->setup_commandline($this->name, array('undoable' => true));
	}
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
}
