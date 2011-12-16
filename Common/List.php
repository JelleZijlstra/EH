<?php
require_once(__DIR__ . '/../Common/common.php');
require_once(BPATH . '/Common/ExecuteHandler.php');
abstract class FileList extends ExecuteHandler {
// this is an abstract class for classes that implement lists of entries, whether references or taxa.
	protected $needsave; // whether we need to save
	protected $c; //array of children
	protected $labels; // first line of CSV file
	protected static $fileloc; // where the file lives that we get our list from
	protected static $childclass; // name of the class that children need to be a member of
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
			'desc' => "Find files that fulfil the condition given in the argument. An argument consists of a field name plus a text or regex pattern (separated by slashes) the field name should fulfil. Examples:\n\tfind_cmd year 1984\nwill find all files published in 1984\n\tfind_cmd title /Sudamerica/\nwill find all files with \"Sudamerica\" in the title",
			'arg' => 'Field plus pattern; see description',
			'execute' => 'callmethodarg'),
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
		'execute_all' => array('name' => 'execute_all',
			'desc' => 'Execute a command on all list entries',
			'arg' => 'Command',
			'execute' => 'callmethodarg'),
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
	protected static $resolve_redirects;
	public function add_entry($file, $paras = '') {
	// very basic add_entry function. Possibly some of the complex stuff that TaxonList and CsvList have should be moved to FileList somehow.
		if($this->has($file->name))
			return false;
		$this->c[$file->name] = $file;
		return true;
	}
	public function remove_entry($file, $paras = '') {
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
	public function save() {
	// should be private (callers from outside the class should use saveifneeded() instead)
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
		if($this->needsave) {
			return $this->save();
		}
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
			trigger_error('Invalid call to ' . __METHOD__ . ' (method ' . $func . ' invalid)', E_USER_WARNING);
			debug_print_backtrace(DEBUG_BACKTRACE_IGNORE_ARGS);
			return false;
		}
		$file = array_shift($args);
		// check file validity. Don't throw a PHP warning here, since this may be an end-user error.
		if(!$this->has($file)) {
			echo 'Entry ' . $file . ' does not exist (method ' . $func . ')' . PHP_EOL;
			return false;
		}
		// resolve redirects. There may be cases where this is not warranted; perhaps check for that or add some kind of switch.
		if(static::$resolve_redirects)
			$file = $this->c[$file]->gettruename();
		if(!$file) return false;
		return call_user_func_array(array($this->c[$file], $func), $args);
	}
	public function getone($cmds = array('q')) {
	// Get one file from the list using stdin input
	// This could be converted to menu() when we can use $this-> in lambda functions. Or it could be scrapped, because it is apparently unused.
		while(true) {
			$file = $this->getline();
			if(in_array($file, $cmds)) 
				return $file;
			if($this->has($file))
				return self::$resolve_redirects ? $this->c[$file]->gettruename() : $this->c[$file]->name;
			else
				echo 'File does not exist' . PHP_EOL;
		}
	}
	public function doall($func, $paras = array()) {
	// $paras['continueiffalse']: whether we go on with the next one if function returns false
		if(!method_exists(static::$childclass, $func)) {
			echo 'Method does not exist: ' . $func . PHP_EOL;
			return false;
		}
		foreach($this->c as $file) {
			if($paras['continueiffalse'])
				$file->$func();
			else
				if(!$file->$func()) return;
		}
	}
	public function execute_all($cmd, $paras = array()) {
	// execute a command on all files in the list
		foreach($this->c as $file) {
			try {
				$file->execute($cmd);
			}
			catch(EHException $e) {
				echo $e;
				return false;
			}
		}
		return true;
	}
	public function doone($func) {
	// wrapper function for various utilities that do the following: get a filename and call function FullFile::$function() on it
	// Doesn't look like it's currently used anywhere, though.
		echo "Type a filename. Alternatively, type 'q' to quit $func()." . PHP_EOL;
		while(true) {
			echo "Name: ";
			$file = $this->getone();
			if($file === "q") return;
			$this->c[$file]->$func();
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
		$this->saveifneeded();
		exec_catch('cp ' . static::$fileloc . ' ' . static::$fileloc . '.save');
		if($paras['w']) ob_start();
		$this->doall('format');
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
		if(self::process_paras($paras, array(
/* hide because this method can take arbitrary other parameters
			'checklist' => array(
				'sort', // sort function to be applied to results
				'function', // function to be applied to results
				'isfunc', // is the query a function?
				'print', // print results
				'groupby', // column to group results by
				'array', // array to search in
			),
*/
			'default' => array('print' => true),
		)) === PROCESS_PARAS_ERROR_FOUND)
			return false;
		$arr = $paras['array'] ?: 'c';
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
		foreach($paras as $para => $content) {
			if($childclass::hasproperty($para)) {
				$dobfind = true;
				$bfindparas[$para] = $content;
			}
		}
		if($dobfind) {
			$bfindparas['array'] = $arr;
			$bfindparas['print'] = false;
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
			if($paras['function']) $value = $paras['function']($value);
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
		if(self::process_paras($paras, array(
/* hide this because bfind can take arbitrary other parameters; kept here for ease of documentation
			'checklist' => array('function', // String function applied to text found
				'openfiles', // Open files found?
				'isfunc', // Is the query parameter a function?
				'print', // Print the files found?
				'printresult', // Print number of files found?
				'current', // Set $this->current?
				'return', // What to return
				'array', // Array to search in
			),
*/
			'default' => array('print' => true,
				'printresult' => true,
				'setcurrent' => true,
				'return' => 'objectarray',
			),
		)) === PROCESS_PARAS_ERROR_FOUND)
			return false;
		$childclass = static::$childclass;
		if($paras['openfiles'] and !method_exists($childclass, 'openf')) 
			$paras['openfiles'] = false;
		// allow searching different arrays than $this->c;
		$arr = $paras['array'] ?: 'c';
		if(!is_array($this->$arr)) {
			debug_print_backtrace();
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
			if(DEBUG > 0) print_r($paras);
			return false;
		}
		// do the search
		$out = array();
		foreach($this->$arr as $file) {
			if($file->isredirect()) continue;
			foreach($queries as $query) {
				$hay = $query['func'] 
					? $file->{$query['field']}() 
					: $file->{$query['field']};
				// apply function
				if($paras['function'])
					$hay = $paras['function']($hay);
				if($query['regex']) 
					$found = preg_match($query['content'], $hay);
				else if($query['>'])
					$found = ($hay > $query['content']);
				else if($query['>='])
					$found = ($hay >= $query['content']);
				else if($query['<'])
					$found = ($hay < $query['content']);
				else if($query['<='])
					$found = ($hay <= $query['content']);
				else
					$found = ($query['content'] == $hay);
				if(!$found) continue 2;
			}
			$out[] = $file;
			if($paras['print']) {
				echo $file->name . PHP_EOL;
				// need to change this for broader applicability
				if(method_exists($file, 'citepaper'))
					echo $file->citepaper() . PHP_EOL;
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
	// Wrapper for bfind
	// @paras: /array(function => <string>, openfiles => <bool>, isfunc => <bool>, print => <bool>, printresult => <bool>, isregex => <bool>, current => true)
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
	public function find_cmd($cmd = '') {
		if(!$cmd) {
			echo 'Syntax: <field> <value>' . PHP_EOL;
			$cmd = $this->getline(array('prompt' => 'find> '));
		}
		if($cmd === 'q') return false;
		$sep = strpos($cmd, ' ');
		$field = substr($cmd, 0, $sep);
		$value = substr($cmd, $sep + 1);
		$this->bfind(array($field => $value));
		return true;
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
	public function smallest($files, $field, $paras = array()) {
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
	public function largest($files, $field, $paras = array()) {
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
	public function getstats($field, $paras) {
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
		$paras['print'] = false;
		if(!$print) $paras['printresult'] = false;
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
	public function listz($field, $paras) {
		if(self::process_paras($paras, array(
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
		if(self::process_paras($paras, array(
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
	public function stats($paras = '') {
		$results = array();
		foreach($this->c as $file) {
			foreach($file as $key => $property) {
				if($property) $results[$key]++;
				if(is_array($property)) {
					foreach($property as $key => $prop) {
						if($prop) $results[$key]++;
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
}
abstract class ListEntry extends ExecuteHandler {
	protected static $parentlist;
	protected static $arrays_to_check;
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
				'execute' => 'callmethodarg',
			));
		}
		$this->setup_execute = true;
	}
	abstract public function toarray();
	public function my_inform() {
		// implementors may want to do more stuff here
		return $this->inform();
	}
	public $discardthis; // flag to $this->p->add_entry() that this entry should not be added
	protected $setup_execute;
	protected static $ListEntry_commands = array(
		'my_inform' => array('name' => 'my_inform',
			'aka' => array('inform', 'i'),
			'desc' => 'Give information about an entry',
			'arg' => 'None',
			'execute' => 'callmethod'),
		'setempty' => array('name' => 'setempty',
			'aka' => array('empty'),
			'desc' => 'Empty a property of the entry',
			'arg' => 'Property to be emptied',
			'execute' => 'callmethodarg'),
/*		'continue' => array('name' => 'continue',
			'aka' => array('a'),
			'desc' => 'Quit this file and continue with the next',
			'arg' => 'None',
			'execute' => 'quit'),*/
	);
	protected function getarray($var, $paras = '') {
	// helper for toarray(), to prepare array variables for storage
		if(!is_array($this->$var)) return NULL;
		foreach($this->$var as $key => &$value) {
			if($value)
				$out[$key] = $value;
		}
		if($paras['func'])
			$func = $paras['func'];
		else
			$func = 'json_encode';
		return $out ? $func($this->$var) : NULL;
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
			case false: return NULL;
			default: return $this->{$arr}[$property];
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
			case false: return;
			default: unset($this->{$arr}[$property]);
		}
	}
	static public function hasproperty($property) {
		if(!$property) return false;
		if(property_exists(get_called_class(), $property))
			return true;
		if(static::findarray($property) !== false)
			return true;
		else
			return false;
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
			if(in_array($property, static::${n_ . $array}))
				return $array;
		}
		return false;
	}
	protected static $inform_exclude = array();
	protected function inform() {
	// provide information for an entry
		foreach($this as $key => $value) {
			if(in_array($key, static::$inform_exclude))
				continue;
			switch(gettype($value)) {
				case 'array': 
					foreach($value as $akey => $prop)
						if($prop and !is_array($prop) and !is_object($prop))
							echo $akey . ': ' . $prop . PHP_EOL;
					break;
				case 'string': case 'double': case 'int':
					if($value)
						echo $key . ': ' . $value . PHP_EOL;
					break;
				case 'bool':
					echo $key . ': ';
					echo $value ? 'true' : 'false';
					echo PHP_EOL;
					break;
			}
		}
	}
	public function edit($paras = '') {
		return $this->cli($paras);
	}
	public function log($msg, $writefull = true) {
		logwrite($msg . ' (file ' . $this->name . '):' . PHP_EOL);
		if($writefull) logwrite($this->toarray());
	}
	protected function listproperties() {
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
		return $out;
	}
	public function __call($name, $arguments) {
		// allow setting properties
		if(substr($name, 0, 3) === 'set') {
			$prop = substr($name, 3);
			if(static::hasproperty($prop)) {
				$new = $arguments[0];
				if($new === NULL or $new === '') {
					if($this->$prop) echo 'Current value: ' . $this->$prop . PHP_EOL;
					$new = $this->getline(array('prompt' => 'New value: '));
				}
				return $this->set(array($prop => $new));
			}
		}
		return NULL;
	}
	public function cli($paras = '') {
	// edit information associated with an entry
		if(!$this->setup_execute) {
			$this->setup_eh_ListEntry();
			$this->setup_execute = true;
		}
		$this->setup_commandline($this->name, array('undoable' => true));
	}
	public function set($paras) {
	// default method; should be overridden by child classes with more precise needs
		foreach($paras as $field => $content) {
			if(self::hasproperty($field)) {
				if($this->$field === $content) continue;
				$this->$field = $content;
				$this->p->needsave();
			}
		}
	}
	public function setempty($para) {
	// sets para $para to empty by calling set()
		return $this->set(array($para => ''));
	}
}
?>
