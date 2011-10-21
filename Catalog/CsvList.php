<?php
require_once(__DIR__ . '/../Common/ExecuteHandler.php');
class CsvList extends FileList {
	public $citetype; // default citation type
	public $verbosecite; // whether citation functions need to be verbose
	public $includesfn; // whether Sfn needs to be included in FullFile::citewp()
	public $includerefharv; // whether |ref=harv needs to be included in FullFile::citewp()
	public $addmanual; // whether we want data adding functions to ask for manual input
	public $lslist;
	public $newlist;
	public $burstlist;
	public $sugglist; // list of suggestions used in FullFile::newadd()
	public $foldertree; // tree of folders used in the List
	public $foldertree_n;
	protected static $resolve_redirects = true;
	protected static $inform_exclude = array('pdfcontent');
	static $CsvList_commands = array(
		'adddata' => array('name' => 'adddata',
			'desc' => 'Add data to existing reference through API lookups',
			'arg' => 'None',
			'execute' => 'doallorcurr'),
		'add_nofile' => array('name' => 'add_nofile',
			'aka' => array('a'),
			'desc' => 'Add a new non-file entry',
			'arg' => 'New file handle',
			'execute' => 'callmethodarg',
			'setcurrent' => true),
		'catalog_backup' => array('name' => 'catalog_backup',
			'aka' => array('backup'),
			'desc' => 'Save a backup of the catalog',
			'arg' => 'None',
			'execute' => 'callfunc'),
		'format' => array('name' => 'format',
			'desc' => 'Format an individual reference',
			'arg' => 'File handle',
			'execute' => 'docurr'),
		'formatall' => array('name' => 'formatall',
			'aka' => array('b'),
			'desc' => 'Format all references',
			'arg' => 'None',
			'execute' => 'callmethod'),
		'echocite' => array('name' => 'echocite',
			'aka' => array('c', 'cite'),
			'desc' => 'Cite a file',
			'arg' => 'File handle',
			'execute' => 'docurr'),
		'add_redirect' => array('name' => 'add_redirect',
			'aka' => array('d'),
			'desc' => 'Add a new redirect',
			'arg' => 'New file handle',
			'execute' => 'callmethodarg',
			'setcurrent' => true),
		'editalltitles' => array('name' => 'editalltitles',
			'desc' => 'Edit the title for files that have not yet had their titles edited',
			'arg' => 'None',
			'execute' => 'doallorcurr'),
		'edit' => array('name' => 'edit',
			'aka' => array('e'),
			'desc' => 'Edit information associated with a file',
			'arg' => 'File handle',
			'execute' => 'docurr'),
		'findhdl' => array('name' => 'findhdl',
			'desc' => 'Find HDL for AMNH titles',
			'arg' => 'None',
			'execute' => 'doallorcurr'),
		'searchgoogletitle' => array('name' => 'searchgoogletitle',
			'aka' => array('g'),
			'desc' => 'Search for the file\'s title in Google',
			'arg' => 'File handle',
			'execute' => 'docurr'),
		'my_inform' => array('name' => 'my_inform',
			'aka' => array('i', 'inform'),
			'desc' => 'Get information about a file',
			'arg' => 'File handle',
			'execute' => 'docurr'),
		'check' => array('name' => 'check',
			'aka' => array('k'),
			'desc' => 'Check for inconsistencies between the catalog and the library and for files to be added to the library',
			'arg' => 'None',
			'execute' => 'callmethod'),
		'move' => array('name' => 'move',
			'aka' => array('m'),
			'desc' => 'Move a file to a new name',
			'arg' => 'File handle',
			'execute' => 'docurr'),
		'openf' => array('name' => 'openf',
			'aka' => array('o', 'open'),
			'desc' => 'Open a file',
			'arg' => 'File handle',
			'execute' => 'docurr'),
		'parse_wlist' => array('name' => 'parse_wlist',
			'aka' => array('p', 'parse'),
			'desc' => 'Parse a string into a reference list',
			'arg' => 'Input string',
			'execute' => 'callfuncarg'),
		'parse_wtext' => array('name' => 'parse_wtext',
			'aka' => array('tparse'),
			'desc' => 'Parse an input file handle into a Wikipedia-ready file',
			'arg' => 'Filename',
			'execute' => 'callmethodarg'),
		'remove' => array('name' => 'remove',
			'aka' => array('r'),
			'desc' => 'Remove a file',
			'arg' => 'File handle',
			'execute' => 'docurr'),
		'edittitle' => array('name' => 'edittitle',
			'aka' => array('s'),
			'desc' => 'Edit the title of a file',
			'arg' => 'File handle',
			'execute' => 'docurr'),
		'setcitetype' => array('name' => 'setcitetype',
			'aka' => array('t'),
			'desc' => 'set the default style used for citing',
			'arg' => 'citetype',
			'execute' => 'callmethodarg'),
		'openurl' => array('name' => 'openurl',
			'aka' => array('u'),
			'desc' => 'Open the URL associated with a file',
			'arg' => 'File handle',
			'execute' => 'docurr'),
		'listinfo' => array('name' => 'listinfo',
			'desc' => 'Give information about the CsvList object',
			'arg' => 'None',
			'execute' => 'callmethod'),
		'removefirstpage' => array('name' => 'removefirstpage',
			'aka' => array('rmfirst'),
			'desc' => 'Remove the first page from the file\'s PDF',
			'arg' => 'File handle',
			'execute' => 'callmethodarg'),
		'dups' => array('name' => 'dups',
			'aka' => array('duplicates'),
			'desc' => 'Search for duplicate entries',
			'arg' => 'None',
			'execute' => 'callmethod'),
		'temp' => array('name' => 'temp',
			'desc' => 'Temporary cleanup command. Does whatever cleanup it is currently programmed to do.',
			'arg' => 'None',
			'execute' => 'callmethod'),
	);
	protected static $fileloc = CATALOG;
	protected static $childclass = 'FullFile';
	/* core utils */
	function __construct() {
		parent::__construct(self::$CsvList_commands);
		// set some stuff initially
		$this->needsave = false;
		$this->citetype = 'wp';
		$this->verbosecite = true;
		$this->includesfn = true;
		$this->includerefharv = true;
		$this->addmanual = true;
	}
	public function __invoke($file) {
		if(!$this->has($file)) {
			trigger_error('Invalid file: ' . $file, E_USER_WARNING);
			return false;
		}
		$file = $this->c[$file]->gettruename();
		return $this->c[$file]();
	}
	public function makeredirect($handle, $target) {
	// redirect one file to another
		if(!$this->has($handle) or !$this->has($target)) return false;
		$this->c[$handle]->name = 'SEE ' . $target;
		$this->c[$handle]->format();
		return true;
	}
	/* load related lists */
	function build_lslist() {
	/*
	 * Gets list of files into $this->lslist, an array of results (FullFile form).
	 */
		echo "acquiring list of files... ";
		// ls output as string
		$list = shell_exec("ls -pR " . LIBRARY);
		if(!$list) {
			echo "Could not find library." . PHP_EOL;
			return false;
		}
		// output associative array
		unset($this->lslist);
		$this->lslist = array();
		$escapelibrary = preg_replace("/\//", "\/", LIBRARY);
		$list = preg_split("/\n" . PHP_EOL . $escapelibrary . "/", $list);
		foreach($list as $folder) {
			$folder = preg_split("/:\n/", $folder);
			$path = preg_split("/\//", $folder[0]);
			$filelist = preg_split("/\n/", $folder[1]);
			foreach($filelist as $file) {
				// do not handle directories
				if(!preg_match("/\/$/", $file) && $file) {
					$this->lslist[$file] = new FullFile(array($file, $path[1], $path[2], $path[3]), 'l');
				}
			}
		}
		echo "done" . PHP_EOL;
	}
	function build_newlist($path = '', $out = 'newlist') {
		if(in_array($out, array('p'))) return false;
		// reset out list
		unset($this->$out);
		$this->$out = array();
		echo "acquiring list of new files... ";
		if(!$path) $path = TEMPPATH;
		// ls output as string
		$list = shell_exec("ls -p " . $path);
		if(!$list) {
			echo "no new files found" . PHP_EOL;
			return false;
		}
		// output associative array
		$list = preg_split("/\n/", $list);
		foreach($list as $file) {
			if(!preg_match("/\/$/", $file) && $file) {
				$this->{$out}[$file] = new FullFile();
				$this->{$out}[$file]->name = $file;
			}
		}
		if(count($this->$out) === 0) {
			echo "no new files found" . PHP_EOL;
			return false;
		}
		echo "done" . PHP_EOL;
	}
	/* adding stuff to the list */
	public function add_entry($file, $paras = '') {
	// Adds a FullFile to this CsvList object
	// $paras has the following members:
	// - string ['name'] - filename to write under (if different from $file->name)
	// - bool ['isnew'] - whether we need to do things we do for new files (as opposed to old ones merely loaded into the catalog)
		if(!$paras['name']) $paras['name'] = $file->name;
		while($this->has($paras['name'])) {
			echo "File " . $paras['name'] . " already exists.";
			if($this->isredirect($file->name)) echo ' The existing file is a redirect.';
			echo PHP_EOL;
			makemenu(array('s' => 'skip this file',
				'r' => 'overwrite the existing file',
				'm' => 'rename the new file',
			));
			switch(getinput()) {
				case 's': return false;
				case 'r': break 2;
				case 'm':
					echo 'New name of file: ';
					$newname = getinput();
					if(!$file->move($newname)) {
						echo 'Error moving file' . PHP_EOL;
						continue 2;
					}
					break;
			}
		}
		$this->c[$paras['name']] = $file;
		if($paras['isnew']) {
			$this->log($file->name, 'Added file to catalog');
			echo "Added to catalog!" . PHP_EOL;
			$this->needsave = true;
			$this->format($file->name);
		}
		return true;
	}
	function add_nofile($handle = '') {
		if(!$handle) while(true) {
			echo 'Handle for new reference ("q" to quit): '; 
			$handle = getinput();
			if($handle === 'q') return false;
			if($this->has($handle) or strpos($handle, '.') !== false)
				echo 'Invalid handle' . PHP_EOL;
			else
				break;
		}
		else if($this->has($handle) or strpos($handle, '.') !== false) {
			echo 'Invalid handle' . PHP_EOL;
			return false;
		}
		return $this->add_entry(new FullFile($handle, 'n'), array('isnew' => true));
	}
	function add_redirect($handle = '', $target = '') {
		if(!$handle) while(true) {
			echo 'Handle for new redirect ("q" to quit): '; 
			$handle = getinput();
			if($handle === 'q') return false;
			if($this->has($handle) or strpos($handle, '.') !== false)
				echo 'Invalid handle' . PHP_EOL;
			else
				break;
		}
		else if($this->has($handle)) {
			echo 'Invalid handle' . PHP_EOL;
			return false;
		}
		if(!$target) while(true) {
			echo 'Target for new redirect ("q" to quit): '; 
			$target = getinput();
			if($target === 'q') return false;
			if(!$this->has($target))
				echo 'Invalid target' . PHP_EOL;
			else
				break;
		}
		else if(!$this->has($target)) {
			echo 'Invalid target' . PHP_EOL;
			return false;
		}
		return $this->add_entry(new FullFile(array($handle, $target), 'r'), array('isnew' => true));
	}	
	/* do things with files */
	protected function parse_wtext($rawarg) {
		if($rawarg) {
			if(strpos($rawarg, '/') === false)
				$rawarg = '/Users/jellezijlstra/Dropbox/Open WP/' . $rawarg;
			return parse_wtext($rawarg);
		}
		else {
			echo 'Error: no argument given' . PHP_EOL;
			return false;
		}
	}
	public function byfile() {
	// deprecated
		$this->cli();
	}
	public function cli() {
	// Performs various functions in a pseudo-command line. A main entry point.
		$this->setup_commandline('Catalog');
	}
	function listinfo() {
		foreach($this as $property => $value) {
			echo $property . ': '; echo $value . PHP_EOL;
		}
	}
	/* check */
	function check($stop = true) {
	/*
	 * Checks the catalog for things to be changed:
	 * - Checks whether there are any files in the catalog that are not in the
	 *   library
	 * - Checks whether there are any files in the library that are not in the
	 *   catalog
	 * - Checks whether there are new files in temporary storage that need to be
	 *   added to the library
	 */
		// always get new ls list, since changes may have occurred since previous check()
		$this->build_lslist();
		if(!$this->lslist) return;
		$date = new DateTime();
		logwrite(PHP_EOL . PHP_EOL . 'check() session ' . $date->format('Y-m-d H:i:s') . PHP_EOL);
		if(!$this->csvcheck()) return;
		if(!$this->lscheck()) return;
		if(!$this->burstcheck()) return;
		$this->newcheck();
	}
	private function csvcheck() {
	/* check CSV list for problems
	 * - detect articles in catalog that are not in the actual library
	 * - correct filepaths
	 */
		echo 'checking whether cataloged articles are in library... ';
		foreach($this->c as $file) {
			if($found = $this->lslist[$file->name]) {
				// update path
				if(($file->folder != $found->folder) or
					($file->sfolder != $found->sfolder) or
					($file->ssfolder != $found->ssfolder)) {
					echo 'Updating folder for file ' . $file->name . PHP_EOL;
					echo 'Stored folder: ' . $file->folder . PHP_EOL;
					echo 'Current folder: ' . $found->folder . PHP_EOL;
					echo 'Stored sfolder: ' . $file->sfolder . PHP_EOL;
					echo 'Current sfolder: ' . $found->sfolder . PHP_EOL;
					echo 'Stored ssfolder: ' . $file->ssfolder . PHP_EOL;
					echo 'Current ssfolder: ' . $found->ssfolder . PHP_EOL;
					$file->folder = $found->folder;
					$file->sfolder = $found->sfolder;
					$file->ssfolder = $found->ssfolder;
					$this->needsave = true;
				}
			}
			else if($file->isfile() && !$file->isredirect()) {
				makemenu(array('i' => 'give information about this file',
						'l' => 'file has been renamed', 
						'r' => 'remove this file from the catalog',
						'm' => 'move to the next component',
						's' => 'skip this file',
						'q' => 'quit the program',
					), PHP_EOL . 'Could not find file ' . $file->name);
				while(true) {
					switch(getinput()) {
						case 'q': return false;
						case 'm': return true;
						case 'i': $file->my_inform(); break;
						case 'l': $file->rename('csv'); break 2;
						case 's': return true;
						// set name to NULL; putcsvlist() will then not write the file
						case 'r':
							$file->log('Removed');
							$this->needsave = true;
							$file->name = NULL;
							return true;
					}
				}
			}
		}
		echo 'done' . PHP_EOL;
		return true;
	}
	private function lscheck() {
	/* check LS list for errors
	 * - Detect articles in the library that are not in the catalog.
	 */
		echo "checking whether articles in library are in catalog... ";
		foreach($this->lslist as $lsfile) {
			if(!$this->has($lsfile->name)) {
				makemenu(array('l' => 'file has been renamed',
					'a' => 'add the file to the catalog',
					's' => 'skip this file',
					'q' => 'quit the program',
					'm' => 'move to the next component of the catalog',
				), PHP_EOL . 'Could not find file ' . $lsfile->name . ' in catalog');
				while(true) {
					switch(getinput()) {
						case 'q': return false;
						case 'm': return true;
						case 'l': $lsfile->rename('ls'); break 2;
						case 's': return true;
						case 'a': 
							$lsfile->add();
							$this->add_entry($lsfile, array('isnew' => true));
							break 2;
					}
				}
			}
		}
		echo "done" . PHP_EOL;
		return true;
	}
	private function burstcheck() {
		echo 'checking for files to be bursted... ';
		$this->build_newlist(BURSTPATH, 'burstlist');
		if($this->burstlist)
			foreach($this->burstlist as $file) {
				if(!$file->burst()) return false;
			}
		echo 'done' . PHP_EOL;
		return true;
	}
	private function newcheck() {
	// look for new files
		echo 'checking for newly added articles... ';
		$this->build_newlist();
		if($this->newlist) {
			foreach($this->newlist as $file) {
				switch($file->newadd()) {
					case 0: return;
					case 1: continue 2;
					case 2: $this->add_entry($file, array('isnew' => true));
				}
			}
		}
		echo 'done' . PHP_EOL;
	}
	private function find_dups($key, $needle, $paras = '') {
		if(!isset($paras['print'])) $paras['print'] = false;
		if(!isset($paras['printresult'])) $paras['printresult'] = false;
		$paras[$key] = $needle;
		$files = $this->bfind($paras);
		if(($files === false) or (count($files) === 0)) 
			return false;
		foreach($files as $file)
			echo $file->name . PHP_EOL . $file->citepaper() . PHP_EOL;
		return $files;
	}
	public function dups() {
	// automatically find (some) duplicate files, and handle them
		/*
		 * First try: find duplicate DOIs
		 */
		$dois = $this->mlist('doi', array('print' => false));
		foreach($dois as $doi => $n) {
			if($n > 1 && $doi) {
				echo "Found $n instances of DOI $doi" . PHP_EOL;
				$files = $this->find_dups('doi', $doi);
				if($files === false) continue;
				if(!$this->dups_core($files)) return;
			}
		}
		/*
		 * Other tries: titles?
		 */
		$titles = $this->mlist('getsimpletitle', array('isfunc' => true, 'print' => false));
		foreach($titles as $title => $n) {
			if($n > 1 && $title) {
				echo "Found $n instances of title $title" . PHP_EOL;
				$files = $this->find_dups('getsimpletitle()', $title);
				if($files === false) 
					continue;
				if(!$this->dups_core($files)) 
					return;
			}
		}
	}
	private function dups_core($files) {
		echo 'Command syntax: ' . PHP_EOL .
		'e <file>: edits file <file>' . PHP_EOL .
		'm <file>: moves file <file>' . PHP_EOL .
		'r <file>: removes file <file>' . PHP_EOL .
		's: quit this duplicate set' . PHP_EOL .
		'q: quit this function' . PHP_EOL .
		'o: open the files' . PHP_EOL;
		while(true) {
			echo 'dups_core> ';
			$in = getinput();
			if(strlen($in) > 2) {
				$file = substr($in, 2);
				if($this->has($file))
					$file = $this->c[$file]->gettruename();
				else {
					echo 'File does not exist' . PHP_EOL;
					continue;
				}
			}
			switch($in[0]) {
				case 'e': $this->c[$file]->edit(); break;
				case 'm':
					echo 'New name of file: ';
					$newname = getinput();
					if($newname === 'q') break;
					$this->c[$file]->move($newname);
					$this->needsave = true;
					break;
				case 'o':
					foreach($files as $fileo) 
						$fileo->openf();
					break;
				case 'r': 
					$this->remove($file);
					// add redirect from old file
					$targets = array();
					foreach($files as $fileo) {
						if($fileo->name and ($fileo->name !== $file)) {
							$targets[] = $fileo->name;
						}
					}
					if(count($targets) === 1)
						$target = array_pop($targets);
					else {
						echo 'Could not determine redirect target. Options:' . PHP_EOL;
						foreach($targets as $target)
							echo '- ' . $target . PHP_EOL;
						echo 'Type the redirect target below (q to quit):' . PHP_EOL;
						$target = getinput();
						if($target === 'q') break;
					}
					$this->add_entry(new FullFile(array($file, $target), 'r'));
					break;
				case 's': return true;
				case 'q': return false;
			}
		}
	}
	public function stats($paras = '') {
	// bool ['includefoldertree'] : whether we need to print the foldertree
		$this->expandargs($paras, array('f' => 'includefoldertree'));
		$results = array();
		foreach($this->c as $file) {
			if($file->isredirect())
				$nredirects++;
			else {
				foreach($file as $key => $property) {
					if($property) $results[$key]++;
					if(is_array($property)) {
						foreach($property as $key => $prop) {
							if($prop) $results[$key]++;
						}
					}
				}
				if(!$file->isfile())
					$nnonfiles++;
			}
		}
		$total = count($this->c);
		echo 'Total number of files is ' . $total . '. Of these, ' . $nredirects . ' are redirects and ' . $nnonfiles . ' are not actual files.' . PHP_EOL;
		$total -= $nredirects;
		ksort($results);
		foreach($results as $field => $number) {
			echo $field . ': ' . $number . ' of ' . $total . ' (' . round($number/$total*100, 1) . '%)' . PHP_EOL;
		}
		if($paras['includefoldertree']) {
			$this->build_foldertree_n();
			foreach($this->foldertree_n as $folder => $fc) {
				echo $folder . ': ' . $fc[0] . PHP_EOL;
				foreach($fc as $sfolder => $sfc) {
					if(!$sfolder) continue;
					echo $folder . '/' . $sfolder . ': ' . $sfc[0] . PHP_EOL;
					foreach($sfc as $ssfolder => $ssfc) {
						if(!$ssfolder) continue;
						echo $folder . '/' . $sfolder . '/' . $ssfolder . ': ' . $ssfc . PHP_EOL;
					}
				}
			}
		}
		return;
	}
	/* citing */
	public function setcitetype($new = '') {
		if($new and $this->validcitetype($new))
			$this->citetype = $new;
		else {
			echo 'Current citetype: ' . $this->citetype . PHP_EOL;
			while(true) {
				echo "New citetype: ";
				$new = getinput();
				if($this->validcitetype($new)) {
					$this->citetype = $new;
					break;
				}
				echo 'Invalid citetype' . PHP_EOL;
			}
		}
		return $this->citetype;
	}
	protected function validcitetype($in = '') {
		if(!$in) $in = $this->citetype;
		return method_exists('FullFile', 'cite' . $in);
	}
	/* first kind of suggestions: full paths */
	public function build_sugglist() {
	// builds an array of Suggester objects, with $file->getkey() (currently first word of the filename) used as the array key.
		foreach($this->c as $file) {
			// get information
			$folders = $file->folderstr();
			$key = $file->getkey();
			// if there is already a Suggester for this object, add the current folderstr to it
			if($this->sugglist[$key])
				$this->sugglist[$key]->add($folders);
			// else, make a new Suggester
			else
				$this->sugglist[$key] = new Suggester($folders);
		}
	}
	/* 2nd kind of suggestions: list folders with IDs */
	public function build_foldertree() {
	// build a tree of folders (array of array of arrays) used for suggestions
		foreach($this->c as $file) {
			//exclude non-files and redirects
			if($file->isor('nofile', 'redirect')) continue;
			if(!$this->foldertree[$file->folder])
				$this->foldertree[$file->folder] = array();
			if(!$this->foldertree[$file->folder][$file->sfolder] and $file->sfolder)
				$this->foldertree[$file->folder][$file->sfolder] = array();
			if(!$this->foldertree[$file->folder][$file->sfolder][$file->ssfolder] and $file->ssfolder)
				$this->foldertree[$file->folder][$file->sfolder][$file->ssfolder] = array();
		}
	}
	private function build_foldertree_n() {
	// as build_foldertree(), but include number of files
		foreach($this->c as $file) {
			//exclude non-files and redirects
			if($file->isor('nofile', 'redirect')) continue;
			// we'll use these so much, the short forms will be easier
			$f = $file->folder;
			$sf = $file->sfolder;
			$ssf = $file->ssfolder;
			if(!$this->foldertree_n[$f]) {
				$this->foldertree_n[$f] = array();
				if(!$sf) $this->foldertree_n[$f][0] = 1;
			}
			else if(!$sf)
				$this->foldertree_n[$f][0]++;
			if(!$this->foldertree_n[$f][$sf] and $sf) {
				$this->foldertree_n[$f][$sf] = array();
				if(!$ssf) $this->foldertree_n[$f][$sf][0] = 1;
			}
			else if($sf && !$ssf)
				$this->foldertree_n[$f][$sf][0]++;
			if($ssf)
				$this->foldertree_n[$f][$sf][$ssf]++;
/*			if(!$this->foldertree_n[$f][$sf][$ssf] and $ssf) {
				$this->foldertree_n[$f][$sf][$ssf] = array();
				$this->foldertree_n[$f][$sf][$ssf][0] = 1;
			}
			else if($ssf)
				$this->foldertree_n[$f][$sf][$ssf][0]++;
*/		}
	}
	/* URL magic */
	private $urls_by_journal;
	public function geturlsjournal($journal, $paras = '') {
		if(!$journal) return false;
		if(!$paras['rebuild'] and !isset($this->urls_by_journal)) {
			$mlistparas = array('groupby' => 'journal', 'print' => false, 'printresult' => false);
			$this->urls_by_journal = $this->mlist('gethost()', $mlistparas);
		}
		if(!is_array($this->urls_by_journal)) return false;
		return $this->urls_by_journal[$journal] ?: false;
	}
	public function temp() {
	// general cleanup function; does whatever it is currently programmed to do
		// remove leading "Mammalia"
		foreach($this->c as $name => $file) {
			if($file->isredirect()) continue;
			if(substr($name, 0, 9) !== 'Mammalia ') continue;
			echo 'processing ' . $name . '...' . PHP_EOL;
			$newname = substr($name, 9);
			if($this->has($newname)) {
				echo 'new name already exists' . PHP_EOL;
				continue;
			}
			echo 'Do you want to rename this file? (y/n; o to open it; e to edit it; q to quit) ';
			while(true) {
				switch(getinput()) {
					case 'y': break 2;
					case 'n': continue 3;
					case 'o': $file->openf();
					case 'e': $file->edit();
					case 'q': break 3;
				}
			}
			$file->move($newname);
		}
	}
}
class Suggester {
// cf. CsvList::$sugglist
	private $suggestions;
	function __construct($folders) {
		$this->suggestions[$folders] = 1;
	}
	function add($folders) {
		$this->suggestions[$folders]++;
	}
	function sort() {
		arsort($this->suggestions);
	}
	function getsugg() {
		$this->sort();
		foreach($this->suggestions as $key => $count)
			$out[] = explode(':', $key);
		return $out;
	}
}
?>
