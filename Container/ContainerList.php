<?php
/*
 * Interface for a ContainerList. This class includes code common to all 
 * implementations of a ContainerList, such as parameter checking. Individual
 * implementations often will have to define methods like "_bfind" which do the
 * real work.
 */
abstract class ContainerList extends ExecuteHandler {
	// array of functions for which __call should not resolve redirects. Entries 
	// are in the form array('Article', 'isredirect')
	public static $resolve_redirect_exclude = array();
	static private $ContainerList_commands = array(
		'add' => array('name' => 'add',
			'desc' => 'Add an entry'),
		'inform' => array('name' => 'inform',
			'aka' => array('i'),
			'desc' => 'Give information about an entry'),
		'set' => array('name' => 'set',
			'aka' => array('setprops'),
			'desc' => 'Set a property of a file'),
		'save' => array('name' => 'save',
			'aka' => array('v'),
			'desc' => 'Save the catalog to disk'),
		'listMembers' => array('name' => 'listMembers',
			'aka' => array('ls'),
			'desc' => 'List the members of this list'),
		'find_cmd' => array('name' => 'find_cmd',
			'aka' => array('f', 'find'),
			'desc' => "Find files that fulfil the condition given in the argument. An argument consists of a field name plus a text or regex pattern (separated by slashes) the field name should fulfil. Examples:\n\tfind_cmd year '1984'\nwill find all files published in 1984\n\tfind_cmd title '/Sudamerica/'\nwill find all files with \"Sudamerica\" in the title"),
		'mlist' => array('name' => 'mlist',
			'aka' => array('list'),
			'desc' => 'List content for a given field'),
		'bfind' => array('name' => 'bfind',
			'desc' => 'Find files according to multiple criteria'),
		'stats' => array('name' => 'stats',
			'desc' => 'Print statistics about the library'),
		'doall' => array('name' => 'doall',
			'desc' => 'Execute a command on all list entries'),
		'edit' => array('name' => 'edit',
			'aka' => array('e'),
			'desc' => 'Edit information associated with an entry'),
		'formatAll' => array('name' => 'formatAll',
			'desc' => 'Format all entries'),
		'getField' => array('name' => 'getField',
			'desc' => 'Get a particular field in a file'),
	);

	/*
	 * Constructor takes in commands to be passed to EH core.
	 *
	 * Constructor will also pre-populate the list of children.
	 */
	protected function __construct(array $commands = array()) {
		parent::__construct(array_merge(self::$ContainerList_commands, $commands));
	}

	/*
	 * Destructor will save any data still in memory.
	 */
	public function __destruct() {
		$this->saveIfNeeded();
	}

	/*
	 * Add some kind of ListEntry to the container.
	 */
	public function addEntry(ListEntry $file, array $paras = array()) {
		if($this->has($file->name)) {
			return false;
		}
		return $this->_addEntry($file, $paras);	
	}
	abstract protected function _addEntry(ListEntry $file, array $paras);

	/*
	 * Add a new entry.
	 */
	public function add(array $paras) {
		$childClass = static::$childClass;
		$myClass = get_called_class();
		if($this->process_paras($paras, array(
			'name' => __FUNCTION__,
			'synonyms' => array(0 => 'name'),
			'checklist' => array('name' => 'Name of new entry'),
			'checkfunc' => function($in) use($childClass) {
				return property_exists($childClass, $in);
			},
			'askifempty' => array('name'),
			'checkparas' => array(
				'name' => function($in) use($myClass) {
					if($myClass::singleton()->has($in)) {
						echo 'An entry with this name already exists.' . PHP_EOL;
						return false;
					} else {
						return true;
					}
				},
			),
		)) === PROCESS_PARAS_ERROR_FOUND) return false;
		return $this->addEntry(
			new $childClass($paras, 'n', $this),
			array('isnew' => true)
		);
	}

	/*
	 * Remove an entry.
	 */
	public function removeEntry(/* string */ $file, array $paras = array()) {
		if(!$this->has($file)) {
			echo 'File ' . $file . ' does not exist' . PHP_EOL;
			return false;
		}
		return $this->_removeEntry($file, $paras);
	}
	abstract protected function _removeEntry($file, array $paras);

