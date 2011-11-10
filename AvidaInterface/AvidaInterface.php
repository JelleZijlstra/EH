<?php
require_once(__DIR__ . "/../Common/common.php");
require_once(BPATH . "/Common/ExecuteHandler.php");
define(AVIDADIR, "/Users/jellezijlstra/Desktop/Avida");
define(AVIDACONFIG, AVIDADIR . "/avida.cfg");
define(AVIDAEVENTS, AVIDADIR . "/events.cfg");
class AvidaInterface extends ExecuteHandler {
	protected static $AvidaInterface_commands = array(
		'config_avida' => array('name' => 'config_avida',
			'desc' => 'Set Avida configuration settings',
			'arg' => 'None',
			'execute' => 'callmethod'),
		'config_avida_default' => array('name' => 'config_avida_default',
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
	);
	public function __construct() {
		parent::__construct(self::$AvidaInterface_commands);
	}
	public function cli() {
		$this->setup_commandline('Avida');
	}
	public function config_avida($paras = array()) {
		if(count($paras) == 0)
			return true;
		$configfile = file_get_contents(AVIDACONFIG);
		if(!$configfile) {
			echo "Error retrieving configuration file";
			return false;
		}
/*		$time = new DateTime();
		// save configuration
		exec_catch("cp " . AVIDACONFIG . ' ' . AVIDACONFIG . "." . $time->getTimestamp());*/
		foreach($paras as $key => $value) {
			$configfile = preg_replace("/^(?<=" . preg_quote($key) . " )[^ ]+(?= )/mu", $value, $configfile, 1, $count);
			if($count !== 1) {
				echo "Invalid configuration setting: " . $key . PHP_EOL;
			}
		}
		if(!file_put_contents(AVIDACONFIG, $configfile)) {
			echo "Error writing configuration" . PHP_EOL;
			return false;
		}
		return true;
	}
	public function config_avida_default() {
		if(!exec_catch("mv " . AVIDACONFIG . ".default " . AVIDACONFIG)) {
			echo "Error restoring default configuration" . PHP_EOL;
			return false;
		}
		if(!exec_catch("mv " . AVIDAEVENTS . ".default " . AVIDAEVENTS)) {
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
		if(!file_put_contents(AVIDACONFIG, $events)) {
			echo 'Error saving new events.cfg' . PHP_EOL;
			return false;
		}
		return true;
	}
}
