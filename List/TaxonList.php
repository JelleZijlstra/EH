<?
class TaxonList extends CsvFileList {
	public $extantonly; // whether output includes extinct species
	public $par; // array of arrays that list the children for each parent
	protected static $fileloc = LISTFILE;
	protected static $childclass = 'Taxon';
	// parsers
	public $wref_p;
	static $TaxonList_commands = array(
		'outputhtml' => array('name' => 'outputhtml',
			'aka' => array('html'),
			'desc' => 'Outputs a HTML version of the list',
			'arg' => 'None',
			'execute' => 'callmethod'),
		'outputtext' => array('name' => 'outputtext',
			'aka' => array('text'),
			'desc' => 'Outputs a text version of the list for a particular genus',
			'arg' => 'Genus to be listed',
			'execute' => 'callmethod'),
		'outputwiki' => array('name' => 'outputwiki',
			'aka' => array('wiki'),
			'desc' => 'Ouputs a wikitext version of the list',
			'arg' => 'None',
			'execute' => 'callmethod'),
		'wikipublish' => array('name' => 'wikipublish',
			'desc' => 'Writes the wikitext version of the list to Wikipedia',
			'arg' => 'None',
			'execute' => 'callmethod'),
		'edit' => array('name' => 'edit',
			'aka' => array('e'),
			'desc' => 'Edits a particular taxon',
			'arg' => 'Taxon name',
			'execute' => 'callmethod'),
		'newtaxon' => array('name' => 'newtaxon',
			'aka' => array('add', 'new'),
			'desc' => 'Adds a taxon to the list',
			'arg' => 'Taxon name',
			'execute' => 'callmethod'),
		'remove' => array('name' => 'remove',
			'desc' => 'Removes a taxon',
			'arg' => 'Taxon name',
			'execute' => 'callmethod'),
		'merge' => array('name' => 'merge',
			'desc' => 'Merge a taxon into another',
			'arg' => 'Taxon name; optionally --into=<taxon to be merged into>',
			'execute' => 'callmethod'),
	);
	function __construct() {
		parent::__construct(self::$TaxonList_commands);
		// initial settings
		$this->extantonly = true;
	}
	public function add_entry(ListEntry $file, array $paras = array()) {
	// Adds a FullFile to this object
		if($this->process_paras($paras, array(
			'name' => __FUNCTION__,
			'checklist' => array(
				'name' => 'Name to write under',
				'isnew' => 'Whether we need to do things we do for new entries (as opposed to old ones merely loaded into the catalog)',
			),
			'default' => array(
				'name' => $file->name,
				'isnew' => false,
			),
		)) === PROCESS_PARAS_ERROR_FOUND) return false;
		if($file->discardthis) {
			unset($file);
			return false;
		}
		while($this->has($paras['name'])) {
			$cmd = $this->menu(array(
				'head' => "Taxon " . $paras['name'] . " already exists.",
				'options' => array(
					's' => 'skip this taxon',
					'r' => 'overwrite the existing taxon',
					'm' => 'rename the new taxon',
				),
			));
			switch($cmd) {
				case 's': return false;
				case 'r': break 2;
				case 'm':
					$newname = $this->getline('New name of taxon: ');
					if(!$file->move($newname)) {
						echo 'Error moving file' . PHP_EOL;
						continue 2;
					}
					break;
			}
		}
		parent::add_entry($file);
		$this->par[$file->parent][$paras['name']] = $paras['name'];
		if($paras['isnew']) {
			// No logging in List for now
			//$this->log($file->name, 'Added file to catalog');
			echo "Added to catalog!" . PHP_EOL;
			$this->needsave();
			$this->format($file->name);
		}
		return true;
	}
	private function call_root() {
		$args = func_get_args();
		$func = array_shift($args);
		// not quite sure why we need current here
		$tax = current($this->par['root']);
		return call_user_func_array(array($this->get($tax), $func), $args);
	}
	public function moveinlist($oldname, $newname) {
		if($oldname === $newname) {
			echo 'Error: old name ' . $oldname . ' and new name ' . $newname . ' are the same.' . PHP_EOL;
			return false;
		}
		$this->move_entry($oldname, $newname);
		$parent = $this->get($newname)->parent;
		$this->par[$parent][$newname] = $newname;
		unset($this->par[$parent][$oldname]);
		return true;
	}
	// static stuff for output
	static public $separate_wiki = array('Rodentia', 'Chiroptera', 'Lipotyphla', 'Primates', 'Muridae', 'Cricetidae', 'Vespertilionidae', 'Carnivora', 'Artiodactyla', 'Sciuridae'); // taxa that need separate output files for wiki output
	static private $start_html = "<html>\n<head>\n\t<title>List of currently recognized mammal species</title>\n\t<meta http-equiv='Content-Type' content='text/html;charset=utf-8'>\n\t<link rel='stylesheet' href='list.css'>\n</head>\n<body>"; // text that gets put at the beginning of HTML output
	static private $end_html = "</body>\n</html>\n"; // at end
	public $html_out; // resource to write HTML output to
	public $refsend_p; // parser for comments
	public function outputtext(array $paras = array()) {
		if($this->process_paras($paras, array(
			'name' => __FUNCTION__,
			'synonyms' => array(0 => 'taxon'),
			'checklist' => array('taxon' => 'Taxon to output'),
			'errorifempty' => array('taxon'),
		)) === PROCESS_PARAS_ERROR_FOUND) return false;
		$obj = $this->get($paras['taxon']);
		if($obj->rank !== 'genus') {
			echo 'Invalid taxon' . PHP_EOL;
			return false;
		}
		$text = $obj->text();
		if($text) {
			echo $text;
			return true;
		}
		else
			return false;
	}
	public function outputhtml(array $paras = array()) {
	// generates a HTML version of The List
		if($this->process_paras($paras, array(
			'name' => __FUNCTION__,
			'synonyms' => array(0 => 'taxon'),
			'checklist' => array(
				'taxon' => 'Taxon to print output for',
				'out' => 'File to write to',
			),
			'default' => array(
				'taxon' => 'list',
				'out' => false, // default used below needs $paras['taxon'] first
			),
		)) === PROCESS_PARAS_ERROR_FOUND) return false;
		$outfile = $paras['out'] ?: BPATH . '/List/data/' . $paras['taxon'] . '.html';
		$this->html_out = $out = @fopen($outfile, 'w');
		if(!$out) {
			echo 'Error: could not open output file ' . $outfile . PHP_EOL;
			return false;
		}
		fwrite($out, self::$start_html . PHP_EOL);
		if($paras['taxon'] !== 'list') {
			if($this->has($paras['taxon'])) {
				$this->completedata($paras['taxon']);
				$this->html($paras['taxon']);
			}
			else {
				echo 'Invalid taxon: ' . $paras['taxon'];
				return false;
			}
		}
		else
			$this->call_root('html');
		fwrite($out, self::$end_html);
		fclose($out);
		$this->html_out = NULL;
		$this->shell('open ' . $outfile);
		return true;
	}
	static $start_wiki = "This is part of a list of all currently recognized species of mammals:\n{{:Special:Prefixindex/User:Ucucha/List of mammals}}\nIt is based on the third edition of ''Mammal Species of the World'' (Wilson and Reeder, 2005) and incorporates changes made since then in the systematic literature. In addition, it strives to incorporate all mammal species that are known to have existed during the [[Holocene]], so as to give a more complete picture of Recent mammal diversity.\n\nThis document is intended primarily to gauge the completeness of our coverage of mammals. Please do not fix links or make changes in spelling and taxonomy, but let me know when you think you have discovered an error. In case of differences between this list and Wikipedia articles in spelling or taxonomy, this list is more likely to be correct. This file is generated automatically from a CSV file by a script.\n";
	public $wiki_out = array();
	static $end_wiki = "\n=References=\n{{reflist|colwidth=30em}}\n";
	public function outputwiki(array $paras = array()) {
		if($this->process_paras($paras, array(
			'name' => __FUNCTION__,
			'checklist' => array(
				'taxon' => 'Taxon to write a file for',
			),
			'default' => array(
				'taxon' => 'list',
			),
		)) === PROCESS_PARAS_ERROR_FOUND) return false;
		global $csvlist;
		$csvlist->citetype = 'wp';
		$this->wiki_out[$paras['taxon']] = fopen(BPATH . '/List/data/' . $paras['taxon'] . '.mw', "w");
		fwrite($this->wiki_out[$paras['taxon']], self::$start_wiki);
		if($paras['taxon'] === 'list') {
			$this->call_root('completedata');
			$mamm = $this->get('Mammalia');
			fwrite($this->wiki_out[$paras['taxon']], 
				"*Total number of genera: " . $mamm->ngen . PHP_EOL 
				. "*Total number of species: " . $mamm->nspec . PHP_EOL);
			$this->call_root('wiki', array('taxon' => 'list'));
		}
		else {
			$taxon = $this->get($paras['taxon']);
			$taxon->completedata();
			$taxon->wiki(array('taxon' => $paras['taxon']));
		}
		fwrite($this->wiki_out[$paras['taxon']], self::$end_wiki);
	}
	static public function wikipublish() {
		require_once(BPATH . '/UcuchaBot/Bot.php');
		$bot = getbot();
		// get pagename
		foreach(glob(BPATH. "/List/data/*.mw") as $file) {
			$filename = preg_replace("/.*\//u", "", $file);
			if($filename === 'list.mw')
				$pagename = 'User:Ucucha/List_of_mammals';
			else
				$pagename = 'User:Ucucha/List_of_mammals/' . ucfirst(substr($filename, 0, -3));
			echo "Writing to $pagename...";
			if(!$bot->writewp(array(
				'page' => $pagename,
				'file' => $file,
				'summary' => 'Update'
			)))
				echo "An error occurred" . PHP_EOL;
			echo " done" . PHP_EOL;
		}
	}
	public function cli() {
		$this->setup_commandline('list');
	}
	public function newtaxon(array $paras = array()) {
		if($this->process_paras($paras, array(
			'name' => __FUNCTION__,
			'synonyms' => array(0 => 'name'),
			'checklist' => array('name' => 'Name of new taxon'),
			'checkfunc' => function($in) {
				return property_exists('Taxon', $in);
			},
			'askifempty' => array('name'),
			'checkparas' => array(
				'name' => function($in) {
					global $taxonlist;
					if($taxonlist->has($in)) {
						echo 'A taxon with this name already exists.' . PHP_EOL;
						return false;
					} else {
						return true;
					}
				},
			),
		)) === PROCESS_PARAS_ERROR_FOUND) return false;
		return $this->add_entry(
			new Taxon($paras, 'n'),
			array('isnew' => true)
		);
	}
	public function remove(array $paras) {
		if($this->process_paras($paras, array(
			'name' => __FUNCTION__,
			'synonyms' => array(0 => 'name'),
			'checklist' => array('name' => 'Taxon to be removed'),
			'checkparas' => array(
				'name' => function($in) {
					global $taxonlist;
					return $taxonlist->has($in);
				},
			),
		)) === PROCESS_PARAS_ERROR_FOUND) return false;
		unset($this->par[$this->get($paras['name'])->parent][$paras['name']]);
		$this->remove_entry($paras['name']);
	}
	public function sortchildren($name) {
		if(!$this->par[$name]) return false;
		ksort($this->par[$name]);
	}
}
?>
