<?
/*
 * Part of this code based on http://www.google.com/codesearch/p?hl=en#WwoXSuQk0PM/trunk/expandFns.php
 * Author: Smith609
 * License: PHP license
 */
require_once(__DIR__ . '/../Common/common.php');
require_once(BPATH . '/Common/FileList.php');
require_once(BPATH . '/Common/ListEntry.php');
require_once(BPATH . '/UcuchaBot/Snoopy.class.php');
function getbot($paras = array()) {
	if(isset($paras['new']) and $paras['new'] === true)
		return new Bot();
	global $bot;
	if(!$bot)
		$bot = new Bot();
	return $bot;
}
class Bot extends Snoopy {
	// debug level
	private $botdebug = 0;
	public $on;
	const api = 'http://en.wikipedia.org/w/api.php';
	const stddate = 'F j, Y'; // standard date format string for TFA/TFL
	const stdmonth = 'F Y';
	const mp_notice_limit = 90; // minimum number of edits for user to get notified of TFA
	static $Bot_commands = array(
		'writewp' => array('name' => 'writewp',
			'aka' => array('write'),
			'desc' => 'Write to a Wikipedia page',
			'arg' => 'Page to write to, and text in --text=',
			'execute' => 'callmethod'),
		'fetchwp' => array('name' => 'fetchwp',
			'aka' => array('fetch'),
			'desc' => 'Fetch the wikitext of a Wikipedia page',
			'arg' => 'Page to fetch',
			'execute' => 'callmethod'),
		'do_fa_bolding' => array('name' => 'do_fa_bolding',
			'desc' => 'Bold TFA on WP:FA (UcuchaBot task 1)',
			'arg' => 'None',
			'execute' => 'callmethod'),
		'do_fac_maintenance' => array('name' => 'do_fac_maintenance',
			'desc' => 'Perform a series of FAC maintenance tasks',
			'arg' => 'None',
			'execute' => 'callmethod'),
		'do_mp_notice' => array('name' => 'do_mp_notice',
			'desc' => 'Add notices to the authors of pages that are about to appear on the Main Page',
			'arg' => 'None',
			'execute' => 'callmethod'),
		'do_move_marker' => array('name' => 'do_move_marker',
			'desc' => 'Move the FAC marker',
			'arg' => 'None',
			'execute' => 'callmethod'),
		'do_wikicup_notice' => array('name' => 'do_wikicup_notice',
			'desc' => 'Add WikiCup notices as necessary',
			'arg' => 'None',
			'execute' => 'callmethod'),
		'setup_facs' => array('name' => 'setup_facs',
			'desc' => 'Set up the FACs database',
			'arg' => 'None',
			'execute' => 'callmethod'),
		'do_add_fa_stats' => array('name' => 'do_add_fa_stats',
			'desc' => 'Add a line to [[Wikipedia:Featured article statistics]]',
			'arg' => 'None',
			'execute' => 'callmethod'),
		'gettfa' => array('name' => 'gettfa',
			'desc' => 'Get information about the TFA for a specific day',
			'arg' => 'None',
			'execute' => 'callmethod'),
		'do_fanmp_update' => array('name' => 'do_fanmp_update',
			'desc' => 'Update [[WP:FANMP]]',
			'arg' => 'None',
			'execute' => 'callmethod'),
		'do_create_fa_logs' => array('name' => 'do_create_fa_logs',
			'desc' => 'Create monthly FA logs',
			'arg' => 'None',
			'execute' => 'callmethod'),
		'report_debug' => array('name' => 'report_debug',
			'desc' => 'Report current debug level',
			'arg' => 'None',
			'execute' => 'callmethod'),
		'set_debug' => array('name' => 'set_debug',
			'desc' => 'Set the debug level',
			'arg' => 'New debug level',
			'execute' => 'callmethod'),
	);
	/* Basic functionality */
	function __construct() {
		ini_set("user_agent", "Onkiro; onkiro@gmail.com");
		parent::__construct(self::$Bot_commands);
		$this->login();
	}
	public function cli() {
		$this->setup_commandline('Bot');
	}
	/* Bot library */
	private function login() {
		require_once(BPATH . '/UcuchaBot/data/onkiro.php');
		// Set POST variables to retrieve a token
		$submit_vars["format"] = "json";
		$submit_vars["action"] = "login";
		$submit_vars["lgname"] = $username;
		$submit_vars["lgpassword"] = $password;
		// Submit POST variables and retrieve a token
		$this->submit(self::api, $submit_vars);
		$first_response = json_decode($this->results);
		$submit_vars["lgtoken"] = $first_response->login->token;
		// Store cookies; resubmit with new request (which has token added to post vars)
		foreach($this->headers as $header) {
			if(substr($header, 0,10) === "Set-Cookie") {
				$cookies = explode(";", substr($header, 12));
				foreach($cookies as $oCook) {
					$cookie = explode("=", $oCook);
					// ignore non-cookies
					if(count($cookie) !== 2)
						continue;
					$this->cookies[trim($cookie[0])] = $cookie[1];
				}
			}
		}
		$this->submit(self::api, $submit_vars);
		$login_result = json_decode($this->results);
		if($login_result->login->result === "Success") {
			echo("Login succeeded. Using account " . $login_result->login->lgusername . "." . PHP_EOL);
			// Add other cookies, which are necessary to remain logged in.
			$cookie_prefix = "enwiki";
			$this->cookies[$cookie_prefix . "UserName"] = $login_result->login->lgusername;
			$this->cookies[$cookie_prefix . "UserID"] = $login_result->login->lguserid;
			$this->cookies[$cookie_prefix . "Token"] = $login_result->login->lgtoken;
			$this->on = true;
			return true;
		}
		else {
			echo "Could not log in to Wikipedia servers. Edits will not be committed." . PHP_EOL;
			return false;
		}
	}
	public function writewp(array $paras) {
	// write to a page
	// @para $page String page to write to
		if($this->process_paras($paras, array(
			'name' => __FUNCTION__,
			'synonyms' => array(0 => 'page'),
			'checklist' => array(
				'page' => 'Page to write to',
				'text' => 'String text to write',
				'file' => 'String file containing text to write. Ignored if the text parameter is set',
				'summary' => 'String edit summary to be used',
				'override' => 'Bool whether to override debug level and edit',
				'kind' => 'String appendtext, prependtext, or text (default). Which of those to use.',
				'abortifexists' => 'Bool whether to abort the edit if the page already exists',
				'donotmarkasbot' => 'Bool whether to pretend we\'re not a bot (useful for talk messages)',
			),
			'default' => array(
				'summary' => 'Bot edit',
				'override' => false,
				'kind' => 'text',
				'abortifexists' => false,
				'donotmarkasbot' => false,
				'text' => false,
				'file' => false,
			),
			'errorifempty' => array('page'),
		)) === PROCESS_PARAS_ERROR_FOUND) return false;
		if(!$this->check_login()) return false;
		if($paras['text'])
			$data = $paras['text'];
		else if($paras['file'])
			$data = file_get_contents($paras['file']);
		else {
			echo 'No text to write to ' . $paras['page'] . PHP_EOL;
			return false;
		}
		$result = $this->fetchapi(array(
			'action' => 'query',
			'prop' => 'info',
			'intoken' => 'edit',
			'titles' => $paras['page'],
		));
		if(!is_array($result)) {
			echo "Failed to fetch page to write to: " . $paras['page'] . PHP_EOL;
			return false;
		}
		foreach($result['query']['pages'] as $i_page) {
			$my_page = $i_page;
		}
		$pagetext = $this->fetchwp($paras['page']);
		if(preg_match("/(\{\{(no)?bots\|deny=UcuchaBot\}\}|\{\{nobots\}\})/", $pagetext)) {
			echo "Edit denied to page " . $paras['page'] . PHP_EOL;
			return false;
		}
		if($pagetext and $paras['abortifexists']) {
			echo 'Page ' . $paras['page'] . ' already exists, so aborting edit.' . PHP_EOL;
			return false;
		}
		// prepare query
		$submit_vars = array();
		$submit_vars["action"] = "edit";
		$submit_vars["title"] = $my_page['title'];
		switch($paras['kind']) {
			case 'text': $submit_vars["text"] = $data; break;
			case 'appendtext': $submit_vars['appendtext'] = $data; break;
			case 'prependtext': $submit_vars['prependtext'] = $data; break;
			default: echo 'Error: invalid kind: ' . $paras['kind'] . PHP_EOL; break;
		}
		$submit_vars["summary"] = $paras['summary'];
		if(!$paras['donotmarkasbot']) {
			$submit_vars["minor"] = "1";
		}
		$submit_vars["bot"] = "1";
		$submit_vars["watchlist"] = "nochange";
		$submit_vars["format"] = "json";
		$submit_vars["token"] = $my_page['edittoken'];
		// submit query
		if($this->botdebug === 0 or $paras['override']) {
			// no debugging; submit edit
			$this->submit(self::api, $submit_vars);
		}
		else {
			$debuginfo = '';
			$debuginfo .= 'Editing to:' . PHP_EOL . self::api . PHP_EOL . PHP_EOL . 'VARIABLES' . PHP_EOL . PHP_EOL;
			foreach($submit_vars as $key => $value)
				if($key !== 'token') $debuginfo .= $key . ':' . PHP_EOL . '<nowiki>' . $value . '</nowiki>' . PHP_EOL . PHP_EOL;
			$debuginfo .= '------------' . PHP_EOL . PHP_EOL;
			echo $debuginfo;
			if($this->botdebug === 2) {
				// write to debug log
				$newparas['override'] = true;
				$newparas['text'] = $debuginfo;
				$newparas['kind'] = 'appendtext';
				$newparas['summary'] = 'Test output';
				$newparas['page'] = 'User:UcuchaBot/Debug';
				$this->writewp($newparas);
			}
			return true;
		}
		// handle result
		$result = json_decode($this->results);
		if(isset($result->edit) and $result->edit->result === "Success") {
			return true;
		}
		else if(isset($result->edit) and $result->edit->result) {
			echo 'Unsuccessful: ' . $result->edit->result . PHP_EOL;
			return false;
		}
		else if(isset($result->error) and $result->error->code) {
			// Return error code
			echo 'Error code: ' . $result->error->code . ": " .  $result->error->info . PHP_EOL;
			return false;
		}
		else {
			var_dump($result);
			echo "Unhandled error." . PHP_EOL;
			return false;
		}
	}
	private function check_login() {
		// check whether we're OK to proceed
		if(!$this->on)
			return false;
		// Check that bot is logged in:
		$this->fetch(self::api . "?action=query&prop=info&meta=userinfo&format=json");
		$result = json_decode($this->results);
		if($result->query->userinfo->id == 0) {
			echo "LOGGED OUT: The bot has been logged out from Wikipedia servers";
			return $this->login();
		}
		return true;
	}
	public function fetchwp(array $paras) {
	// fetch a page from WP
		if($this->process_paras($paras, array(
			'name' => __FUNCTION__,
			'synonyms' => array(0 => 'page'),
			'checklist' => array(
				'page' => 'Page to fetch',
				'action' => 'Action to fetch',
			),
			'default' => array('action' => 'raw'),
			'errorifempty' => array('page'),
		)) === PROCESS_PARAS_ERROR_FOUND) return false;
		if($this->botdebug > 0) {
			echo 'Fetching page: ' . $paras['page'] . PHP_EOL;
		}
		if(!$this->check_login()) {
			return false;
		}
		if(!$this->fetch("http://en.wikipedia.org/w/index.php?action=" . $paras['action'] . "&title=" . urlencode($paras['page']))) {
			return false;
		}
		return $this->results ?: false;
	}
	public function fetchapi(array $apiparas, array $paras = array()) {
		if($this->process_paras($paras, array(
			'name' => __FUNCTION__,
			'checklist' => array(
				'pageonly' => 'Whether to return only information about the page itself',
			),
			'default' => array('pageonly' => false),
		)) === PROCESS_PARAS_ERROR_FOUND) return false;
		$url = self::api . '?';
		if(!isset($apiparas['format']))
			$apiparas['format'] = 'json';
		foreach($apiparas as $key => $value)
			$url .= urlencode($key) . '=' . urlencode($value) . '&';
		// remove last &
		$url = substr($url, 0, -1);
		if($this->botdebug > 0) {
			echo 'Fetching page: ' . $url . PHP_EOL;
		}
		if(!$this->check_login()) return false;
		$this->fetch($url);
		$out = $this->results ?: false;
		if($apiparas['format'] === 'json') {
			$decoded = json_decode($out, true);
			if($paras['pageonly']) {
				// we usually want just the page info, which by default is hidden behind a semi-random key (the page ID)
				$tmp = $decoded['query']['pages'];
				return array_pop($tmp);
			}
			else
				return $decoded;
		}
		else
			return $out;
	}
	/* Bot tasks */
	/* TFA */
	public function do_fa_bolding() {
		$writepage = 'Wikipedia:Featured articles';
		$time = new DateTime();
		$date = $time->format('F j, Y');
		echo $date . ": finding today's TFA..." . PHP_EOL;
		$tfa = $this->fetchwp(array('Template:TFA title/' . $date));
		if(!$tfa) {
			echo "Could not find TFA name." . PHP_EOL;
			return false;
		}
		echo "Done: ". $tfa . PHP_EOL;
		echo "Editing $writepage... " . PHP_EOL;
		$wpfa = $this->fetchwp(array($writepage));
		$pattern = "/(?<!BeenOnMainPage\||BeenOnMainPage\|\"|BeenOnMainPage\|'')((''|\")?\[\[". escape_regex($tfa) . "(\|[^\]]+)?\]\](''|\")?)/";
		$wpfa = preg_replace($pattern, "{{FA/BeenOnMainPage|$1}}", $wpfa);
		$this->writewp(array(
				'page' => $writepage,
				'text' => $wpfa,
				'summary' => "Bot: bolding today's featured article"
		));
		echo "done" . PHP_EOL;
		return true;
	}
	public function do_mp_notice() {
		// GET PAGES TO HANDLE
		$datefile = BPATH . '/UcuchaBot/data/do_tfa_notice_mp.txt';
		$currdate = file_get_contents($datefile); // stored date of last TFA
		$date = new DateTime($currdate);
		$pages = array(); // pages that we are handling
		while(true) {
			$newpage = $this->gettfa(array('date' => $date));
			$date->modify('+1 day');
			if(!is_array($newpage)) // no more TFAs
				break;
			if(!$newpage['name']) // could not find TFA name
				continue;
			$pages[] = $newpage;
		}
		file_put_contents($datefile, $date->format(self::stddate));
		// HANDLE EACH PAGE
		foreach($pages as $page) {
			if(!$this->mp_notice_page($page)) {
				echo 'Error notifying contributors for page: ' . $page . PHP_EOL;
			}
		}
	}
	private function mp_notice_page(array $page) {
		echo "Notifying contributors for page " . $page['name'] . PHP_EOL;
		$notifiedusers = array(); // users to notify
		//edit TFA talk page
		$talkpage = $this->fetchwp(array('Talk:' . $page['name']));
		if(preg_match('/\|\s*maindate\s*=/u', $talkpage)) {
			echo 'Maindate already exists on talkpage of page ' .
				$page['name'] . PHP_EOL;
		} else {
			$text = preg_replace('/(\|\s*currentstatus\s*=\s*FA\s*)/u',
				"$1|maindate=" . $page['date'] . "\n",
				$talkpage,
				-1,
				$count);
			if($count !== 1) {
				echo 'Adding maindate failed for page ' . $page['name'] .
					PHP_EOL;
				var_dump($text, $count);
				// tell me that it failed
				$this->writewp(array(
					'page' => 'User talk:Ucucha',
					'kind' => 'appendtext',
					'summary' => 'Failed to write maindate: ' .
						$page['name'],
					'text' => '==Failed to insert maindate on [[Talk:' .
						$page['name'] . "]]==\n" .
						' I was unable to insert a ' .
						'<code>|maindate=</code> on the page [[Talk:' .
						$page['name'] . ']]. It is TFA on ' .
						$page['date'] . '. Thank you! ~~~~',
					'donotmarkasbot' => true,
				));
			}
			else {
				$this->writewp(array(
					'page' => 'Talk:' . $page['name'],
					'text' => $text,
					'summary' => 'Bot edit: This page will appear as ' .
						'today\'s featured article in the near future',
				));
			}
		}
		// get noms
		$ahparas = $this->parse_ah(array('text' => $talkpage));
		$fac = '';
		for($i = 1; $i < 50; $i++) {
			if(!isset($ahparas['action' . $i]))
				break;
			if($ahparas['action' . $i] !== 'FAC')
				continue;
			if($ahparas['action' . $i . 'result'] !== 'promoted')
				continue;
			$fac = $ahparas['action' . $i . 'link'];
		}
		$this->setup_facs();
		$facobj = new FacsEntry(array('name' => $fac), 'n');
		$facobj->addnoms();
		if(is_array($facobj->nominators)) foreach($facobj->nominators as $nom) {
			$notifiedusers[$nom] = true;
		}
		// get top users
		$paras = array(
			'action' => 'query',
			'prop' => 'revisions',
			'titles' => $page['name'],
			'rvprop' => 'user',
			'rvlimit' => 5000,
		);
		$api = $this->fetchapi($paras);
		// array key here is page ID, so unpredictable
		$apipage = array_pop($api['query']['pages']);
		$users = array();
		foreach($apipage['revisions'] as $rev) {
			if(!isset($users[$rev['user']]))
				$users[$rev['user']] = 0;
			$users[$rev['user']]++;
		}
		foreach($users as $user => $number) {
			if($number > self::mp_notice_limit)
				$notifiedusers[$user] = true;
		}
		//notify users
		foreach($notifiedusers as $user => $bool) {
			if(!$bool) continue;
			echo "Notifying user: " . $user . PHP_EOL;
			$this->writewp(array(
				'page' => 'User talk:' . $user,
				'text' => '{{subst:User:UcuchaBot/TFA notice' .
					'|page=' . $page['name'] .
					'|date=' . $page['date'] .
					'|blurb=' . $page['blurb'] .
					'}}',
				'summary' => 'Bot edit: Notice that [[' . $page['name'] .
					']] will appear as today\'s featured article in the ' .
					'near future',
				'kind' => 'appendtext',
				'donotmarkasbot' => true,
			));
		}
		return true;
	}
	public function do_fanmp_update() {
	// update [[WP:FANMP]].
		echo 'Updating WP:FANMP... ';
		$writetitle = 'Wikipedia:Featured articles that haven\'t been on the Main Page';
		$fetchtitle = 'Wikipedia:Featured articles';
		// get input pages
		$fetchpage = $this->fetchwp(array($fetchtitle));
		$writepage = $this->fetchwp(array($writetitle));
		// total articles not on main page
		$totalnmp = 0;
		// get header and footer for WP:FANMP
		// == indicates first section of articles
		$writeheader = substr($writepage, 0, strpos($writepage, '=='));
		// |} ends list of articles
		$writefooter = substr($writepage, strrpos($writepage, '|}'));
		$writebody = '';
		$falist = explode("\n", $fetchpage);
		$inheader = true;
		// number of FAs in current sublist
		$currlist = 0;
		// print the number of articles in a section; update totals
		$printnumber = function() use(&$currlist, &$totalnmp, &$writebody) {
			$totalnmp += $currlist;
			if($currlist === 0) $writebody .= "'''None'''\n";
			$writebody .= "<!-- $currlist -->\n\n";
			$currlist = 0;
		};
		foreach($falist as $line) {
			// ignore empty lines
			if($line === '') continue;
			// ignore header
			if($inheader and $line[0] !== '=') continue;
			// we got to the end
			if($line === '|}') {
				$printnumber();
				break;
			}
			// ignore once that have been on the MP
			if(strpos($line, 'FA/BeenOnMainPage') !== false)
				continue;
			// if we're in the FA list and get a link, it's an FA
			if(strpos($line, '[[') !== false)
				$currlist++;
			// section headers
			if($line[0] === '=') {
				if($inheader)
					$inheader = false;
				else
					$printnumber();
			}
			// add to body
			$writebody .= $line . "\n";
		}
		// clean up
		$writebody = preg_replace('/(?<===\n)· /u', '', $writebody);
		// update total number
		$writeheader = preg_replace(
			"/(?<=''')[\d,]+(?=''' articles are listed here\.)/u",
			number_format($totalnmp),
			$writeheader
		);
		$writetext = $writeheader . $writebody . $writefooter;
		// commit edit
		$success = $this->writewp(array(
			'page' => $writetitle,
			'text' => $writetext,
			'summary' => "Bot: updating WP:FANMP",
		));
		if($success === false)
			echo "failed";
		else
			echo "done";
		echo PHP_EOL;
		return true;
	}
	public function do_fl_bolding() {
		$writepage = 'Wikipedia:Featured lists';
		$time = new DateTime();
		$date = $time->format('F j, Y');
		echo $date . ": finding today's TFL..." . PHP_EOL;
		$tfa = $this->fetchwp(array('Template:TFL title/' . $date));
		if(!$tfa) {
			echo "Could not find TFL name." . PHP_EOL;
			return false;
		}
		echo "Done: ". $tfa . PHP_EOL;
		echo "Editing $writepage..." . PHP_EOL;
		$wpfa = $this->fetchwp(array($writepage));
		$pattern = "/(?<!BeenOnMainPage\||BeenOnMainPage\|\"|BeenOnMainPage\|'')((''|\")?\[\[". escape_regex($tfa) . "(\|[^\]]+)?\]\](''|\")?)/";
		$wpfa = preg_replace($pattern, "{{FA/BeenOnMainPage|$1}}", $wpfa);
		$this->writewp(array(
			'page' => $writepage,
			'text' => $wpfa,
			'summary' => "Bot: bolding today's featured list"
		));
		echo "Done" . PHP_EOL;
		return true;
	}
	/* FAC */
	public function do_fac_maintenance() {
		$this->setup_facs();
		$this->do_wikicup_notice();
		$this->do_move_marker();
		global $FacsList;
		$FacsList->saveifneeded();
	}
	public function do_wikicup_notice() {
		global $FacsList;
		echo 'Checking for WikiCup participants... ';
		// get WP:CUP and list of Cup participants
		$date = new DateTime();
		$year = $date->format('Y');
		// WikiCup does not run in November or December, so do not add notices
		$month = $date->format('n');
		if($month > 10) {
			echo 'We\'re not currently in WikiCup time' . PHP_EOL;
			return true;
		}
		$wpcup = $this->fetchwp(array('Wikipedia:WikiCup/History/' . $year));
		preg_match_all(
			'/(?<=\{\{Wikipedia:WikiCup\/Participant\d\|)[^\}]*(?=\}\})/u',
			$wpcup,
			$matches,
			PREG_PATTERN_ORDER
		);
		$cuppers = array();
		foreach($matches[0] as $person)
			$cuppers[] = $person;
		if(count($cuppers) < 1) {
			echo 'Unable to retrieve list of WikiCup participants' . PHP_EOL;
			return false;
		}
		// get FACs we need to check
		$tocheck = $FacsList->bfind(array(
			'checkedcup' => '/^$/',
			'quiet' => true,
		));
		$cupnoms = array();
		if(is_array($tocheck)) foreach($tocheck as $fac) {
			$cupnoms[$fac->name] = array();
			if(!is_array($fac->nominators)) {
				echo 'No nominators for FAC ' . $fac->name . PHP_EOL;
				continue;
			}
			foreach($fac->nominators as $nom) {
				if(in_array($nom, $cuppers))
					$cupnoms[$fac->name][] = $nom;
			}
			if(count($cupnoms[$fac->name]) > 0) {
				$nomstring = '[[User:' . implode('|]], [[User:', 	$cupnoms[$fac->name]) . '|]]';
				echo 'Found WikiCup nominators for FAC: ' . $fac->name . ': ' . $nomstring . PHP_EOL;
				$msg = PHP_EOL . '{{subst:User:Ucucha/Cup|' . $nomstring . '}}';
				$this->writewp(array(
					'page' => $fac->name,
					'text' => $msg,
					'summary' => 'Bot adding notice that this is a WikiCup nomination',
					'kind' => 'appendtext',
				));
			}
			$fac->checkedcup = true;
		}
		echo 'done' . PHP_EOL;
		return true;
	}
	public function do_move_marker() {
		global $FacsList;
		$dt = new DateTime('-14 days');
		$date = $dt->format('Ymd');
		// get FAC that older noms needs to be above
		$oldfacs = $FacsList->bfind(array(
			'date' => '<' . $date,
			'archived' => false,
			'quiet' => true,
		));
		if(!is_array($oldfacs) or count($oldfacs) === 0) return false;
		$newlocobj = $FacsList->largest($oldfacs, 'id', array('return' => 'object'));
		$newloc = $newlocobj->name;
		// TODO
		$fac = $this->fetchwp(array(FacsList::fac));
		preg_match('/==\s*Older nominations\s*==\s*{{([^\}]*)/u', $fac, $matches);
		$currloc = $matches[1];
		if($currloc === $newloc) {
			echo 'Marker does not need to be moved.' . PHP_EOL;
			return true;
		}
		$fac = preg_replace(
			array('/_/u',
				'/ +(?=\n)/u',
				'/(?<=\d)\s+(?=\}\})/u',
				'/(?<=\n)\s*==\s*Older nominations\s*==\n/u',
			),
			array(' ',
				'',
				'',
				'',
			),
			$fac);
		// put in new line
		$fac = preg_replace(
			'/(?={{' . escape_regex($newloc) . '}})/u',
			"\n== Older nominations ==\n",
			$fac,
			1,
			$count);
		if($count !== 1) {
			echo 'Error: could not find new location' . PHP_EOL;
			return false;
		}
		return $this->writewp(array(
			'page' => FacsList::fac,
			'text' => $fac,
			'summary' => 'Bot edit: Move marker'));
	}
	/* Miscellaneous FA */
	public function do_create_fa_logs() {
		$date = new DateTime();
		$month = $date->format('F Y');
		$writeparas = array(
			'text' => "{{TOClimit|3}}\n==$month==",
			'abortifexists' => true,
			'summary' => 'Bot creating new monthly log page',
		);
		$writeparas['page'] = 'Wikipedia:Featured article candidates/Featured log/' . $month;
		$this->writewp($writeparas);
		$writeparas['page'] = 'Wikipedia:Featured article candidates/Archived nominations/' . $month;
		$this->writewp($writeparas);
		$writeparas['text'] = "{{Featured list log}}\n{{TOClimit|3}}";
		$writeparas['page'] = 'Wikipedia:Featured list candidates/Featured log/' . $month;
		$this->writewp($writeparas);
		$writeparas['page'] = 'Wikipedia:Featured list candidates/Failed log/' . $month;
		$this->writewp($writeparas);
		$writeparas['text'] = "{{Featured list log}}\n\n==Kept==\n\n==Delisted==";
		$writeparas['page'] = 'Wikipedia:Featured list removal candidates/log/' . $month;
		$this->writewp($writeparas);
		return true;
	}
	public function do_add_fa_stats() {
		$date = new DateTime('-1 month');
		// date
		$tparas['date'] = $date->format(self::stdmonth);
		$tparas['date2'] = str_replace(' ', '&nbsp;', $date->format('M Y'));
		// WP:FA oldid
		$info = $this->fetchapi(array(
			'titles' => 'Wikipedia:Featured articles',
			'action' => 'query',
			'prop' => 'info',
		), array('pageonly' => true));
		$tparas['FAoldid'] = $info['lastrevid'];
		// FAs promoted
		$countfacs = function ($page) {
			$text = $this->fetchwp(array($page));
			if(!$text) return false;
			return preg_match_all('/\{\{Wikipedia:Featured article candidates\//u', $text);
		};
		$logpage = 'Wikipedia:Featured article candidates/Featured log/' . $tparas['date'];
		$text = $this->fetchwp(array($logpage));
		if(!$text) {
			echo 'Error: could not retrieve text of page ' . $logpage . PHP_EOL;
			return false;
		}
		$tparas['FAs promoted'] = preg_match_all('/\{\{Wikipedia:Featured article candidates\//u', $text, $matches);
		// FAs demoted
		$logpage = 'Wikipedia:Featured article review/archive/' . $tparas['date'];
		$text = $this->fetchwp(array($logpage));
		if(!$text) {
			echo 'Error: could not retrieve text of page ' . $logpage . PHP_EOL;
			return false;
		}
		$days = cal_days_in_month(CAL_GREGORIAN, $date->format('n'), $date->format('Y'));
		$difference = $tparas['FAs promoted'] - $tparas['FAs demoted'];
		if($difference < 0)
			$tparas['templateused'] = 'no';
		else if($difference > 2 * $days)
			$tparas['templateused'] = 'yes';
		else if($difference > $days)
			$tparas['templateused'] = 'yes2';
		else
			$tparas['templateused'] = 'no2';
		// kill stuff we don't want
		$text = preg_replace('/==\s*Kept status\s*==.*(?=$|==\s*Removed status\s*==)/su', '', $text);
		$tparas['FAs demoted'] = preg_match_all('/\{\{Wikipedia:Featured article review\//u', $text, $matches);
		// Current FACs
		global $FacsList;
		if(!$FacsList) $this->setup_facs();
		$tparas['current FACs'] = $FacsList->bfind(array(
			'archived' => '/^$/',
			'return' => 'count',
			'quiet' => true,
		));
		$newtext = maketemplate('subst:User:UcuchaBot/FAS line', $tparas);
		echo $newtext . PHP_EOL;
	}
	/* Helper methods */
	private function parse_ah(array $paras) {
	// parse an article's ArticleHistory template
		//@para ['text'] text of article talk page
		//@para ['page'] page to be checked. If ['text'] is given, this is disregarded, to avoid an unnecessary API call.
		if(!isset($paras['text'])) {
			$paras['text'] = $this->fetchwp(array('Talk:' . $paras['page']));
			if(!isset($paras['text'])) {
				echo 'Could not retrieve talk page.';
				return false;
			}
		}
		if(!preg_match('/{{\s*ArticleHistory\s*([^}]*?)\s*}}/u', $paras['text'], $matches)) {
			echo 'Could not retrieve AH template' . PHP_EOL;
			return false;
		}
		$ahtext = $matches[1];
		$ahparas = array();
		$split1 = explode('|', $ahtext);
		foreach($split1 as $para) {
			$split2 = explode('=', $para);
			if(!is_array($split2)) continue;
			switch(count($split2)) {
				case 1: $ahparas[] = $split2[0]; break; // unnamed parameter
				case 2: $ahparas[trim($split2[0])] = trim($split2[1]); break;
			}
		}
		return $ahparas;
	}
	private function gettfa(array $paras = array()) {
	// get information about a TFA for a day
	// @return Array with keys
	// 'page' String page where the TFA is located
	// 'name' String name of the TFA. Set to false if name cannot be located.
	// 'date' String date of the TFA
	// 'blurb' String TFA blurb
		if($this->process_paras($paras, array(
			'name' => __FUNCTION__,
			'checklist' => array(
				'base' => 'Base page name',
				'date' => 'DateTime object holding date requested',
				'rawdate' => 'String holding date requested',
				'print' => 'Whether to print data about the TFA',
			),
			'default' => array(
				'base' => 'Wikipedia:Today\'s featured article',
				'print' => false,
				'rawdate' => false,
			),
		)) === PROCESS_PARAS_ERROR_FOUND) return false;
		if($paras['rawdate']) {
			$paras['date'] = new DateTime($paras['rawdate']);
		}
		if(!isset($paras['date']))
			$paras['date'] = new DateTime();
		else if(!$paras['date'] instanceof DateTime) {
			echo __METHOD__ . ': date parameter is invalid' . PHP_EOL;
			$paras['date'] = new DateTime();
		}
		$newpage = array();
		$newpage['page'] = $paras['base'] . '/' . $paras['date']->format(self::stddate);
		$tfatext = $this->fetchwp(array($newpage['page']));
		if(!$tfatext or strpos($tfatext, '{{TFAempty}}') !== false) // day hasn't been set yet
			return false;
		if(!preg_match(
			// this regex based on the code for Anomiebot II
			"/(?:'''|<b>)\s*\[\[\s*([^|\]]+?)\s*(?:\|[^]]+)?\]\]\s*('''|<\/b>)/u",
			$tfatext,
			$matches)) {
			echo 'Error: could not retrieve TFA name from page ' . $newpage['page'] . PHP_EOL;
			$newpage['name'] = false;
		}
		else {
			$newpage['name'] = str_replace('&nbsp;', ' ', $matches[1]);
		}
		$newpage['date'] = $paras['date']->format(self::stddate);
		$newpage['blurb'] = trim(preg_replace(
			'/(Recently featured:|\{\{TFAfooter)[^\n]+(\n|$)/u',
			'',
			$tfatext
		));
		if($paras['print']) {
			foreach($newpage as $key => $value)
				echo $key . ': ' . $value . PHP_EOL;
		}
		return $newpage;
	}
	protected function setup_facs() {
		global $FacsList;
		if(!class_exists('FacsList') or !$FacsList instanceof FacsList) {
			require_once(BPATH . '/UcuchaBot/Facs.php');
			$FacsList = new FacsList();
			$FacsList->update();
		}
	}
	public function set_debug(array $paras) {
		if($this->process_paras($paras, array(
			'name' => __FUNCTION__,
			'synonyms' => array(0 => 'level'),
			'checklist' => array('level' => 'Level to set to'),
			'errorifempty' => array('level'),
		)) === PROCESS_PARAS_ERROR_FOUND) return false;
		$level = (int) $paras['level'];
		if($level !== 0 && $level !== 1 && $level !== 2) {
			echo 'Invalid debug level: ' . $level . PHP_EOL;
			return false;
		}
		$this->botdebug = $level;
		return true;
	}
	public function report_debug(array $paras = array()) {
		echo 'Current debugging level: ' . $this->botdebug . PHP_EOL;
		return $this->botdebug;
	}
}
