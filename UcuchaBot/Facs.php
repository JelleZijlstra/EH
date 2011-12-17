<?php
require_once(__DIR__  . '/../Common/common.php');
require_once(BPATH . '/Common/List.php');
require_once(BPATH . '/UcuchaBot/Bot.php');
class FacsList extends FileList {
	protected static $fileloc;
	protected static $childclass = 'FacsEntry';
	public $bot;
	const fac = 'Wikipedia:Featured article candidates';
	public function __construct() {
		parent::__construct();
		$this->bot = getbot();
		self::$fileloc = BPATH . '/UcuchaBot/facs.csv';
	}
	public function cli() {
		$this->setup_commandline('Facs');
	}
	public function update() {
		$wpfac = $this->bot->fetchwp(self::fac);
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
				$this->set($fac, array('isonfac' => true));
				$id = $this->get($fac, 'id');
				if($id < $lastid)
					$this->set($fac, array('id' => ++$lastid, 'date' => $facdate));
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
		$this->needsave = true;
	}
}
class FacsEntry extends ListEntry {
	// stuff that is standard across GeneralList outputs
	protected static $parentlist = 'FacsList';
	protected static $FacsEntry_commands = array(
	
	);
	protected static $FacsEntry_synonyms = array(
	
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
	public function addnoms() {
	// add or overwrite nominators
		// get noms
		$factext = $this->p->bot->fetchwp($this->name);
		preg_match('/(?<=\n)\s*:\s*<small>\'\'Nominator\(s\): (.*)(?=\n)/u', $factext, $nomstext);
		$nomstext = $nomstext[0];
		preg_match_all('/\[\[\s*[uU]ser:\s*([^\|\]]+)(\||\]\])/u', $nomstext, $matches, PREG_PATTERN_ORDER);
		$noms = array();
		foreach($matches[1] as $nom) {
			$cd = trim($nom);
			// some people link subpages
			if(strpos($cd, '/') !== false)
				continue;
			$noms[] = $cd;
		}
		$this->nominators = $noms;
	}
}
