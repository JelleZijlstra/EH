<?php
require_once(BPATH . '/Catalog/load.php');

trait CommonArticleList {
	/*
	 * Properties
	 */
	public $citetype = 'wp'; // default citation type
	public $verbosecite = true; // whether citation functions need to be verbose
	public $includesfn = true; // whether Sfn needs to be included in Article::citewp()
	public $includerefharv = true; // whether |ref=harv needs to be included in Article::citewp()
	public $addmanual = true; // whether we want data adding functions to ask for manual input
	private $lslist = NULL;
	private $newlist = NULL;
	private $burstlist = NULL;
	public $sugglist = NULL; // list of suggestions used in Article::newadd()
	public $foldertree = NULL; // tree of folders used in the List
	public $foldertree_n = NULL;
	public $pdfcontentcache = array(); // cache for Article::$pdfcontent
	protected static $inform_exclude = array('pdfcontent');
	private static $ArticleList_commands = array(
		'countNameParser' => array('name' => 'countNameParser',
			'desc' => 'Count NameParser results'),
		'renameRegex' => array('name' => 'renameRegex',
			'desc' => 'Rename files that match a regex'),
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
		'format' => array('name' => 'format',
			'desc' => 'Format an individual reference',
			'arg' => 'File handle',
			'execute' => 'callmethod'),
		'echocite' => array('name' => 'echocite',
			'aka' => array('c', 'cite'),
			'desc' => 'Cite a file',
			'arg' => 'File handle',
			'execute' => 'callmethod'),
		'addRedirect' => array('name' => 'addRedirect',
			'aka' => array('d'),
			'desc' => 'Add a new redirect',
			'arg' => 'None',
			'execute' => 'callmethod',
			'setcurrent' => true),
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
		'testNameParser' => array('name' => 'testNameParser',
			'desc' => 'Test the NameParser'),
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
			'desc' => 'Give information about the ArticleList object',
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
			'desc' => 'Test Article::findtitle_pdfcontent()\'s capabilities',
			'arg' => 'None',
			'execute' => 'callmethod'),
	);
	
	/*
	 * Managing objects
	 */
	abstract public function makeredirect($handle, $target);
	abstract public function addRedirect(array $paras);
}