	/*
	 * Move an entry.
	 */
	public function moveEntry(/* string */ $file, /* string */ $newName, array $paras = array()) {
		if(!$this->has($file)) {
			echo 'File ' . $file . ' does not exist' . PHP_EOL;
			return false;
		}
		if($this->has($newName)) {
			echo 'File ' . $newname . ' already exists' . PHP_EOL;
			return false;
		}
		return $this->_moveEntry($file, $newName, $paras);
	}
	abstract protected function _moveEntry($file, $newName, array $paras);
	
	/*
	 * Whether we have a child of this name.
	 */
	abstract public function has(/* string */ $file);
	
	/*
	 * Get a particular child entry.
	 */
	abstract public function get(/* string */ $file);

	/*
	 * Save anything necessary.
	 */
	abstract public function saveIfNeeded();
	
	/*
	 * Tell the object that a save is needed.
	 */
	abstract public function needsave();
	
	/*
	 * Calls a method on a child.
	 */
	public function __call($func, $args) {
	// call method for appropriate ListEntry
	// example: $csvlist->edit('Agathaeromys nov.pdf'); equals $csvlist->c['Agathaeromys nov.pdf']->edit();
		// check method validity
		if(!method_exists(static::$childClass, $func)) {
			throw new EHException(
				'Invalid call to ' . __METHOD__ . ' (method ' . $func 
					. ' invalid)', 
				EHException::E_RECOVERABLE);
		}
		// parameters to send to called function
		$paras = $args[0];
		// if we're called with a string argument, place it in $paras[0]
		if(!is_array($paras)) {
			$paras = array($paras);
		}
		// files to call
		$files = array();
		// retrieve files, which are numeric entries in the paras array
		foreach($paras as $key => $value) {
			if(is_int($key)) {
				// check validity
				if(!$this->has($value)) {
					echo 'Entry ' . $paras[$key] . ' does not exist (method ' 
						. $func . ')' . PHP_EOL;
					unset($paras[$key]);
					continue;
				}
				// resolve redirect if desired
				if($func !== 'resolve_redirect' and 
					method_exists(static::$childClass, 'resolve_redirect') and 
					!in_array(array(static::$childClass, $func), 
						self::$resolve_redirect_exclude, true)) {
					$value = $this->get($value)->resolve_redirect();
				}
				// check validity (again)
				if($value === false or !$this->has($value)) {
					echo 'Entry ' . $paras[$key] . ' does not exist (method ' 
						. $func . ')' . PHP_EOL;
					unset($paras[$key]);
					continue;
				}
				// add the file to the array of files to be called
				$files[] = $value;
				unset($paras[$key]);
			}
		}
		$ret = NULL;
		foreach($files as $file) {
			$ret = call_user_func(array($this->get($file), $func), $paras);
		}
		return $ret;
	}
	
	/*
	 * Calls the __invoke method of a child.
	 */
	public function __invoke($file) {
		// invoke calls the child and invokes it
		if(!$this->has($file)) {
			echo 'No such file: ' . $file . PHP_EOL;
			return false;
		}
		$obj = $this->get($file);
		$obj = $this->get($obj->resolve_redirect());
		return $obj();
	}

	/*
	 * Prevent access to non-existent properties.
	 */
	public function __get($prop) {
		throw new EHException('Attempt to read non-existent property ' . $prop,
			EHException::E_RECOVERABLE);
	}
	public function __set($prop, $value) {
		throw new EHException('Attempt to write to non-existent property ' 
			. $prop, EHException::E_RECOVERABLE);
	}

