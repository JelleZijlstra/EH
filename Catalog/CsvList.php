<?php
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
	public $pdfcontentcache = array(); // cache for FullFile::$pdfcontent
	protected static $resolve_redirects = true;
	protected static $inform_exclude = array('pdfcontent');
	static $CsvList_commands = array(
		'adddata' => array('name' => 'adddata',
			'desc' => 'Add data to existing reference through API lookups',
			'arg' => 'None',
			'execute' => 'callmethod'),
		'add_nofile' => array('name' => 'add_nofile',
			'aka' => array('a'),
			'desc' => 'Add a new non-file entry',
			'arg' => 'New file handle',
			'execute' => 'callmethod',
			'setcurrent' => true),
		'backup' => array('name' => 'backup',
			'desc' => 'Save a backup of the catalog',
			'arg' => 'None',
			'execute' => 'callmethod'),
		'format' => array('name' => 'format',
			'desc' => 'Format an individual reference',
			'arg' => 'File handle',
			'execute' => 'callmethod'),
		'formatall' => array('name' => 'formatall',
			'aka' => array('b'),
			'desc' => 'Format all references',
			'arg' => 'None',
			'execute' => 'callmethod'),
		'echocite' => array('name' => 'echocite',
			'aka' => array('c', 'cite'),
			'desc' => 'Cite a file',
			'arg' => 'File handle',
			'execute' => 'callmethod'),
		'add_redirect' => array('name' => 'add_redirect',
			'aka' => array('d'),
			'desc' => 'Add a new redirect',
			'arg' => 'None',
			'execute' => 'callmethod',
			'setcurrent' => true),
		'editalltitles' => array('name' => 'editalltitles',
			'desc' => 'Edit the title for files that have not yet had their titles edited',
			'arg' => 'None',
			'execute' => 'callmethod'),
		'email' => array('name' => 'email',
			'desc' => 'Email a file to someone',
			'arg' => 'File handle',
			'execute' => 'callmethod'),
		'findhdl' => array('name' => 'findhdl',
			'desc' => 'Find HDL for AMNH titles',
			'arg' => 'None',
			'execute' => 'callmethod'),
		'searchgoogletitle' => array('name' => 'searchgoogletitle',
			'aka' => array('g'),
			'desc' => 'Search for the file\'s title in Google',
			'arg' => 'File handle',
			'execute' => 'callmethod'),
		'check' => array('name' => 'check',
			'aka' => array('k'),
			'desc' => 'Check for inconsistencies between the catalog and the library and for files to be added to the library',
			'arg' => 'None',
			'execute' => 'callmethod'),
		'move' => array('name' => 'move',
			'aka' => array('m'),
			'desc' => 'Move a file to a new name',
			'arg' => 'File handle',
			'execute' => 'callmethod'),
		'openf' => array('name' => 'openf',
			'aka' => array('o', 'open'),
			'desc' => 'Open a file',
			'arg' => 'File handle',
			'execute' => 'callmethod'),
		'parse_wlist' => array('name' => 'parse_wlist',
			'aka' => array('p', 'parse'),
			'desc' => 'Parse a string into a reference list',
			'arg' => 'Input string',
			'execute' => 'callmethod'),
		'parse_wtext' => array('name' => 'parse_wtext',
			'aka' => array('tparse'),
			'desc' => 'Parse an input file handle into a Wikipedia-ready file',
			'arg' => 'Filename',
			'execute' => 'callmethod'),
		'remove' => array('name' => 'remove',
			'aka' => array('r'),
			'desc' => 'Remove a file',
			'arg' => 'File handle',
			'execute' => 'callmethod'),
		'edittitle' => array('name' => 'edittitle',
			'aka' => array('s'),
			'desc' => 'Edit the title of a file',
			'arg' => 'File handle',
			'execute' => 'callmethod'),
		'setcitetype' => array('name' => 'setcitetype',
			'aka' => array('t'),
			'desc' => 'set the default style used for citing',
			'arg' => 'citetype',
			'execute' => 'callmethod'),
		'openurl' => array('name' => 'openurl',
			'aka' => array('u'),
			'desc' => 'Open the URL associated with a file',
			'arg' => 'File handle',
			'execute' => 'callmethod'),
		'listinfo' => array('name' => 'listinfo',
			'desc' => 'Give information about the CsvList object',
			'arg' => 'None',
			'execute' => 'callmethod'),
		'removefirstpage' => array('name' => 'removefirstpage',
			'aka' => array('rmfirst'),
			'desc' => 'Remove the first page from the file\'s PDF',
			'arg' => 'File handle',
			'execute' => 'callmethod'),
		'dups' => array('name' => 'dups',
			'aka' => array('duplicates'),
			'desc' => 'Search for duplicate entries',
			'arg' => 'None',
			'execute' => 'callmethod'),
		'temp' => array('name' => 'temp',
			'desc' => 'Temporary cleanup command. Does whatever cleanup it is currently programmed to do.',
			'arg' => 'None',
			'execute' => 'callmethod'),
		'testtitles' => array('name' => 'testtitles',
			'desc' => 'Test FullFile::findtitle_pdfcontent()\'s capabilities',
			'arg' => 'None',
			'execute' => 'callmethod'),
	);
	protected static $fileloc = CATALOG;
	protected static $childclass = 'FullFile';
	/* core utils */
	function __construct() {
		parent::__construct(self::$CsvList_commands);
		// set some stuff initially
		$this->citetype = 'wp';
		$this->verbosecite = true;
		$this->includesfn = true;
		$this->includerefharv = true;
		$this->addmanual = true;
	}
	public function makeredirect($handle, $target) {
	// redirect one file to another
		if(!$this->has($handle) or !$this->has($target)) return false;
		$this->c[$handle]->name = 'SEE ' . $target;
		$this->c[$handle]->format();
		return true;
	}
	public function backup(array $paras) {
		if($this->process_paras($paras, array(
			'name' => __FUNCTION__,
			'checklist' => array( /* No paras */ ),
		)) === PROCESS_PARAS_ERROR_FOUND) return false;
		$date = new DateTime();
		exec_catch('cp ' . CATALOG . ' ' . BPATH . '/Catalog/Backup/catalog.csv.' . $date->format('YmdHis'));
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
		$this->lslist = array();
		$escapelibrary = preg_replace("/\//", "\/", LIBRARY);
		$list = preg_split("/\n" . PHP_EOL . $escapelibrary . "/", $list);
		foreach($list as $folder) {
			$folder = preg_split("/:\n/", $folder);
			if(!isset($folder[1])) continue;
			$path = preg_split("/\//", $folder[0]);
			if(!isset($path[2])) $path[2] = '';
			if(!isset($path[3])) $path[3] = '';
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
	public function add_entry(ListEntry $file, array $paras = array()) {
	// Adds a FullFile to this CsvList object
	// Type hint is ListEntry instead of FullFile to keep E_STRICT happy
		if($this->process_paras($paras, array(
			'name' => __FUNCTION__,
			'checklist' => array(
				'name' =>
					'Filename to write under (if different from $file->name',
				'isnew' =>
					'Whether we need to do things we do for new files (as opposed to old ones merely loaded into the catalog)',
			),
			'default' => array(
				'name' => $file->name,
				'isnew' => false,
			),
		)) === PROCESS_PARAS_ERROR_FOUND) return false;
		while($this->has($paras['name'])) {
			echo "File " . $paras['name'] . " already exists.";
			if($this->isredirect($file->name)) echo ' The existing file is a redirect.';
			echo PHP_EOL;
			$cmd = $this->menu(array(
				'options' => array(
					's' => 'skip this file',
					'r' => 'overwrite the existing file',
					'm' => 'rename the new file',
				),
			));
			switch($cmd) {
				case 's': return false;
				case 'r': break 2;
				case 'm':
					$newname = $this->getline(array(
						'prompt' => 'New name of file: '
					));
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
			$this->needsave();
			$this->format($file->name);
		}
		return true;
	}
	public function add_nofile(array $paras = array()) {
		if($this->process_paras($paras, array(
			'name' => __FUNCTION__,
			'synonyms' => array(
				0 => 'handle',
			),
			'checklist' => array(
				'handle' => 'Handle of new entry',
			),
			'askifempty' => array(
				'handle',
			),
			'checkparas' => array(
				'handle' => function($in) {
					// NOFILEs can't have dots in their names
					if(strpos($in, '.') !== false)
						return false;
					// when we're in PHP 5.4, use $this->has instead here
					global $csvlist;
					return !$csvlist->has($in);
				},
			),
		)) === PROCESS_PARAS_ERROR_FOUND) return false;
		return $this->add_entry(
			new FullFile($paras['handle'], 'n'),
			array('isnew' => true)
		);
	}
	public function add_redirect(array $paras = array()) {
		if($this->process_paras($paras, array(
			'name' => __FUNCTION__,
			'synonyms' => array(
				// eh returns them in reverse order, though I'd like to change that
				1 => 'handle',
				0 => 'target',
			),
			'checklist' => array(
				'handle' => 'Handle of new redirect',
				'target' => 'Target of new redirect',
			),
			'askifempty' => array('handle', 'target'),
			'checkparas' => array(
				'handle' => function($in) {
					global $csvlist;
					return !$csvlist->has($in);
				},
				'target' => function($in) {
					global $csvlist;
					return $csvlist->has($in);
				},
			),
		)) === PROCESS_PARAS_ERROR_FOUND) return false;
		return $this->add_entry(
			new FullFile(array($paras['handle'], $paras['target']), 'r'),
			array('isnew' => true)
		);
	}
	/* parsing - needs overall revision, and coordination with Parser class */
	protected function parse_wlist(array $paras) {
		if($this->process_paras($paras, array(
			'name' => __FUNCTION__,
			'synonyms' => array(0 => 'file'),
			'default' => array('file' => 'File to parse'),
			'errorifempty' => array('file'),
		)) === PROCESS_PARAS_ERROR_FOUND) return false;
		return parse_wlist($paras['file']);
	}
	protected function parse_wtext(array $paras) {
		if($this->process_paras($paras, array(
			'name' => __FUNCTION__,
			'synonyms' => array(0 => 'file'),
			'default' => array('file' => 'File to parse'),
			'errorifempty' => array('file'),
		)) === PROCESS_PARAS_ERROR_FOUND) return false;
		if(strpos($paras['file'], '/') === false)
			$paras['file'] = '/Users/jellezijlstra/Dropbox/Open WP/' . $paras['file'];
		return parse_wtext($paras['file']);
	}
	/* do things with files */
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
	public function check(array $paras = array()) {
	/*
	 * Checks the catalog for things to be changed:
	 * - Checks whether there are any files in the catalog that are not in the
	 *   library
	 * - Checks whether there are any files in the library that are not in the
	 *   catalog
	 * - Checks whether there are new files in temporary storage that need to be
	 *   added to the library
	 */
	 	if($this->process_paras($paras, array(
	 		'name' => __FUNCTION__,
	 		'checklist' => array( /* No paras */ ),
	 	)) === PROCESS_PARAS_ERROR_FOUND) return false;
		// always get new ls list, since changes may have occurred since previous check()
		$this->build_lslist();
		if(!$this->lslist) {
			return false;
		}
		$date = new DateTime();
		logwrite(PHP_EOL . PHP_EOL . 'check() session ' . $date->format('Y-m-d H:i:s') . PHP_EOL);
		// check whether all files in the catalog are in the actual library
		if(!$this->csvcheck()) {
			return false;
		}
		// check whether all files in the actual library are in the catalog
		if(!$this->lscheck()) {
			return false;
		}
		// check whether there are any files to be burst
		if(!$this->burstcheck()) {
			return false;
		}
		// check whether there are any new files to be added
		$this->newcheck();
		return true;
	}
	private function csvcheck() {
	/* check CSV list for problems
	 * - detect articles in catalog that are not in the actual library
	 * - correct filepaths
	 */
		echo 'checking whether cataloged articles are in library... ';
		foreach($this->c as $file) {
			if(isset($this->lslist[$file->name])) {
				// update path
				$file->setpath(array('fromfile' => $this->lslist[$file->name]));
			}
			else if($file->isfile() && !$file->isredirect()) {
				echo PHP_EOL;
				$cmd = $this->menu(array(
					'options' => array(
						'i' => 'give information about this file',
						'l' => 'file has been renamed',
						'r' => 'remove this file from the catalog',
						'm' => 'move to the next component',
						's' => 'skip this file',
						'q' => 'quit the program',
						'e' => 'edit the file',
					),
					'head' => 'Could not find file ' . $file->name,
					'process' => array(
						'i' => function() use($file) {
							$file->my_inform();
						},
						'e' => function() use($file) {
							$file->edit();
						},
					),
				));
				switch($cmd) {
					case 'q': return false;
					case 'm': return true;
					case 'l': $file->rename('csv'); break;
					case 's': break;
					case 'r':
						// set name to NULL; putcsvlist() will then not write the file
						$file->log('Removed');
						$this->needsave();
						$file->name = NULL;
						return true;
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
				echo PHP_EOL;
				$cmd = $this->menu(array(
					'options' => array(
						'l' => 'file has been renamed',
						'a' => 'add the file to the catalog',
						's' => 'skip this file',
						'q' => 'quit the program',
						'm' => 'move to the next component of the catalog',
					),
					'head' => 'Could not find file ' . $lsfile->name . ' in catalog',
				));
				switch($cmd) {
					case 'q': return false;
					case 'm': return true;
					case 'l': $lsfile->rename('ls'); break;
					case 's': break;
					case 'a':
						$lsfile->add();
						$this->add_entry($lsfile, array('isnew' => true));
						break;
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
	private function find_dups($key, $needle, array $paras = array()) {
		if(!isset($paras['quiet']))
			$paras['quiet'] = true;
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
		'i: give information about the files' . PHP_EOL;
		$history = array();
		while(true) {
			$file = '';
			$in = $this->getline(array(
				'lines' => $history,
				'prompt' => 'dups_core> ',
			));
			$history[] = $in;
			if(strlen($in) > 2) {
				$file = substr($in, 2);
				if($this->has($file))
					$file = $this->c[$file]->resolve_redirect();
				else
					$file = false;
				// if there is no file or the redirect is broken
				if($file === false) {
					echo 'File does not exist' . PHP_EOL;
					continue;
				}
			}
			switch($in[0]) {
				case 'e':
					$this->c[$file]->edit();
					break;
				case 'm':
					$newname = $this->getline(array(
						'prompt' => 'New name of file: '
					));
					if($newname === 'q') break;
					$this->c[$file]->move($newname);
					$this->needsave();
					break;
				case 'o':
					foreach($files as $fileo)
						$fileo->openf();
					break;
				case 'i':
					foreach($files as $fileo)
						$fileo->inform();
					break;
				case 'r':
					$this->c[$file]->remove();
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
						$target = $this->menu(array(
							'head' => 'Type the redirect target below',
							'options' => array('q' => 'Quit this file'),
						));
						if($target === 'q') break;
					}
					$this->add_entry(new FullFile(array($file, $target), 'r'));
					break;
				case 's':
					return true;
				case 'q':
					return false;
			}
		}
	}
	public function stats(array $paras = array()) {
		if($this->process_paras($paras, array(
			'name' => __FUNCTION__,
			'synonyms' => array(
				'f' => 'includefoldertree',
			),
			'checklist' => array(
				'includefoldertree' =>
					'Whether we need to print the foldertree',
			),
			'default' => array(
				'includefoldertree' => false,
			),
		)) === PROCESS_PARAS_ERROR_FOUND) return false;
		$results = array();
		$nredirects = $nnonfiles = 0;
		foreach($this->c as $file) {
			if($file->isredirect())
				$nredirects++;
			else {
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
	public function setcitetype(array $paras = array()) {
		if($this->process_paras($paras, array(
			'name' => __FUNCTION__,
			'synonyms' => array(
				0 => 'new',
			),
			'checklist' => array(
				'new' => 'New citetype',
			),
			'askifempty' => array(
				'new',
			),
			'checkparas' => array(
				'new' => function($in) {
					global $csvlist;
					return $csvlist->validcitetype($in);
				},
			),
		)) === PROCESS_PARAS_ERROR_FOUND) return false;
		$this->citetype = $paras['new'];
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
			if(isset($this->sugglist[$key]))
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
			if(!isset($this->foldertree[$file->folder]))
				$this->foldertree[$file->folder] = array();
			if(!isset($this->foldertree[$file->folder][$file->sfolder]) and $file->sfolder)
				$this->foldertree[$file->folder][$file->sfolder] = array();
			if(!isset($this->foldertree[$file->folder][$file->sfolder][$file->ssfolder]) and $file->ssfolder)
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
			if(!isset($this->foldertree_n[$f])) {
				$this->foldertree_n[$f] = array(0 => 0);
				if(!$sf) $this->foldertree_n[$f][0] = 1;
			}
			else if(!$sf)
				$this->foldertree_n[$f][0]++;
			if(!isset($this->foldertree_n[$f][$sf]) and $sf) {
				$this->foldertree_n[$f][$sf] = array(0 => 0);
				if(!$ssf) $this->foldertree_n[$f][$sf][0] = 1;
			}
			else if($sf && !$ssf)
				$this->foldertree_n[$f][$sf][0]++;
			if($ssf) {
				if(!isset($this->foldertree_n[$f][$sf][$ssf]))
					$this->foldertree_n[$f][$sf][$ssf] = 0;
				$this->foldertree_n[$f][$sf][$ssf]++;
			}
		}
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
	// general cleanup/test function; does whatever it is currently programmed to do (which at the moment is nothing)
	}
	public function testtitles($paras = array()) {
	// Test the findtitle_pdfcontent() method.
		if($this->process_paras($paras, array(
			'name' => __FUNCTION__,
			'checklist' => array(
				'file' => 'File to write results too',
			),
			'default' => array('file' => false),
		)) === PROCESS_PARAS_ERROR_FOUND) return false;
		if($paras['file'])
			$fp = fopen($paras['file'], 'w');
		$matches = $mismatches = $impossible = 0;
		foreach($this->c as $child) {
			$pdftitle = $child->findtitle_pdfcontent();
			if(!$pdftitle or $child->title[0] === '/') {
				$impossible++;
				continue;
			}
			$rectitle = $child->getsimpletitle();
			$dettitle = $child->getsimpletitle($pdftitle);
			if(($dettitle != $rectitle) and
				($rectitle ? (strpos($dettitle, $rectitle) === false) : true) and
				($dettitle ? (strpos($rectitle, $dettitle) === false) : true)
			) {
				echo 'Title mismatch for file ' . $child->name . PHP_EOL;
				echo 'Levenshtein distance: ' . ($levenshtein = levenshtein($dettitle, $rectitle)) . PHP_EOL;
				echo 'Actual title: ' . $child->title . PHP_EOL;
				echo "\tSimplified as $rectitle\n";
				echo "\tSimplified as $dettitle\n";
				echo 'Detected title: ' . $pdftitle . PHP_EOL;
				$mismatches++;
				if($paras['file'])
					fputcsv($fp, array($child->name, $child->title, $pdftitle, $rectitle, $dettitle, $levenshtein));
			}
			else {
				$matches++;
			}
		}
		if($paras['file'])
			fclose($fp);
		echo "\n\nTotal matches: $matches\nTotal mismatches: $mismatches\nCould not determine title: $impossible\n";
	}
	public function getpdfcontentcache() {
		if(count($this->pdfcontentcache) !== 0) return false;
		// this may take huge amounts of memory...
		ini_set('memory_limit', 1e10);
		$this->pdfcontentcache = json_decode(file_get_contents(PDFCONTENTCACHE), true);
		if($this->pdfcontentcache === NULL) {
			echo 'Error retrieving PDF content cache' . PHP_EOL;
			return false;
		}
		return true;
	}
	public function putpdfcontentcache() {
		// only save if we've actually retrieved the cache
		if(count($this->pdfcontentcache) > 0) {
			file_put_contents(
				PDFCONTENTCACHE,
				json_encode($this->pdfcontentcache)
			);
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
		if(!isset($this->suggestions[$folders]))
			$this->suggestions[$folders] = 0;
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
