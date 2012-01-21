<?php
/*
 * FileList.php
 *
 * A class that holds a collection of entries (represented by FileList objects).
 * This class contains the all-important $c property—an array of FileList
 * objects indexed by name, as well as a variety of method used to manage the
 * list. There is code to retrieve data from a CSV file to form a FileList, and
 * to save a modified FileList back into CSV. This class also contains methods
 * to find child ListEntries and summarize data in them, such as the bfind
 * method to query the list for children satisfying particular criteria.
 */
require_once(__DIR__ . '/../Common/common.php');
require_once(BPATH . '/Common/ExecuteHandler.php');
require_once(BPATH . '/Common/ListEntry.php');
abstract class FileList extends ExecuteHandler {
// this is an abstract class for classes that implement lists of entries, whether references or taxa.
	protected $needsave; // whether we need to save
	protected $c; //array of children
	protected $labels; // first line of CSV file
	protected static $fileloc; // where the file lives that we get our list from
	protected static $childclass; // name of the class that children need to be a member of
	// array of functions for which __call should not resolve redirects. Entries are in the form array('FullFile', 'isredirect')
	public static $resolve_redirect_exclude = array();
	private static $FileList_commands = array(
		'my_inform' => array('name' => 'my_inform',
			'aka' => array('inform'),
			'desc' => 'Give information about an entry',
			'arg' => 'Entry handle',
			'execute' => 'callmethodarg'),
		'set' => array('name' => 'set',
			'desc' => 'Set a property of a file',
			'arg' => 'Entry handle, plus fields to be changed in the form "--<field>=<content>"',
			'execute' => 'callmethodarg'),
		'save' => array('name' => 'save',
			'aka' => array('v'),
			'desc' => 'Save the catalog to disk',
			'arg' => 'None',
			'execute' => 'callmethod'),
		'switchcli' => array('name' => 'switchcli',
			'aka' => array('switch'),
			'desc' => 'Switch to a different command line',
			'arg' => 'Name of command line to switch to',
			'execute' => 'callmethodarg'),
		'listmembers' => array('name' => 'listmembers',
			'aka' => array('ls'),
			'desc' => 'List the members of this list',
			'arg' => 'None',
			'execute' => 'callmethod'),
		'find_cmd' => array('name' => 'find_cmd',
			'aka' => array('f', 'find'),
			'desc' => "Find files that fulfil the condition given in the argument. An argument consists of a field name plus a text or regex pattern (separated by slashes) the field name should fulfil. Examples:\n\tfind_cmd year '1984'\nwill find all files published in 1984\n\tfind_cmd title '/Sudamerica/'\nwill find all files with \"Sudamerica\" in the title",
			'arg' => 'Field plus pattern; see description',
			'execute' => 'callmethod'),
		'mlist' => array('name' => 'mlist',
			'aka' => array('list'),
			'desc' => 'List content for a given field',
			'arg' => 'Field',
			'execute' => 'callmethodarg'),
		'bfind' => array('name' => 'bfind',
			'desc' => 'Find files according to multiple criteria',
			'arg' => 'Set of arguments according to the syntax --<field>=<content>',
			'execute' => 'callmethod'),
		'getstats' => array('name' => 'getstats',
			'desc' => 'Give numerical statistics about entries fulfilling the given criteria',
			'arg' => 'Field, plus a series of criteria in the form --<field>=<content>',
			'execute' => 'callmethodarg'),
		'getstats_group' => array('name' => 'getstats_group',
			'desc' => 'As for getstats, but give statistics for entries for each value of the groupby parameter',
			'arg' => 'As for getats, plus --groupby=<field to group by>',
			'execute' => 'callmethodarg'),
		'listz' => array('name' => 'listz',
			'desc' => 'List Z value for entries found in a getstats query',
			'arg' => 'As for getstats, plus --index=<field given as index>',
			'execute' => 'callmethodarg'),
		'listz_group' => array('name' => 'listz_group',
			'desc' => 'List Z value for groups of entries found in a getstats query',
			'arg' => 'As for listz, plus --groupby=<field to group by>',
			'execute' => 'callmethodarg'),
		'formatall' => array('name' => 'formatall',
			'desc' => 'Format all entries',
			'arg' => 'None',
			'execute' => 'callmethod'),
		'stats' => array('name' => 'stats',
			'desc' => 'Print statistics about the library',
			'arg' => '"-f" will also print the number of files in each folder',
			'execute' => 'callmethod'),
		'doall' => array('name' => 'doall',
			'desc' => 'Execute a command on all list entries',
			'arg' => 'Command',
			'execute' => 'callmethodarg'),
		'sort' => array('name' => 'sort',
			'desc' => 'Sort the c array',
			'arg' => 'Field to sort by',
			'execute' => 'callmethod'),
	);
	public function __construct($commands = array()) {
		echo "processing CSV catalog... ";
		$cat = fopen(static::$fileloc, "r");
		if(!$cat) mydie('Failed to open input file');
		// consume first line (column labels)
		$this->labels = fgets($cat);
		while($line = fgetcsv($cat))
			$this->add_entry(new static::$childclass($line, 'f'));
		// close
		fclose($cat);
		echo "done" . PHP_EOL;
		parent::__construct(array_merge(self::$FileList_commands, $commands));
	}
	public function __destruct() {
		$this->saveifneeded();
	}
	protected static $resolve_redirects;
	public function add_entry(ListEntry $file, array $paras = array()) {
	// very basic add_entry function. Possibly some of the complex stuff that TaxonList and CsvList have should be moved to FileList somehow.
		if($this->has($file->name))
			return false;
		$this->c[$file->name] = $file;
		return true;
	}
	public function remove_entry($file, $paras = array()) {
		if(!$this->has($file)) {
			echo 'File ' . $file . ' does not exist' . PHP_EOL;
			return false;
		}
		unset($this->c[$file]);
		return true;
	}
	public function has($file) {
		return isset($this->c[$file]);
	}
	protected function save() {
	// save the list to disk
		if(count($this->c) < 10) {
			echo __FUNCTION__ . ' warning: input appears too short. Aborting.' . PHP_EOL;
			return false;
		}
		echo "saving catalog... ";
		$cat = fopen(static::$fileloc, "w");
		if(!$cat) {
			echo __FUNCTION__ . ' warning: could not open output file. Aborting.' . PHP_EOL;
			return false;
		}
		fwrite($cat, $this->labels);
		foreach($this->c as $file) {
			// do not put back in files where name has been set to NULL
			if($file->name) {
				$line = $file->toarray();
				if(!fputcsv($cat, $line))
					echo "Error writing data for " . $file->name . ".";
			}
		}
		echo "done" . PHP_EOL;
		// HACK: replace this with a hook system when we're ready for it
		if(method_exists($this, 'putpdfcontentcache'))
			$this->putpdfcontentcache();
		// we saved it, so we don't need to right now
		$this->needsave = false;
		return true;
	}
	public function get($file, $field = '') {
	// returns FullFile with name $file, or a particular field of that file
		if($this->has($file)) {
			if($field) {
				if(self::is_childproperty($field))
					return $this->c[$file]->$field;
				else {
					echo 'Invalid field: ' . $field . PHP_EOL;
					return NULL;
				}
			}
			else
				return clone $this->c[$file];
		}
		else {
			echo 'Invalid file: ' . $file . PHP_EOL;
			return NULL;
		}
	}
	public function saveifneeded() {
		if($this->needsave)
			return $this->save();
		else
			return false;
	}
	public function needsave() {
		$this->needsave = true;
	}
	public function __call($func, $args) {
	// call method for appropriate FullFile
	// example: $csvlist->edit('Agathaeromys nov.pdf'); equals $csvlist->c['Agathaeromys nov.pdf']->edit();
		// check method validity
		if(!$func or !method_exists(static::$childclass, $func)) {
			throw new EHException('Invalid call to ' . __METHOD__ . ' (method ' . $func . ' invalid)', EHException::E_RECOVERABLE);
			return false;
		}
		$file = array_shift($args);
		// check file validity. Don't throw a PHP warning here, since this may be an end-user error.
		if(!$this->has($file)) {
			echo 'Entry ' . $file . ' does not exist (method ' . $func . ')' . PHP_EOL;
			return false;
		}
		// resolve redirects, if those exist in the implementation and that is desired for this function
		if($func !== 'resolve_redirect' and method_exists(static::$childclass, 'resolve_redirect') and !in_array(array(static::$childclass, $func), self::$resolve_redirect_exclude))
			$file = $this->c[$file]->resolve_redirect();
		if($file === false)
			return false;
		return call_user_func_array(array($this->c[$file], $func), $args);
	}
	public function __invoke($file) {
		// invoke calls the child and invokes it
		if(!$this->has($file)) {
			echo 'No such file: ' . $file . PHP_EOL;
			return false;
		}
		if(method_exists(static::$childclass, 'resolve_redirect'))
			$file = $this->c[$file]->resolve_redirect();
		return $this->c[$file]();
	}
	public function doall($func, array $paras = array()) {
	// execute a function on all files in the list. Don't actually execute the command, since that is prohibitively expensive (requires EH to be initialized on every single ListEntry).
		if($this->process_paras($paras, array(
			'name' => __FUNCTION__,
			'checklist' => array(
				'continueiffalse' => 'Whether the command should continue calling child objects if one call returns "false"',
				'askafter' => 'Ask the user whether he wants to continue after n child objects',
				'countfalse' => 'Whether to count calls that return false for the purposes of askafter',
				'arg' => 'Argument to be passed to called object',
			),
			// can take arbitrary arguments, which are passed to child
			'checkfunc' => function($in) {
				return true;
			},
			'default' => array(
				'continueiffalse' => false,
				'countfalse' => false,
				'askafter' => 100,
			),
			'name' => __FUNCTION__,
		)) === PROCESS_PARAS_ERROR_FOUND) return false;
		if(!method_exists(static::$childclass, $func)) {
			echo 'Method does not exist: ' . $func . PHP_EOL;
			return false;
		}
		$askafter = $paras['askafter'];
		$continueiffalse = $paras['continueiffalse'];
		$countfalse = $paras['countfalse'];
		unset($paras['askafter'], $paras['continueiffalse'], $paras['countfalse'], $paras[0]);
		if(isset($paras['arg'])) {
			$arg = $paras[0] = $paras['arg'];
			unset($paras['arg']);
		}
		$i = 0;
		foreach($this->c as $file) {
			if($askafter and $i and ($i % $askafter === 0)) {
				switch($this->ynmenu('Do you still want to continue?')) {
					case 'y':
						// otherwise we'll sometimes ask this twice in a row
						$i++;
						break;
					case 'n': return;
				}
			}
			try {
				if(isset($arg))
					$ret = $file->$func($arg, $paras);
				else
					$ret = $file->$func($paras);
			}
			catch(EHException $e) {
				echo $e->getMessage();
				if(!$continueiffalse) return;
			}
			if($countfalse or $ret)
				$i++;
			if(!$continueiffalse and !$ret)
				return;
		}
	}
	static protected function is_childproperty($field) {
		$childclass = static::$childclass;
		return $childclass::hasproperty($field);
	}
	public function unsetf($file) {
		if($this->has($file)) {
			unset($this->c[$file]);
			return true;
		}
		else
			return false;
	}
	public function formatall($paras = array()) {
		if($this->process_paras($paras, array(
			'name' => __FUNCTION__,
			'checklist' => array('w' => 'Write output to a file'),
			'default' => array('w' => false),
		)) === PROCESS_PARAS_ERROR_FOUND) return false;
		$this->saveifneeded();
		exec_catch('cp ' . static::$fileloc . ' ' . static::$fileloc . '.save');
		if($paras['w']) ob_start();
		$this->doall('format', array('askafter' => 0));
		if($paras['w']) {
			// TODO: get DATAPATH straight
			file_put_contents(DATAPATH . 'formatoutput.txt', preg_replace('/Warning \(file: (.*?)\): /', '$1' . PHP_EOL, ob_get_contents()));
			ob_end_clean();
			exec_catch('edit ' . DATAPATH . 'formatoutput.txt');
		}
		// need to save here for the diff to work
		$this->save();
		echo shell_exec('diff ' . static::$fileloc . ' ' . static::$fileloc . '.save');
		exec_catch('rm ' . static::$fileloc . '.save');
	}
	public function switchcli($to) {
		global ${$to};
		if(!${$to} or !is_object(${$to}) or !method_exists(${$to}, 'cli')) {
			echo 'No such variable or CLI' . PHP_EOL;
			return false;
		}
		return ${$to}->cli();
	}
	public function listmembers() {
		foreach($this->c as $child)
			echo $child->name . PHP_EOL;
	}
	/* finding files etcetera */
	public function mlist($field, $paras = '') {
	// @paras: array('sort' => 'ksort', 'function' => , 'isfunc' => false, 'print' => <bool>)
		if($this->process_paras($paras, array(
			'name' => __FUNCTION__,
			'checklist' => array(
				'sort' => 'Sort function to be applied to results',
				'function' => 'Function to be applied to results',
				'isfunc' => 'Is the query a function?',
				'print' => 'Whether to print results',
				'groupby' => 'Column to group results by',
				'array' => 'Array to search in',
			),
			'checkfunc' => function($in) {
				return true;
			},
			'default' => array(
				'print' => true,
				'array' => 'c',
				'function' => '',
				'isfunc' => false,
				'groupby' => '',
				'sort' => '',
			),
		)) === PROCESS_PARAS_ERROR_FOUND) return false;
		$arr = $paras['array'];
		if(!is_array($this->$arr)) {
			echo 'Invalid array' . PHP_EOL;
			return false;
		}
		$childclass = static::$childclass;
		// check for groupby
		if($paras['groupby']) {
			if(!$childclass::hasproperty($paras['groupby'])) {
				echo 'No such property: ' . $paras['groupby'] . PHP_EOL;
				return false;
			}
			$childparas = $paras;
			unset($childparas['groupby']);
			$values = $this->mlist($paras['groupby'], $childparas);
			if(!is_array($values)) {
				echo 'Error retrieving value list' . PHP_EOL;
				return false;
			}
			$out = array();
			foreach($values as $value => $ignore) {
				$childparas[$paras['groupby']] = $value;
				if($paras['print']) echo PHP_EOL . $paras['groupby'] . ': ' . $value . PHP_EOL;
				$out[$value] = $this->mlist($field, $childparas);
			}
			return $out;
		}
		// detect method
		if(substr($field, -2) === '()') {
			$field = substr($field, 0, -2);
			$paras['isfunc'] = true;
		}
		// check whether property is valid
		if($paras['isfunc']) {
			if(!method_exists($childclass, $field)) {
				echo 'No such method: ' . $field . PHP_EOL;
				return false;
			}
		}
		else {
			if(!$childclass::hasproperty($field)) {
				echo 'No such property: ' . $field . PHP_EOL;
				return false;
			}
		}
		if($paras['function'] and !function_exists($paras['function'])) {
			echo 'No such function: ' . $paras['function'] . PHP_EOL;
			return false;
		}
		if($paras['sort'] and !function_exists($paras['sort'])) {
			echo 'No such function: ' . $paras['sort'] . PHP_EOL;
			return false;
		}
		// test whether we need bfind
		$dobfind = false;
		foreach($paras as $para => $content) {
			if($childclass::hasproperty($para)) {
				$dobfind = true;
				$bfindparas[$para] = $content;
			}
		}
		if($dobfind) {
			$bfindparas['array'] = $arr;
			$bfindparas['quiet'] = true;
			$files = $this->bfind($bfindparas);
			if(!$files)
				return false;
		}
		else
			$files = $this->$arr;
		// array of values for the field; $values["<value>"] == number of occurrences
		$values = array();
		// fill array
		foreach($files as $file) {
			$value = $paras['isfunc'] ? $file->$field() : $file->$field;
			if($paras['function'])
				$value = $paras['function']($value);
			if(!isset($values[$value]))
				$values[$value] = 0;
			$values[$value]++;
		}
		// sort (arsort to list by number, ksort to list alphabetically)
		if(!$paras['sort']) $paras['sort'] = 'ksort';
		$paras['sort']($values);
		if($paras['print']) {
			echo strtoupper($field) . PHP_EOL;
			foreach($values as $value => $number)
				echo $value . " — " . $number . PHP_EOL;
		}
		return $values;
	}
	public function bfind($paras = '') {
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
				'setcurrent' =>
					'Whether $this->current should be set',
				'return' =>
					'What to return. Options are "objectarray" (an array containing the entries found), "namearray" (an array containing the names of the entries found) and "count" (the count of entries found)',
				'array' =>
					'The array to search in',
			),
			'checkfunc' => function($para) {
				return true;
				// when we get to PHP 5.4, instead do:
				// return property_exists($this, $para);
			},
			'default' => array(
				'quiet' => false,
				'printentries' => true,
				'printvalues' => false,
				'printresult' => true,
				'setcurrent' => true,
				'return' => 'objectarray',
				'openfiles' => false,
				'array' => 'c',
				'function' => false,
				'isfunc' => false,
			),
		)) === PROCESS_PARAS_ERROR_FOUND)
			return false;
		// be really quiet
		if($paras['quiet']) {
			$paras['printentries'] = false;
			$paras['printvalues'] = false;
			$paras['printresult'] = false;
		}
		$childclass = static::$childclass;
		if($paras['openfiles'] and !method_exists($childclass, 'openf'))
			$paras['openfiles'] = false;
		// allow searching different arrays than $this->c;
		$arr = $paras['array'];
		if(!is_array($this->$arr)) {
			echo 'Invalid array' . PHP_EOL;
			return false;
		}
		// process input
		$queries = array();
		foreach($paras as $key => $para) {
			$query = array();
			if($childclass::hasproperty($key)) {
				$query['field'] = $key;
				$query['content'] = $para;
				// test special syntax
				switch($para[0]) {
					case '/':
						if(!self::testregex($para)) {
							echo 'Invalid regex: ' . $para . PHP_EOL;
							continue 2;
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
						// espace
						$query['content'] = substr($para, 1);
						break;
				}
				$queries[] = $query;
			}
			else if(substr($key, -2) === '()') {
				$func = substr($key, 0, -2);
				if(method_exists($childclass, $func)) {
					$query['field'] = $func;
					$query['func'] = true;
					$query['content'] = $para;
					if($para[0] === '/') {
						if(!self::testregex($para)) {
							echo 'Invalid regex: ' . $para . PHP_EOL;
							continue;
						}
						$query['regex'] = true;
					}
					$queries[] = $query;
				}
			}
		}
		if(count($queries) === 0) {
			if($paras['printresult']) echo 'Invalid query' . PHP_EOL;
			if($this->config['debug']) print_r($paras);
			return false;
		}
		// do the search
		$out = array();
		foreach($this->$arr as $file) {
			if($file->isredirect()) continue;
			if($paras['printvalues'])
				$values = array();
			foreach($queries as $query) {
				$hay = isset($query['func'])
					? $file->{$query['field']}()
					: $file->{$query['field']};
				// apply function
				if($paras['function'])
					$hay = $paras['function']($hay);
				if(isset($query['regex']))
					$found = preg_match($query['content'], $hay);
				else if(isset($query['>']))
					$found = ($hay > $query['content']);
				else if(isset($query['>=']))
					$found = ($hay >= $query['content']);
				else if(isset($query['<']))
					$found = ($hay < $query['content']);
				else if(isset($query['<=']))
					$found = ($hay <= $query['content']);
				else
					$found = ($query['content'] == $hay);
				if(!$found) continue 2;
				// remember values if necessary
				if($paras['printvalues'])
					$values[$query['field']] = $hay;
			}
			$out[$file->name] = $file;
			if($paras['printentries']) {
				echo $file->name . PHP_EOL;
				// need to change this for broader applicability
				if(method_exists($file, 'citepaper'))
					echo $file->citepaper() . PHP_EOL;
			}
			if($paras['printvalues']) {
				foreach($values as $key => $value)
					echo $key . ': ' . $value . PHP_EOL;
			}
			// TODO: add method of the sort where $paras['dothis'] = array('openfiles') will replace this
			if($paras['openfiles']) $file->openf();
		}
		$count = count($out);
		if($paras['printresult']) {
			if($count === 0)
				echo 'No entries found' . PHP_EOL;
			else
				echo $count . ' entries found' . PHP_EOL;
		}
		if($count !== 0 and $paras['setcurrent']) {
			$this->current = array();
			foreach($out as $result)
				$this->current[] = $result->name;
		}
		switch($paras['return']) {
			case 'objectarray': return $out;
			case 'namearray':
				$namearray = array();
				foreach($out as $file)
					$namearray[] = $file->name;
				return $namearray;
			case 'count': return $count;
		}
	}
	public function find($key, $needle, $paras = '') {
	// Wrapper for bfind. Is this used anywhere?
	// @paras: /array(function => <string>, openfiles => <bool>, isfunc => <bool>, print => <bool>, printresult => <bool>, isregex => <bool>, current => true)
		trigger_error("FileList::find is deprecated", E_USER_NOTICE);
		$paras[$key] = $needle;
		return $this->bfind($paras);
	}
	public function find_input($key = '') {
		$childclass = static::$childclass;
		while(true) {
			if(!$key) {
				$key = $this->getline(array('prompt' => 'Key to search on: '));
			}
			if(strpos($key, '()') !== false) {
				$paras['isfunc'] = true;
				$rkey = substr($key, 0, -2);
				if(!method_exists($childclass, $rkey)) {
					echo 'No such method' . PHP_EOL;
					unset($key);
				}
				else break;
			}
			else if(!$childclass::hasproperty($key)) {
				echo 'No such property' . PHP_EOL;
				unset($key);
			}
			else break;
		}
		while(true) {
			$value = $this->getline(array('prompt' => 'Value to search for: '));
			if($value === 'q') return true;
			if($value) break;
		}
		$this->bfind(array($key => $value));
	}
	public function find_cmd(array $paras = array()) {
		if($this->process_paras($paras, array(
			'name' => __FUNCTION__,
			'synonyms' => array(
				// they get returned in back-to-front order by ehi... need to do something about that
				1 => 'field',
				0 => 'value',
			),
			'checklist' => array(
				'field' => 'Field to search in',
				'value' => 'Value to search for',
			),
			'askifempty' => array('field', 'value'),
		)) === PROCESS_PARAS_ERROR_FOUND) return false;
		return $this->bfind(array($paras['field'] => $paras['value']));
	}
	public function average($files, $field) {
		if(!is_array($files) or count($files) === 0) return false;
		$childclass = static::$childclass;
		if(!$childclass::hasproperty($field)) {
			echo 'Invalid property' . PHP_EOL;
			return false;
		}
		$sum = 0;
		$i = 0;
		foreach($files as $file) {
			if(!$this->has($file->name) or !is_numeric($file->$field)) continue;
			$sum += $file->$field;
			$i++;
		}
		if($i == 0) return false;
		return $sum / $i;
	}
	public function stdev($files, $field) {
		if(!is_array($files) or count($files) === 0) return false;
		$childclass = static::$childclass;
		if(!$childclass::hasproperty($field)) {
			echo 'Invalid property' . PHP_EOL;
			return false;
		}
		$average = $this->average($files, $field);
		$sum = 0;
		$i = 0;
		foreach($files as $file) {
			if(!$this->has($file->name) or !is_numeric($file->$field)) continue;
			$sum += pow(($file->$field - $average), 2);
			$i++;
		}
		if($i == 0) return false;
		return sqrt($sum / $i);
	}
	public function smallest($files, $field, array $paras = array()) {
		if(!is_array($files) or count($files) === 0) return false;
		$childclass = static::$childclass;
		if(!$childclass::hasproperty($field)) {
			echo 'Invalid property' . PHP_EOL;
			return false;
		}
		$i = 0;
		$out = 0;
		foreach($files as $file) {
			if(!$this->has($file->name) or !is_numeric($file->$field)) continue;
			if($i == 0)
				$out = $file;
			else if($file->$field < $out->$field)
				$out = $file;
			$i++;
		}
		if($i == 0) return false;
		$paras['return'] = $paras['return'] ?: 'value';
		switch($paras['return']) {
			case 'object': return $out;
			case 'value': return $out->$field;
			default: return false;
		}
	}
	public function largest($files, $field, array $paras = array()) {
		if(!is_array($files) or count($files) === 0) return false;
		$childclass = static::$childclass;
		if(!$childclass::hasproperty($field)) {
			echo 'Invalid property' . PHP_EOL;
			return false;
		}
		$i = 0;
		$out = 0;
		foreach($files as $file) {
			if(!$this->has($file->name) or !is_numeric($file->$field)) continue;
			if($i == 0)
				$out = $file;
			else if($file->$field > $out->$field)
				$out = $file;
			$i++;
		}
		if($i == 0) return false;
		$paras['return'] = $paras['return'] ?: 'value';
		switch($paras['return']) {
			case 'object': return $out;
			case 'value': return $out->$field;
			default: return false;
		}
	}
	public function getstats($field, array $paras = array()) {
	// @para $field String field that is used
	// @para $paras Array associative arrays with pairs of fields and values to search in
		if(!is_array($paras)) return false;
		$print = isset($paras['print']) ? $paras['print'] : true;
		$childclass = static::$childclass;
		if(!$childclass::hasproperty($field)) {
			echo 'Invalid property' . PHP_EOL;
			return false;
		}
		// bfind will check the input for us
		$paras[$field] = '/^\s*\d+(\.\d+)?\s*$/'; // only include things where the field is actually numeric
		if($print)
			$paras['printentries'] = false;
		else
			$paras['quiet'] = true;
		$files = $this->bfind($paras);
		if(!$files) return false;
		$out = array();
		$out['count'] = count($files);
		foreach(array('average', 'stdev', 'smallest', 'largest') as $var) {
			$out[$var] = $this->$var($files, $field);
			if($print) echo ucfirst($var) . ': ' . round($out[$var], 3) . PHP_EOL;
		}
		if($paras['includefiles']) $out['files'] = $files;
		return $out;
	}
	public function listz($field, array $paras = array()) {
		if($this->process_paras($paras, array(
			'name' => __FUNCTION__,
			'errorifempty' => array('index'),
		)) === PROCESS_PARAS_ERROR_FOUND)
			return false;
		$childclass = static::$childclass;
		if(!$childclass::hasproperty($paras['index'])) return false;
		if(isset($paras['into']) and $childclass::hasproperty($paras['into']))
			$into = $paras['into'];
		$index = $paras['index'];
		$paras['includefiles'] = true;
		$stats = $this->getstats($field, $paras);
		if(!$stats) {
			echo 'No entries found' . PHP_EOL;
			return false;
		}
		if($stats['stdev'] == 0) {
			echo 'SD = 0, so Z = 0 for all files' . PHP_EOL;
			return false;
		}
		foreach($stats['files'] as $file) {
			$z = ($file->$field - $stats['average']) / $stats['stdev'];
			echo $file->$index . "\t" . $z . PHP_EOL;
			if($into) $this->set($file->name, array($into => $z));
		}
		return true;
	}
	public function listz_group($field, $paras) {
		if($this->process_paras($paras, array(
			'name' => __FUNCTION__,
			'errorifempty' => array('groupby'),
		)) === PROCESS_PARAS_ERROR_FOUND)
			return false;
		$childclass = static::$childclass;
		if(!$childclass::hasproperty($paras['groupby'])) return false;
		$mlistparas = array('print' => false);
		$groups = $this->mlist($paras['groupby'], $mlistparas);
		foreach($groups as $group => $i) {
			$paras[$paras['groupby']] = $group;
			$this->listz($field, $paras);
		}
	}
	public function getstats_group($field, $paras) {
		if(!isset($paras['groupby'])) return false;
		$childclass = static::$childclass;
		if(!$childclass::hasproperty($paras['groupby'])) return false;
		$mlistparas = array('print' => false);
		$groups = $this->mlist($paras['groupby'], $mlistparas);
		foreach($groups as $group => $i) {
			ob_start();
			echo '-------' . PHP_EOL . 'Group: ' . $group . PHP_EOL;
			$paras[$paras['groupby']] = $group;
			$success = $this->getstats($field, $paras);
			if($success)
				ob_end_flush();
			else
				ob_end_clean();
		}
	}
	public function stats(array $paras = array()) {
		$results = array();
		foreach($this->c as $file) {
			foreach($file as $key => $property) {
				if($property) {
					if(!isset($results[$key])) $results[$key] = 0;
					$results[$key]++;
				}
				if(is_array($property)) {
					foreach($property as $key => $prop) {
						if($prop) {
							if(!isset($results[$key])) $results[$key] = 0;
							$results[$key]++;
						}
					}
				}
			}
		}
		$total = count($this->c);
		echo 'Total number of files is ' . $total . '.' . PHP_EOL;
		ksort($results);
		foreach($results as $field => $number) {
			echo $field . ': ' . $number . ' of ' . $total . ' (' . round($number/$total*100, 1) . '%)' . PHP_EOL;
		}
		return;
	}
	public function sort(array $paras = array()) {
		if($this->process_paras($paras, array(
			'name' => __FUNCTION__,
			'synonyms' => array(
				0 => 'field',
				'n' => 'numeric',
				'r' => 'reverse',
			),
			'checklist' => array(
				'field' => 'Field to sort under',
				'numeric' => 'Perform a numeric sort',
				'reverse' => 'Whether to reverse the sort',
			),
			'default' => array(
				'field' => false,
				'numeric' => false,
				'reverse' => false,
			),
		)) === PROCESS_PARAS_ERROR_FOUND) return false;
		$field = $paras['field'];
		// we'll need to save
		$this->needsave();
		// if field not set, sort by name (= array key)
		if($field === false) {
			$this->needsave();
			if($paras['reverse'])
				return krsort($this->c);
			else
				return ksort($this->c);
		}
		// check whether the field is valid
		$childclass = static::$childclass;
		if(!$childclass::hasproperty($field)) {
			echo 'No such property';
			return false;
		}
		// create numeric sort function
		if($paras['numeric']) {
			$func = function($obj1, $obj2) use($field) {
				$field1 = $obj1->$field;
				$field2 = $obj2->$field;
				if($field1 > $field2)
					return 1;
				else if($field1 < $field2)
					return -1;
				else
					return 0;
			};
		}
		// or create string sort function
		else {
			$func = function($obj1, $obj2) use($field) {
				return strcmp($obj1->$field, $obj2->$field);
			};
		}
		if($paras['reverse']) {
			// wrap $func in a new lambda function
			return uasort($this->c, function($obj1, $obj2) use($func) {
				return -1 * $func($obj1, $obj2);
			});
		}
		else
			return uasort($this->c, $func);
	}
}
?>
