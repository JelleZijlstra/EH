<?php
require_once(__DIR__ . "/../Common/common.php");
require_once(BPATH . "/Common/ExecuteHandler.php");
define(AVIDADIR, "/Users/jellezijlstra/Desktop/Avida");
define(AVIDACONFIG, AVIDADIR . "/avida.cfg");
define(AVIDAEVENTS, AVIDADIR . "/events.cfg");
define(AVIDAPROG, AVIDADIR . "/avida");
define(AVIDAANALYZE, AVIDADIR . "/analyze.cfg");
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
			'execute' => 'callmethodarg'),
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
			'execute' => 'callmethodarg'),
		'avida_get_org' => array('name' => 'avida_get_org',
			'desc' => 'Place an Avida organism in the main directory',
			'arg' => 'Organism name',
			'execute' => 'callmethodarg'),
		'avida_lineages' => array('name' => 'avida_lineages',
			'desc' => 'Compute lineage statistics for an Avida saved population file',
			'arg' => 'Filename',
			'execute' => 'callmethodarg'),
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
		if(!exec_catch("cp " . AVIDACONFIG . ".default " . AVIDACONFIG)) {
			echo "Error restoring default configuration" . PHP_EOL;
			return false;
		}
		if(!exec_catch("cp " . AVIDAEVENTS . ".default " . AVIDAEVENTS)) {
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
			'errorifempty' => array('type', 'time'),
			'checklist' => array(
				'type', // type of operation
				'time', // time at which operation is performed
				'repeat', // time at which operation is repeated
				'end', // time at which operation is ended
				'arg', // argument to be given
				'comment', // explaining what this does
			),
		)) == PROCESS_PARAS_ERROR_FOUND)
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
	public function avida_run($file) {
		// random seed
		static $seed = 42;
		chdir(AVIDADIR);
		$cmd = AVIDAPROG . " -s $seed 1> '$file' 2> /dev/null; echo \$?";
		$seed++;
		$before = time();
		$ret = trim(shell_exec($cmd));
		$after = time();
		echo 'Avida exited with exit code ' . $ret . ' after ' . ($after - $before) . ' seconds' . PHP_EOL;
		return true;
	}
	public function avida_analyze($file) {
		chdir(AVIDADIR);
		$cmd = AVIDAPROG . " -a 1> '" . $file. "' 2> /dev/null; echo \$?";
		$before = time();
		$ret = trim(shell_exec($cmd));
		$after = time();
		echo 'Avida exited from analysis mode with exit code ' . $ret . ' after ' . ($after - $before) . ' seconds' . PHP_EOL;
		return true;
	}
	public function avida_analyze_start() {
	// start assembling events
		$this->avida_analyze = array();
	}
	public function avida_analyze_add($paras = array()) {
		if($this->process_paras($paras, array(
			'errorifempty' => array('cmd'),
			'checklist' => array(
				'cmd', // Avida command
				'arg', // argument to be given
				'comment', // explaining what this does
			),
		)) == PROCESS_PARAS_ERROR_FOUND)
			return false;
		// we always start events with u for now
		$line = $paras['cmd'];
		if($paras['arg']) $line .= ' ' . $paras['arg'];
		if($paras['comment']) $line .= ' # ' . $paras['comment'];
		$line .= "\n";
		$this->avida_analyze[] = $line;
		return true;
	}
	public function avida_analyze_push() {
		$events = implode('', $this->avida_analyze);
		if(!file_put_contents(AVIDAANALYZE, $events)) {
			echo 'Error saving new analyze.cfg' . PHP_EOL;
			return false;
		}
		return true;
	}
	public function avida_get_org($org) {
	// place an organism in the main directory
		$cmd = "cp " . AVIDADIR . "/data/$org/$org " . AVIDADIR . "/$org";
		if(!exec_catch($cmd))
			$success = false;
		else
			$success = true;
		// report organism fitness
		$fp = file_get_contents(AVIDADIR . "/$org");
		$count = preg_match("/# Fitness\.\.\.\.\.\.\.\.\.: (\d+\.\d+)/u", $fp, $matches);
		if($count === 0)
			echo 'Unable to find fitness' . PHP_EOL;
		else
			echo 'Fitness: ' . $matches[1] . PHP_EOL;
		return $success;
	}
	public function avida_lineages($in) {
		$fh = fopen(AVIDADIR . '/' . $in, 'r');
		if(!$fh) {
			echo __METHOD__ . ': Could not open input file: ' . $in;
			return false;
		}
		$counts = array();
		while($line = fgets($fh)) {
			if(!$line or ($line[0] === '#'))
				continue;
			$lineage = (int) substr($line, -3, 1);
			$counts[$lineage]++;
		}
		foreach($counts as $n => $count) {
			echo "Lineage $n: $count\n";
		}
		return true;
	}
}
