<?php
/*
 * CsvListEntry.php
 *
 * Defines a PHP class that can be used to implement objects that are part of
 * some kind of collection (implemented as a ContainerList). This class mostly
 * contains methods that enable saving the collection into and retrieving it
 * from a CSV file, as well as editing its properties at runtime.
 */
abstract class CsvListEntry extends ListEntry {
	protected static $arrays_to_check;
	protected $props; // properties that are not otherwise specified
	protected static $CsvListEntry_commands = array(
	);
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
		return (count($out) > 0) ? $func($out) : NULL;
	}
	protected function warn($text, $field) {
		echo 'Warning (entry: ' . $this->name . '): ' . $text . ' in field ' . $field . ': ' . $this->$field . PHP_EOL;
	}
	/* OVERLOADING */
	public function __set($property, $value) {
		if($property_exists($this, $property)) {
			$this->$property = $value;
		} else {
			$arr = static::findarray($property);
			if($arr and $arr !== 'props')
				$this->{$arr}[$property] = $value;
			else
				$this->props[$property] = $value;
		}
	}
	public function __get($property) {
		switch($arr = $this->findarray_dyn($property)) {
			case false:
				return NULL;
			default:
				if(isset($this->{$arr}[$property])) {
					return $this->{$arr}[$property];
				} else {
					return NULL;
				}
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
	private function findarray_dyn($property) {
		$out = self::findarray($property);
		if($out)
			return $out;
		else if(is_array($this->props) and array_key_exists($property, $this->props))
			return 'props';
		else
			return false;
	}
	static private function findarray($property) {
	// used in overloading methods
		foreach(static::$arrays_to_check as $array) {
			if(in_array($property, static::${'n_' . $array}))
				return $array;
		}
		return false;
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
}
