<?php
require_once(__DIR__  . '/../Common/common.php');
require_once(BPATH . '/Common/FileList.php');
require_once(BPATH . '/Common/ListEntry.php');
require_once(BPATH . '/UcuchaBot/Bot.php');
class FacsList extends FileList {
	protected static $fileloc;
	protected static $childclass = 'FacsEntry';
	public $bot;
	const fac = 'Wikipedia:Featured article candidates';
	protected static $FacsList_commands = array(
	);
	public function __construct() {
		self::$fileloc = BPATH . '/UcuchaBot/data/facs.csv';
		parent::__construct(self::$FacsList_commands);
		$this->bot = getbot();
	}
	public function cli() {
		$this->setup_commandline('Facs');
	}
	public function update() {
		echo 'Updating the FAC database...' . PHP_EOL;
		$wpfac = $this->bot->fetchwp(array('page' => self::fac));
		if($wpfac === false) {
			echo 'Unable to retrieve FAC' . PHP_EOL;
			return false;
		}
		$lines = preg_split('/[\r\n]+/u', $wpfac);
		$facs = array();
		$date = new DateTime();
		$facdate = $date->format('Ymd'); // check what this yields
		foreach($lines as $line) {
			if(preg_match('/^\s*{{\s*((Wikipedia|WP):\s*Featured[\s_]+article[\s_]+candidates\/.*?\/archive\d+)\s*}}\s*$/', $line, $match))
				$facs[] = mw_normalize($match[1]);
		}
		// reverse so oldest are up
		$facs = array_reverse($facs);
		$lastid = 0; //initialize
		foreach($facs as $key => $fac) {
			if($this->has($fac)) {
				$this->set(array(
					0 => $fac, 
					'isonfac' => true, 
					'archived' => false
				));
				$id = $this->get($fac, 'id');
				if($id < $lastid)
					$this->set(array(
						0 => $fac,
						'id' => ++$lastid,
						'date' => $facdate
					));
				else
					$lastid = $id;
			}
			else {
				$paras['resolution'] = 'HOLD';
				$paras['name'] = $fac;
				$paras['isonfac'] = true;
				$paras['isnew'] = true;
				$paras['date'] = $facdate;
				$paras['id'] = ++$lastid;
				if(!$this->add_entry(new FacsEntry($paras, 'n'))) {
					echo 'Error adding entry ' . $fac . PHP_EOL;
					continue;
				}
				$this->addnoms($fac);
			}
		}
		if(is_array($this->c)) foreach($this->c as $entry) {
			if(!$entry->isonfac and !$entry->archived) {
				echo 'Found archived entry: ' . $entry->name . PHP_EOL;
				$entry->archived = 1;
			}
		}
		$this->needsave();
		echo 'done' . PHP_EOL;
	}
}
class FacsEntry extends ListEntry {
	// stuff that is standard across GeneralList outputs
	protected static $parentlist = 'FacsList';
	protected static $FacsEntry_commands = array(
		'addnoms' => array('name' => 'addnoms',
			'desc' => 'Add nominators for an FAC',
			'arg' => 'None',
			'execute' => 'callmethod',
		),
	);
	public $name; // String name of FAC page
	public $nominators; // Array noms
	public $bools; // Array boolean vars
	static $n_bools = array('checkedcup');
	protected static $arrays_to_check = array('bools');
	// checked for WikiCup noms
	public $comments; // String comments
	public $date; // String date added to FAC
	public $resolution; // String what we're doing with it
	public $archived; // String whether it's still on
	public $isonfac; // Bool used internally and not saved; whether it's on current version of FAC
	public $isnew; // Bool used internally; whether it was found on current update()
	public $id; // ID on FAC
	public function cli(array $paras = array()) {
		$this->setup_commandline($this->name);
	}
	public function __construct($in = '', $code = '') {
		global ${self::$parentlist};
		if(!${self::$parentlist}) $this->p = ${self::$parentlist};
		switch($code) {
			case 'f': // loading from file (only one implemented by GeneralList)
				$this->name = $in[0];
				if($in[1]) $this->nominators = json_decode($in[1], true);
				if($in[2]) $this->bools = json_decode($in[2], true);
				$this->comments = $in[3];
				$this->date = $in[4];
				$this->resolution = $in[5];
				$this->archived = $in[6];
				$this->id = $in[7];
				break;
			case 'n': // associative array
				if(!$in['name']) {
					echo 'Error: name must be provided' . PHP_EOL;
					return false;
				}
				foreach($in as $key => $value) {
					if(self::hasproperty($key))
						$this->$key = $value;
				}
				break;
			default:
				echo 'Invalid code' . PHP_EOL;
				break;
		}
		parent::__construct(self::$FacsEntry_commands);
	}
	function toarray() {
		$out = array();
		$out[] = $this->name;
		$out[] = $this->getarray('nominators');
		$out[] = $this->getarray('bools');
		$out[] = $this->comments;
		$out[] = $this->date;
		$out[] = $this->resolution;
		$out[] = $this->archived;
		$out[] = $this->id;
		return $out;
	}
	function format() {
		$bools = array();
		foreach($bools as $bool) {
			$this->$bool = $this->$bool ? 1 : NULL;
		}
	}
	public function addnoms(array $paras = array()) {
	// add or overwrite nominators
		// get noms
		$factext = $this->p->bot->fetchwp(array('page' => $this->name));
		$name = $this->name;
		$bot = $this->p->bot;
		$failure = function() use($name, $bot) {
			echo 'Unable to retrieve nominators for page ' . $name . PHP_EOL;
			$bot->writewp(array(
				'page' => 'User talk:Ucucha',
				'kind' => 'appendtext',
				'text' => '
==Nominators for ' . $name . '==
I was unable to find the list of nominators for the FAC [[' . $name . ']]. Please fix the cause and run me again. ~~~~',
				'donotmarkasbot' => true,
				'summary' => 'Unable to retrieve nominators',
			));
		};
		preg_match('/(?<=\n)\s*:\s*<small>\'\'Nominator(\(?s\)?)?: (.*)(?=\n)/u',
			$factext,
			$matches
		);
		if(!isset($matches[0])) {
			$failure();
			return false;
		}
		$nomstext = $matches[0];
		preg_match_all('/\[\[\s*[uU]ser( talk)?:\s*([^\|\]\/]+)/u',
			$nomstext,
			$matches,
			PREG_PATTERN_ORDER
		);
		if(!isset($matches[2])) {
			$failure();
			return false;
		}
		$noms = array();
		foreach($matches[2] as $nom)
			$noms[trim($nom)] = true;
		foreach($noms as $key => $value)
			$this->nominators[] = $key;
		return true;
	}
}
