<?
/*
 * Part of this code based on http://www.google.com/codesearch/p?hl=en#WwoXSuQk0PM/trunk/expandFns.php
 * Author: Smith609
 * License: PHP license
 */
define(DEBUG, 0);
require_once(__DIR__ . '/../Common/common.php');
require_once(BPATH . '/Common/List.php');
require_once(BPATH . '/UcuchaBot/Snoopy.class.php');
function getbot($paras = '') {
	if($paras['new'] === true)
		return new Bot();
	global $bot;
	if(!$bot)
		$bot = new Bot();
	return $bot;
}
class Bot extends Snoopy {
	public $on;
	const api = 'http://en.wikipedia.org/w/api.php';
	const stddate = 'F j, Y'; // standard date format string for TFA/TFL
	const stdmonth = 'F Y';
	static $Bot_commands = array(
		'writewp' => array('name' => 'writewp',
			'aka' => array('write'),
			'desc' => 'Write to a Wikipedia page',
			'arg' => 'Page to write to, and text in --text=',
			'execute' => 'callmethodarg'),
		'fetchwp' => array('name' => 'fetchwp',
			'aka' => array('fetch'),
			'desc' => 'Fetch the wikitext of a Wikipedia page',
			'arg' => 'Page to fetch',
			'execute' => 'callmethodarg'),
		'do_fa_bolding' => array('name' => 'do_fa_bolding',
			'desc' => 'Bold TFA on WP:FA (UcuchaBot task 1)',
			'arg' => 'None',
			'execute' => 'callmethod'),
		'do_fac_maintenance' => array('name' => 'do_fac_maintenance',
			'desc' => 'Add WikiCup notices to new FACs (UcuchaBot proposed task 2)',
			'arg' => 'None',
			'execute' => 'callmethod'),
		'do_mp_notice' => array('name' => 'do_mp_notice',
			'desc' => 'Add notices to the authors of pages that are about to appear on the Main Page',
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
		require_once(BPATH . '/UcuchaBot/onkiro.php');
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
					$this->cookies[trim($cookie[0])] = $cookie[1];
				}
			}
		}
		$this->submit(self::api, $submit_vars);
		$login_result = json_decode($this->results);
		if($login_result->login->result == "Success") {
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
	public function writewp($page, $paras = '') {
	// write to a page
	// @para $page String page to write to
	// @para $paras Array parameters
	// @para ['text'] String text to write
	// @para ['file'] String file containing text to write. Ignored if ['text'] is present
	// @para ['summary'] String edit summary to be used. Defaults to "Bot edit".
	// @para ['override'] Bool whether to override DEBUG and edit
	// @para ['kind'] String appendtext, prependtext, or text (default). Which of those to use.
	// @para ['abortifexists'] Bool whether to abort the edit if the page already exists. Defaults to "false".
	// @para ['donotmarkasbot'] Bool whether to pretend we're not a bot (useful for talk messages)
		if(!$this->check_login()) return false;
		if(is_string($paras['text']))
			$data = $paras['text'];
		else if(is_string($paras['file']))
			$data = file_get_contents($paras['file']);
		else {
			echo 'No text to write to ' . $page . PHP_EOL;
			return false;
		}
		$paras['summary'] = $paras['summary'] ?: 'Bot edit';
		$paras['kind'] = $paras['kind'] ?: 'text';
		$result = $this->fetchapi(array(
			'action' => 'query',
			'prop' => 'info',
			'intoken' => 'edit',
			'titles' => $page,
		));
		if(!is_array($result))
			return false;
		foreach($result['query']['pages'] as $i_page) {
			$my_page = $i_page;
		}
		$pagetext = $this->fetchwp($page);
		if(preg_match("/(\{\{(no)?bots\|deny=UcuchaBot\}\}|\{\{nobots\}\})/", $pagetext)) {
			echo "Edit denied to page " . $page . PHP_EOL;
			return false;
		}
		if($pagetext and $paras['abortifexists']) {
			echo 'Page' . $page . ' already exists, so aborting edit.' . PHP_EOL;
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
		if(DEBUG === 0 or $paras['override']) {
			// no debugging; submit edit
			$this->submit(self::api, $submit_vars);
		}
		else {
			$debuginfo = '';
			$debuginfo .= 'Editing to:' . PHP_EOL . api . PHP_EOL . PHP_EOL . 'VARIABLES' . PHP_EOL . PHP_EOL;
			foreach($submit_vars as $key => $value)
				if($key !== 'token') $debuginfo .= $key . ':' . PHP_EOL . '<nowiki>' . $value . '</nowiki>' . PHP_EOL . PHP_EOL;
			$debuginfo .= '------------' . PHP_EOL . PHP_EOL;
			echo $debuginfo;
			if(DEBUG === 2) {
				// write to debug log
				$newparas['override'] = true;
				$newparas['text'] = $debuginfo;
				$newparas['kind'] = 'appendtext';
				$newparas['summary'] = 'Test output';
				$this->writewp('User:UcuchaBot/Debug', $newparas);
			}
			return true;
		}
		// handle result
		$result = json_decode($this->results);
		if($result->edit->result == "Success") {
			return true;
		}
		else if($result->edit->result) {
			echo 'Unsuccessful: ' . $result->edit->result . PHP_EOL;
			return false;
		}
		else if($result->error->code) {
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
	public function fetchwp($page, $paras = array()) {
	// fetch a page from WP
	// @para $page String page to fetch
	// @para ['action'] String action to fetch. Defaults to "raw"
		if(!isset($paras['action'])) $paras['action'] = 'raw';
		if(DEBUG > 0) {
			echo 'Fetching page: ' . $page . PHP_EOL;
		}
		if(!$this->check_login()) return false;
		if(!$this->fetch("http://en.wikipedia.org/w/index.php?action=raw&title=" . urlencode($page)))
			return false;
		return $this->results ?: false;
	}
	public function fetchapi($apiparas, $paras = array()) {
		$url = self::api . '?';
		$apiparas['format'] = $apiparas['format'] ?: 'json';
		foreach($apiparas as $key => $value)
			$url .= urlencode($key) . '=' . urlencode($value) . '&';
		// remove last &
		$url = substr($url, 0, -1);
		if(DEBUG > 0) {
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
	public function do_fa_bolding() {
		$writepage = 'Wikipedia:Featured articles';
		$time = new DateTime();
		$date = $time->format('F j, Y');
		echo $date . ": finding today's TFA..." . PHP_EOL;
		$tfa = $this->fetchwp('Template:TFA title/' . $date);
		if(!$tfa) {
			echo "Could not find TFA name." . PHP_EOL;
			return false;
		}
		echo "Done: ". $tfa . PHP_EOL;
		echo "Editing $writepage..." . PHP_EOL;
		$wpfa = $this->fetchwp($writepage);
		$pattern = "/(?<!BeenOnMainPage\||BeenOnMainPage\|\"|BeenOnMainPage\|'')((''|\")?\[\[". escape_regex($tfa) . "(\|[^\]]+)?\]\](''|\")?)/";
		$wpfa = preg_replace($pattern, "{{FA/BeenOnMainPage|$1}}", $wpfa);
		$this->writewp($writepage, 
			array(
				'text' => $wpfa, 
				'summary' => "Bot: bolding today's featured article"
			));
		echo "Done" . PHP_EOL;
		// update [[WP:FANMP]]. Perhaps replace this with a wholesale copying of WP:FA plus regex/other changes
		$writepage = 'Wikipedia:Featured articles that haven\'t been on the Main Page';
		$wpfanmp = $this->fetchwp($writepage);
		$pattern = "/(?<=\n)·?\s*(''|\")?\[\[". escape_regex($tfa) . "(\|[^\]]+)?\]\](''|\")?\s*/u";
		$wpfa = preg_replace($pattern, '', $wpfanmp);
		$this->writewp($writepage, 
			array(
				'text' => $wpfanmp, 
				'summary' => "Bot: removing today's featured article"
			));
		echo "Done" . PHP_EOL;
		return true;
	}
	public function do_fl_bolding() {
		$writepage = 'Wikipedia:Featured lists';
		$time = new DateTime();
		$date = $time->format('F j, Y');
		echo $date . ": finding today's TFL..." . PHP_EOL;
		$tfa = $this->fetchwp('Template:TFL title/' . $date);
		if(!$tfa) {
			echo "Could not find TFL name." . PHP_EOL;
			return false;
		}
		echo "Done: ". $tfa . PHP_EOL;
		echo "Editing $writepage..." . PHP_EOL;
		$wpfa = $this->fetchwp($writepage);
		$pattern = "/(?<!BeenOnMainPage\||BeenOnMainPage\|\"|BeenOnMainPage\|'')((''|\")?\[\[". escape_regex($tfa) . "(\|[^\]]+)?\]\](''|\")?)/";
		$wpfa = preg_replace($pattern, "{{FA/BeenOnMainPage|$1}}", $wpfa);
		$this->writewp($writepage, 
			array(
				'text' => $wpfa, 
				'summary' => "Bot: bolding today's featured list"
			));
		echo "Done" . PHP_EOL;
		return true;
	}
	private function setup_facs() {
		global $FacsList;
		if(!class_exists('FacsList') or !$FacsList instanceof FacsList) {
			require_once(BPATH . '/UcuchaBot/Facs.php');
			$FacsList = new FacsList();
			$FacsList->update();
		}
	}
	public function do_fac_maintenance() {
		$this->setup_facs();
		$this->do_wikicup_notice();
		$this->do_move_marker();
		global $FacsList;
		$FacsList->saveifneeded();
	}
	public function do_wikicup_notice() {
		global $FacsList;
		// get WP:CUP and list of Cup participants
		$date = new DateTime();
		$year = $date->format('Y');
		$wpcup = $this->fetchwp('Wikipedia:WikiCup/History/' . $year);
		preg_match_all(
			'/(?<=\{\{Wikipedia:WikiCup\/Participant2\|)[^\}]*(?=\}\})/u',
			$wpcup,
			$matches,
			PREG_PATTERN_ORDER);
		$cuppers = array();
		foreach($matches[0] as $person)
			$cuppers[] = $person;
		// get FACs we need to check
		$tocheck = $FacsList->bfind(array('checkedcup' => '/^$/'));
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
				$msg = PHP_EOL . '{{subst:User:Ucucha/Cup|' . $nomstring . '}}';
				$this->writewp(
					$fac->name,
					array('text' => $msg,
						'summary' => 'Bot adding notice that this is a WikiCup nomination',
						'kind' => 'appendtext',
				));
			}
			$fac->checkedcup = true;
		}
	}
	public function do_move_marker() {
		global $FacsList;
		$dt = new DateTime('-14 days');
		$date = $dt->format('Ymd');
		// get FAC that older noms needs to be above
		$oldfacs = $FacsList->bfind(array('date' => '<' . $date, 'archived' => '/^$/', 'print' => false, 'printresult' => false));
		if(!is_array($oldfacs) or count($oldfacs) === 0) return false;
		$newlocobj = $FacsList->largest($oldfacs, 'id', array('return' => 'object'));
		$newloc = $newlocobj->name;
		// TODO
		$fac = $this->fetchwp(FacsList::fac);
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
		return $this->writewp(FacsList::fac, array(
			'text' => $fac,
			'summary' => 'Bot edit: Move marker'));
	}
	private function parse_ah($paras = array()) {
	// parse an article's ArticleHistory template
		//@para ['text'] text of article talk page
		//@para ['page'] page to be checked. If ['text'] is given, this is disregarded, to avoid an unnecessary API call.
		if(!isset($paras['text'])) {
			$paras['text'] = $this->fetchwp('Talk:' . $paras['page']);
			if(!isset($paras['text'])) {
				echo 'Could not retrieve talk page.';
				return false;
			}
		}
		preg_match('/{{\s*ArticleHistory\s*([^}]*?)\s*}}/u', $paras['text'], $matches);
		$ahtext = $matches[1];
		if(!$ahtext) {
			echo 'Could not retrieve AH template' . PHP_EOL;
			return false;
		}
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
	protected function gettfa($paras = array()) {
	// get information about a TFA for a day
	// @return Array with keys
	// 'page' String page where the TFA is located
	// 'name' String name of the TFA. Set to false if name cannot be located.
	// 'date' String date of the TFA
	// 'blurb' String TFA blurb
		if(!isset($paras['base'])) 
			$paras['base'] = 'Wikipedia:Today\'s featured article';
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
		$tfatext = $this->fetchwp($newpage['page']);
		if(!$tfatext or strpos($tfatext, '{{TFAempty}}') !== false) // day hasn't been set yet
			return false;
		if(!preg_match(
			// this regex based on the code for Anomiebot II
			"/(?:'''|<b>)\s*\[\[\s*([^|\]]+?)\s*(?:\|[^]]+)?\]\]\s*('''|<\/b>)/u", 
			$tfatext, 
			$matches)) {
			echo 'Error: could not retrieve TFA name from page ' . $tfapage . PHP_EOL;
			$newpage['name'] = false;
		}
		else
			$newpage['name'] = str_replace('&nbsp;', ' ', $matches[1]);
		$newpage['date'] = $paras['date']->format(self::stddate);
		$newpage['blurb'] = trim(preg_replace(
			'/(Recently featured:|\{\{TFAfooter)[^\n]+(\n|$)/u',
			'',
			$tfatext));
		if($paras['print']) {
			foreach($newpage as $key => $value)
				echo $key . ': ' . $value . PHP_EOL;
		}
		return $newpage;
	}
	public function do_mp_notice() {
		$revnotifylimit = 90;
		// GET PAGES TO HANDLE
		$datefile = 'do_tfa_notice_mp.txt';
		$currdate = file_get_contents($datefile); // stored date of last TFA
		$date = new DateTime($currdate);
		$pages = array(); // pages that we are handling
		while(true) {
			$newpage = $this->gettfa(array('date' => $date));
			if(!is_array($newpage)) // no more TFAs
				break;
			if(!$newpage['name']) // could not find TFA name
				continue;
			$pages[] = $newpage;
			$date->modify('+1 day');
		}
		file_put_contents($datefile, $date->format(self::stddate));
		// HANDLE EACH PAGE
		foreach($pages as $page) {
			$notifiedusers = array(); // users to notify
			//edit TFA talk page
			$talkpage = $this->fetchwp('Talk:' . $page['name']);
			if(preg_match('/\|\s*maindate\s*=/u', $talkpage))
				echo 'Maindate already exists on talkpage of page ' . $page['name'] . PHP_EOL;
			else {
				$text = preg_replace('/(\|\s*currentstatus\s*=\s*FA\s*)/u',
					"$1|maindate=" . $page['date'] . "\n",
					$talkpage,
					-1,
					$count);
				if($count !== 1) {
					echo 'Adding maindate failed for page ' . $page['name'] . PHP_EOL;
					var_dump($text, $count);
				}
				else
					$this->writewp('Talk:' . $page['name'], array(
						'text' => $text,
						'summary' => 'Bot edit: This page will appear as today\'s featured article in the near future',
					));
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
			foreach($facobj->nominators as $nom) {
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
				$users[$rev['user']]++;
			}
			foreach($users as $user => $number) {
				if($number > $revnotifylimit)
					$notifiedusers[$user] = true;
			}
			//notify users
			foreach($notifiedusers as $user => $bool) {
				if(!$bool) continue;
				$this->writewp('User talk:' . $user, array(
					'text' => '{{subst:User:UcuchaBot/TFA notice' .
						'|page=' . $page['name'] .
						'|date=' . $page['date'] .
						'|blurb=' . $page['blurb'] .
						'}}',
					'summary' => 'Bot edit: Notice that [[' . $page['name'] . ']] will appear as today\'s featured article in the near future',
					'kind' => 'appendtext',
					'donotmarkasbot' => true,
				));
			}
		}
	}
	public function do_create_fa_logs() {
		$date = new DateTime();
		$month = $date->format('m Y');
		$editsummary = 'Bot creating new monthly log page';
		$deftext = "{{TOClimit|3}}\n==$month==";
		$writeparas = array(
			'text' => $deftext,
			'abortifexists' => true,
			'summary' => $editsummary,
		);
		$this->writewp('Wikipedia:Featured article candidates/Featured log/' . $month, $writeparas);
		$this->writewp('Wikipedia:Featured article candidates/Archived nominations/' . $month, $writeparas);
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
			$text = $this->fetchwp($page);
			if(!$text) return false;
			return preg_match_all('/\{\{Wikipedia:Featured article candidates\//u', $text);
		};
		$logpage = 'Wikipedia:Featured article candidates/Featured log/' . $tparas['date'];
		$text = $this->fetchwp($logpage);
		if(!$text) {
			echo 'Error: could not retrieve text of page ' . $logpage . PHP_EOL;
			return false;
		}
		$tparas['FAs promoted'] = preg_match_all('/\{\{Wikipedia:Featured article candidates\//u', $text, $matches);
		// FAs demoted
		$logpage = 'Wikipedia:Featured article review/archive/' . $tparas['date'];
		$text = $this->fetchwp($logpage);
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
			'print' => false,
			'printresult' => false,
		));
		$newtext = maketemplate('subst:User:UcuchaBot/FAS line', $tparas);
		echo $newtext . PHP_EOL;
	}
}