	/*
	 * Execute a method on all children.
	 */
	public function doall(array $paras) {
	// execute a function on all files in the list. Don't actually execute a 
	// command, since that is prohibitively expensive (requires EH to be 
	// initialized on every single ListEntry).
		$childClass = static::$childClass;
		if($this->process_paras($paras, array(
			'name' => __FUNCTION__,
			'checklist' => array(
				0 => 'Function to execute',
				'continueiffalse' => 'Whether the command should continue calling child objects if one call returns "false"',
				'askafter' => 'Ask the user whether he wants to continue after n child objects',
				'countfalse' => 'Whether to count calls that return false for the purposes of askafter',
				'arg' => 'Argument to be passed to called object',
			),
			// can take arbitrary arguments, which are passed to child
			'checkfunc' => function($in) {
				return true;
			},
			'checkparas' => array(
				0 => function($in) use($childClass) {
					return method_exists($childClass, $in);
				}
			),
			'errorifempty' => array(0),
			'default' => array(
				'continueiffalse' => false,
				'countfalse' => false,
				'askafter' => false,
			),
		)) === PROCESS_PARAS_ERROR_FOUND) return false;
		$func = $paras[0];
		$askafter = $paras['askafter'];
		$continueiffalse = $paras['continueiffalse'];
		$countfalse = $paras['countfalse'];
		unset($paras['askafter'], $paras['continueiffalse'], 
			$paras['countfalse'], $paras[0]);
		if(isset($paras['arg'])) {
			$paras[0] = $paras['arg'];
			unset($paras['arg']);
		}
		$i = 0;
		try {
			$obj = $this;
			$this->each(function($file) use(&$i, $askafter, $countfalse, $continueiffalse, $obj, $func, $paras) {
				if($askafter and $i and ($i % $askafter === 0)) {
					if($obj->ynmenu('Do you still want to continue?')) {
						// otherwise we'll sometimes ask this twice in a row
						$i++;
					} else {
						throw new StopException;
					}
				}
				try {
					$ret = $file->$func($paras);
				} catch(EHException $e) {
					$e->handle();
					if(!$continueiffalse) {
						throw new StopException;
					}
					$ret = false;
				}
				if($countfalse or $ret) {
					$i++;
				}
				if(!$continueiffalse and !$ret) {
					throw new StopException;
				}
			});
		} catch(StopException $e) {
			echo 'Stopping doall' . PHP_EOL;
		}
	}

	/*
	 * Call a function on all children.
	 */
	abstract public function each(/* callable */ $f);

	/*
	 * Go through all children and check validity, make minor corrections, 
	 * etcetera.
	 */
	public function formatAll(array $paras) {
		if($this->process_paras($paras, array(
			'name' => __FUNCTION__,
			'checklist' => array('w' => 'Write output to a file'),
			'default' => array('w' => false),
		)) === PROCESS_PARAS_ERROR_FOUND) return false;
		$this->saveIfNeeded();
		$this->_formatAll($paras);
	}
	abstract protected function _formatAll(array $paras);

	/*
	 * List all children.
	 */
	public function listMembers(array $paras) {
		if($this->process_paras($paras, array(
			'name' => __FUNCTION__,
			'checklist' => array( /* No paras */ ),
		)) === PROCESS_PARAS_ERROR_FOUND) return false;
		return $this->_listMembers($paras);
	}
	abstract protected function _listMembers(array $paras);
	
	/*
	 * Provide statistics.
	 */
	public function stats(array $paras = array()) {
		if($this->process_paras($paras, array(
			'name' => __FUNCTION__,
			'checklist' => array( /* No paras */ ),
		)) === PROCESS_PARAS_ERROR_FOUND) return false;
		return $this->_stats($paras);
	}
	abstract protected function _stats(array $paras);
	
	/*
	 * Return the total number of entries.
	 */
	abstract public function count();
	
