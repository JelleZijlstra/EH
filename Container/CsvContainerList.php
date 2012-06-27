<?php
/*
 * CsvContainerList.php
 *
 * A class that holds a collection of entries (represented by ListEntry 
 * objects).
 * This class contains the all-important $c property—an array of ListEntry
 * objects indexed by name—as well as a variety of methods used to manage the
 * list. There is code to retrieve data from a CSV file to form a ContainerList, 
 * and to save a modified ContainerList back into CSV. This class also contains 
 * methods to find child ListEntries and summarize data in them, such as the 
 * bfind method to query the list for children satisfying particular criteria.
 */
require_once(__DIR__ . '/../Common/common.php');
abstract class CsvContainerList extends ContainerList {
// this is an abstract class for classes that implement lists of entries, whether references or taxa.
	/*
	 * Whether we need to save the data on destruction. This is set to false
	 * initially, and may be set to true through a call to the needsave()
	 * method. Only methods that are actually involved with saving should
	 * modify this directly.
	 */
	private $needsave = false;
	protected $c; //array of children
	protected $labels; // first line of CSV file
	protected static $fileloc; // where the file lives that we get our list from
	protected static $logfile; // location of the log file
	protected static $childClass; // name of the class that children need to be a member of
	private static $CsvContainerList_commands = array(
		'getstats' => array('name' => 'getstats',
			'desc' => 'Give numerical statistics about entries fulfilling the given criteria'),
		'listz' => array('name' => 'listz',
			'desc' => 'List Z value for entries found in a getstats query'),
		'sort' => array('name' => 'sort',
			'desc' => 'Sort the c array'),
		'backup' => array('name' => 'backup',
			'desc' => 'Save a backup of the catalog'),
	);
	protected function __construct(array $commands = array()) {
		echo "processing CSV catalog... ";
		$cat = fopen(static::$fileloc, "r");
		if($cat === false) {
			throw new EHException('Failed to open input file');
		}
		// consume first line (column labels)
		$this->labels = fgets($cat);
		while($line = fgetcsv($cat)) {
			$this->addEntry(new static::$childClass($line, 'f', $this));
		}
		// close
		fclose($cat);
		echo "done" . PHP_EOL;
		parent::__construct(array_merge(self::$CsvContainerList_commands, $commands));
	}
	protected function _addEntry(ListEntry $file, array $paras) {
		$this->c[$file->name] = $file;
		return true;
	}
	public function _removeEntry(/* string */ $file, array $paras) {
		unset($this->c[$file]);
		return true;
	}
	public function _moveEntry($file, $newName, array $paras) {
		$this->c[$newName] = $this->c[$file];
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
			if($file->name !== NULL) {
				$line = $file->toArray();
				if(!fputcsv($cat, $line)) {
					echo "Error writing data for " . $file->name . ".";
				}
			}
		}
		echo "done" . PHP_EOL;
		// HACK: replace this with a hook system when we're ready for it
		if(method_exists($this, 'putpdfcontentcache')) {
			$this->putpdfcontentcache();
		}
		// we saved it, so we don't need to right now
		$this->needsave = false;
		return true;
	}
	public function get($file) {
	// returns ListEntry with name $file, or a particular field of that file.
	// If $field === true, resolves redirects.
		if($this->has($file)) {
			return $this->c[$file];
		} else {
			throw new EHException('Invalid file: ' . $file);
			return false;
		}
	}
	public function saveIfNeeded() {
		if($this->needsave) {
			return $this->save();
		} else {
			return false;
		}
	}
	public function needSave() {
		// Tell the ContainerList that we need to save the catalog.
		$this->needsave = true;
	}
	public function each(/* callable */ $f) {
	// performs function f on each entry
		foreach($this->c as $entry) {
			$f($entry);
		}
	}
	static protected function is_childproperty($field) {
		$childClass = static::$childClass;
		return $childClass::hasproperty($field);
	}
	protected function _formatAll(array $paras) {
		$this->shell(array(
			'cmd' => 'cp', 
			'arg' => array(static::$fileloc, static::$fileloc . '.save')
		));
		if($paras['w']) ob_start();
		$this->doall(array(0 => 'format', 'askafter' => 0));
		if($paras['w']) {
			// TODO: get DATAPATH straight
			file_put_contents(DATAPATH . 'formatoutput.txt', preg_replace('/Warning \(file: (.*?)\): /', '$1' . PHP_EOL, ob_get_contents()));
			ob_end_clean();
			$this->shell(array('edit', array(DATAPATH . 'formatoutput.txt')));
		}
		// need to save here for the diff to work
		$this->save();
		$this->shell(array(
			'cmd' => 'diff',
			'arg' => array(static::$fileloc, static::$fileloc . '.save'),
			// diff returns one if two files differ
			'exceptiononerror' => false,
		));
		$this->shell(array(
			'cmd' => 'rm',
			'arg' => array(static::$fileloc . '.save'),
		));
	}
	/* listing, manipulating, and summarizing the whole list */
	protected function _listMembers(array $paras) {
		$this->each(function($entry) {
			echo $entry->name . PHP_EOL;
		});
		return true;	
	}
	protected function _stats(array $paras) {
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
		$total = $this->count();
		echo 'Total number of files is ' . $total . '.' . PHP_EOL;
		ksort($results);
		foreach($results as $field => $number) {
			echo $field . ': ' . $number . ' of ' . $total . ' (' . round($number/$total*100, 1) . '%)' . PHP_EOL;
		}
		return true;
	}
	public function count() {
		return count($this->c);
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
		$childClass = static::$childClass;
		if(!$childClass::hasproperty($field)) {
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
	/* finding files and making lists */
	protected function _mlist(array $paras) {
		$childClass = static::$childClass;
		// test whether we need bfind
		$dobfind = false;
		$bfindparas = array();
		foreach($paras as $para => $content) {
			if($childClass::hasproperty($para)) {
				$dobfind = true;
				$bfindparas[$para] = $content;
			}
		}
		if($dobfind) {
			$bfindparas['array'] = $paras['array'];
			$bfindparas['quiet'] = true;
			$files = $this->bfind($bfindparas);
			if(!$files) {
				return array();
			}
		} else {
			$files = $this->{$paras['array']};
		}
		// array of values for the field; $values["<value>"] gives the number of occurrences
		$values = array();
		$field = $paras['field'];
		$hasRedirect = method_exists(static::$childClass, 'isredirect');
		// fill array
		foreach($files as $file) {
			if($hasRedirect && $file->isredirect()) {
				continue;
			}
			$value = $paras['isfunc'] ? $file->$field() : $file->$field;
			if($paras['function']) {
				$value = $paras['function']($value);
			}
			if(!isset($values[$value])) {
				$values[$value] = 0;
			}
			$values[$value]++;
		}
		return $values;
	}
	protected function _bfind(array $queries, array $paras) {
		// do the search
		$out = array();
		$hasRedirect = method_exists(static::$childClass, 'isredirect');
		
		foreach($this->{$paras['array']} as $file) {
			if($hasRedirect && $file->isredirect()) {
				continue;
			}
			if($paras['printvalues']) {
				$values = array();
			}
			foreach($queries as $query) {
				$hay = isset($query['func'])
					? $file->{$query['field']}()
					: $file->{$query['field']};
				// apply function
				if($paras['function']) {
					$hay = $paras['function']($hay);
				}
				if(isset($query['regex'])) {
					$found = preg_match($query['content'], $hay);
				} elseif(isset($query['>'])) {
					$found = ($hay > $query['content']);
				} elseif(isset($query['>='])) {
					$found = ($hay >= $query['content']);
				} elseif(isset($query['<'])) {
					$found = ($hay < $query['content']);
				} elseif(isset($query['<='])) {
					$found = ($hay <= $query['content']);
				} else {
					// loose comparison is intentional here
					$found = ($query['content'] == $hay);
				}
				if(!$found) {
					continue 2;
				}
				// remember values if necessary
				if($paras['printvalues']) {
					$values[$query['field']] = $hay;
				}
			}
			$out[$file->name] = $file;
			if($paras['printentries']) {
				echo $file->name . PHP_EOL;
				// need to change this for broader applicability
				if(method_exists($file, 'citepaper')) {
					echo ' ' . $file->citepaper() . PHP_EOL;
				}
			}
			if($paras['printvalues']) {
				foreach($values as $key => $value) {
					echo $key . ': ' . $value . PHP_EOL;
				}
			}
			if($paras['printproperties']) {
				foreach($paras['printproperties'] as $prop) {
					echo $prop . ': ';
					Sanitizer::printVar($file->$prop);
					echo PHP_EOL;
				}
			}
			// TODO: add method of the sort where $paras['dothis'] = array('openfiles') will replace this
			if($paras['openfiles']) {
				$file->openf();
			}
		}
		return $out;
	}
	/* statistics */
	public function average($files, $field) {
		if(!is_array($files) or count($files) === 0)
			return false;
		$childClass = static::$childClass;
		if(!$childClass::hasproperty($field)) {
			echo 'Invalid property' . PHP_EOL;
			return false;
		}
		$sum = 0;
		$i = 0;
		foreach($files as $file) {
			if(!$this->has($file->name) or !is_numeric($file->$field))
				continue;
			$sum += $file->$field;
			$i++;
		}
		if($i === 0)
			return false;
		return $sum / $i;
	}
	public function stdev($files, $field) {
		if(!is_array($files) or count($files) === 0) return false;
		$childClass = static::$childClass;
		if(!$childClass::hasproperty($field)) {
			echo 'Invalid property' . PHP_EOL;
			return false;
		}
		$average = $this->average($files, $field);
		$sum = 0;
		$i = 0;
		foreach($files as $file) {
			if(!$this->has($file->name) or !is_numeric($file->$field))
				continue;
			$sum += pow(($file->$field - $average), 2);
			$i++;
		}
		if($i === 0)
			return false;
		return sqrt($sum / $i);
	}
	public function smallest($files, $field, array $paras = array()) {
		if($this->process_paras($paras, array(
			'name' => __FUNCTION__,
			'checklist' => array('return' => 'Type of return value'),
			'default' => array('return' => 'value'),
			'listoptions' => array(
				'return' => array('object', 'value'),
			),
		)) === PROCESS_PARAS_ERROR_FOUND) return false;
		if(!is_array($files) or count($files) === 0)
			return false;
		$childClass = static::$childClass;
		if(!$childClass::hasproperty($field)) {
			echo 'Invalid property' . PHP_EOL;
			return false;
		}
		$i = 0;
		$out = 0;
		foreach($files as $file) {
			if(!$this->has($file->name) or !is_numeric($file->$field))
				continue;
			if($i === 0)
				$out = $file;
			else if($file->$field < $out->$field)
				$out = $file;
			$i++;
		}
		if($i === 0)
			return false;
		switch($paras['return']) {
			case 'object': return $out;
			case 'value': return $out->$field;
		}
	}
	public function largest($files, $field, array $paras = array()) {
		if($this->process_paras($paras, array(
			'name' => __FUNCTION__,
			'checklist' => array('return' => 'Type of return value'),
			'default' => array('return' => 'value'),
			'listoptions' => array(
				'return' => array('object', 'value'),
			),
		)) === PROCESS_PARAS_ERROR_FOUND) return false;
		if(!is_array($files) or count($files) === 0) return false;
		$childClass = static::$childClass;
		if(!$childClass::hasproperty($field)) {
			echo 'Invalid property' . PHP_EOL;
			return false;
		}
		$i = 0;
		$out = 0;
		foreach($files as $file) {
			if(!$this->has($file->name) or !is_numeric($file->$field)) continue;
			if($i === 0)
				$out = $file;
			else if($file->$field > $out->$field)
				$out = $file;
			$i++;
		}
		if($i === 0)
			return false;
		switch($paras['return']) {
			case 'object': return $out;
			case 'value': return $out->$field;
		}
	}
	public function getstats(array $paras) {
		$childClass = static::$childClass;
		// paras for this method
		$gs_paras = array();
		if($this->process_paras($paras, array(
			'name' => __FUNCTION__,
			'synonyms' => array(0 => 'field', 'q' => 'getstats_quiet'),
			'checklist' => array(
				'field' => 'Field to give statistics for',
				'includefiles' => 'Whether to include files in the output',
				'groupby' => 'Parameter to group by',
				'getstats_quiet' => 'Whether output should be printed (note that the parameter "quiet" will be passed to mlist and bfind)',
			),
			'checkfunc' => function($in) {
				// paras are passed to bfind/mlist, which will check them more fully
				return true;
			},
			'default' => array(
				'includefiles' => false,
				'groupby' => false,
				'getstats_quiet' => false,
				'quiet' => true, // be quiet when we're in mlist and bfind
			),
			'errorifempty' => array(
				'field',
			),
			'checkparas' => array(
				'field' => function($in) use ($childClass) {
					return $childClass::hasproperty($in);
				},
				'groupby' => function($in) use ($childClass) {
					return $childClass::hasproperty($in);
				},
			),
			'split' => true,
		), $gs_paras) === PROCESS_PARAS_ERROR_FOUND) return false;
		// do "groupby" if desired
		if($gs_paras['groupby'] !== false) {
			// call mlist to get groups
			$groups = $this->mlist(array(
				'print' => false,
				'field' => $gs_paras['groupby'],
			));
			// check for mlist errors
			if(!$groups) {
				return false;
			}
			// output
			$out = array();
			// use original paras for quiet, includefiles, and field
			$paras['field'] = $gs_paras['field'];
			$paras['includefiles'] = $gs_paras['includefiles'];
			$paras['getstats_quiet'] = $gs_paras['getstats_quiet'];
			foreach($groups as $group => $i) {
				ob_start();
				if(!$gs_paras['getstats_quiet'])
					echo '-------' . PHP_EOL . 'Group: ' . $group . PHP_EOL;
				$paras[$gs_paras['groupby']] = $group;
				$success = $this->getstats($paras);
				if($success) {
					$out[$group] = $success;
					ob_end_flush();
				}
				else {
					// we failed
					ob_end_clean();
				}
			}
			return $out;
		}
		// only include things where the field is actually numeric
		$paras[$gs_paras['field']] = '/^\s*\d+(\.\d+)?\s*$/';
		$paras['return'] = 'objectarray';
		// perform search
		$files = $this->bfind($paras);
		if(!$files) {
			return false;
		}
		// assemble output
		$out = array();
		$out['count'] = count($files);
		// add statistics
		foreach(array('average', 'stdev', 'smallest', 'largest') as $var) {
			$out[$var] = $this->$var($files, $gs_paras['field']);
			if(!$gs_paras['getstats_quiet']) {
				echo ucfirst($var) . ': ' . round($out[$var], 3) . PHP_EOL;
			}
		}
		if($gs_paras['includefiles']) {
			$out['files'] = $files;
		}
		return $out;
	}
	public function listz(array $paras) {
		// initialize variables used in process_paras call
		$listz_paras = array();
		$childClass = static::$childClass;
		$checkparas_f = function($in) use($childClass) {
			return $childClass::hasproperty($in);
		};
		if($this->process_paras($paras, array(
			'name' => __FUNCTION__,
			'synonyms' => array(0 => 'field'),
			'checklist' => array(
				'index' => 'Field to print for entries',
				'field' => 'Field to give statistics for',
				'into' => 'Field to place the computed Z-value into',
				'groupby' => 'Field to group by',
			),
			'default' => array(
				'into' => false,
				'index' => 'name',
				'groupby' => false,
			),
			'checkfunc' => function($in) { return true; },
			'checkparas' => array(
				'index' => $checkparas_f,
				'into' => $checkparas_f,
				'groupby' => $checkparas_f,
			),
			'errorifempty' => array('field'),
			'split' => array('into', 'index', 'groupby'),
		), $listz_paras) === PROCESS_PARAS_ERROR_FOUND) return false;
		// use groupby if needed
		if($listz_paras['groupby']) {
			$groups = $this->mlist(array(
				'field' => $listz_paras['groupby'],
				'print' => false,
			));
			$paras['index'] = $listz_paras['index'];
			$paras['into'] = $listz_paras['into'];
			foreach($groups as $group => $i) {
				$paras[$listz_paras['groupby']] = $group;
				$this->listz($paras);
			}
			return true;
		}
		$paras['includefiles'] = true;
		$stats = $this->getstats($paras);
		if(!$stats) {
			echo 'No entries found' . PHP_EOL;
			return false;
		}
		if($stats['stdev'] === 0) {
			echo 'SD = 0, so Z = 0 for all files' . PHP_EOL;
			return false;
		}
		foreach($stats['files'] as $file) {
			$z = ($file->{$paras['field']} - $stats['average']) / $stats['stdev'];
			echo $file->{$listz_paras['index']} . "\t" . $z . PHP_EOL;
			if($listz_paras['into']) {
				$file->set(array($listz_paras['into'] => $z));
			}
		}
		return true;
	}
	/* logging and backups */
	public function backup(array $paras = array()) {
		if($this->process_paras($paras, array(
			'name' => __FUNCTION__,
			'checklist' => array(
				'revert' => 'Revert to the last backup',
				'diff' => 'Show a diff with the last backup',
			),
			'default' => array(
				'revert' => false,
				'diff' => false,
			),
		)) === PROCESS_PARAS_ERROR_FOUND) return false;
		$file = static::$fileloc;
		$backupdir = dirname($file) . '/../Backup/';
		$lastbackup = $this->getbackup($backupdir);
		if($paras['revert']) {
			return $this->shell(array(
				'cmd' => 'cp', 
				'arg' => array($lastbackup, $file)
			));
		} else if($paras['diff']) {
			return $this->shell(array(
				'cmd' => 'diff',
				'arg' => array($lastbackup, $file),
			));
		} else {
			$date = new DateTime();
			return $this->shell(array(
				'cmd' => 'cp',
				'arg' => array(
					$file,
					$backupdir . basename($file) . '.' . $date->format('YmdHis')
				),
			));
		}
	}
	private function getbackup($backupdir) {
		$backuplist = explode(PHP_EOL, $this->shell(array(
			'cmd' => 'ls -t ' . escapeshellarg($backupdir),
			'return' => 'output',
			'printout' => false,
		)));
		return $backupdir . array_shift($backuplist);		
	}
}
