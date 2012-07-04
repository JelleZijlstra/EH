<?php
/*
 *
 * An interface for the Avida digital evolution platform.
 *
 * I've updated this file when I changed the EH interfaces, but not tested it,
 * so it may not currently work.
 */
require_once(__DIR__ . "/../Common/common.php");
require_once(BPATH . "/Common/ExecuteHandler.php");
define('AVIDADIR', "/Users/jellezijlstra/Desktop/Avida");
define('AVIDACONFIG', AVIDADIR . "/avida.cfg");
define('AVIDAEVENTS', AVIDADIR . "/events.cfg");
define('AVIDAPROG', AVIDADIR . "/avida");
define('AVIDAANALYZE', AVIDADIR . "/analyze.cfg");
class AvidaInterface extends ExecuteHandler {
	protected static $AvidaInterface_commands = array(
		'avida_config' => array('name' => 'avida_config',
			'desc' => 'Set Avida configuration settings',
			'arg' => 'None',
			'execute' => 'callmethod'),
		'avida_config_default' => array('name' => 'avida_config_default',
			'desc' => 'Restore default Avida settings',
			'arg' => 'None',
			'execute' => 'callmethod'),
		'avida_events_start' => array('name' => 'avida_events_start',
			'desc' => 'Start assembling Avida event file',
			'arg' => 'None',
			'execute' => 'callmethod'),
		'avida_events_add' => array('name' => 'avida_events_add',
			'desc' => 'Start adding files to Avida events list',
			'arg' => 'None',
			'execute' => 'callmethod'),
		'avida_events_push' => array('name' => 'avida_events_push',
			'desc' => 'Save assembled events list to event file',
			'arg' => 'None',
			'execute' => 'callmethod'),
		'avida_run' => array('name' => 'avida_run',
			'desc' => 'Run Avida',
			'arg' => 'File to save results to',
			'execute' => 'callmethod'),
		'avida_analyze_start' => array('name' => 'avida_analyze_start',
			'desc' => 'Start assembling Avida analysis file',
			'arg' => 'None',
			'execute' => 'callmethod'),
		'avida_analyze_add' => array('name' => 'avida_analyze_add',
			'desc' => 'Start adding files to Avida analyze list',
			'arg' => 'None',
			'execute' => 'callmethod'),
		'avida_analyze_push' => array('name' => 'avida_analyze_push',
			'desc' => 'Save assembled analyze list to config file',
			'arg' => 'None',
			'execute' => 'callmethod'),
		'avida_analyze' => array('name' => 'avida_analyze',
			'desc' => 'Run Avida in analysis mode',
			'arg' => 'File to save results to',
			'execute' => 'callmethod'),
		'avida_get_org' => array('name' => 'avida_get_org',
			'desc' => 'Place an Avida organism in the main directory',
			'arg' => 'Organism name',
			'execute' => 'callmethod'),
		'avida_lineages' => array('name' => 'avida_lineages',
			'desc' => 'Compute lineage statistics for an Avida saved population file',
			'arg' => 'Filename',
			'execute' => 'callmethod'),
	);
	public function __construct() {
		parent::__construct(self::$AvidaInterface_commands);
		chdir(AVIDADIR);
	}
	public function cli() {
		$this->setup_commandline('Avida');
	}
	public function avida_config($paras = array()) {
		if(count($paras) === 0)
			return true;
		$configfile = file_get_contents(AVIDACONFIG);
		if(!$configfile) {
			echo "Error retrieving configuration file";
			return false;
		}
		foreach($paras as $key => $value) {
			$configfile = preg_replace("/" . preg_quote($key) . " [^ ]*/u", $key . ' ' . $value, $configfile, 1, $count);
			if($count !== 1) {
				// then just add it
				$configfile .= "\n" . $key . ' ' . $value. "\n";
			}
		}
		//echo $configfile;
		if(!file_put_contents(AVIDACONFIG, $configfile)) {
			echo "Error writing configuration" . PHP_EOL;
			return false;
		}
		return true;
	}
	public function avida_config_default() {
		if(!$this->shell("cp " . AVIDACONFIG . ".default " . AVIDACONFIG)) {
			echo "Error restoring default configuration" . PHP_EOL;
			return false;
		}
		if(!$this->shell("cp " . AVIDAEVENTS . ".default " . AVIDAEVENTS)) {
			echo "Error restoring default configuration" . PHP_EOL;
			return false;
		}
		return true;
	}
	/* handle events */
	private $avida_events = array();
	public function avida_events_start() {
	// start assembling events
		$this->avida_events = array();
	}
	public function avida_events_add($paras = array()) {
		if($this->process_paras($paras, array(
			'name' => __FUNCTION__,
			'errorifempty' => array('type', 'time'),
			'checklist' => array(
				'type' => 'Type of operation',
				'time' => 'Time at which operation is performed',
				'repeat' => 'Time at which operation is repeated',
				'end' => 'Time at which operation is ended',
				'arg' => 'Argument to be given',
				'comment' => 'Comment explaining what the operation does',
			),
			'default' => array(
				'repeat' => 0,
				'end' => 0,
				'arg' => '',
				'comment' => '',
			),
		)) === PROCESS_PARAS_ERROR_FOUND)
			return false;
		// we always start events with u for now
		$event = 'u ';
		$event .= $paras['time'];
		if($paras['repeat']) $event .= ':' . $paras['repeat'];
		if($paras['end']) {
			if(!$paras['repeat']) {
				echo 'Error: end but not repeat set' . PHP_EOL;
			}
			else
				$event .= ':' . $paras['end'];
		}
		$event .= ' ' . $paras['type'];
		if($paras['arg']) $event .= ' ' . $paras['arg'];
		if($paras['comment']) $event .= ' # ' . $paras['comment'];
		$event .= "\n";
		$this->avida_events[] = $event;
	}
	public function avida_events_push() {
		$events = implode('', $this->avida_events);
		if(!file_put_contents(AVIDAEVENTS, $events)) {
			echo 'Error saving new events.cfg' . PHP_EOL;
			return false;
		}
		return true;
	}
	public function avida_run(array $paras) {
		if($this->process_paras($paras, array(
			'name' => __FUNCTION__,
			'synonyms' => array(0 => 'file'),
			'checklist' => array('file' => 'File to write to'),
			'errorifempty' => array('file'),
		)) === PROCESS_PARAS_ERROR_FOUND) return false;
		// random seed
		static $seed = 42;
		chdir(AVIDADIR);
		$cmd = AVIDAPROG . " -s $seed 1> '" . $paras['file'] . "' 2> /dev/null; echo \$?";
		$seed++;
		$before = time();
		$ret = trim(shell_exec($cmd));
		$after = time();
		echo 'Avida exited with exit code ' . $ret . ' after ' . ($after - $before) . ' seconds' . PHP_EOL;
		return true;
	}
	public function avida_analyze(array $paras) {
		if($this->process_paras($paras, array(
			'name' => __FUNCTION__,
			'synonyms' => array(0 => 'file'),
			'checklist' => array('file' => 'File to write to'),
			'errorifempty' => array('file'),
		)) === PROCESS_PARAS_ERROR_FOUND) return false;
		chdir(AVIDADIR);
		$cmd = AVIDAPROG . " -a 1> '" . $paras['file'] . "' 2> /dev/null; echo \$?";
		$before = time();
		$ret = trim(shell_exec($cmd));
		$after = time();
		echo 'Avida exited from analysis mode with exit code ' . $ret . ' after ' . ($after - $before) . ' seconds' . PHP_EOL;
		return true;
	}
	public function avida_analyze_start(array $paras) {
	// start assembling events
		// no paras, ignore anything we might get
		$this->avida_analyze = array();
	}
	public function avida_analyze_add(array $paras) {
		if($this->process_paras($paras, array(
			'name' => __FUNCTION__,
			'errorifempty' => array('cmd'),
			'checklist' => array(
				'cmd' => 'Avida command',
				'arg' => 'Argument to be given',
				'comment' => 'Comment explaining what the command does',
			),
			'default' => array(
				'arg' => false,
				'comment' => false,
			),
		)) === PROCESS_PARAS_ERROR_FOUND)
			return false;
		// we always start events with u for now
		$line = $paras['cmd'];
		if($paras['arg']) $line .= ' ' . $paras['arg'];
		if($paras['comment']) $line .= ' # ' . $paras['comment'];
		$line .= "\n";
		$this->avida_analyze[] = $line;
		return true;
	}
	public function avida_analyze_push(array $paras) {
		// no paras, ignore anything we might get
		$events = implode('', $this->avida_analyze);
		if(!file_put_contents(AVIDAANALYZE, $events)) {
			echo 'Error saving new analyze.cfg' . PHP_EOL;
			return false;
		}
		return true;
	}
	public function avida_get_org(array $paras) {
	// place an organism in the main directory
		if($this->process_paras($paras, array(
			'name' => __FUNCTION__,
			'synonyms' => array(0 => 'org'),
			'checklist' => array('org' => 'Organism to write'),
			'errorifempty' => array('org'),
		)) === PROCESS_PARAS_ERROR_FOUND) return false;
		$cmd = "cp " . AVIDADIR . "/data/$org/$org " . AVIDADIR . "/" . $paras['org'];
		$success = $this->shell($cmd);
		// report organism fitness
		$fp = file_get_contents(AVIDADIR . '/' . $paras['org']);
		$count = preg_match("/# Fitness\.\.\.\.\.\.\.\.\.: (\d+\.\d+)/u", $fp, $matches);
		if($count === 0)
			echo 'Unable to find fitness' . PHP_EOL;
		else {
			$fitness = (float) $matches[1];
			echo 'Fitness: ' . round($fitness, 2) . PHP_EOL;
		}
		return $success;
	}
	public function avida_lineages(array $paras) {
		if($this->process_paras($paras, array(
			'name' => __FUNCTION__,
			'synonyms' => array(0 => 'file'),
			'checklist' => array('file' => 'File to write to'),
			'errorifempty' => array('file'),
		)) === PROCESS_PARAS_ERROR_FOUND) return false;
		$fh = fopen(AVIDADIR . '/' . $paras['file'], 'r');
		if(!$fh) {
			echo __METHOD__ . ': Could not open input file: ' . $paras['file'];
			return false;
		}
		$counts = array(0, 0);
		while($line = fgets($fh)) {
			if(!$line or ($line[0] === '#'))
				continue;
			$lineage = substr($line, -3, 1);
			// get only active organisms
			if($lineage === '0' || $lineage === '1') {
				$lineage = (int) $lineage;
				$counts[$lineage]++;
			}
		}
		for($i = 0; $i < 2; $i++) {
			echo "Lineage $i: {$counts[$i]}\n";
		}
		return true;
	}
}