	/*
	 * Make a list of the possible values of a field.
	 */
	public function mlist(array $paras) {
	// Make a list of possible values for a field with their frequency.
	// TODO: make this more efficient; especially with 'groupby' used, this may
	// entail lots of calls to mlist itself and to bfind. Both should probably
	// accept a 'files' parameter to search in a smaller array of files instead
	// of the whole $this->c.
		$childClass = static::$childClass;
		if($this->process_paras($paras, array(
			'name' => __FUNCTION__,
			'synonyms' => array(
				0 => 'field',
				'p' => 'print',
			),
			'checklist' => array(
				'field' => 'Field to separate by',
				'sort' => 'Sort function to be applied to results',
				'function' => 'Function to be applied to results',
				'isfunc' => 'Is the query a function?',
				'print' => 'Whether to print results',
				'groupby' => 'Column to group results by',
				'array' => 'Array to search in',
			),
			'checkfunc' => function($in) use($childClass) {
				return $childClass::haspm($in);
			},
			'default' => array(
				'print' => true,
				'array' => 'c',
				'function' => '',
				'isfunc' => false,
				'groupby' => '',
				'sort' => 'ksort',
			),
			'errorifempty' => array(
				'field',
			),
			'checkparas' => array(
				'function' => function($in) {
					return function_exists($in);
				},
				'sort' => function($in) {
					return function_exists($in);
				},
				'field' => function($in, $paras) use($childClass) {
					if($paras['isfunc']) {
						if(!$childClass::hasmethod($in)) {
							echo 'No such method: ' . $in . PHP_EOL;
							return false;
						}
					} elseif(!$childClass::haspm($in)) {
						echo 'No such property or method: ' . $in . PHP_EOL;
						return false;
					}
					return true;
				},
				'groupby' => function($in) use($childClass) {
					return $childClass::hasproperty($in);
				},
			),
		)) === PROCESS_PARAS_ERROR_FOUND) return false;
		// check for groupby
		if($paras['groupby']) {
			$childparas = $paras;
			unset($childparas['groupby']);
			$childparas['field'] = $paras['groupby'];
			$values = $this->mlist($childparas);
			if(!is_array($values)) {
				echo 'Error retrieving value list' . PHP_EOL;
				return false;
			}
			$out = array();
			$childparas['field'] = $paras['field'];
			foreach($values as $value => $ignore) {
				$childparas[$paras['groupby']] = $value;
				if($paras['print']) {
					echo PHP_EOL . $paras['groupby'] . ': ' . $value . PHP_EOL;
				}
				$out[$value] = $this->mlist($childparas);
			}
			return $out;
		}
		$values = $this->_mlist($paras);
		// sort (arsort to list by number, ksort to list alphabetically)
		$paras['sort']($values);
		if($paras['print']) {
			echo strtoupper($paras['field']) . PHP_EOL;
			foreach($values as $value => $number) {
				echo $value . " — " . $number . PHP_EOL;
			}
		}
		return $values;
	}
	abstract protected function _mlist(array $paras);
	
