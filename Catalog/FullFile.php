<?php
class FullFile extends ListEntry {
	public $name; //name of file (or handle of citation)
	public $folder; //folder (NOFILE when "file" is not a file)
	public $sfolder; //subfolder
	public $ssfolder; //sub-subfolder
	public $addmonth; //month added to catalog
	public $addday; //day added to catalog
	public $addyear; //year added to catalog
	public $authors; //authors in form <last1>, <initial1>.<initial1>.; <last2>, ...
	public $year; //year published
	public $title; //title (chapter title for book chapter; book title for full book or thesis)
	public $journal; //journal published in
	public $series; //journal series
	public $volume; //journal volume
	public $issue; //journal issue
	public $start; //start page
	public $end; //end page
	public $url; //url where available
	public $doi; //DOI
	public $bookauthors; //authors of book (if file is a chapter)
	public $booktitle; //title of book (if file is a chapter)
	public $publisher; //publisher
	public $location; //geographical location published
	public $bookpages; //number of pages in book
	public $status; //status of the file
	public $ids;
	public $comm;
	public $bools;
	static $n_ids = array('isbn', 'eurobats', 'hdl', 'jstor', 'pmid', 'edition', 'issn', 'pmc'); // names of identifiers supported
	static $n_comm = array('pages', 'newtaxa', 'draft', 'muroids'); // names of commentary fields supported
	static $n_bools = array('editedtitle', 'parturl', 'fullissue', 'triedfindurl', 'triedfinddoi', 'triedadddata'); // variables (mostly boolean) supported
	static $parentlist = 'csvlist';
	private $truename_return;
	private $pdfcontent; // holds text of first page of PDF
	private $adddata_return; // private variable used in FullFile::adddata()
	static $FullFile_commands = array(
		'edittitle' => array('name' => 'edittitle',
			'aka' => array('t'),
			'desc' => 'Edit the title',
			'arg' => 'None',
			'execute' => 'callmethod'),
		'echocite' => array('name' => 'echocite',
			'aka' => array('c'),
			'desc' => 'Get a citation for this file',
			'arg' => 'Optionally, citation type to be used',
			'execute' => 'callmethod'),
		'echopdfcontent' => array('name' => 'echopdfcontent',
			'aka' => array('echopdf'),
			'desc' => 'Print the contents of the first page of the PDF file',
			'arg' => 'None',
			'execute' => 'callmethod'),
		'openf' => array('name' => 'openf',
			'aka' => array('o', 'open'),
			'desc' => 'Open the file',
			'arg' => 'None',
			'execute' => 'callmethod'),
		'openurl' => array('name' => 'openurl',
			'aka' => array('u'),
			'desc' => 'Open the URL associated with the file',
			'arg' => 'None',
			'execute' => 'callmethod'),
		'searchgoogletitle' => array('name' => 'searchgoogletitle',
			'aka' => array('g'),
			'desc' => 'Search for the file\'s title in Google',
			'arg' => 'None',
			'execute' => 'callmethod'),
		'removefirstpage' => array('name' => 'removefirstpage',
			'aka' => array('rmfirst'),
			'desc' => 'Remove the first page from the file\'s PDF',
			'arg' => 'None',
			'execute' => 'callmethod'),
		'expanddoi' => array('name' => 'expanddoi',
			'desc' => 'Retrieve data for this DOI',
			'arg' => 'None required',
			'execute' => 'callmethod'),
		'format' => array('name' => 'format',
			'desc' => 'Format the file',
			'arg' => 'None required',
			'execute' => 'callmethod'),
	);
	static $FullFile_synonyms = array();
	protected static $arrays_to_check = array('ids', 'comm', 'bools');
	/* OBJECT CONSTRUCTION AND BASIC OPERATIONS */
	public function __construct($in = '', $code = '') {
	// $in: input data (array or string)
	// $code: kind of FullFile to make
		global $csvlist;
		if($csvlist) $this->p = $csvlist;
		if(!$code) return;
		switch($code) {
			case 'f': // loading from file
				if(!is_array($in)) {
					trigger_error('Invalid input to ' . __METHOD__, E_USER_NOTICE);
					return;
				}
				$this->name = $in[6];
				$this->folder = $in[3];
				$this->sfolder = $in[4];
				$this->ssfolder = $in[5];
				$this->addmonth = $in[0];
				$this->addday = $in[1];
				$this->addyear = $in[2];
				$this->authors = $in[7];
				$this->year = $in[8];
				$this->title = $in[9];
				$this->journal = $in[10];
				$this->series = $in[11];
				$this->volume = $in[12];
				$this->issue = $in[13];
				$this->start = $in[14];
				$this->end = $in[15];
				$this->url = $in[16];
				$this->doi = $in[17];
				$this->bookauthors = $in[18];
				$this->booktitle = $in[19];
				$this->publisher = $in[20];
				$this->location = $in[21];
				$this->bookpages = $in[22];
				$this->status = $in[23];
				if($in[24]) $this->ids = json_decode($in[24], true);
				if($in[25]) $this->comm = json_decode($in[25], true);
				if($in[26]) $this->bools = json_decode($in[26], true);
				return;
			case 'r': // make new redirect
				if(!is_array($in)) {
					trigger_error('Invalid input to ' . __METHOD__, E_USER_NOTICE);
					return;
				}
				$this->name = $in[0];
				$this->folder = 'SEE ' . $in[1];
				break;
			case 'n': // new NOFILE entry
				if(!is_string($in)) {
					trigger_error('Invalid input to ' . __METHOD__, E_USER_NOTICE);
					return;
				}
				$this->folder = 'NOFILE';
				$this->name = $in;
				break;
			case 'b': case 'l': // add new entry from basic information. 'l' if calling FullFile::add() is not desired
				$this->name = $in[0];
				$this->folder = $in[1];
				$this->sfolder = $in[2];
				$this->ssfolder = $in[3];
				if($code === 'l')
					return;
				else
					break;
		}
		// add additional data
		$this->add();
	}
	public function toarray() {
		$out = array();
		$out[] = $this->addmonth;
		$out[] = $this->addday;
		$out[] = $this->addyear;
		$out[] = $this->folder;
		$out[] = $this->sfolder;
		$out[] = $this->ssfolder;
		$out[] = $this->name;
		$out[] = $this->authors;
		$out[] = $this->year;
		$out[] = $this->title;
		$out[] = $this->journal;
		$out[] = $this->series;
		$out[] = $this->volume;
		$out[] = $this->issue;
		$out[] = $this->start;
		$out[] = $this->end;
		$out[] = $this->url;
		$out[] = $this->doi;
		$out[] = $this->bookauthors;
		$out[] = $this->booktitle;
		$out[] = $this->publisher;
		$out[] = $this->location;
		$out[] = $this->bookpages;
		$out[] = $this->status;
		$out[] = $this->getarray('ids');
		$out[] = $this->getarray('comm');
		$out[] = $this->getarray('bools');
		return $out;
	}
	protected static $inform_exclude = array('pdfcontent');
	public function my_inform() {
	// provide information for a file
		$this->inform();
		if($this->isfile()) echo shell_exec('ls -l ' . $this->path()) . PHP_EOL;
	}
	public function path(array $paras = array()) {
	// returns path to file
		$this->process_paras($paras, array(
			'checklist' => array(
				'type', // type of path: shell, url, or none (no processing)
				'folder', // whether we want the folder only
			),
			'default' => array(
				'type' => 'shell',
				'folder' => false,
			),
		));
		if(!$this->isfile()) {
			return false;
		}
		switch($paras['type']) {
			case 'shell':
				$process = function($in) {
					return escape_shell($in);
				};
				break;
			case 'url':
				$process = function($in) {
					return str_replace('%2F', '/', rawurlencode($in));
				};
				break;
			case 'none':
				$process = function($in) {
					return $in;
				};
				break;
		}
		// if there is no folder, just return filename and hope for the best
		if($this->folder === NULL)
			return $process($this->name);
		$out = $process(LIBRARY) . "/" . $process($this->folder);
		if($this->sfolder) {
			$out .= "/" . $process($this->sfolder);
			if($this->ssfolder)
				$out .= "/" . $process($this->ssfolder);
		}
		if($paras['folder']) return $out;
		$out .= "/" . $process($this->name);
		return $out;
	}
	public function openf($paras = '') {
		if(!isset($paras['place'])) $paras['place'] = 'catalog';
		if($this->isfile()) {
			if($paras['place'] === 'catalog')
				exec_catch('open ' . $this->path());
			else if($paras['place'] === 'temp')
				exec_catch("open " . TEMPPATH . "/" . escape_shell($this->name));
			else
				echo 'Unrecognized place: ' . $paras['place'] . PHP_EOL;
		}
		else
			echo 'Not a file; cannot open' . PHP_EOL;
		return true;
	}
	public function remove() {
	// remove a file
		$cmd = $this->ynmenu('Are you sure you want to remove file ' . $this->name . '?');
		switch($cmd) {
			case 'y':
				if($this->isfile()) exec_catch('rm ' . $this->path());
				$this->log('Removed file');
				$this->p->remove_entry($this->name);
				echo "File $this->name removed." . PHP_EOL;
				unset($this->name); // this will prevent CsvList::save() from writing this file
				$this->p->needsave();
				break;
			case 'n': break;
		}
		return true;
	}
	function move($newname = '') {
	// renames a file
		if($this->isredirect()) return true;
		if($this->name === $newname) return true;
		if(!$newname or !is_string($newname)) while(true) {
			$newname = $this->getline('New name: ');
			if($newname === 'q') return false;
			break;
		}
		// what to do if new filename already exists
		if($this->p->has($newname)) {
			echo 'New name already exists: ' . $newname . PHP_EOL;
			if($this->p->isredirect($newname)) {
				$cmd = $this->ynmenu('The existing file is a redirect. Do you want to overwrite it?');
				switch($cmd) {
					case 'y': break;
					case 'n': return false;
				}
			}
			else
				return false;
		}
		// change the name internally
		$oldname = $this->name;
		$oldpath = $this->path();
		$this->name = $newname;
		// move the physical file
		if($this->isfile() && ($newpath = $this->path())) {
			$cmd = 'mv -n ' . $oldpath . ' ' . $newpath;
			if(!exec_catch($cmd)) {
				echo 'Error moving file ' . $oldname;
				return false;
			}
			$this->log('Moved file ' . $oldname);
		}
		// change the catalog
		if($this->p->has($oldname)) {
			$this->p->add_entry($this);
			// make redirect
			$this->p->unsetf($oldname);
			$this->p->add_redirect($oldname, $newname);
		}
		return true;
	}
	function rename($mode) {
	// files that have been renamed in the catalog but not the library, or vice versa
	// function edits the library and/or renames the file to correct them
	// @param mode: may be either "ls" (called during lscheck, renames actual file) or "csv" (called during csvcheck, renames file in catalog)
		switch($mode) {
			case 'ls':
				$elist = $this->p->lslist;
				$searchlist = $this->p->c;
				break;
			case 'csv':
				$elist = $this->p->c;
				$searchlist = $this->p->lslist;
				break;
			default: echo "Unrecognized mode"; return false;
		}
		echo PHP_EOL . "Type 'q' to quit and 'i' to get information about this file." . PHP_EOL;
		// ask user
		while(true) {
			$newname = $this->getline("New name of file: ");
			// get a way to escape
			if($newname === "q")
				return false;
			if($newname === "i")
				$this->my_inform();
			else {
				$searchres = $searchlist[$newname];
				if($searchres) {
					if($elist[$searchres->name])
						echo "File already exists in catalog." . PHP_EOL;
					else {
						$this->p->needsave();
						logwrite("File " . $this->name . " renamed to " . $newname . " (" . $mode . "check)." . PHP_EOL);
						if($mode === 'ls') {
							if(!exec_catch("mv -n " . $this->path() . " " . $searchres->path()))
								echo "Error moving file $this->name to $searchres->name." . PHP_EOL;
						}
						$oldname = $this->name;
						$this->name = $searchres->name;
						$this->folder = $searchres->folder;
						$this->sfolder = $searchres->sfolder;
						$this->ssfolder = $searchres->ssfolder;
						$this->p->add_entry($this);
						// make redirect
						$this->p->unsetf($oldname);
						$this->p->add_redirect($oldname, $newname);
						echo "Found new filename." . PHP_EOL;
						return true;
					}
				}
				else
					echo "Could not find a file with this name; try again." . PHP_EOL;
			}
		}
	}
	/* FORMATTING */
	public function format() {
		/*
		 * completion of partial citations
		 */
		// Natural History of Madagascar completion
		if($this->booktitle === 'The Natural History of Madagascar' or $this->booktitle === 'nathistmad') {
			$this->booktitle = 'The Natural History of Madagascar';
			if(!$this->bookauthors) $this->bookauthors = 'Goodman, S.M.; Benstead, J.P. (eds.)';
			if(!$this->year) $this->year = 2003;
			if(!$this->publisher) $this->publisher = 'The University of Chicago Press';
			if(!$this->bookpages) $this->bookpages = 1709;
			if(!$this->isbn) $this->isbn = '9780226303079';
			if(!$this->location) $this->location = 'Chicago, Illinois, and London, England';
		}
		// MSW 3 completion
		if(in_array($this->booktitle, array('msw3', 'MSW3', 'Mammal Species of the World', 'Mammal Species of the World: A Taxonomic and Geographic Reference', 'Mammal Species of the World: A Taxonomic and Geographic Reference. 3rd ed.'))) {
			$this->booktitle = 'Mammal Species of the World: A Taxonomic and Geographic Reference';
			$this->bookauthors = 'Wilson, D.E.; Reeder, D.M. (eds.)';
			if(!$this->year) $this->year = 2005;
			$this->publisher = 'Johns Hopkins University Press';
			if(!$this->bookpages) $this->bookpages = 2142;
			if(!$this->isbn) $this->isbn = '978-0-8018-8221-0';
			$this->location = 'Baltimore, Maryland';
			if(!$this->url) $this->url = 'http://www.bucknell.edu/msw3';
			if(!$this->edition) $this->edition = '3rd';
		}
		// Mammals of South America
		if(in_array($this->booktitle, array('Mammals of South America', 'Mammals of South America. Volume 1: Marsupials, Xenarthrans, Shrews, and Bats'))) {
			$this->booktitle = 'Mammals of South America. Volume 1: Marsupials, Xenarthrans, Shrews, and Bats';
			$this->year = 2008;
			$this->publisher = 'The University of Chicago Press';
			$this->location = 'Chicago, Illinois';
			$this->bookpages = 690;
			$this->isbn = '9780226282404';
		}
		// Cenozoic Mammals of Africa
		if(in_array($this->booktitle, array('Cenozoic Mammals of Africa'))) {
			$this->booktitle = 'Cenozoic Mammals of Africa';
			$this->year = 2010;
			$this->publisher = 'University of California Press';
			$this->location = 'Berkeley, California';
			$this->bookpages = 1008;
			$this->isbn = '9780520257214';
		}
		// expand journals
		$this->expandjournal();
		// automatically detect title in MS papers
		if(!$this->title && preg_match("/^MS /", $this->name))
			$this->title = preg_replace("/^MS | n|.pdf$/", "", $this->name);
		/*
		 * conversion
		 */
		if($this->isredirect()) {
			// no other data for redirects
			$redirect_remove = array('sfolder', 'ssfolder', 'authors', 'year', 'title', 'journal', 'volume', 'series', 'issue', 'start', 'end', 'bookauthors', 'booktitle', 'pages', 'bookpages', 'ids', 'comm', 'doi', 'url', 'location', 'status', 'bools');
			foreach($redirect_remove as $key)
				unset($this->$key);
			$target = $this->gettruename();
			// this line crashed the program once; can't reproduce now
			if(!$this->p->has($target))
				$this->warn('invalid redirect target', 'folder');
			else if($this->p->isredirect($target))
				$this->folder = 'SEE ' . $this->p->gettruename($target);
		}
		if($this->issupplement()) {
			$supplement_remove = array('authors', 'year', 'journal', 'volume', 'series', 'issue', 'start', 'end', 'bookauthors', 'booktitle', 'pages', 'bookpages', 'ids', 'comm', 'doi', 'url', 'location', 'status', 'bools');
			foreach($supplement_remove as $key)
				unset($this->$key);
			$target = $this->supp_getbasic();
			// resolve redirect
			if($this->p->isredirect($target))
				$this->title = preg_replace('@^/([^/]+)/.*$@', '/$1/' . $this->p->gettruename($target), $this->title);

		}
		if($this->parturl)
			$this->parturl = 1;
		else
			unset($this->parturl);
		if($this->editedtitle)
			$this->editedtitle = 1;
		else
			unset($this->editedtitle);
		// replace with DOI
		if($this->jstor) {
			$this->doi = '10.2307/' . $this->jstor;
			unset($this->jstor);
		}
		// those are unnecessary
		if($this->url)
			unset($this->triedfindurl);
		if($this->doi)
			unset($this->triedfinddoi);
		if(!$this->needsdata() and $this->doi)
			unset($this->triedadddata);
		// this indicates it's in press
		if($this->start === 'no') {
			$this->start = 'in press';
			$this->end = NULL;
		}
		// correct dashes (also prevent OpenOffice from bad things)
		$this->volume = str_replace('-', "–", $this->volume);
		$this->issue = preg_replace("/[-_]/u", "–", $this->issue);
		$this->year = str_replace('-', "–", $this->year);
		// all-uppercase author names corrected (disabled because of "IUCN")
		$this->authors = preg_replace_callback("/(?<=^|\s)[A-Z]+(?=[,\s])/u", create_function('$uppercase', 'return ucfirst(mb_strtolower($uppercase[0], "UTF-8"));'), $this->authors);
		// typo I often make (> for .), and stuff that just happens
		$this->authors = str_replace(array('>', ';;'), array('.', ';'), $this->authors);
		// Jr. problems
		$this->authors = preg_replace('/; Jr\.(?=;|$)/', ', Jr.', $this->authors);
		// cap after hyphen
		$this->authors = preg_replace_callback("/(?<=-)(\w)(?!\.)/u", create_function('$matches', 'return mb_strtoupper($matches[0], \'UTF-8\');'), $this->authors);
		// some journals place the volume in "issue"
		if($this->issue && !$this->volume) {
			$this->volume = $this->issue;
			$this->issue = NULL;
		}
		$this->booktitle = preg_replace('/\.$/u', '', $this->booktitle);
		// clean up titles a little: space before and after parentheses, but not in e.g. "origin(s)" (hidden as this is buggy)
		//$this->title = trim(preg_replace(array("/(?<!\s|^)\((?!s\))/u", "/\)(?!\s|$|,|\.|:|<)/u"), array(" (", ") "), $this->title));
		//$this->title = preg_replace("/(?<!\s|\(|-|^)([A-Z])/u", " $1", $this->title);
		if(substr($this->url, 0, 28) === 'http://www.jstor.org/stable/') {
		// put JSTOR in
			$this->jstor = substr($this->url, 28);
			$this->url = NULL;
		}
		if(substr($this->url, 0, 22) === 'http://hdl.handle.net/') {
		// and HDL
			$this->hdl = substr($this->url, 22);
			$this->url = NULL;
		}
		if(preg_match("/^http:\/\/dx\.doi\.org\//", $this->url)) {
		// put DOI in and remove redundant URL
			$this->doi = preg_replace("/^http:\/\/dx\.doi\.org\//", "", $this->url);
			$this->url = NULL;
		}
		if(preg_match("/^http:\/\/www\.bioone\.org\/doi\/(full|abs|pdf)\/(.*)$/", $this->url, $doim)) {
			$this->doi = $doim[2];
			$this->url = NULL;
		}
		if(preg_match("/^http:\/\/onlinelibrary\.wiley\.com\/doi\/(.*?)\/(abs|full|pdf|abstract)$/", $this->url, $doim)) {
			$this->doi = $doim[1];
			$this->url = NULL;
		}
		if(preg_match('/^http:\/\/(digitallibrary\.amnh\.org\/dspace|deepblue\.lib\.umich\.edu)\/handle\/(.*)$/', $this->url, $hdlm)) {
			$newhdl = $hdlm[2];
			if(!$this->hdl) {
				$this->hdl = $newhdl;
			}
			else if($this->hdl != $newhdl) {
				echo 'Recorded HDL for file ' . $this->name . ': ' . $this->hdl . '. HDL in URL: ' . $newhdl . PHP_EOL;
			}
			$this->url = NULL;
		}
		// fix DOI formatting
		$this->doi = rawurldecode($this->doi);
		$this->title = preg_replace(
		// remove final period and curly quotes, italicize cyt b, other stuff
			array(
				'/(?<!\\\\)\.$|^\s+|\s+$|(?<=^<i>)\s+|<i><\/i>|☆/u',
				'/<\/i>\s+<i>|\s+/u',
				'/^" |(?<= )" |&quot;/u',
				'/([,:();]+)<\/i>/u',
				'/<i>([,:();]+)/u',
				'/[`‘’]/u',
				'/(?<=\bcytochrome[- ])b\b/u',
				'/ - /u',
				'/(?<=<|<\/)I(?=>)/u',
			), array(
				'',
				' ',
				'"',
				'</i>$1',
				'$1<i>',
				"'",
				'<i>b</i>',
				' – ',
				'i',
			), $this->title);
		if(strpos($this->publisher, ': ') !== false) {
		// remove final period and curly quotes
			$tmp = explode(': ', $this->publisher);
			$this->publisher = $tmp[1];
			$this->location = $tmp[0];
		}
		if(preg_match('/^(PhD|MSc) thesis, /', $this->journal)) {
		// "PhD thesis" should be in "publisher" field
			$this->publisher = $this->journal;
			$this->journal = NULL;
		}
		if($this->isthesis() and $this->pages) {
			$this->bookpages = $this->pages;
			unset($this->pages);
		}
		if(!$this->pages) unset($this->pages);
		/*
		 * do things that are necessary for all properties
		 */
		$this->formatallprops();
		/*
		 * check possible errors
		 */
		if(!$this->folder)
			$this->warn('no content', 'folder');
		if(!$this->isor('redirect', 'fullissue') && !$this->title)
			$this->warn('no content', 'title');
		if(!$this->isor('redirect', 'fullissue', 'supplement', 'erratum') && !$this->authors)
			$this->warn('no content', 'authors');
		if(!$this->isor('redirect', 'inpress') && !$this->volume && $this->journal)
			$this->warn('no content', 'volume');
		// odd stuff in authors
		if(strpos($this->authors, ';;') !== false)
			$this->warn('double semicolon', 'authors');
		if(preg_match('/; ([JS]r\.|[Ii]+)(;|$)/', $this->authors))
			$this->warn('stray junior', 'authors');
		// content of "year" is tightly constrained
		if(!$this->isor('redirect', 'supplement', 'erratum', 'inpress') and !preg_match('/\d{4}|\d{4}–\d{4}|undated/', $this->year))
			$this->warn('invalid content', 'year');
		// DOI titles tend to produce this kind of mess
		if(preg_match("/[A-Z] [A-Z] [A-Z]/", $this->title))
			$this->warn('spaced caps', 'title');
		// periods may indicate abbreviations, which we don't want
		// allow escaping with \
		if(preg_match("/(?<!\\\\)\./", $this->journal))
			$this->warn('period', 'journal');
		// buggy AMNH code tends to cause this
		if(preg_match("/\((?!ed\.\)|eds\.\))/", $this->authors))
			$this->warn('parenthesis', 'authors');
		// bug in previous code
		if($this->bookauthors and ($this->bookauthors == $this->booktitle))
			$this->warn('bookauthors equal with booktitle', 'bookauthors');
		// buggy Geodiversitas and AMNH code tends to cause this
		// OpenOffice weirdness
		if(preg_match("/\//", $this->issue))
			$this->warn('slash', 'issue');
		if(preg_match("/\//", $this->volume))
			$this->warn('slash', 'volume');
		if(strpos($this->volume, ':') !== false)
			$this->warn('colon', 'volume');
		if($this->start === 'no')
			$this->warn('text "no"', 'start');
		// AMNH journals need HDL thingy
		if($this->isamnh() && !$this->hdl)
			$this->warn('no HDL for AMNH title', 'journal');
		// deprecated
		if(!$this->isor('inpress', 'nopagenumberjournal', 'web') and $this->pages)
			$this->warn('deprecated parameter', 'pages');
		if(!$this->isor('inpress', 'nopagenumberjournal', 'fullissue') and $this->journal and !$this->start)
			$this->warn('no content in "start" for journal article', 'journal');
		$this->p->needsave();
		return true;
	}
	private function formatallprops() {
		// TODO: cap after apostrophes (D'elia)
		// get rid of fancy apostrophes and quotes
		foreach($this as $key => $field) {
			if(is_array($field) or is_object($field)) continue;
			if(is_string($field)) $this->$key = preg_replace(array("/([`’‘]|&apos;)/u", '/[“”]/u'), array("'", '"'), $this->$key);
			if(($key !== 'pdfcontent') and preg_match("/(\n|\r)/", $field))
				$this->warn('line break', $key);
			if(strpos($field, '??') !== false)
				$this->warn('double question mark', $key);
		}

	}
	private function expandjournal($in = '') {
		$i = $in ? $in : $this->journal;
		switch($i) {
			case 'AC': $o = 'Acta Chiropterologica'; break;
			case 'AGH': case 'Acta Geologica Hispanica': $o = 'Acta Geológica Hispánica'; break;
			case 'AJPA': $o = 'American Journal of Physical Anthropology'; break;
			case 'AMN': $o = 'American Museum Novitates'; break;
			case 'AMNR': case 'Arquivos do Museu Nacional, Rio de Janeiro': $o = 'Arquivos do Museu Nacional'; break;
			case 'ANMW': $o = 'Annales des Naturhistorischen Museums in Wien'; break;
			case 'APP': $o = 'Acta Palaeontologica Polonica'; break;
			case 'APR': $o = 'Acta Palaeontologica Romaniae'; break;
			// note that this was apparently 'Acta Societatis Zoologicae Bohemoslovenicae' before 1989 and 'ASZ Bohemoslovacae' 1990–1992
			case 'ASZB': $o = 'Acta Societatis Zoologicae Bohemicae'; break;
			case 'AZM': case 'Acta Zoologica Mexicana': $o = 'Acta Zoológica Mexicana'; break;
			case 'AZC': case 'Acta zoologica cracoviensia': $o = 'Acta Zoologica Cracoviensia'; break;
			case 'AZF': case 'Annales Zoologicae Fennici': $o = 'Annales Zoologici Fennici'; break;
			case 'BAMNH': $o = 'Bulletin of the American Museum of Natural History'; break;
			case 'BFMNH': $o = 'Bulletin of the Florida Museum of Natural History'; break;
			case 'BKBIN': $o = "Bulletin de l'Institut royal des sciences naturales de Belgique / Bulletin van het Koninklijk Belgisch Instituut voor Natuurwetenschappen"; break;
			case 'BMNHN': $o = "Bulletin du Muséum national d'histoire naturelle"; break;
			case 'BSHNB': case 'Boll. Soco Hist. Nat. Balears': $o = "Bolleti de la Societat d'Historia Natural de les Balears"; break;
			case 'BZB': case 'Bonner zoologische beiträge': $o = 'Bonner zoologische Beiträge'; break;
			case 'BZN': $o = 'Bulletin of Zoological Nomenclature'; break;
			case 'BZP': $o = 'Beiträge zur Paläontologie'; break;
			case 'CFS': $o = 'Courier Forschungsinstitut Senckenberg'; break;
			case 'CJS': $o = 'Caribbean Journal of Science'; break;
			case 'CJZ': $o = 'Canadian Journal of Zoology'; break;
			case 'CL': $o = 'Check List'; break;
			case 'CMPUM': case 'Contributions from the Museum of Paleontology, The University of Michigan': $o = 'Contributions from the Museum of Paleontology, University of Michigan'; break;
			case 'CN': $o = 'Chiroptera Neotropical'; break;
			case 'CPMHNM': case 'Comunicaciones Paleontologicas del Museo de Historia Natural de Montevideo': $o = 'Comunicaciones Paleontológicas del Museo de Historia Natural de Montevideo'; break;
			case 'Comptes Rendus de l\'Académie des Sciences de Paris, Earth and Planetary Sciences': case 'Comptes Rendus de l\'Académie des Sciences de Paris, Sciences de la Terre et des planètes': $o = 'Comptes Rendus de l\'Académie des Sciences - Series IIA - Earth and Planetary Science'; break;
			case 'CP': case 'COL-PA': $o = 'Coloquios de Paleontología'; break;
			case 'EA': case 'Evolutionary Anthropology': $o = 'Evolutionary Anthropology: Issues, News, and Reviews'; break;
			case 'EG': case 'Estudios Geologicos': $o = 'Estudios Geológicos'; break;
			case 'HIJM': $o = 'Hystrix Italian Journal of Mammalogy'; break;
			case 'HJG': $o = 'Hellenic Journal of Geosciences'; break;
			case 'Iheringia': case 'Iheringia Zoología': $o = 'Iheringia, Série Zoologia'; break;
			case 'JM': $o = 'Journal of Mammalogy'; break;
			case 'JHE': $o = 'Journal of Human Evolution'; break;
			case 'JME': $o = 'Journal of Mammalian Evolution'; break;
			case 'JOZ': case 'Journal of Zoology': $o = 'Journal of Zoology, London'; break;
			case 'JPSI': $o = 'Journal of the Palaeontological Society of India'; break;
			case 'JVP': $o = 'Journal of Vertebrate Paleontology'; break;
			case 'JSP': $o = 'Journal of Systematic Palaeontology'; break;
			case 'JSZ': case 'JZS': $o = 'Journal of Zoological Systematics and Evolutionary Research'; break;
			case 'JVMS': case 'Journal of Veterinary and Medical Sciences': $o = 'Journal of Veterinary Medical Science'; break;
			case 'MB': case 'Mammalian Biology - Zeitschrift fur Saugetierkunde': case 'Mammalian Biology - Zeitschrift fur Säugetierkunde': case 'Mammalian Biology - Zeitschrift für Säugetierkunde': $o = 'Mammalian Biology'; break;
			case 'MBE': $o = 'Molecular Biology and Evolution'; break;
			case 'MIOC': case 'Mem. Inst. Oswaldo Cruz': $o = 'Memórias do Instituto Oswaldo Cruz'; break;
			case 'MN': $o = 'Mastozoología Neotropical'; break;
			case 'MNSM': case 'Memoirs of the National Science Museum, Tokyo': $o = 'Memoirs of the National Science Museum'; break;
			case 'MPE': $o = 'Molecular Phylogenetics and Evolution'; break;
			case 'MPMZUM': $o = 'Miscellaneous Publications, Museum of Zoology, University of Michigan'; break;
			case 'MS': $o = 'Mammalian Species'; break;
			case 'MTEPHE': case 'Mém. Trav. E.P.H.E., Inst. Montpellier': $o = "Mémoires et Travaux de l'École Pratique des Hautes Études, Institut de Montpellier"; break;
			case 'NJGPA': case 'Neues Jahrbuch für Geologie und Paläontologie - Abhandlungen': $o = 'Neues Jahrbuch für Geologie und Paläontologie, Abhandlungen'; break;
			case 'NJGPMH': case 'NJGPM': case 'Neues Jahrbuch für Geologie und Paläontologie - Monatshefte': $o = 'Neues Jahrbuch für Geologie und Paläontologie, Monatshefte'; break;
			case 'OPMZUM': $o = 'Occasional Papers of the Museum of Zoology, University of Michigan'; break;
			case 'OPMZLSU': $o = 'Occasional Papers of the Museum of Zoology, Louisiana State University'; break;
			case 'OPTTU': case 'OPMTTU': case 'Occasional Papers, The Museum, Texas Tech University': $o = 'Occasional Papers, Museum of Texas Tech University'; break;
			case 'PAZ': case 'Papéis Avulsos de Zoologia, Museu de Zoologia da Universidade de São Paulo': $o = 'Papéis Avulsos de Zoologia'; break;
			case 'PBSW': $o = 'Proceedings of the Biological Society of Washington'; break;
			case 'PE': $o = 'Palaeontologia Electronica'; break;
			case 'PIE': $o = 'Paleontologia i Evolució'; break;
			case 'PLB': case 'PLOS Biology': $o = 'PLoS Biology'; break;
			case 'PLO': case 'PLoS one': case 'PloS ONE': $o = 'PLoS ONE'; break;
			case 'PLSNSW': $o = 'Proceedings of the Linnean Society of New South Wales'; break;
			case 'PKNAW': case 'Proceedings of the Koninklijke Nederlandse Academie van Wetenschappen': $o = 'Proceedings of the Koninklijke Nederlandse Akademie van Wetenschappen'; break;
			case 'PN': case 'Palaeontologica Nova, Seminario de Paleontología de Zaragoza': $o = 'Palaeontologica Nova'; break;
			case 'PNAS': $o = 'Proceedings of the National Academy of Sciences'; break;
			case 'POP': case 'Papers on Paleontology, The Museum of Palaeontology, University of Michigan': $o = 'Papers on Paleontology'; break;
			case 'PPP': case 'Palaeogeography,Palaeoclimatology,Palaeoecology': $o = 'Palaeogeography, Palaeoclimatology, Palaeoecology'; break;
			case 'PZ': case 'Paläontologisches Zeitschrift': $o = 'Paläontologische Zeitschrift'; break;
			case 'QSR': case 'Quaternary Science Review': $o = 'Quaternary Science Reviews'; break;
			case 'RAPBBA': $o = 'RAP Bulletin of Biological Assessment'; break;
			case 'RBG': case 'Rev. Brasil. Genet.': $o = 'Revista Brasileira de Genetica'; break;
			case 'RJT': $o = 'Russian Journal of Theriology'; break;
			case 'RSGE': $o = 'Revista de la Sociedad Geológica de España'; break;
			case 'RWAM': $o = 'Records of the Western Australian Museum'; break;
			case 'Science, New Series': $o = 'Science'; break;
			case 'SBN': $o = 'Stuttgarter Beiträge zur Naturkunde'; break;
			case 'SCC': $o = 'Small Carnivore Conservation'; break;
			case 'SCZ': $o = 'Smithsonian Contributions to Zoology'; break;
			case 'SG': $o = 'Scripta Geologica'; break;
			case 'TMGB': $o = 'Treballs del Museu de Geologia de Barcelona'; break;
			case 'VPA': $o = 'Vertebrata PalAsiatica'; break;
			case 'WNAN': $o = 'Western North American Naturalist'; break;
			case 'ZA': case 'Zoologischer Anzeiger - A Journal of Comparative Zoology': case 'Zoologischer Anzeiger, Jena': $o = 'Zoologischer Anzeiger'; break;
			case 'ZM': case 'Zoologische Mededelingen Leiden': case 'Zoölogische Mededelingen': $o = 'Zoologische Mededelingen'; break;
			case 'ZME': $o = 'Zoology in the Middle East'; break;
			default: $o = $in; break;
		}
		if($in)
			return $o;
		else
			if($o) $this->journal = $o;
	}
	/* KINDS OF FILES AND CONSEQUENCES */
	public function isor() {
		$args = func_get_args();
		foreach($args as $arg) {
			if(method_exists($this, 'is' . $arg)) {
				if($this->{'is' . $arg}())
					return true;
			}
			else
				trigger_error('Invalid input to ' . __METHOD__ . ': ' . $arg, E_USER_NOTICE);
		}
		return false;
	}
	public function isfullissue() {
		return $this->fullissue ? true : false;
	}
	public function iserratum() {
		return (strpos($this->name, 'erratum') !== false);
	}
	public function isnopagenumberjournal() {
	// these journals don't have page numbers, only "volume" and "issue". They should get rid of their failure to implement standard practices, but in the meantime we have to handle them.
		return in_array($this->journal,
			array('Palaeontologia Electronica',
				'BMC Biology',
				'BMC Evolutionary Biology',
				'PLoS ONE',
				'PLoS Biology',
				'Genome Biology',
				'Canid News',
				'Frontiers in Zoology',
				'Journal of Biomedicine and Biotechnology',
		));
	}
	public function isfile() {
	// returns whether this 'file' is a file
		return ($this->folder !== 'NOFILE');
	}
	public function ispdf() {
		return in_array(substr($this->name, -4, 4), array('.pdf', '.PDF'));
	}
	public function isnofile() {
		return !$this->isfile();
	}
	public function isredirect() {
		return (substr($this->folder, 0, 4) === 'SEE ');
	}
	public function truename() {
	// if file is a redirect, return true name
	/* example use
		if($this->truename()) return $this->truename_return;
	and when you have arguments
		if($this->truename()) return $this->__FUNCTION__(<args>);
	*/
		if($this->isredirect()) {
			$backtrace = debug_backtrace();
			$function = $backtrace[1]['function'];
			$truename = substr($this->folder, 4);
			if(!$this->p->has($truename)) {
				echo 'Invalid redirect target: ' . $truename . PHP_EOL;
				return false;
			}
			if($backtrace[1]['args'])
				return $truename;
			$this->truename_return = $this->p->{$function}($truename);
			return true;
		}
		else
			return false;
	}
	public function gettruename() {
		if($this->isredirect()) {
			$target = substr($this->folder, 4);
			if($this->p->has($target))
				return $target;
			else {
				$this->warn('invalid redirect target', 'folder');
				return false;
			}
		}
		else
			return $this->name;
	}
	public function isinpress() {
	// checks whether file is in "in press" (and therefore, year etcetera cannot be given
		return ($this->start === 'in press');
	}
	public function issupplement() {
		// some pieces have no title
		if(!isset($this->title[0])) return false;
		return ($this->title[0] === '/');
	}
	public function isamnh() {
		return in_array(
			$this->journal,
			array(
				'American Museum Novitates',
				'Bulletin of the American Museum of Natural History',
				'Anthropological Papers of the American Museum of the Natural History',
				'Memoirs of the American Museum of Natural History',
				)
			);
	}
	public function isweb() {
	// is this a web publication?
		return (!$this->bookpages && !$this->volume && !$this->start && !$this->journal && !$this->isbn && $this->url);
	}
	public function isthesis() {
		return preg_match('/^(PhD|MSc|BSc) thesis/', $this->publisher);
	}
	public function supp_getbasic() {
		$out = preg_replace('/^.*\//u', '', $this->title);
		if($this->p->has($out))
			return $out;
		else {
			$this->warn('unknown supplement target', 'title');
			return false;
		}
	}
	private function hasid() {
	// whether this file has any kind of online ID
		return ($this->url or $this->doi or $this->jstor or $this->hdl or $this->pmid or $this->pmc);
	}
	/* MANUAL EDITING */
	public function editalltitles() {
	// wrapper function for use in the edittitlesall command
		if($this->editedtitle == 1 or $this->isor('redirect', 'supplement'))
			return true;
		else {
			echo 'Editing title of file ' . $this->name . PHP_EOL;
			return $this->edittitle();
		}
	}
	public function edittitle() {
		$makesplit = function($title) use (&$splittitle) {
			$splittitle = explode(' ', $title);
			foreach($splittitle as $key => $word) {
				echo $key . ': ' . $word . PHP_EOL;
			}
		};
		$unite = function() use (&$splittitle) {
			$title = implode(' ', $splittitle);
			$title = preg_replace('/^\s+|\s+$|\s+(?= )/u', '', $title);
			return $title;
		};
		echo 'Current title: ' . $this->title . PHP_EOL;
		$makesplit($this->title);
		makemenu(array('l<n>' => 'make word <n> lowercase',
			'u<n>' => 'make word <n> uppercase',
			'i<n>' => 'make word <n> italicized',
			'e<n>' => 'edit word <n> only',
			't<n>' => 'merge word <n> with the next word',
			'r<n>' => 'remove word <n>',
			'e' => 'edit the entire title',
			'p' => 'preview the edited title',
			'c' => 'recalculate the words',
			'o' => 'open this file',
			'f' => 'edit this file',
			'a' => 'quit this file without saving changes',
			'q' => 'quit this program',
			's' => 'save the changed title',
			'm<filename>' => 'move to editing the title of file <filename>',
		), 'Command syntax:');
		while(true) {
			$cmd = $this->getline('edittitle> ');
			if(preg_match('/^([a-z]\d+(-\d+)?|m.*)$/', $cmd)) {
				$n = substr($cmd, 1);
				if(strpos($n, '-') !== false) {
					$sp = explode('-', $n);
					$nbeg = (int) $sp[0];
					$nend = (int) $sp[1];
					if($nbeg > $nend or !isset($splittitle[$nend])) {
						echo 'Invalid range' . PHP_EOL;
						continue;
					}
				}
				if(preg_match('/^\d+$/', $n) and !isset($splittitle[$n])) {
					echo 'Invalid word' . PHP_EOL;
					continue;
				}
			}
			switch($cmd[0]) {
				case 'l':
					if(isset($nbeg)) {
						for($i = $nbeg; $i <= $nend; $i++)
							$splittitle[$i] = mb_strtolower($splittitle[$i]);
					}
					else if(isset($n))
						$splittitle[$n] = mb_strtolower($splittitle[$n]);
					break;
				case 'u':
					if(isset($nbeg)) {
						for($i = $nbeg; $i <= $nend; $i++)
							$splittitle[$i] = mb_ucfirst($splittitle[$i]);
					}
					else if(isset($n))
						$splittitle[$n] = mb_ucfirst($splittitle[$n]);
					break;
				case 'i':
					if(isset($nbeg)) {
						$splittitle[$nbeg] = '<i>' . $splittitle[$nbeg];
						$splittitle[$nend] .= '</i>';
					}
					else if(isset($n))
						$splittitle[$n] = '<i>' . $splittitle[$n] . '</i>';
					break;
				case 'o':
					$this->openf();
					break;
				case 'f':
					$this->edit();
					break;
				case 'p':
					echo $unite() . PHP_EOL;
					break;
				case 'c':
					$newtitle = $unite();
					$makesplit($newtitle);
					break;
				case 'm':
					if($this->p->has($n))
						$this->p->edittitle($n);
					else
						echo 'Invalid title' . PHP_EOL;
					break;
				case 'q': return false;
				case 'a': $this->editedtitle = 1; return true;
				case 't':
					if(!isset($n)) break;
					$splittitle[$n] .= $splittitle[$n + 1];
					$splittitle[$n + 1] = '';
					break;
				case 'r':
					if(isset($nbeg)) {
						for($i = $nbeg; $i <= $nend; $i++)
							$splittitle[$i] = '';
					}
					else if(isset($n))
						$splittitle[$n] = '';
					break;
				case 's':
					$this->title = $unite();
					$this->format();
					echo 'New title: ' . $this->title . PHP_EOL;
					$this->log('Edited title');
					$this->editedtitle = 1;
					$this->p->needsave();
					return true;
				case 'e':
					if(isset($n)) {
						echo 'Current value of word ' . $n . ': ' . $splittitle[$n] . PHP_EOL;
						$splittitle[$n] = $this->getline('New value: ');
						break;
					}
					else {
						echo 'Current title: ' . implode(' ', $splittitle) . PHP_EOL;
						makemenu(array('r' => 'save new title and return to word-by-word editing',
							's' => 'save as is',
							'a' => 'quit this file without saving changes',
							'q' => 'quit this program',
							'b' => 'return to word-by-word editing without saving new title',
						), 'Enter new title. Other commands:');
						while(true) {
							$cmd2 = $this->getline();
							if(strlen($cmd2) > 1) $newtitle = $cmd2;
							else switch($cmd2) {
								case 'q': return false;
								case 's':
									$this->title = $newtitle;
									$this->log('Edited title');
								case 'a':
									$this->editedtitle = 1;
									$this->p->needsave();
									return true;
								case 'r':
									if(!$newtitle) break 2;
									$this->log('Edited title');
									$this->title = $newtitle;
									return $this->edittitle();
								case 'b': echo 'Quit full-title editing' . PHP_EOL; break 2;
							}
						}
					}
			}
			unset($nbeg, $nend, $n);
		}
	}
	public function set($paras) {
		if(self::process_paras($paras, array(
			'default' => array('cannotmove' => false),
		)));
		foreach($paras as $field => $content) {
			if(self::hasproperty($field)) {
				if($this->$field === $content) continue;
				switch($field) {
					case 'title':
						$this->editedtitle = 1;
						$this->title = $content;
						break;
					case 'name':
						if(!$paras['cannotmove'])
							$this->move($content);
						break;
					default:
						$this->$field = $content;
						break;
				}
				$this->p->needsave();
			}
		}
	}
	/* CITING */
	public function cite() {
		return $this->__invoke();
	}
	public function __invoke() {
	// cite according to specified style
		if($this->issupplement()) {
			if($name = $this->supp_getbasic())
				return $this->p->cite($name);
			else
				return false;
		}
		if($this->truename()) return $this->truename_return;
		if(!$this->p->citetype) $this->p->citetype = 'wp';
		$func = 'cite' . $this->p->citetype;
		return $this->$func();
	}
	private function cite_getclass() {
	// get the type of citation needed (journal, book, chapter, etc.)
		// redirect resolution magic?
		if($this->issupplement)
			return 'n/a';
		if($this->isweb())
			return 'web';
		if($this->journal)
			return 'journal';
		if($this->title && $this->booktitle)
			return 'chapter';
		if($this->isthesis())
			return 'thesis';
		if($this->title)
			return 'book';
		return 'unknown';
	}
	public function citepaper() {
	// like citenormal(), but without WP style links and things
		return $this->citenormal(false);
	}
	public function citenormal($mw = true) {
	// cites according to normal WP citation style
	// if $mw = false, no MediaWiki markup is used
		// this is going to be the citation
		if($mw)
			$out = '*';
		else
			$out = '';
		// replace last ; with ", and"; others with ","
		$out .= preg_replace(array("/;(?=.*;)/", "/;/", "/\\\\/"), array(",", " and", "\\"), $this->authors);
		$out .= " $this->year. ";
		if($mw) {
			if($this->url)
				$out .= "[" . $this->url . " ";
			else if($this->doi)
				$out .= '[http://dx.doi.org/' . $this->doi . ' ';
		}
		$out .= $this->title;
		// TODO: guess whether "subscription required" is needed based on URL
		if($mw and ($this->url or $this->doi))
			$out .= "] (subscription required)";
		$out .= ". ";
		// journals (most common case)
		if($this->journal) {
			$out .= $this->journal . " ";
			if($this->series)
				// need to catch "double series"
				$out .= "(" . preg_replace("/;/", ") (", $this->series) . ")";
			$out .= $this->volume;
			if($this->issue)
				$out .= "($this->issue)";
			$out .= ":";
			if($this->start == $this->end)
				$out .= $this->start;
			else
				$out .= $this->start . "–" . $this->end;
			$out .= ".";
		}
		else if($this->booktitle) {
			if($this->start == $this->end)
				$out .= "P. $this->start in ";
			else
				$out .= "Pp. " . $this->start . "–" . $this->end . " in ";
			$out .= preg_replace(array("/;(?=.*;)/", "/;/", "/\\\\/"), array(",", ", and", "\\"), $this->bookauthors) . " (eds.). ";
			$out .= $this->booktitle . ". ";
			$out .= $this->publisher;
			if($this->bookpages)
				$out .= ", " . $this->bookpages . " pp.";
			else
				$out .= ".";
		}
		if(!$mw && $this->doi) {
			$out .= ' doi:' . $this->doi;
		}
		// final cleanup
		$out = preg_replace(array("/\s\s/", "/\.\./"), array(" ", "."), $out);
		if($mw) $out = wikify($out);
		return $out;
	}
	public function citewp() {
	// cites according to {{cite journal}} etc.
		/*
		 * stuff related to {{cite doi}} and friends
		 */
		// determines whether only one citation is returned or two if {{cite doi}} or friends can be used
		$verbosecite = $this->p->verbosecite;
		if($this->doi)
			// to fix bug 28212
			$doi = str_replace(array('<', '>'), array('.3C', '.3E'), $this->doi);
		$out1 = '';
		if(!$this->parturl) {
			// {{cite doi}}
			if($this->doi)
				$out1 = "{{cite doi|" . $doi . "}}";
			// {{cite jstor}}
			else if($this->jstor)
				$out1 = "{{cite jstor|" . $this->jstor . "}}";
			// {{cite hdl}}
			else if($this->hdl)
				$out1 = "{{cite hdl|" . $this->hdl . "}}";
			if(!$verbosecite && $out1) return $out1;
		}
		switch($class = $this->cite_getclass()) {
			case 'journal': $temp = 'journal'; break;
			case 'book': case 'chapter': $temp = 'book'; break;
			case 'thesis': $temp = 'thesis'; break;
			case 'unknown': if($out1) return $out1; else return false;
			case 'web': $temp = 'web'; break;
			default: trigger_error('Unrecognized class in ' . __FUNCTION__, E_USER_NOTICE);
		}
		$paras = array();
		// authors
		$authors = explode("; ", $this->authors);
		foreach($authors as $key => $author) {
			if($key < 9) {
			// templates only support up to 9 authors
				$author = explode(", ", $author);
				$paras['last' . ($key + 1)] = $author[0];
				if(isset($author[1]))
					$paras['first' . ($key + 1)] = $author[1];
			}
			else {
				if(!isset($paras['coauthors']))
					$paras['coauthors'] = '';
				$paras['coauthors'] .= $author . '; ';
			}
		}
		if(isset($paras['coauthors']))
			$paras['coauthors'] = preg_replace('/; $/u', '', $paras['coauthors']);
		// easy stuff we need in all classes
		$paras['year'] = $this->year;
		if($this->hdl) $paras['id'] = '{{hdl|' . $this->hdl . '}}';
		$paras['jstor'] = $this->jstor;
		$paras['pmid'] = $this->pmid;
		$paras['url'] = $this->url;
		$paras['doi'] = isset($doi) ? $doi : '';
		$paras['pmc'] = $this->pmc;
		$paras['publisher'] = $this->publisher;
		$paras['location'] = $this->location;
		$paras['isbn'] = $this->isbn;
		if(($this->start === $this->end) or $this->end == NULL)
			$paras['pages'] = $this->start;
		else
			$paras['pages'] = $this->start . "–". $this->end;
		if($temp === 'journal') {
			$paras['title'] = $this->title;
			$paras['journal'] = $this->journal;
			$paras['volume'] = $this->volume;
			$paras['issue'] = $this->issue;
		}
		else if($temp === 'book') {
			if(!$this->booktitle)
				$paras['title'] = $this->title;
				if(!$paras['pages']) $paras['pages'] = $this->bookpages;
			else {
				$paras['chapter'] = $this->title;
				$paras['title'] = $this->booktitle;
			}
			$paras['edition'] = $this->edition;
			if($this->bookauthors) {
				$bauthors = explode("; ", preg_replace('/ \([Ee]ds?\.\)$/', '', $this->bookauthors));
				foreach($bauthors as $key => $author) {
				// only four editors supported
					if($key < 4) {
						$author = explode(', ', $author);
						$paras['editor' . ($key + 1) . '-last'] = $author[0];
						$paras['editor' . ($key + 1) . '-first'] = $author[1];
					}
					else {
					// because cite book only supports four editors, we have to hack by putting the remaining editors in |editor4-last=
						if($key === 4) {
							unset($paras['editor4-first']);
							$paras['editor4-last'] = $bauthors[3] . '; ';
						}
						$paras['editor4-last'] .= $author . '; ';
					}
				}
				// double period bug
				if(isset($paras['editor4-last']) and strpos($paras['editor4-last'], ';') !== false)
					$paras['editor4-last'] = preg_replace(array('/; $/u', '/\.$/u'), array('', ''), $paras['editor4-last']);
				else
					$paras['editor' . count($bauthors) . '-first'] = preg_replace('/\.$/u', '', $paras['editor' . ($key + 1) . '-first']);
			}
		}
		else if($temp === 'thesis') {
			$paras['title'] = $this->title;
			$tmp = explode(' thesis, ', $this->publisher);
			$paras['degree'] = $tmp[0];
			$paras['publisher'] = $tmp[1];
			$paras['pages'] = $this->bookpages;
		}
		else if($temp === 'web') {
			$paras['title'] = $this->title;
			$paras['publisher'] = $this->publisher;
		}
		if($this->p->includerefharv)
			$paras['ref'] = 'harv';
		$out = $sfn = '';
		if($this->p->includesfn)
			$out = $sfn = '<!--' . $this->getsfn() . '-->';
		$out .= '{{cite ' . $temp;
		foreach($paras as $key => $value) {
			if($value) $out .= ' | ' . $key . ' = ' . $value;
		}
		$out .= '}}';
		// final cleanup
		$out = preg_replace(array("/\s\s/", "/\.\./"), array(" ", "."), wikify($out));
		return ($verbosecite and $out1) ? array($sfn . $out1, $out) : $out;
	}
	public function getsfn() {
		$sfn = '{{Sfn|';
		$auts = explode('; ', $this->authors);
		for($i = 0; $i < 4; $i++) {
			if(isset($auts[$i])) {
				$sp = explode(',', $auts[$i]);
				$sfn .= $sp[0] . '|';
			}
		}
		$sfn .= $this->year . '}}';
		return $sfn;
	}
	public function citelemurnews() {
		if($this->journal)
			$out = $this->authors . '. ' . $this->year . '. ' . $this->title . '. ' . $this->journal . ' ' . $this->volume . ': ' . $this->start . '–' . $this->end . '.';
		// TODO: support non-journal citations
/*
Ranaivoarisoa, J.F.; Ramanamahefa, R.; Louis, Jr., E.E.; Brenneman, R.A. 2006. Range extension of Perrier’s sifaka, <i>Propithecus perrieri</i>, in the Andrafiamena Classified Forest. Lemur News 11: 17-21.

Book chapter
Ganzhorn, J.U. 1994. Les lémuriens. Pp. 70-72. In: S.M. Goodman; O. Langrand (eds.). Inventaire biologique; Forêt de Zombitse. Recherches pour le Développement, Série Sciences Biologiques, n° Spécial. Centre d’Information et de Documentation Scientifique et Technique, Antananarivo, Madagascar.

Book
Mittermeier, R.A.; Konstant, W.R.; Hawkins, A.F.; Louis, E.E.; Langrand, O.; Ratsimbazafy, H.J.; Rasoloarison, M.R.; Ganzhorn, J.U.; Rajaobelina, S.; Tattersall, I.; Meyers, D.M. 2006. Lemurs of Madagascar. Second edition. Conservation International, Washington, DC, USA.

Thesis
Freed, B.Z. 1996. Co-occurrence among crowned lemurs (<i>Lemur coronatus</i>) and Sanford’s lemur (<i>Lemur fulvus sanfordi</i>) of Madagascar. Ph.D. thesis, Washington University, St. Louis, USA.

Website
IUCN. 2008. IUCN Red List of Threatened Species. <www.iucnredlist.org>. Downloaded on 21 April 2009.
*/
		// final cleanup
		$out = preg_replace(array("/\s\s/", "/\.\./"), array(" ", "."), $out);
		return $out;
	}
	public function citebzn() {
	// cites according to BZN citation style
		$class = $this->cite_getclass();
		// replace last ; with " &"; others with ","
		$out .= '<b>' . preg_replace(array("/;(?=.*;)/", "/;/", "/\\\\/"), array(",", " &", "\\"), $this->authors);
		$out .= "</b> $this->year. ";
		// journals (most common case)
		if($class === 'journal') {
			$out .= $this->title;
			$out .= ". ";
			$out .= '<i>' . $this->journal . '</i>, ';
			if($this->series)
				// need to catch "double series"
				$out .= "(" . str_replace(";", ") (", $this->series) . ")";
			$out .= '<b>' . $this->volume . '</b>: ';
			if($this->start == $this->end)
				$out .= $this->start;
			else
				$out .= $this->start . "–" . $this->end;
			$out .= '.';
		}
		else if($class === 'chapter') {
			$out .= $this->title . '. <i>in</i> ';
			$out .= str_replace("(Ed", '(ed', $this->bookauthors);
			$out .= ', <i>' . $this->booktitle . '</i>. ' . $this->bookpages . ' pp.';
			$out .= ' ' . $this->publisher;
			if($this->location) $out .= ', ' . $this->location;
			$out .= '.';
		}
		else if($class === 'book') {
			$title = $this->title ? $this->title : $this->booktitle;
			$out .= '<i>' . $title . '.</i>';
			if($this->bookpages)
				$out .= ' ' . $this->bookpages . ' pp.';
			if(preg_match('/: /', $this->publisher)) {
				$tmp = preg_split('/: /', $this->publisher);
				$out .= ' ' . $tmp[1] . ', ' . $tmp[0] . '.';
			}
			else {
				$out .= ' ' . $this->publisher;
				if($this->location) $out .= ', ' . $this->location;
			}
			$out .= '.';
		}
		// final cleanup
		$out = str_replace(array("  ", ".."), array(" ", "."), $out);
		return $out;
	}
	public function citepalaeontology() {
		$class = $this->cite_getclass();
		// this is going to be the citation
		$out = '';
		$processauthors = function($in) {
			// author names in uppercase
			$pa_out = mb_strtoupper($in);
			// replace last ; with ", and"; others with ","; space initials
			$pa_out = preg_replace(
				array("/(?<=\.)(?!;)/u", "/;(?=.*;)/u", "/;/u", "/\\\\/u"),
				array(" ", ",", " and", "\\"),
				$pa_out);
			return $pa_out;
		};
		$out .= $processauthors($this->authors);
		$out .= " $this->year. ";
		switch($class) {
			case 'journal':
				$out .= "$this->title. ";
				$out .= "<i>$this->journal</i>, ";
				// TODO: series
				$out .= "<b>$this->volume</b>, ";
				if($this->start == $this->end)
					$out .= $this->start;
				else
					$out .= $this->start . "–" . $this->end;
				break;
			case 'book':
				$out .= "<i>$this->title.</i> ";
				$out .= "$this->publisher, ";
				if($this->location) $out .= "$this->location, ";
				$out .= "$this->bookpages pp.";
				break;
			case 'chapter':
				$out .= "$this->title. <i>In</i> ";
				$out .= $processauthors($this->bookauthors);
				$out .= " (eds). ";
				$out .= "<i>$this->booktitle.</i> ";
				$out .= "$this->publisher, ";
				if($this->location) $out .= "$this->location, ";
				$out .= "$this->bookpages pp.";
				break;
			default:
				$out .= "$this->title. ";
				$out .= "<!--Unknown citation type; fallback citation-->";
				break;
		}
		// final cleanup
		$out .= ".";
		$out = preg_replace(array("/\s\s/u", "/\.\./u"), array(" ", "."), $out);
		return $out;
	}
	public function citejpal() {
		$class = $this->cite_getclass();
		// this is going to be the citation
		$out = '';
		$processauthors = function($in, $type = 'normal') {
			// author names in uppercase
			switch($type) {
				case 'normal': $and = ' AND '; break;
				case 'editors': $and = ' and '; break;
				default: 'Unrecognized type: ' . $type . PHP_EOL; return false;
			}
			if($type === 'normal') $in = mb_strtoupper($in);
			// space initials
			$in = preg_replace("/(?<=\.)(?![-;])/u", ' ', $in);
			$in = explode('; ', $in);
			$n = count($in);
			$pa_out = '';
			foreach($in as $key => $aut) {
				// put initials before last name
				$paut = preg_replace('/^(.*?), (.*)$/u', '$2 $1', $aut);
				// if type == normal, first author should not be paut
				if($key === 0 and $type === 'normal') {
					$pa_out .= $aut;
					continue;
				}
				// put "and" for last author
				if($key === $n - 1 and $n !== 1) {
					if($n > 2) $pa_out .= ',';
					$pa_out .= $and . $paut;
				}
				// normal
				else {
					if($key > 0) $pa_out .= ', ';
					$pa_out .= $paut;
				}
			}
			return $pa_out;
		};
		$out .= $processauthors($this->authors);
		$out .= " $this->year. $this->title";
		switch($class) {
			case 'journal':
				$out .= ". $this->journal, ";
				if($this->series) $out .= "ser. $this->series, ";
				$out .= "$this->volume:";
				if($this->start === $this->end)
					$out .= $this->start;
				else
					$out .= $this->start . '–' . $this->end;
				break;
			case 'chapter':
				$out .= ', ' . $this->start . '–' . $this->end . '. <i>In</i> ';
				$out .= $processauthors($this->bookauthors, 'editors');
				if(strpos($this->bookauthors, ';') !== false)
					$out .= " (eds.), ";
				else
					$out .= " (ed.), ";
				$out .= $this->booktitle;
			case 'book':
				$out .= ". $this->publisher";
				if($this->location) $out .= ", $this->location";
				if($this->bookpages) $out .= ", $this->bookpages p.";
				break;
			default:
				$out .= "$this->title. ";
				$out .= "<!--Unknown citation type; fallback citation-->";
				break;
		}
		// final cleanup
		$out .= ".";
		$out = preg_replace(array("/\s\s/u", "/\.(?=,)/u", "/\.\./u"), array(" ", "", "."), $out);
		return $out;
	}
	public function citebibtex() {
		// lambda function to add a property to the output
		$add = function($key, $value, $mandatory = false) use(&$out) {
			if(is_null($value)) {
				if($mandatory)
					echo 'Bibtex error: required property ' . $key . ' is empty' . PHP_EOL;
				return;
			}
			$out .= "\t" . $key . ' = "' . $value . "\",\n";
		};
		$out = '@';
		switch($class = $this->cite_getclass()) {
			case 'journal':
				$out .= 'article';
				break;
			case 'book':
				$out .= 'book';
				break;
			case 'chapter':
				$out .= 'incollection';
				break;
			case 'thesis':
				switch(substr($this->publisher, 0, 3)) {
					case 'PhD': $out .= 'phdthesis'; break;
					case 'MSc': $out .= 'mscthesis'; break;
					case 'BSc': $out .= 'misc'; break;
				}
				break;
			default:
				$out .= 'misc';
				break;
		}
		$out .= '{' . $this->getrefname() . ",\n";
		$authors = preg_replace(
			array("/\./u", "/;/u", "/\s+/u", "/\s\$/u"),
			array(". ", " and", " ", ""),
			$this->authors
		);
		// stuff that goes in every citation type
		$add('author', $authors, true);
		$add('year', $this->year, true);
		$title = str_replace(
			array('<i>', '</i>'),
			array('\textit{', '}'),
			$this->title
		);
		$add('title', $title, true);
		switch($class) {
			case 'thesis':
				$add('school', substr($this->publisher, 12), true);
				break;
			case 'journal':
				$add('journal', $this->journal, true);
				$add('volume', $this->volume);
				$add('number', $this->issue);
				$add('pages', $this->start . '--' . $this->end);
				break;
			case 'book':
				$add('publisher', $this->publisher, true);
				$add('address', $this->location);
				break;
		}
		$out .= '}';
		return $out;
	}
	public function echocite($paras = array()) {
		if(self::process_paras($paras, array(
			'checklist' => array(
				'mode',
			),
			'default' => array(
				'mode' => false,
			),
		)) === PROCESS_PARAS_ERROR_FOUND) return false;
		$this->p->verbosecite = true;
		echo PHP_EOL;
		if($paras['mode'])
			$func = 'cite' . $paras['mode'];
		else
			$func = '__invoke';
		if(!method_exists($this, $func)) {
			echo __METHOD__ . ': invalid method' . PHP_EOL;
			return false;
		}
		if(is_array($cite = $this->$func()))
			foreach($cite as $cit)
				echo $cit . PHP_EOL;
		else
			echo $cite . PHP_EOL;
		return true;
	}
	public function __toString() {
		$out = $this();
		if(is_array($out))
			return implode(PHP_EOL, $out);
		else
			return $out;
	}
	public function get_citedoiurl($var = 'doi') {
	// returns URL to Wikipedia cite doi-family template for this FullFile
	// by default uses cite doi; by setting var to something else, jstor and a few others can be set
		$url = 'http://secure.wikimedia.org/wikipedia/en/w/index.php?action=edit&title=Template:Cite_';
		switch($var) {
			case 'doi':
				// TODO: need some encoding here
				if($this->doi)
					$url .= 'doi/' . $this->doi;
				else
					return false;
				break;
			case 'jst': case 'jstor': // 'jst' due to limitations in Parser
				if($this->jstor)
					$url .= 'jstor/' . $this->jstor;
				else
					return false;
				break;
			case 'hdl':
				if($this->hdl)
					$url .= 'hdl/' . $this->hdl;
				else
					return false;
				break;
			default:
				return false;
		}
		return $url;
	}
	public function getharvard($paras = array()) {
		// TODO: implement getting both Zijlstra et al. (2010) and (Zijlstra et al., 2010)
		// TODO: more citetype dependence
		$splitauthors = explode('; ', $this->authors);
		$lastname = function ($in) {
			return substr($in, 0, strpos($in, ','));
		};
		$naut = count($splitauthors);
		$out = '';
		switch($naut) {
			case 0:
				return ''; //incomplete info
			case 1:
				$out .= $lastname($splitauthors[0]);
				break;
			case 2:
				$out .= $lastname($splitauthors[0]) . ' and ' . $lastname($splitauthors[1]);
				break;
			default:
				$out .= $lastname($splitauthors[0]);
				switch($citetype) {
					case 'jpal':
						$out .= ' <i>et al.</i>';
						break;
					case 'paper': case 'normal': default:
						$out .= ' et al.';
						break;
				}
				break;
		}
		$out .= ' (' . $this->year . ')';
		return $out;
	}
	/* ADDING DATA */
	public function newadd() {
	// add from CsvList::newcheck(). This function gets the path and moves the file into the library.
		makemenu(array('o' => 'open this file',
				'q' => 'quit',
				's' => 'skip this file',
				'n' => 'move this file to "Not to be cataloged"',
				'r' => 'rename this file',
				'<enter>' => 'add this file to the catalog',
			), "Adding file $this->name");
		while(true) switch($this->getline()) {
			case 'o': $this->openf(array('place' => 'temp')); break;
			case 'q': return 0;
			case 's': return 1;
			case 'r':
				$oldname = $this->name;
				$newname = $this->getline('New name: ');
				if($newname === 'q')
					break;
				// allow renaming to existing name, for example to replace in-press files, but warn
				if($this->p->has($newname))
					echo 'Warning: file already exists' . PHP_EOL;
				$this->name = $newname;
				$cmd = 'mv ' . TEMPPATH . '/' . escape_shell($oldname) . ' ' . TEMPPATH . '/' . escape_shell($newname);
				if(!exec_catch($cmd)) echo "Error moving file" . PHP_EOL;
				break;
			case 'n':
				$cmd = 'mv ' . TEMPPATH . '/' . escape_shell($this->name) . ' ' . TEMPPATH . '/Not\ to\ be\ cataloged/' . escape_shell($this->name);
				if(!exec_catch($cmd))
					echo "Error moving file" . PHP_EOL;
				return 1;
			case '': break 2;
		}
		/*
		 * get data
		 */
		$oldname = $this->name;
		// loop until the value of $this->name doesn't exist yet
		while(isset($this->p->lslist[$this->name])) {
			makemenu(array('r' => 'move over the existing file',
					'd' => 'delete the new file',
					'o' => 'open the new and existing files',
				), 'A file with this name already exists. Please enter a new filename');
			while(true) {
				switch($newname = $this->getline()) {
					case 'o':
						$this->openf(array('place' => 'temp'));
						$this->p->lslist[$this->name]->openf();
						break;
					case 'r':
						$cmd = 'mv ' . TEMPPATH . '/' .
							escape_shell($this->name) . ' ' .
							$this->p->lslist[$this->name]->path();
						if(!exec_catch($cmd)) echo "Error moving file" . PHP_EOL;
						$this->p->edit($this->name);
						return 1;
					case 'd':
						exec_catch('rm ' . TEMPPATH . '/' . escape_shell($this->name));
						return 1;
					case 'q': return 0;
					default:
						$this->name = $newname;
						break 2;
				}
			}
		}
		// loop in case of incorrect folders
		while(true) {
			if(!$this->p->sugglist) $this->p->build_sugglist();
			$key = $this->getkey();
			if(isset($this->p->sugglist[$key])) {
				$suggs = $this->p->sugglist[$key]->getsugg();
				foreach($suggs as $sugg) {
					echo 'Suggested placement. Folder: ' . $sugg[0];
					if($sugg[1]) {
						echo '; subfolder: ' . $sugg[1];
						if($sugg[2]) echo '; sub-subfolder: ' . $sugg[2];
					}
					$cmd = $this->menu(array(
						'head' => PHP_EOL,
						'options' => array(
							'y' => 'if this suggestion is correct',
							'n' => 'this suggestion is not correct',
							's' => 'stop suggestions',
							'q' => 'quit this file',
						),
					));
					switch($cmd) {
						case 'y':
							$this->folder = $sugg[0];
							$this->sfolder = $sugg[1];
							$this->ssfolder = $sugg[2];
							break 2;
						case 'n': break;
						case 's': break 2;
						case 'q': return 1;
					}
				}
			}
			if(!$this->folder) {
				if(!$this->p->foldertree) $this->p->build_foldertree();
				/* folder */
				echo 'Suggestions: ';
				$suggs = $this->sugg_helper($this->p->foldertree);
				$cmd = $this->getline('Folder: ');
				if($cmd === 'q') continue 2;
				if(is_numeric($cmd)) $this->folder = $suggs[$cmd];
				else $this->folder = $cmd;
				/* subfolder */
				if(count($this->p->foldertree[$this->folder]) !== 0) {
					echo 'Suggestions: ';
					$suggs = $this->sugg_helper($this->p->foldertree[$this->folder]);
					$cmd = $this->getline('Subfolder: ');
					if($cmd === 'q') continue 2;
					if(is_numeric($cmd)) $this->sfolder = $suggs[$cmd];
					else $this->sfolder = $cmd;
				/* sub-subfolder */
					if(isset($this->p->foldertree
							[$this->folder]
							[$this->sfolder]) 
						and count($this->p->foldertree
							[$this->folder]
							[$this->sfolder]) !== 0) {
						echo 'Suggestions: ';
						$suggs = $this->sugg_helper($this->p->foldertree[$this->folder][$this->sfolder]);
						$cmd = $this->getline(array(
							'prompt' => 'Sub-subfolder: ',
						));
						if($cmd === 'q') continue 2;
						if(is_numeric($cmd)) $this->ssfolder = $suggs[$cmd];
						else $this->ssfolder = $cmd;
					}
				}
			}

			$command = "mv -n " . TEMPPATH . "/" . escape_shell($oldname) . " " . $this->path();
			// move file
			if(!exec_catch($command))
				echo "Error moving file $this->name into library. Type 'q' for \"Folder\" to quit this file." . PHP_EOL;
			else
				break;
		}
		return $this->add() ? 2 : 1;
	}
	public function add(array $paras = array()) {
	// fill in data
		if(self::process_paras($paras, array(
			'checklist' => array(
				'noedittitle'
			),
			'default' => array(
				'noedittitle' => false,
			),
		)) === PROCESS_PARAS_ERROR_FOUND) return false;
		/*
		 * start adding data
		 */
		// add time added
		$time = getdate();
		$this->addmonth = $time["mon"];
		$this->addday = $time["mday"];
		$this->addyear = $time["year"];
		if($this->isredirect()) return true;
		if($this->isfile()) {
			switch(substr($this->name, -4)) {
				case '.PDF': echo "Warning: uppercase file extension." . PHP_EOL;
				case '.pdf':
					$this->putpdfcontent();
					/*
					 * Try various options
					 */
					$successful = ($this->tryjstor() or
						$this->trybioone() or
						$this->trydoi() or
						$this->trygeodiv() or
						$this->trype() or
						$this->trygoogle() or
						$this->doiamnhinput() or
						$this->trymanual());
					break;
				default:
					// can't do tryjstor() etcetera when we're not in a PDF
					$successful = ($this->doiamnhinput() or
						$this->trymanual());
					break;
			}
		}
		else
			$successful = ($this->doiamnhinput() or $this->trymanual());
		if(!$this->editedtitle and !$paras['noedittitle']) $this->edittitle();
		return $successful;
	}
	public function adddata() {
		if($this->isor('redirect', 'supplement') or $this->triedadddata)
			return true;
		if(!$this->needsdata()) {
			if(!$this->triedfinddoi and !$this->doi)
				$this->finddoi();
			return true;
		}
		// apply global settings
		$this->p->addmanual = false;
		$this->adddata_specifics();
		$this->triedadddata = true;
		// if there already is a DOI, little chance that we found something
		if($this->doi) {
			if(!$this->year or !$this->volume or !$this->start) {
				$this->expanddoi(array('verbose' => true));
				$this->p->needsave();
			}
			return true;
		}
		else if($this->hasid()) {
			if(!$this->triedfinddoi)
				$this->finddoi();
			return true;
		}
		echo "Adding data for file $this->name... ";
		$tmp = new Fullfile(array($this->name, $this->folder, $this->sfolder, $this->ssfolder), 'l');
		if(!$tmp->add(array('noedittitle' => true)))
			echo "nothing found" . PHP_EOL;
		else {
			foreach($tmp as $key => $value) {
				if($value && !$this->$key)
					$this->$key = $value;
			}
			$this->log('Added data');
			$this->p->needsave();
			echo "data added" . PHP_EOL;
		}
		if($this->hasid())
			return true;
		if(!$this->triedfinddoi)
			$this->finddoi();
		if(!$this->triedfindurl)
			$this->findurl();
		return $this->adddata_return ? false : true;
	}
	private function findurl() {
		echo 'Trying to find a URL for file ' . $this->name . '... ';
		$json = self::fetchgoogle($this->googletitle());
		if($json === false) {
			$this->adddata_return = true;
			return false;
		}
		// Google behaved correctly, but found nothing
		if($json === NULL) {
			echo 'nothing found' . PHP_EOL;
			return true;
		}
		$first = true;
		foreach($json->items as $result) {
			if($first) {
				echo PHP_EOL . $this->title . PHP_EOL;
				$this->echocite(array('mode' => 'paper'));
				echo PHP_EOL;
				makemenu(array(
				));
				$first = false;
			}
			echo "Title: " . $result->title . PHP_EOL;
			echo "URL: " . $result->link . PHP_EOL;
			while(true) {
				$cmd = $this->menu(array(
					'options' => array(
						'y' => 'this URL is correct',
						'n' => 'this URL is not correct',
						'q' => 'quit the function',
						'o' => 'open the URL',
						'r' => 'stop adding data',
						'u' => 'add a different URL',
						'i' => 'give information about this file',
						'e' => 'edit this file',
					),
					'head' => '',
				));
				switch($cmd) {
					case 'y':
						$this->p->needsave();
						$this->url = $result->link;
						echo 'data added' . PHP_EOL;
						return true;
					case 'q':
						echo 'nothing found' . PHP_EOL;
						$this->triedfindurl = true;
						return false;
					case 'o':
						exec_catch("open '" . str_replace("'", "\'", $result->link) . "'");
						break;
					case 'i':
						$this->my_inform();
						break;
					case 'n':
						break 2;
					case 'u':
						$this->url = $this->getline('New url: ');
						$this->p->needsave();
						echo 'data added' . PHP_EOL;
						return true;
					case 'r':
						$this->adddata_return = true;
						echo 'nothing found' . PHP_EOL;
						return false;
					case 'e':
						$this->edit();
						break;
				}
			}
		}
		$this->triedfindurl = true;
	}
	private function finddoi() {
		if($this->doi) return true;
		if($this->journal and $this->volume and $this->start) {
			echo "Trying to find DOI for file $this->name... ";
			$url = "http://www.crossref.org/openurl?pid=" . CROSSREFID . "&title=" . urlencode($this->journal) . "&volume=" . urlencode($this->volume) . "&spage=" . urlencode($this->start) . "&noredirect=true";
			$xml = @simplexml_load_file($url);
			// check success
			if($xml and $doi = $xml->query_result->body->query->doi) {
				$this->doi = $doi;
				echo "data added" . PHP_EOL;
				$this->p->needsave();
				return true;
			}
			else {
				echo "nothing found" . PHP_EOL;
				$this->triedfinddoi = true;
				return false;
			}
		}
		return false;
	}
	public function findhdl() {
		if(!$this->isamnh()) return true;
		if($this->hdl) return true;
		echo 'Finding HDL for file ' . $this->name .
			' (' . $this->journal . ' ' . $this->volume . ')' . PHP_EOL;
		switch($this->journal) {
			case 'American Museum Novitates':
				$journalhdl = '2246/9';
				break;
			case 'Bulletin of the American Museum of Natural History':
				$journalhdl = '2246/7';
				break;
			case 'Anthropological Papers of the American Museum of Natural History':
				$journalhdl = '2246/6';
				break;
		}
		// construct search URL
		$url = 'http://digitallibrary.amnh.org/dspace/handle/' .
			$journalhdl .
			'/simple-search?query=series:' .
			$this->volume;
		$html = @file_get_contents($url);
		if(!$html) {
			echo 'Could not retrieve data (file ' . $this->name . ')' . PHP_EOL;
			return false;
		}
		else
			echo 'Retrieved data from AMNH' . PHP_EOL;
		// check whether we got one or several results
		if(strpos($html, 'Search produced no results.') !== false) {
			echo 'Could not find paper at AMNH' . PHP_EOL;
			return true;
		}
		if(strpos($html, 'Results 1-1 of 1.') !== false) {
			preg_match('/<td class="evenRowEvenCol"><A HREF="\/dspace\/handle\/2246\/(\d+)">/', $html, $matches);
			$this->hdl = '2246/' . $matches[1];
		}
		else {
			exec_catch('open \'' . $url . '\'');
			$hdl = $this->getline('HDL: ');
			if($hdl === 'q') return false;
			if(strpos($hdl, 'http') !== false) {
				preg_match('/2246\/\d+$/', $hdl, $matches);
				$hdl = $matches[0];
			}
			$this->hdl = $hdl;
		}
		echo 'Added HDL: ' . $this->hdl . PHP_EOL;
		$this->p->needsave();
		return true;
	}
	private function adddata_specifics() {
	// kitchen sink function for various stuff in adddata() territory, in order to keep addata() clean and to avoid lots of limited-use functions
		if(($this->journal === 'Estudios Geológicos' or preg_match('/springerlink|linkinghub\.elsevier|biomedcentral|ingentaconnect/', $this->url)) and !$this->doi) {
			echo 'File ' . $this->name . ' has no DOI, but its data suggest it should have one.' . PHP_EOL;
			if(!$this->openurl())
				$this->searchgoogletitle();
			$this->echocite();
			$doi = $this->getline('DOI: ');
			if($doi === 'q') return false;
			else if($doi) $this->doi = $doi;
		}
	}
	private function putpdfcontent() {
		// only do actual PDF files
		if(!$this->ispdf() or $this->isredirect())
			return false;
		// only get first page
		$shcommand = PDFTOTEXT . " " . $this->path() . " - -l 1 2>> " . __DIR__ . "/data/pdftotextlog";
		$this->pdfcontent = trim(utf8_encode(shell_exec($shcommand)));
		return $this->pdfcontent ? true : false;
	}
	public function getpdfcontent($paras = array()) {
		if(!$this->ispdf() or $this->isredirect()) return false;
		$this->process_paras($paras, array(
			'checklist' => array('force'),
			'default' => array('force' => false),
		));
		if(!$paras['force']) {
			$this->p->getpdfcontentcache();
			if(isset($this->p->pdfcontentcache[$this->name])) {
				$this->pdfcontent =& $this->p->pdfcontentcache[$this->name];
				return $this->pdfcontent;
			}
			if(!$this->pdfcontent) $this->putpdfcontent();
		}
		else {
			$this->putpdfcontent();
		}
		$this->p->pdfcontentcache[$this->name] =& $this->pdfcontent;
		if(!$this->pdfcontent) return false;
		return $this->pdfcontent;
	}
	public function echopdfcontent() {
		$c = $this->getpdfcontent();
		if($c === false)
			return false;
		echo $c;
		return true;
	}
	public function findtitle_pdfcontent() {
	// tries to detect the title from $this->pdfcontent
	// assumes it is the first line with real running text
		$lines = explode("\n", $this->getpdfcontent());
		foreach($lines as $line) {
			// empty line
			if(!$line) continue;
			$line = trim($line);
			// titles are generally more than two words
			if(preg_match_all('/ /', $line, $matches) <= 2) continue;
			// probably not a title
			if(preg_match('/[©@·]/u', $line)) continue;
			// this looks like a volume-issue-pages kind of thing, or a date. This regex is matching a substantial number of existing articles; check how much of it is needed.
			if(preg_match('/\(\s*\d+\s*\)\s*:|\d{5}|\d\s*,\s*\d|\d+\s*\(\d+\)\s*\d+|\d+\s*\(\s*\d+\s*\),|\d+\s*\.\s*\d+\s*\.\s*\d+|Volume \d+|\s*(January|February|April|June|July|August|September|October|November|December)\s*\d+|\/\d|\d:\s*\d|\d\(\d+\)|\d\s*\(\d|\d\s*\)\s*:\s*\d|;\s*\d|\s+\/\s+|\d\s+\d|doi:|Vol\.?\s*\d+\s*No\.?\s*\d+|Pages?\s*\d|Vol\.\s*\d+,|\*{2}|\d\s+&|,\s*pp\.\s*\d|\d\)\s*\d/ui', $line)) continue;
			// no URLs in title
			if((strpos($line, 'http://') !== false) or (strpos($line, 'www.') !== false)) continue;
			// if there is no four-letter word in it, it's probably not a title
			if(!preg_match('/[a-zA-Z]{4}/', $line)) continue;
			// title will contain at least one uppercase letter
			if(!preg_match('/[A-Z]/', $line)) continue;
			// looks like an author listing (but this matches 24 real articles)
			if(preg_match('/Jr\.|\s([a-z]\s){2,}/u', $line)) continue;
			// [A-Z]\.\s*[A-Z]\.|
			// JSTOR, ScienceDirect stuff, probable author line, publisher line, other stuff
			if(preg_match('/collaborating with JSTOR|ScienceDirect|^By|^Issued|^Geobios|^Palaeogeography|^Published|^Printed|^Received|^Mitt\.|,$|^Journal compilation|^E \d+|^Zeitschrift|^J Mol|^Open access|^YMPEV|x{3}|^Reproduced|^BioOne|^Alcheringa|^MOLECULAR PHYLOGENETICS AND EVOLUTION|Ann\. Naturhist(or)?\. Mus\. Wien|Letter to the Editor|Proc\. Natl\. Acad\. Sci\. USA|American Society of Mammalogists|CONTRIBUTIONS FROM THE MUSEUM OF PALEONTOLOGY|American College of Veterinary Pathologists|Stuttgarter Beiträge zur Naturkunde|^The Newsletter|^Short notes|^No\. of pages|Verlag|^This copy|Southwestern Association of Naturalists|^Peabody Museum|^(c) |^Number|^Occasional Papers|^Article in press|^Museum|^The university|^University|^Journal|^Key words|^International journal|^Terms? of use|^Bulletin|^A journal|^The Bulletin|^Academy|en prensa|^American Journal|^Contributions from|Museum of Natural History$|^The American|^Notes on geographic distribution$|Publications$|Sistema de Información Científica|Press$|^Downloaded|^Serie|issn|^Society|University of|Elsevier|^Australian Journal/ui', $line)) continue;
			// if it starts with a year, it's unlikely to be a title
			if(preg_match('/^\d{3}/u', $line)) continue;
			// if we got to such a long line, we're probably already in the abstract and we're not going to find the title. Longest title in database as of November 24, 2011 is 292
			if(strlen($line) > 300) return false;
			// how is this?
			else return $line;
		}
		return false;
	}
	private function findtitle_specific() {
	// find title in specific journals
		if(
			$mammalia = preg_match("/(Mammalia|MAMMALIA), (t|I)\. /", $this->pdfcontent) or
			// this is also likely Mammalia ("par" on 2nd line)
			$mammalia = preg_match("/^[^\n]*\n+(par|PAR)\n+/", $this->pdfcontent) or
			// this too
			$mammalia = preg_match("/^[^\n]*\n+(by|BY)\n+/", $this->pdfcontent) or
			// and this
			$mammalia = preg_match("/MAMMALIA · /", $this->pdfcontent) or
			$jvp = preg_match("/^\s*Journal of Vertebrate Paleontology/", $this->pdfcontent) or
			$jparas = preg_match("/^\s*J. Parasitol.,/", $this->pdfcontent) or
			$bioljlinnsoc = preg_match("/^\s*Biological Journal of the Linnean Society, /", $this->pdfcontent) or
			$bioljlinnsoc2 = preg_match("/^[^\n]+\n\nBiological Journal of the Linnean Society, /", $this->pdfcontent) or
			$zooljlinnsoc = preg_match("/^\s*Zoological Journal of the Linnean Society, /", $this->pdfcontent) or
			$swnat = preg_match("/^\s*THE SOUTHWESTERN NATURALIST/", $this->pdfcontent) or
			$wnanat = preg_match("/^\s*Western North American Naturalist/", $this->pdfcontent) or
			$mammreview = preg_match("/^\s*Mammal Rev. /", $this->pdfcontent) or
			$mammstudy = preg_match("/^\s*Mammal Study /", $this->pdfcontent) or
			$jpaleont = preg_match("/^\s*(Journal of Paleontology|J. Paleont.)/", $this->pdfcontent) or
			$jbiogeogr = preg_match("/^s*(Journal of Biogeography)/", $this->pdfcontent) or
			$amjprim = preg_match("/^\s*American Journal of Primatology/", $this->pdfcontent) or
			$ajpa = preg_match("/^\s*AMERICAN JOURNAL OF PHYSICAL ANTHROPOLOGY/", $this->pdfcontent) or
			$oryx = preg_match("/^\s*Oryx /", $this->pdfcontent) or
			$jmamm = preg_match("/^\s*Journal of Mammalogy,/", $this->pdfcontent)
			) {
			echo "Found BioOne/Mammalia/Wiley paper; searching Google to find a DOI." . PHP_EOL;
			/*
			 * find title
			 */
			// title ought to be first line
			if($mammalia) $title = preg_replace("/^([^\n]+).*/su", "$1", $this->pdfcontent);
			else if($jvp) $title = preg_replace("/^[^\n]+\n+((ARTICLE|SHORT COMMUNICATION|RAPID COMMUNICATION|NOTE|FEATURED ARTICLE|CORRECTION|REVIEW)\n+)?([^\n]+)\n.*/su", "$3", $this->pdfcontent);
			else if($jparas) $title = preg_replace("/.*American Society of Parasitologists \d+\s*([^\n]+)\n.*/su", "$1", $this->pdfcontent);
			else if($swnat) $title = preg_replace("/^[^\n]+\n+[^\n]+\n+([^\n]+).*/su", "$1", $this->pdfcontent);
			// title is after a line of text plus two newlines; works often
			else if($wnanat || $mammreview || $jpaleont || $jbiogeogr || $ajpa || $oryx || $jmamm || $bioljlinnsoc || $mammstudy || $zooljlinnsoc) $title = preg_replace("/^[^\n]+\n+((ORIGINAL ARTICLE|SHORT COMMUNICATION|Short communication|Blackwell Science, Ltd|CORRECTION|REVIEW)\n+)?([^\n]+).*/su", "$3", $this->pdfcontent);
			// this needs some other possibilities
			else if($amjprim) $title = preg_replace("/^[^\n]+\n\n((BRIEF REPORT|RESEARCH ARTICLES)\s)?([^\n]+).*/su", "$3", $this->pdfcontent);
			else if($bioljlinnsoc2) $title = preg_replace("/^[^\n]+\n\n[^\n]+\n\n([^\n]+).*/su", "$1", $this->pdfcontent);
			if(!$title || preg_match("/\n/", trim($title))) {
				echo "Error: could not find title." . PHP_EOL;
				return false;
			}
			if($jvp || $jparas || $wnanat || $swnat || $mammstudy || $jpaleont || $jmamm)
				$title .= " site:bioone.org";
			else if($mammalia)
				$title .= " site:reference-global.com";
			else if($bioljlinnsoc || $bioljlinnsoc2 || $mammreview || $jbiogeogr || $ajpa || $zooljlinnsoc)
				$title .= " site:wiley.com";
			return $title;
		}
		return false;
	}
	public function test_pdfcontent() {
	// test whether the PDFcontent functions are producing correct results
	// also useful for detecting damaged PDF files
		echo 'File ' . $this->name . PHP_EOL;
		if($this->putpdfcontent())
			echo $this->findtitle_pdfcontent() . PHP_EOL;
		return true;
	}
	private function tryjstor() {
		if(!preg_match("/(Stable URL: http:\/\/www\.jstor\.org\/stable\/| Accessed: )/", $this->pdfcontent))
			return false;
		echo "Detected JSTOR file; extracting data." . PHP_EOL;
		$splittext = preg_split("/\nYour use of the JSTOR archive indicates your acceptance of JSTOR's Terms and Conditions of Use/", $this->pdfcontent);
		$head = $splittext[0];
		// get rid of occasional text above relevant info
		$head = preg_replace("/^.*\n\n/", "", $head);
		// split into fields
		$head = preg_split("/( Author\(s\): |\s*(Reviewed work\(s\):.*)?Source: | Published by: | Stable URL: |( \.)? Accessed: )/", $head);
		// handle the easy ones
		$this->title = $head[0];
		$this->doi = '10.2307/' . substr($head[4], 28);
		/*
		 * Process "source" field
		 */
		$source = preg_split("/(, Vol\. |, No\. | \(|\), pp?\. )/", $head[2]);
		$this->journal = $source[0];
		$this->volume = $source[1];
		// issue may have been omitted
		$this->issue = $source[4] ? $source[2] : NULL;
		// year
		$year = $source[4] ? $source[3] : $source[2];
		$this->year = preg_replace("/^.*, /", "", $year);
		// start and end pages
		$pages = $source[4] ? $source[4] : $source[3];
		$pages = explode('-', $pages);
		$this->start = $pages[0];
		$this->end = $pages[1] ? $pages[1] : $pages[0];
		/*
		 * Process "authors" field
		 * Will fail with various variants, including double surnames
		 */
		$authors = $head[1];
		$authors = preg_split("/(, | and )/", $authors);
		// array for correctly formatted authors
		$fmtauth = array();
		foreach($authors as $author) {
			$author = preg_split("/\s/", $author);
			$lastname = array_pop($author) . ", ";
			foreach($author as $firstname) {
				$firstname = preg_replace("/(?<=^[A-Z]).*$/", ".", $firstname);
				$lastname .= $firstname;
			}
			$fmtauth[] = $lastname;
		}
		$authorstring = '';
		foreach($fmtauth as $key => $author) {
			if($key !== 0)
				$authorstring .= "; ";
			$authorstring .= $author;
		}
		$this->authors = $authorstring;
		return true;
	}
	private function trybioone() {
		if(!preg_match("/BioOne \(www\.bioone\.org\) is an electronic aggregator of bioscience research content/", $this->pdfcontent))
			return false;
		echo "Detected BioOne file; extracting data." . PHP_EOL;
		$pdftext = preg_split("/\n\nBioOne \(www\.bioone\.org\) is an electronic aggregator of bioscience research content/", $this->pdfcontent);
		$head = trim($pdftext[0]);
		// get rid of occasional text above relevant info
		$head = preg_replace("/^.*\n\n/", "", $head);
		// split into fields
		$head = preg_split("/(\sAuthor\(s\): | Source: |\. Published By: | URL: )/", $head);
		// handle the easy ones
		$this->title = $head[0];
		$this->doi = preg_replace("/^http:\/\/www\.bioone\.org\/doi\/full\//", '', $head[4]);
		/*
		 * Process "source" field
		 */
		$source = $head[2];
		$source = preg_split("/(, |\(|\)?:|\. )/", $source);
		$this->journal = $source[0];
		$this->volume = str_replace('Number ', '', $source[1]);
		// issue may have been omitted
		$this->issue = $source[4] ? $source[2] : NULL;
		// year
		$year = $source[4] ? $source[4] : $source[3];
		$this->year = preg_replace("/[\.\s]/", "", $year);
		// start and end pages
		$pages = $source[4] ? $source[3] : $source[2];
		$pages = preg_split("/-/", $pages);
		$this->start = $pages[0];
		$this->end = $pages[1] ? $pages[1] : $pages[0];
		/*
		 * Process "authors" field
		 * Will fail with various variants, including double surnames
		 */
		$authors = $head[1];
		$authors = preg_split("/(,? and |, | & )/", $authors);
		// array for correctly formatted authors
		$fmtauth = array();
		foreach($authors as $author) {
			$author = preg_split("/\s/", $author);
			$lastname = array_pop($author) . ", ";
			foreach($author as $firstname) {
				$firstname = preg_replace("/(?<=^[A-Z]).*$/", ".", $firstname);
				$lastname .= $firstname;
			}
			$fmtauth[] = $lastname;
		}
		foreach($fmtauth as $key => $author) {
			if($key !== 0)
				$authorstring .= "; ";
			$authorstring .= $author;
		}
		$this->authors = $authorstring;
		return true;
	}
	private function trygeodiv() {
		if(!preg_match("/GEODIVERSITAS · /", $this->pdfcontent))
			return false;
		echo "Detected Geodiversitas paper." . PHP_EOL;
		preg_match("/\n\n([^\n]*)\n\nKEY/s", $this->pdfcontent, $cite);
		if(!$cite = trim($cite[1]))
			return false;
		// split into fields
		$head = preg_split("/((?<=\.) (?=\d{4})|\. -- |\. Geodiversitas |(?<=\d) \((?=\d)|(?<=\d)\) : (?=\d))/", $cite);
		// handle the easy ones
		$this->journal = "Geodiversitas";
		$this->title = $head[2];
		$this->year = $head[1];
		$this->volume = $head[3];
		$this->issue = $head[4];
		/*
		 * Process pages
		 */
		$pages = str_replace(".", "", $head[5]);
		$pages = explode("-", $pages);
		$this->start = $pages[0];
		$this->end = preg_replace("/\n.*$/", "", $pages[1]);
		/*
		 * Process "authors" field
		 */
		// kill extraneous stuff
		$authors = preg_replace("/.*\n([^\n]+)$/", "$1", $head[0]);
		// kill spaces between initials
		$authors = str_replace(". ", ".", $authors);
		// semicolons between authors
		$authors = preg_replace("/((?<=\.), |& )/", "; ", $authors);
		// comma before initials
		$this->authors = preg_replace("/(?<=[a-z]) (?=[A-Z]\.)/", ", ", $authors);
		return true;
	}
	private function trype() {
	// Palaeontologia Electronica
		// PE has a citation block after either a new line or something like "Accepted: 5 October 2011"
		if(!preg_match("/(\n|Acceptance:\s*(\d+\s*[A-Z][a-z]+|\?\?)\s*\d+\s*)(?!PE Article Number)([^\n]+Palaeontologia Electronica Vol\.[^\n]+)(\s*\$|\n)/u", $this->pdfcontent, $matches))
			return false;
		$citation = $matches[3];
		$processauthors = function($in) {
			$authors = explode(', ', $in);
			$out = '';
			foreach($authors as $key => $aut) {
				if(($key % 2) === 0) { // even index
					$out .= preg_replace('/^and /u', '', $aut) . ', ';
				}
				else {
					$auta = explode(' ', $aut);
					foreach($auta as $autp) {
						$out .= $autp[0] . '.';
					}
					$out .= '; ';
				}
			}
			return preg_replace('/; $/u', '', $out);
		};
		if(preg_match(
			"/^(.*?), (\d{4})\. (.*?), Palaeontologia Electronica Vol\. (\d+), Issue (\d+); ([\dA-Z]+):(\d+)p, [\dA-Za-z]+; ([^\s]*)\$/u",
			$citation,
			$matches
		)) {
			$this->authors = $processauthors($matches[1]);
			$this->year = $matches[2];
			$this->title = $matches[3];
			$this->journal = 'Palaeontologia Electronica';
			$this->volume = $matches[4];
			$this->issue = $matches[5];
			$this->start = $matches[6];
			$this->pages = $matches[7];
			$this->url = $matches[8];
			return true;
		}
		else if(preg_match(
			"/^(.*?) (\d{4})\. (.*?). Palaeontologia Electronica Vol\. (\d+), Issue (\d+); ([\dA-Z]+):(\d+)p; ([^\s]*)\$/u",
			$citation,
			$matches
		)) {
			$this->authors = $processauthors($matches[1]);
			$this->year = $matches[2];
			$this->title = $matches[3];
			$this->journal = 'Palaeontologia Electronica';
			$this->volume = $matches[4];
			$this->issue = $matches[5];
			$this->start = $matches[6];
			$this->pages = $matches[7];
			$this->url = 'http://' . $matches[8];
			return true;
		}
		return false;
	}
	private function trydoi() {
		if(preg_match_all("/(doi|DOI)\s*(\/((full|abs|pdf)\/)?|:|\.org\/)?\s*(?!URL:)([^\s]*?),?\s/su", $this->pdfcontent, $matches)) {
			echo "Detected possible DOI." . PHP_EOL;
			foreach($matches[5] as $match) {
				$doi = trimdoi($match);
				// PNAS tends to return this
				if(preg_match('/^10.\d{4}\/?$/', $doi))
					$doi = preg_replace("/.*?10\.(\d{4})\/? ([^\s]+).*/s", "10.$1/$2", $this->pdfcontent);
				// Elsevier accepted manuscripts
				if(in_array($doi, array("Reference:", "Accepted Manuscript"))) {
					preg_match("/Accepted date: [^\s]+ ([^\s]+)/s", $this->pdfcontent, $doi);
					$doi = $doi[1];
				}
				// get rid of false positive DOIs containing only letters or numbers, or containing line breaks
				if($doi && !preg_match("/^([a-z\(\)]*|\d*)$/", $doi) && !preg_match("/\n/", $doi)) {
					// remove final period
					$this->doi = preg_replace("/\.$/", "", $doi);
					echo "Found DOI: $this->doi" . PHP_EOL;
					return $this->expanddoi();
				}
				else {
					echo "Could not find DOI: $doi." . PHP_EOL;
				}
			}
		}
		return false;
	}
	private function trygoogle() {
		// find title
		($title = $this->findtitle_specifics()) or
			($title = $this->findtitle_pdfcontent());
		if($title === false) return false;
		// show title so it's possible to confirm it's right
		echo "Title: $title" . PHP_EOL;
		/*
		 * get data
		 */
		// construct url
		$search = self::googletitle($title);
		// fetch data
		$cjson = self::fetchgoogle($search);
		if($cjson === false) {
			// we didn't find anything
			echo "Could not find any results in Google" . PHP_EOL;
			return false;
		}
		/*
		 * process
		 */
		foreach($cjson->items as $result) {
			echo "Title: " . $result->title . PHP_EOL;
			echo "URL: " . $result->link . PHP_EOL;
			$url = $result->link;
			$cmd = $this->menu(array(
				'options' => array(
					'y' => 'this URL is correct',
					'n' => 'this URL is not correct',
					'q' => 'quit the function',
					'o' => 'open the URL',
					'r' => 'stop adding data',
				),
				'process' => array(
					'o' => function() use(&$url) {
						exec_catch("open '" . str_replace("'", "\'", $url) . "'");
					},
				),
			));
			switch($cmd) {
				case 'y':
					$doi = preg_replace(array("/^.*\/doi\/(abs|pdf|full|url|pdfplusdirect)?\/?/", "/(\?.*|\/(abstract|full|pdf))$/"), array("", ""), $result->link);
					if($doi === $result->link) {
						$this->url = $result->link;
						// return false, not true, because this doesn't mean we don't have to do manual input for other stuff
						return false;
					}
					else {
						$this->doi = trim($doi);
						return $this->expanddoi();
					}
				case 'q':
					return false;
				case 'n':
					break;
				case 'r':
					$this->adddata_return = true;
					return false;
			}
		}
	}
	private function googletitle($title = '') {
		if(!$title) $title = $this->title;
		return urlencode(preg_replace("/\(|\)|;|-+\\/(?=\s)|(?<=\s)-+|<\/?i>/", "", $title));
	}
	private static function fetchgoogle($search) {
		// get data from Google
		$url = "https://www.googleapis.com/customsearch/v1?key=" . GOOGLEKEY . "&cx=" . GOOGLECUS . "&q=" . $search;
		$json = @file_get_contents($url);
		if(!$json) {
			echo "Error: nothing found in Google." . PHP_EOL;
			return false;
		}
		// decode into PHP-readable format
		$cjson = json_decode($json);
		if($cjson === NULL and $json !== 'null' and $json !== 'NULL') {
			echo "Error: could not decode Google results." . PHP_EOL;
			return false;
		}
		// this means we got good input, but found no results in Google
		if((!isset($cjson->items)) or (!is_array($cjson->items))) {
			echo "Could not find anything in Google." . PHP_EOL;
			return false;
		}
		return $cjson;
	}
	private function doiamnhinput() {
		if(!$this->p->addmanual) return false;
		if($this->pdfcontent) echo $this->pdfcontent . PHP_EOL;
		$name = $this->name;
		$that = $this;
		$doi = $this->menu(array(
			'head' => 'If this file has a DOI or AMNH handle, please enter it.',
			'options' => array(
				'c' => "continue to direct input of data",
				'o' => "open the file",
				'r' => "re-use a citation from a NOFILE entry",
			),
			'processcommand' => function($in) {
				return trimdoi(preg_replace("/doi:\s*/", "", $in));
			},
			'validfunction' => function($in, $options) {
				if(in_array($in, $options))
					return true;
				if(strlen($in) > 2)
					return true;
				return false;
			},
			'process' => array(
				'o' => function() use($that) {
					// this is a hack, until we can actually use $this in an anonymous function
					$that->openf();
				},
			),
		));
		switch($doi) {
			case 'c': return false;
			case 'r':
				echo 'Enter the citation handle.' . PHP_EOL .
					"'q': stop trying csvrefs" . PHP_EOL;
				while(true) {
					$handle = $this->getline();
					if($handle === 'q') break 2;
					if($this->p->has($handle))
						break;
					else
						echo 'Could not find handle' . PHP_EOL;
				}
				$blacklist = array('addmonth', 'addday', 'addyear', 'name', 'folder', 'sfolder', 'ssfolder');
				foreach($this->p->get($handle) as $key => $value) {
					if(!in_array($key, $blacklist))
						$this->$key = $value;
				}
				// make redirect
				$this->p->makeredirect($handle, $this->name);
				$cmd = $this->menu(array(
					'head' => "Data copied.",
					'options' => array(
						'e' => "Type 'e' to review and edit information now associated with this file",
					),
					'validfunction' => function($in) {
						return true;
					},
				));
				if($cmd === 'e') {
					$this->my_inform();
					$this->edit();
				}
				return true;
		}
		if(substr($doi, 0, 22) === 'http://hdl.handle.net/') {
			$this->hdl = substr($doi, 22);
			if($this->expandamnh())
				return true;
			else {
				echo "Could not find data at the AMNH." . PHP_EOL;
				return false;
			}
		}
		$this->doi = $doi;
		return $this->expanddoi();
	}
	private function trymanual() {
		// apply global setting
		if(!$this->p->addmanual)
			return false;
		echo "Please enter data for this file" . PHP_EOL .
		"'q' in any place: quit and move to the next file" . PHP_EOL .
		"'s': save the file" . PHP_EOL .
		"'e': edit all information" . PHP_EOL .
		"'d': try entering a DOI again" . PHP_EOL .
		"'o': open the file" . PHP_EOL;
		$params = array("authors", "year", "title", "journal", "volume", "issue", "start", "end", "pages", "url", "bookauthors", "booktitle", "publisher", "bookpages", "isbn", "location", "parturl");
		foreach($params as $key) {
			while(true) {
				switch($cmd = $this->getline($key . ": ")) {
					case 'q': return false;
					case 's': break 3;
					case 'd': $this->doiamnhinput(); break;
					case 'o': $this->openf(); break;
					case 'e': $this->edit(array('cannotmove' => true)); break;
					default: $this->$key = $cmd; break 2;
				}
			}
		}
		// we assume that the title is input correctly now
		$this->editedtitle = 1;
		return true;
	}
	private function expandamnh() {
		$html = @file_get_contents('http://hdl.handle.net/' . $this->hdl);
		if(!$html) {
			echo 'Could not retrieve data' . PHP_EOL;
			return false;
		}
		else
			echo 'Retrieved data from AMNH' . PHP_EOL;
		$html = preg_split("/<tr><td class=\"metadataFieldLabel\">/", $html);
		// If "Other Titles" field present, bump indices; same for abstract, keyword
		if(preg_match("/Other Titles/", $html[2]))
			$other = 1;
		else
			$other = 0;
		if(preg_match("/Keywords/", $html[3 + $other]))
			$keywords = 1 + $other;
		else
			$keywords = $other;
		if(preg_match("/Abstract/", $html[6 + $keywords]))
			$abstract = 1 + $keywords;
		else
			$abstract = $keywords;
		/*
		 * "Title" field
		 */
		$titleplus = preg_replace("/Title:&nbsp;<\/td><td class=\"metadataFieldValue\">/", "", $html[1]);
		// journal
		if(preg_match("/Bulletin of the AMNH/", $titleplus) || preg_match("/Bulletin of the American Museum of Natural History/", $titleplus))
			$this->journal = "Bulletin of the American Museum of Natural History";
		else if(preg_match("/American Museum novitates/", $titleplus))
			$this->journal = "American Museum Novitates";
		else if(preg_match("/Anthropological papers of the AMNH/", $titleplus))
			$this->journal = "Anthropological Papers of the American Museum of Natural History";
		else if(preg_match("/Memoirs of the AMNH/", $titleplus))
			$this->journal = "Memoirs of the American Museum of Natural History";
		else
			echo "Could not detect journal." . PHP_EOL;
		$titleplus = preg_split("/Bulletin of the AMNH|American Museum novitates|Anthropological papers of the AMNH|Memoirs of the AMNH|Bulletin of the American Museum of Natural History/", $titleplus);
		// title
		$this->title = preg_replace("/ :/", ":", trimplus($titleplus[0]));
		// volume/issue
		$tmp = preg_replace("/[^\d]*(\d+)[^\d]+(\d+).*/", "$1 $2", $titleplus[1]);
		if($tmp === $titleplus[1]) {
			// no "part"
			$this->volume = preg_replace("/.*?(\d+).*/s", "$1", $tmp);
		}
		else {
			// also a "part"
			$tmp = preg_split("/ /", $tmp);
			$this->volume = trim($tmp[0]);
			$this->issue = trim($tmp[1]);
		}
		/*
		 * "Authors" field
		 */
		$authors = preg_replace(array("/Authors:&nbsp;<\/td><td class=\"metadataFieldValue\">/", "/<\/td><\/tr>/"), array("", ""), $html[2 + $other]);
		$authors = preg_split("/<br>/", $authors);
		foreach($authors as $key => $author) {
			$tmp = trim($author);
			// get rid of years
			$tmp = preg_replace("/, \d+-?\d*\.*/", "", $tmp);
			$tmp = preg_split("/, /", $tmp);
			// abbreviate given names
			$tmp[1] = preg_replace(array("/(?<=[A-Z])[^\s]*( |$)/", "/\.\./"), array(".", "."), $tmp[1]);
			$tmp[0] .= ", " . $tmp[1];
			$authors[$key] = $tmp[0];
		}
		foreach($authors as $key => $author) {
			if($key > 0)
				$aut .= "; ";
			$aut .= $author;
		}
		$this->authors = $aut;
		/*
		 * "Issue Date" field
		 */
		$this->year = trim(preg_replace(array("/Issue Date:&nbsp;<\/td><td class=\"metadataFieldValue\">/", "/<\/td><\/tr>/"), array("", ""), $html[3 + $keywords]));
		/*
		 * "Description" field
		 * Format of this field is *very* variable on the AMNH site
		 */
		$desc = trim(preg_replace(array("/Description:&nbsp;<\/td><td class=\"metadataFieldValue\">/", "/<\/td><\/tr>/"), array("", ""), $html[6 + $abstract]));
		// find multi-page range
		if(preg_match("/^(p\. )?\d+-\d+/", $desc)) {
			$pages = preg_replace("/^(p\. )?(\d+-\d+).*/", "$2", $desc);
			$pages = preg_split("/-/", $pages);
			$this->start = $pages[0];
			$this->end = $pages[1];
		}
		else {
			$this->start = 1;
			$this->end = preg_replace("/.*?(\d+).*/", "$1", $desc);
		}
		return true;
	}
	protected function expanddoi($paras = array()) {
	// in $paras:
	// bool ['overwrite'] whether we overwrite existing data
	// bool ['verbose'] whether we mention data that differs from existing data
		if(self::process_paras($paras, array(
			'checklist' => array(
				'overwrite',
				'verbose',
			),
			'default' => array(
				'overwrite' => false,
				'verbose' => false
			),
		)) === PROCESS_PARAS_ERROR_FOUND) return false;
		// fetch data
		$url = "http://www.crossref.org/openurl/?pid=" . CROSSREFID . "&id=doi:" . $this->doi . "&noredirect=true";
		$xml = @simplexml_load_file($url);
		// check success
		if($xml &&
			$xml->query_result && $xml->query_result->body->query &&
			($arr = $xml->query_result->body->query->attributes()) &&
			(string)$arr === "resolved") {
			echo 'Retrieved data for DOI ' . $this->doi . PHP_EOL;
			$result = $xml->query_result->body->query;
		}
		else {
			echo 'Could not retrieve data for DOI ' . $this->doi . PHP_EOL;
			return false;
		}
		/*
		 * process data
		 */
		// variables we process from the API result
		$vars = array('volume', 'issue', 'start', 'end', 'year', 'title', 'journal', 'booktitle', 'isbn', 'authors');
		// kill leading zeroes
		$volume = preg_replace("/^0/u", "", (string)$result->volume);
		$issue = preg_replace("/^0/u", "", (string)$result->issue);
		$start = preg_replace("/^0/u", "", (string)$result->first_page);
		$end = preg_replace("/^0/u", "", (string)$result->last_page);
		$year = (string)$result->year;
		$title = (string)$result->article_title;
		$journal = (string)$result->journal_title;
		$booktitle = (string)$result->volume_title;
		$isbn = (string)$result->isbn;
		$authorsraw = $result->contributors->children();
		$authors = '';
		if($authorsraw) foreach($authorsraw as $author) {
			if((string)$author->attributes() !== "first")
				$authors .= "; ";
			$authors .= ucwords(mb_strtolower((string)$author->surname, 'UTF-8'));
			$authors .= ", ";
			$authors .= preg_replace(array("/([^\s])[^\s]*/u", "/\s/u"), array("$1.", ""), (string)$author->given_name);
		}
		foreach($vars as $var) {
			if(${$var}) {
				// echo differences if verbose is set
				if($paras['verbose']) {
					if(simplify(${$var}) != simplify($this->$var)) {
						echo 'Different data from expanddoi(). File ' . $this->name . '; var ' . $var . PHP_EOL;
						echo 'Existing data: ' . $this->$var . PHP_EOL;
						echo 'New data: ' . ${$var} . PHP_EOL;
					}
				}
				// overwrite everything if overwrite is set; else only if no existing data
				if($paras['overwrite'])
					$this->$var = ${$var};
				else if(!$this->$var)
					$this->$var = ${$var};
			}
		}
		return true;
	}
	function burst() {
	// bursts a PDF file into several files
		echo 'Bursting file ' . $this->name . '. Opening file.' . PHP_EOL;
		$cmd = 'open \'' . BURSTPATH . '/' . $this->name . '\'';
		exec_catch($cmd);
		makemenu(array('q' => 'quit',
			'c' => 'continue with the next file'),
			'Enter file names and page ranges');
		while(true) {
			switch($name = $this->getline('File name: ')) {
				case 'c':
					$cmd = 'mv -n \'' . BURSTPATH . '/' . $this->name . '\' \'' . BURSTPATH . '/Old/' . $this->name . '\'';
					if(!exec_catch($cmd))
						echo 'Error moving file ' . $this->name . ' to Old' . PHP_EOL;
					return true;
				case 'q': return false;
				default:
					if($this->p->has($name)) {
						$cmd = $this->ynmenu('A file with this name already exists. Do you want to continue anyway?');
						switch($cmd) {
							case 'y': break 3;
							case 'n': continue 4;
						}
					}
			}
			while(true) {
				$range = $this->getline('Page range: ');
				if(!preg_match('/^\d+-\d+$/', $range)) {
					echo 'Invalid range' . PHP_EOL;
					continue;
				}
				$srange = explode('-', $range);
				$from = $srange[0];
				$to = $srange[1];
				break;
			}
			$cmd = 'gs -dBATCH -dNOPAUSE -q -sDEVICE=pdfwrite -dFirstPage=' . $from . ' -dLastPage=' . $to . ' -sOUTPUTFILE=\'' . TEMPPATH . '/' . $name . '\' \'' . BURSTPATH . '/' . $this->name . '\'';
			if(!exec_catch($cmd)) {
				echo 'Could not burst PDF' . PHP_EOL;
				return false;
			}
			else
				echo 'Split off file ' . $name . PHP_EOL;
		}
	}
	public function removefirstpage() {
		$tmppath = $this->path(array('folder' => true)) . "/tmp.pdf";
		$cmd = "gs -dBATCH -dNOPAUSE -q -sDEVICE=pdfwrite -dFirstPage=2 -sOUTPUTFILE=$tmppath {$this->path()}";
		exec_catch($cmd);
		// open files for review
		exec_catch("open $tmppath");
		$this->openf();
		$cmd = $this->ynmenu("Do you want to replace the file?");
		switch($cmd) {
			case 'y':
				return exec_catch("mv $tmppath {$this->path()}");
			case 'n':
				exec_catch('rm ' . $tmppath);
				return false;
		}
	}
	/* SMALL UTILITIES */
	public function needsdata() {
		return !($this->hasid() and $this->year and $this->volume and $this->start);
	}
	function folderstr() {
		if($this->truename()) return $this->truename_return;
		return $this->folder . ':' . $this->sfolder . ':' . $this->ssfolder;
	}
	function getkey() {
		if(strpos($this->name, 'MS ') === 0)
			return preg_replace('/^MS ([^\s]+).*$/', '$1', $this->name);
		$tmp = preg_split('/[\s\-,]/', $this->name);
		return $tmp[0];
	}
	function getrefname() {
	// generate refname, which should usually be unique with this method
		if($this->truename()) return $this->truename_return;
		$tmp = explode(',', $this->authors);
		$refname = $tmp[0] . $this->year . $this->volume . $this->start;
		if(!$refname) $refname = $this->title;
		if(is_numeric($refname)) $refname = 'ref' . $refname;
		$refname = str_replace("'", "", $refname);
		return $refname;
	}
	public function getsimpletitle($title = '') {
	// should probably get rid of this
		if(!$title) $title = $this->title;
		$title = preg_replace(
			array(
				'/<\/?i>/u', // |[\'"`*,\.:;\-+()–´«»\/!—]|\s*
				'/[áàâ]/u',
				'/[éèê]/u',
				'/[îíì]/u',
				'/[óôòõ]/u',
				'/[ûúù]/u',
				'/ñ/u',
				'/[çč]/u',
				'/[^a-z0-9]/u',
			),
			array(
				'',
				'a',
				'e',
				'i',
				'o',
				'u',
				'n',
				'c',
				'',
			),
			mb_strtolower($title, mb_detect_encoding($title))
		);
		return $title;
	}
	function openurl() {
		if($this->url)
			$url = $this->url;
		else if($this->doi)
			$url = 'http://dx.doi.org.ezp-prod1.hul.harvard.edu/' . $this->doi;
		else if($this->jstor)
			$url = 'http://www.jstor.org.ezp-prod1.hul.harvard.edu/stable/' . $this->jstor;
		else if($this->hdl)
			$url = 'http://hdl.handle.net/' . $this->hdl;
		else if($this->pmid)
			$url = 'http://www.ncbi.nlm.nih.gov/pubmed/' . $this->pmid;
		else if($this->pmc)
			$url = 'http://www.ncbi.nlm.nih.gov/pmc/articles/PMC' . $this->pmc . '/';
		else {
			echo 'No URL to open' . PHP_EOL;
			return false;
		}
		return exec_catch("open '" . $url . "'");
	}
	function searchgoogletitle() {
	// searches for the title of the article in Google
		$url = 'http://www.google.com/search?q=' . $this->googletitle();
		return exec_catch("open '" . $url . "'");
	}
	public function gethost() {
		if(!$this->url) return false;
		return parse_url($this->url, PHP_URL_HOST);
	}
	private function sugg_helper($in) {
	// in folder suggestions part 2, print a list of suggestions and return useful array
	// input: array of folders
		if(!is_array($in)) return;
		// input is array with key: name of folder; value: array with contents
		// discard value, use key as value for out array
		foreach($in as $key => $value)
			$out[] = $key;
		// print new keys (ints) to be used in user input
		foreach($out as $key => $value)
			echo $key . ': ' . $value . '; ';
		echo PHP_EOL;
		return $out;
	}
	public function email($paras = array()) {
	// Inspired by http://www.webcheatsheet.com/PHP/send_email_text_html_attachment.php#attachment
		$this->process_paras($paras, array(
			// what do we want?
			'checklist' => array(
				'to', // who to e-mail to
				'message', // e-mail message
				'subject', // subject
			),
			'default' => array(
				'subject' => $this->name,
			),
			'askifempty' => array(
				'to',
			),
		));
		if(!isset($paras['message'])) {
			$paras['message'] = "<p>Please find attached the following paper:</p>\r\n<ul><li>" . $this->citepaper() . "</li></ul>\r\n";
		}
		// generate boundary hash
		$boundary_hash = md5(date('r', time()));
		$headers = "From: " . FROMADDRESS .
			"\r\nReply-To: " . FROMADDRESS .
			"\r\nContent-Type: multipart/mixed; boundary=\"PHP-mixed-" .
			$boundary_hash . "\"";
		$message = '
--PHP-mixed-' . $boundary_hash . '
Content-Type: multipart/alternative; boundary="PHP-alt-' . $boundary_hash . '"

--PHP-alt-' . $boundary_hash . '
Content-Type: text/html; charset="iso-8859-1"
Content-Transfer-Encoding: 7bit

' . $paras['message'] . '

--PHP-alt-' . $boundary_hash . '--

--PHP-mixed-' . $boundary_hash . '
Content-Type: application/zip; name="' . $this->name . '"
Content-Transfer-Encoding: base64
Content-Disposition: attachment

' . chunk_split(base64_encode(file_get_contents($this->path(array('type' => 'none'))))) . '
--PHP-mixed-' . $boundary_hash . '--
';
		if(!mail($paras['to'], $paras['subject'], $message, $headers)) {
			echo 'Error sending e-mail' . PHP_EOL;
			return false;
		}
		return true;
	}
	/* PROGRAMMING HELPS */
	public function test() {
		self::fetchgoogle("test");
	}
}
?>