	/*
	 * Queries.
	 */
	public function bfind(array $paras) {
	// Query the database.
		$childClass = static::$childClass;
		if($this->process_paras($paras, array(
			'name' => __FUNCTION__,
			'synonyms' => array(
				'q' => 'quiet',
			),
			'checklist' => array(
				'function' =>
					'Name of function applied to text found',
				'openfiles' =>
					'Whether files found by the function should be opened',
				'isfunc' =>
					'Whether the query parameter is a function',
				'quiet' =>
					'If this parameter is set, nothing is printed, regardless of whether printentries, printresult, and printvalues are set',
				'printvalues' =>
					'Whether the values of the entries found should be printed',
				'printentries' =>
					'Whether the names of the entries found should be printed',
				'printresult' =>
					'Whether the count of entries found should be printed',
				'printproperties' =>
					'Array of properties of entries found that should be printed',
				'setcurrent' =>
					'Whether $this->current should be set',
				'return' =>
					'What to return. Options are "objectarray" (an array containing the entries found), "namearray" (an array containing the names of the entries found) and "count" (the count of entries found)',
				'array' =>
					'The array to search in',
			),
			'checkfunc' => function($in) use($childClass) {
				return $childClass::haspm($in);
			},
			'default' => array(
				'quiet' => false,
				'printentries' => true,
				'printvalues' => false,
				'printresult' => true,
				'printproperties' => false,
				'setcurrent' => true,
				'return' => 
					isset($paras['_ehphp']) ? 'namearray' : 'objectarray',
				'openfiles' => false,
				'array' => 'c',
				'function' => false,
				'isfunc' => false,
			),
			'checkparas' => array(
				'openfiles' => function($in) use($childClass) {
					return method_exists($childClass, 'openf');
				},
				'printproperties' => function($in) use($childClass) {
					// check whether it's even an array
					if(!is_array($in)) {
						return false;
					}
					// check all array entries
					foreach($in as $prop) {
						if(!is_string($prop) or !$childClass::hasproperty($prop)) {
							echo 'bfind: invalid printproperties entry: ';
							Sanitizer::printVar($prop);
							return false;
						}
					}
					return true;
				},
			),
			'listoptions' => array(
				'return' => array('objectarray', 'namearray', 'count'),
			),
		)) === PROCESS_PARAS_ERROR_FOUND) return false;
		// be really quiet
		if($paras['quiet']) {
			$paras['printentries'] = false;
			$paras['printvalues'] = false;
			$paras['printresult'] = false;
			$paras['printproperties'] = false;
		}
		// function to show error
		$error = function($msg) {
			echo 'bfind: error: ' . $msg . PHP_EOL;
		};
		// allow searching different arrays than $this->c;
		$arr = $paras['array'];
		if(!is_array($this->$arr)) {
			$error('invalid array');
			return false;
		}
		// process input
		$queries = array();
		foreach($paras as $key => $para) {
			$query = array();
			if($childClass::hasproperty($key)) {
				$query['field'] = $key;
				$query['content'] = $para;
				// test special syntax
				if(strlen($para) > 0) switch($para[0]) {
					case '/':
						if(!Sanitizer::testRegex($para)) {
							$error('invalid regex: ' . $para);
							return false;
						}
						$query['regex'] = true;
						break;
					case '>':
						if($para[1] === '=') {
							$query['content'] = substr($para, 2);
							$query['>='] = true;
						}
						else {
							$query['content'] = substr($para, 1);
							$query['>'] = true;
						}
						break;
					case '<':
						if($para[1] === '=') {
							$query['content'] = substr($para, 2);
							$query['<='] = true;
						}
						else {
							$query['content'] = substr($para, 1);
							$query['<'] = true;
						}
						break;
					case '\\':
						// escape
						$query['content'] = substr($para, 1);
						break;
				}
				$queries[] = $query;
			} elseif(substr($key, -2) === '()') {
				$func = substr($key, 0, -2);
				if(method_exists($childClass, $func)) {
					$query['field'] = $func;
					$query['func'] = true;
					$query['content'] = $para;
					if($para[0] === '/') {
						if(!Sanitizer::testRegex($para)) {
							$error('invalid regex: ' . $para);
							return false;
						}
						$query['regex'] = true;
					}
					$queries[] = $query;
				}
			}
		}
		if(count($queries) === 0) {
			if($paras['printresult']) {
				$error('invalid query');
				return false;
			}
			if($this->config['debug']) {
				print_r($paras);
			}
			return false;
		}
		$out = $this->_bfind($queries, $paras);	
		$count = count($out);
		if($paras['printresult']) {
			if($count === 0) {
				echo 'No entries found' . PHP_EOL;
			} else {
				echo $count . ' entries found' . PHP_EOL;
			}
		}
		if($count !== 0 and $paras['setcurrent']) {
			$this->current = array();
			foreach($out as $result) {
				$this->current[] = $result->name;
			}
		}
		switch($paras['return']) {
			case 'objectarray': return $out;
			case 'namearray':
				$namearray = array();
				foreach($out as $file) {
					$namearray[] = $file->name;
				}
				return $namearray;
			case 'count': return $count;
		}
	}
	abstract protected function _bfind(array $queries, array $paras);
	
	/*
	 * Simple wrapper for bfind.
	 */
	final public function find_cmd(array $paras) {
		if($this->process_paras($paras, array(
			'name' => __FUNCTION__,
			'synonyms' => array(
				0 => 'field',
				1 => 'value',
			),
			'checklist' => array(
				'field' => 'Field to search in',
				'value' => 'Value to search for',
			),
			'askifempty' => array('field', 'value'),
		)) === PROCESS_PARAS_ERROR_FOUND) return false;
		$bfindparas = array($paras['field'] => $paras['value']);
		if(isset($paras['_ehphp'])) {
			$bfindparas['_ehphp'] = true;
		}
		return $this->bfind($bfindparas);
	}
	
	/*
	 * Log a message
	 */
	public function log($msg) {
		if(static::$logfile === NULL) {
			throw new EHException(
				"Call to " . __METHOD__ . " without a set logfile",
				EHException::E_RECOVERABLE
			);
		}
		static $log = false;
		if($log === false) {
			$log = fopen(static::$logfile, "a");
			if($log === false) {
				throw new EHException(
					"Unable to open logfile",
					EHException::E_RECOVERABLE
				);
			}
		}
		// write array as CSV
		if(is_array($msg)) {
			fputcsv($log, $msg);
		} else {
			fwrite($log, $msg);
		}
		return true;
	}
	
	/*
	 * Get an instance.
	 */
	public static function singleton() {
		static $instance = NULL;
		if($instance === NULL) {
			$instance = new static();
		}
		return $instance;
	}
}
