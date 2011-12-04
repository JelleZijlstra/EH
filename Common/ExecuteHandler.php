<?php
define(PROCESS_PARAS_ERROR_FOUND, 0x1);
// TODO: more effectively ignore Ctrl+P and stuff like that.
abstract class ExecuteHandler {
	/* Class constants */
	const EVALUATE_ERROR = 0x1;
	const EVALUATE_FUNCTION_CALL = 0x2;
	const CHECK_FLOW_IN_IF = 0x1;
	const CHECK_FLOW_IN_FOR = 0x2;
	const CHECK_FLOW_IN_FUNC = 0x3;
	const CHECK_FLOW_IN_WHILE = 0x4;
	const CHECK_FLOW_NOT_EXECUTING_IN_IF = 0x5;
	const EXECUTE_NEXT = 0x0; // execute says: go on with next
	const EXECUTE_PC = 0x1; // execute whatever is in the PC now
	const EXECUTE_SYNTAX_ERROR = 0x2; // execute returned syntax error
	const EXECUTE_QUIT = 0x3; // execute asked to quit the program
	/* Private properties */
	private $commands;
	private $synonyms;
	private $config = array(
		'debug' => false,
	);
	/* command history */
	// holds all executed commands
	private $history = array();
	// history we're currently in
	private $currhist = 0;
	// length of history array
	private $histlen = array(
		0 => 0,
	);
	// where we are in execution (program counter)
	private $pc = array(
		0 => 0,
	);
	// array of variables defined internally. Three-dimensional, indexed by 
	// currhist, currscope, var name.
	private $vars = array(
		0 => array(
			0 => array(),
		),
	);
	// current scope (set to > 0 on function calls when we implement those)
	private $currscope = array(
		0 => 0,
		1 => 0,
	);
	// array of control flow structures we're currently in
	private $flow = array(
		0 => array(
			0 => array('type' => 'global', 'execute' => true),
		),
	);
	// counter that holds the structure we're in at the moment
	private $flowctr = array(
		0 => 0,
	);
	// functions that we have defined (not indexed by hist)
	private $funcs = array();
	// holds last function return value
	private $eax;
	// set to true when a function has just executed, to let a line know that its function call has completed
	private $funcexecuted = false;
	// line to which function needs to return
	private $retline = -1;
	// holds arguments to a function being called
	private $funcargs = array();
	// return code of $this->evaluate()
	private $evaluate_ret = 0;
	// currently handled files
	protected $current; 
	// set to true to try additional stuff in expand_cmd
	protected $trystatic; 
	// array of codes that can be given in the 'execute' field of a command, and 
	// descriptions
	private static $handlers = array(
		'doallorcurr' => 'Execute a function for the argument only if there is one, and else for all entries. Users that use this handler must implement the doall() method and the method defined by the command\'s name.',
		'docurr' => 'Execute a function for the current entries.',
		'callmethod' => 'Execute the given method, with as its argument $paras.',
		'callmethodarg' => 'As callmethod, with $rawarg as the first argument.',
		'callfunc' => 'Execute the given function, with as its argument $paras.',
		'callfuncarg' => 'As callfunc, with $rawarg as the first argument.',
		'quit' => 'Quit the command line.',
	);
	// array of functions that shell code is allowed to call
	private static $php_funcs = array(
		'getinput',
		'strlen',
		'substr',
	);
	private static $basic_commands = array(
		'execute_help' => array('name' => 'execute_help',
			'aka' => array('h', 'help', 'man'),
			'desc' => 'Get help information about the command line or a specific command',
			'arg' => 'Command name',
			'execute' => 'callmethodarg'),
		'quit' => array('name' => 'quit',
			'aka' => array('q'),
			'desc' => 'Quit the program',
			'arg' => 'None',
			'execute' => 'quit'),
		'listcommands' => array('name' => 'listcommands',
			'desc' => 'List legal commands',
			'arg' => 'None',
			'execute' => 'callmethod'),
		'exec_file' => array('name' => 'exec_file',
			'desc' => 'Execute a series of commands from a file',
			'arg' => 'File path',
			'execute' => 'callmethodarg'),
		'shell' => array('name' => 'shell',
			'aka' => 'exec_catch',
			'desc' => 'Execute a command from the shell',
			'arg' => 'Shell command',
			'execute' => 'callmethodarg'),
		'configset' => array('name' => 'configset',
			'aka' => 'setconfig',
			'desc' => 'Set a configuration variable',
			'arg' => 'Variables to be set',
			'execute' => 'callmethod'),
		'myecho' => array('name' => 'myecho',
			'aka' => 'echo',
			'desc' => 'Echo input',
			'arg' => 'Text to be echoed',
			'execute' => 'callmethodarg'),
		'put' => array('name' => 'put',
			'desc' => 'Print input to terminal',
			'arg' => 'Text to be printed',
			'execute' => 'callmethodarg'),
		'test' => array('name' => 'test',
			'desc' => 'Do something random',
			'arg' => 'None',
			'execute' => 'callmethod'),
	);
	private static $constructs = array(
		'$' => array('name' => '$',
			'desc' => 'Declare and set a variable'
			),
		'if' => array('name' => 'if',
			'desc' => 'Sets a condition',
			),
		'else' => array('name' => 'else',
			'desc' => 'Executes if condition in if is false',
			),
		'endif' => array('name' => 'endif',
			'desc' => 'Ends an if condition',
			),
		'for' => array('name' => 'for',
			'desc' => 'Introduces a for loop',
			),
		'endfor' => array('name' => 'endfor',
			'desc' => 'End a for loop',
			),
		'//' => array('name' => '//',
			'desc' => 'Introduce a comment',
			),
		'while' => array('name' => 'while',
			'desc' => 'Introduces a while loop',
			),
		'endwhile' => array('name' => 'endwhile',
			'desc' => 'Ends a while loop',
			),
		'func' => array('name' => 'func',
			'desc' => 'Starts a function definition',
			),
		'ret' => array('name' => 'ret',
			'desc' => 'Returns from a function',
			),
		'endfunc' => array('name' => 'endfunc',
			'desc' => 'Ends a function definition',
			),
		'call' => array('name' => 'call',
			'desc' => 'Call a function (and discard its return value)',
			),
		'global' => array('name' => 'global',
			'desc' => 'Includes a global variable',
			),
	);
	public function __construct($commands) {
		$this->setup_ExecuteHandler($commands);
	}
	public function setup_ExecuteHandler($commands = NULL) {
	// sets up a handler.
	// @param commands Array of commands to be added to the object's library
		$this->current = array();
		foreach(self::$basic_commands as $command) {
			$this->addcommand($command);
		}
		if($commands) foreach($commands as $command) {
			$this->addcommand($command);
		}
		if($this->trystatic) {
			foreach(static::${get_called_class() . '_commands'} as $cmd) {
				if($cmd['aka']) foreach($cmd['aka'] as $aka)
					static::${get_called_class() . '_synonyms'}[$aka] = $cmd['name'];
			}
		}
	}
	public function addcommand($command) {
	// adds a command to the object's library
	// @param command Array of data forming a command
	// command can have the following components
	// - name: String basic name of the command
	// - aka: Array synonyms
	// - execute: String member of self::$handlers used to execute the command
	// - desc: String description of what the command does
	// - arg: String description of the arguments the command takes
	// - setcurrent: Bool whether $this->current needs to be set to $arg
	// - unnamedseparate: Bool whether all unnamed parameters should be placed in $arg together
	// - method: String method called by the command
		if($this->commands[$command['name']]) {
			if(!$paras['ignoreduplicates']) trigger_error('Command ' . $command['name'] . ' already exists', E_USER_NOTICE);
			return false;
		}
		if(!self::testcommand($command)) return false;
		if($command['aka']) {
			if(!is_array($command['aka'])) {
				if($this->synonyms[$command['aka']])
					trigger_error('Error: ' . $aka . ' already exists as a synonym for ' . $this->synonyms[$aka], E_USER_NOTICE);
				else
					$this->synonyms[$command['aka']] = $command['name'];			
			}
			else foreach($command['aka'] as $aka) {
				if($this->synonyms[$aka])
					trigger_error('Error: ' . $aka . ' already exists as a synonym for ' . $this->synonyms[$aka], E_USER_NOTICE);
				else
					$this->synonyms[$aka] = $command['name'];
			}
		}
		$this->commands[$command['name']] = $command;
		return true;
	}
	static public function addstaticcommand($command, $paras = '') {
		if(static::${get_called_class() . '_commands'}[$command['name']]) {
			if(!$paras['ignoreduplicates']) trigger_error('Command ' . $command['name'] . ' already exists', E_USER_NOTICE);
			return false;
		}
		if(!self::testcommand($command)) return false;
		if($command['aka']) {
			foreach($command['aka'] as $aka) {
				if(static::${get_called_class() . '_synonyms'}[$aka])
					trigger_error('Error: ' . $aka . ' already exists as a synonym for ' . static::${get_called_class() . '_synonyms'}[$aka], E_USER_NOTICE);
				else
					static::${get_called_class() . '_synonyms'}[$aka] = $command['name'];
			}
		}
		static::${get_called_class() . '_commands'}[$command['name']] = $command;
		return true;
	}
	static private function testcommand($command) {
		/* check and handle input */
		// if we don't have those, little point in proceeding
		if(!$command['name']) {
			trigger_error('No name given for new command', E_USER_NOTICE);
			return false;
		}
		if(!$command['execute'] or !array_key_exists($command['execute'], self::$handlers)) {
			trigger_error('No valid execute sequence given for new command ' . $command['name'], E_USER_NOTICE);
			return false;
		}
		// warn if no documentation
		if(!$command['desc']) {
			trigger_error('No documentation given for new command ' . $command['name'], E_USER_NOTICE);
		}
		if(!$command['arg']) {
			trigger_error('No listing of arguments given for new command ' . $command['name'], E_USER_NOTICE);
		}
		return true;	
	}
	private function setvar($var, $value) {
	// set a variable in the internal language
		$this->vars[$this->currhist][$this->curr('currscope')][$var] = $value;
	}
	private function getvar($var) {
	// get the value of an internal variable
		if(!isset($this->vars[$this->currhist][$this->curr('currscope')][$var]))
			return NULL;
		else
			return $this->vars[$this->currhist][$this->curr('currscope')][$var];
	}
	private function evaluate($in) {
		if($this->config['debug']) echo 'Evaluating: "' . $in . '"' . PHP_EOL;
		$this->evaluate_ret = 0;
		// function calls
		if(preg_match(
			"/^([a-zA-Z]+):\s+((.+,\s+)*.+)\$/u", 
			$in, 
			$matches)) {
			if($this->funcexecuted) { 
			// we already executed the program
				$this->funcexecuted = false;
				return $this->eax;
			}
			else {
				$funcname = $matches[1];
				$vars = preg_split("/,\s+/u", $matches[2]);
				$func = $this->funcs[$funcname];
				if(!$func) {
					// use inbuilt PHP functions
					// TODO: add argcount checking here
					if(in_array($funcname, self::$php_funcs)) {
						$ret = call_user_func_array($funcname, $vars);
						if(is_string($ret))
							$ret = '"' . $ret . '"';
						return $ret;
					}
					echo 'Error: unrecognized function ' . $funcname . PHP_EOL;
					$this->evaluate_ret = self::EVALUATE_ERROR;
					return NULL;
				}
				$argcount = count($vars);
				if($argcount != $func['argcount']) {
					echo "Error: incorrect variable number for function  $funcname (expected {$func['argcount']}, got $argcount)\n";
					$this->evaluate_ret = self::EVALUATE_ERROR;
					return NULL;
				}
				$this->funcargs = $vars;
				$this->retline = $this->curr('pc');
				$this->evaluate_ret = self::EVALUATE_FUNCTION_CALL;
				$this->curr('pc', $func['line']);
				return NULL;
			}
		}
		// function call without argument
		else if(preg_match("/^([a-zA-Z]+):\s*\$/u", $in, $matches)) {
			if($this->funcexecuted) { 
			// we already executed the program
				$this->funcexecuted = false;
				return $this->eax;
			}
			else {
				$funcname = $matches[1];
				$func = $this->funcs[$funcname];
				if(!$func) {
					if(in_array($funcname, self::$php_funcs)) {
						$ret = call_user_func($funcname);
						if(is_string($ret))
							$ret = '"' . $ret . '"';
						return $ret;
					}
					echo 'Error: unrecognized function ' . $funcname . PHP_EOL;
					$this->evaluate_ret = self::EVALUATE_ERROR;
					return NULL;
				}
				$this->retline = $this->curr('pc');
				$this->evaluate_ret = self::EVALUATE_FUNCTION_CALL;
				$this->curr('pc', $func['line']);
				return NULL;
			}
		}
		// math
		else if(preg_match(
			"/^(?![\"'])([^\s]*)\s*([+\-*\/=><]|!=|>=|<=)\s*([^\s]*)(?<![\"'])$/u", 
			$in, 
			$matches)) {
			$lval = (float) trim($matches[1]);
			$rval = (float) trim($matches[3]);
			switch($matches[2]) {
				case '+': return $lval + $rval;
				case '-': return $lval - $rval;
				case '*': return $lval * $rval;
				case '/': return $lval / $rval;
				case '=': return ($lval == $rval);
				case '>': return ($lval > $rval);
				case '<': return ($lval < $rval);
				case '!=': return ($lval != $rval);
				case '>=': return ($lval >= $rval);
				case '<=': return ($lval <= $rval);
			}
		}
		else
			return $in;
	}
	private function substitutevars($in) {
		// substitute variable references
		if(preg_match_all("/\\\$(\{[a-zA-Z]+\}|[a-zA-Z]+)/u", $in, $matches)) {
			foreach($matches[1] as $reference) {
				// check for ${varname} syntax
				if($reference[0] === '{') 
					$fmreference = substr($reference, 1, -1);
				else
					$fmreference = $reference;
				// get variables in current scope
				$cvars = $this->curr('vars');
				if(isset($cvars[$fmreference])) {
					// substitute them
					$in = preg_replace(
						"/\\\$" . preg_quote($reference) . "/u",
						$cvars[$fmreference],
						$in,
						1
					);
				}
				else {
					echo "Notice: unrecognized variable " . 
						$fmreference . 
						" (in scope " . 
						$this->curr('currscope') . 
						")" . PHP_EOL;
				}
			}
		}
		return $in;	
	}
	private function check_flow() {
		$f = $this->curr('flowo');
		switch($f['type']) {
			case 'global':
				return 0;
			case 'if':
				if(!$f['execute'])
					return self::CHECK_FLOW_NOT_EXECUTING_IN_IF;
				if($f['part'] === 'then') {
					if($f['condition'])
						return 0;
					else
						return self::CHECK_FLOW_IN_IF;
				}
				else if($f['part'] === 'else') {
					if($f['condition'])
						return self::CHECK_FLOW_IN_IF;
					else
						return 0;
				}
			case 'for':
				// only execute if the loop is supposed to be executed
				if($f['max'] > 0)
					return 0;
				else
					return self::CHECK_FLOW_IN_FOR;
			case 'while':
				if(!$f['execute'])
					return self::CHECK_FLOW_IN_WHILE;
				else
					return 0;
			case 'func':
				if(!$f['execute'])
					return self::CHECK_FLOW_IN_FUNC;
				else
					return 0;
		}
		return 0;
	}
	public function execute($in = NULL) {
	// functions as an interpreter of the byfile() "command line"
		// by default, take current value of PC
		if($in === NULL) {
			$in = $this->curr('pcres');
		}
		// handle empty commands (do nothing)
		if($in === '') {
			return self::EXECUTE_NEXT;
		}
		// divide input into keyword and argument
		preg_match("/^\s*([^\s]+)(\s+(.*))?\s*\$/u", $in, $matches);
		$rawcmd = $matches[1];
		$rawarg = $matches[3];
		// if we're in an if statement that's executing, we need special rules
		$inif = false;
		// handle control flow, and exit if we are not executing this code
		switch($this->check_flow()) {
			case 0: break;
			case self::CHECK_FLOW_IN_IF: 
				$inif = true;
				if(in_array($rawcmd, array('if', 'else', 'endif')))
					break;
				else
					return self::EXECUTE_NEXT;
			case self::CHECK_FLOW_NOT_EXECUTING_IN_IF:
				if(in_array($rawcmd, array('if', 'endif')))
					break;
				else
					return self::EXECUTE_NEXT;
			case self::CHECK_FLOW_IN_FOR: 
				if(in_array($rawcmd, array('for', 'endfor')))
					break;
				else
					return self::EXECUTE_NEXT;
			case self::CHECK_FLOW_IN_FUNC: 
				if(in_array($rawcmd, array('func', 'endfunc')))
					break;
				else
					return self::EXECUTE_NEXT;
			case self::CHECK_FLOW_IN_WHILE:
				if(in_array($rawcmd, array('while', 'endwhile')))
					break;
				else
					return self::EXECUTE_NEXT;
		}
		// substitute variables in argument
		$arg = $this->substitutevars($rawarg);
		if($this->config['debug']) echo 'Executing command: ' . $rawcmd . PHP_EOL;
		if(array_key_exists($rawcmd, self::$constructs)) {
			// execute language construct
			switch($rawcmd) {
				case '$': // variable assignment
					if(preg_match('/^([a-zA-Z]+)\s*=\s*(.*)$/u', $arg, $matches)) {
						$var = $matches[1];
						if(!preg_match("/^[a-zA-Z]+$/u", $var)) {
							echo "Syntax error: Invalid variable name: $var" . PHP_EOL;
							return self::EXECUTE_SYNTAX_ERROR;
						}
						$rawassigned = $matches[2];
						$rawassigned = $this->evaluate($rawassigned);
						if($this->evaluate_ret === self::EVALUATE_FUNCTION_CALL)
							return self::EXECUTE_PC;
						if(preg_match("/^(\"|').*(\"|')$/u", $rawassigned, $matches)) {
							// string assignment
							$rawassigned = substr($rawassigned, 1, -1);
							// remove quote escapes
							$regex = "/\\\\(?=" . $matches[1] . ")/u";
							$assigned = preg_replace($regex, '', $rawassigned);
						}
						else if(preg_match("/^-?(\d+|\d+\.\d+|0x\d+)$/u", $rawassigned)) {
							// number
							$assigned = $rawassigned;
						}
						else {
							echo "Syntax error: Unrecognized assignment value: $rawassigned" . PHP_EOL;
							return self::EXECUTE_SYNTAX_ERROR;
						}
						$this->setvar($var, $assigned);
						return self::EXECUTE_NEXT;
					}
					else if(preg_match('/^([a-zA-Z]+)(\+\+|\-\-)$/u', $arg, $matches)) {
						$varname = $matches[1];
						$var = $this->getvar($varname);
						if($var === NULL) {
							echo 'Notice: Unrecognized variable ' . $varname;
							return self::EXECUTE_NEXT;
						}
						switch($matches[2]) {
							case '++': $var++; break;
							case '--': $var--; break;
						}
						$this->setvar($varname, $var);
						return self::EXECUTE_NEXT;
					}
					else {
						echo "Syntax error: In line: " . $in . PHP_EOL;
						return self::EXECUTE_SYNTAX_ERROR;
					}
				case 'if':
					// evaluate condition
					$condition = (bool) $this->evaluate($arg);
					switch($this->evaluate_ret) {
						case self::EVALUATE_FUNCTION_CALL:
							return self::EXECUTE_PC;
						case self::EVALUATE_ERROR:
							return self::EXECUTE_SYNTAX_ERROR;
					}
					// execute?
					// this gets set when we're in a non-executing part of an if
					// statement that is getting evaluated
					if($inif)
						// not executing code we're in, so not executing this if either
						$execute = false;
					else {
						// are we executing the outer flow object?
						$f = $this->curr('flowo');
						$execute = $f['execute'];
					}
					$this->curr('flowctr', '++');
					$this->curr('flowo', array(
						'type' => 'if',
						'part' => 'then',
						'condition' => $condition,
						'line' => $this->curr('pc'),
						'execute' => $execute,
					));
					return self::EXECUTE_NEXT;
				case 'else':
					$f =& $this->curr('flowo');
					if($f['type'] !== 'if') {
						echo 'Unexpected "else"' . PHP_EOL;
						$this->pcinc();
						return self::EXECUTE_SYNTAX_ERROR;
					}
					$f['part'] = 'else';
					return self::EXECUTE_NEXT;
				case 'endif':
					$f =& $this->curr('flowo');
					if($f['type'] !== 'if') {
						echo 'Unexpected "endif"' . PHP_EOL;
						$this->pcinc();
						return self::EXECUTE_SYNTAX_ERROR;
					}
					$this->curr('flowctr', '--');
					return self::EXECUTE_NEXT;
				case 'for':
					if(preg_match("/^(\d+)\s+count\s+(.*)$/u", $arg, $matches)) {
						$max = (int) $matches[1];
						$var = $matches[2];
						if($max < 1)
							$execute = false;
						else {
							$f = $this->curr('flowo');
							$execute = $f['execute'];
						}
						$this->setvar($var, 0);
						$this->curr('flowctr', '++');
						$this->pcinc();
						$this->curr('flowo', array(
							'type' => 'for',
							'subtype' => 'count',
							'counter' => 0,
							'max' => $max,
							'countervar' => $var,
							'line' => $this->curr('pc'),
							'execute' => $execute,
						));
						return self::EXECUTE_PC;
					}
					else if(preg_match("/^(\d+)$/u", $arg, $matches)) {
						$max = (int) $matches[1];
						if($max < 1)
							$execute = false;
						else {
							$f = $this->curr('flowo');
							$execute = $f['execute'];
						}
						$this->curr('flowctr', '++');
						$this->pcinc();
						$this->curr('flowo', array(
							'type' => 'for',
							'subtype' => 'barecount',
							'counter' => 0,
							'max' => $max,
							'line' => $this->curr('pc'),
							'execute' => $execute,
						));
						return self::EXECUTE_PC;
					}
					else {
						echo 'Syntax error: In line: ' . $in . PHP_EOL;
						return self::EXECUTE_SYNTAX_ERROR;
					}
				case 'endfor':
					$f =& $this->curr('flowo');
					if($f['type'] !== 'for') {
						echo 'Syntax error: Unexpected "endfor"' . PHP_EOL;
						return self::EXECUTE_SYNTAX_ERROR;
					}
					// if we're not executing this, no point in looping
					if(!$f['execute']) {
						$this->curr('flowctr', '--');
						return self::EXECUTE_NEXT;
					}
					if($f['subtype'] === 'count') {
						$ctr = $this->getvar($f['countervar']);
						$ctr++;
						if($ctr < $f['max']) {
							$this->curr('pc', $f['line']);
							$this->setvar($f['countervar'], $ctr);
						}
						else {
							$this->curr('flowctr', '--');
							$this->pcinc();
						}
						return self::EXECUTE_PC;
					}
					else if($f['subtype'] === 'barecount') {
						$f['counter']++;
						if($f['counter'] < $f['max']) {
							$this->curr('pc', $f['line']);
						}
						else {
							$this->curr('flowctr', '--');
							$this->pcinc();
						}
						return self::EXECUTE_PC;
					}
					else {
						echo 'Unrecognized subtype: ' . $f['subtype'] . PHP_EOL;
						return self::EXECUTE_SYNTAX_ERROR;
					}
				case '//': // comment, ignored
					return self::EXECUTE_NEXT;
				case 'while':
					$condition = $rawarg;
					$execute = (bool) $this->evaluate($this->substitutevars($condition));
					switch($this->evaluate_ret) {
						case self::EVALUATE_FUNCTION_CALL:
							return self::EXECUTE_PC;
						case self::EVALUATE_ERROR:
							return self::EXECUTE_SYNTAX_ERROR;
					}
					// check whether we're executing this area at all
					if($execute) {
						$f = $this->curr('flowo');
						if(!$f['execute'])
							$execute = false;
					}
					$this->pcinc();
					$this->curr('flowctr', '++');
					$this->curr('flowo', array(
						'type' => 'while',
						'condition' => $condition,
						'execute' => $execute,
						'line' => $this->curr('pc'),
					));
					return self::EXECUTE_PC;
				case 'endwhile':
					$f =& $this->curr('flowo');
					if($f['type'] !== 'while') {
						echo 'Syntax error: Unexpected "endwhile"' . PHP_EOL;
						return self::EXECUTE_SYNTAX_ERROR;
					}
					// if we're not executing this, no point in looping
					if(!$f['execute']) {
						$this->curr('flowctr', '--');
						return self::EXECUTE_NEXT;
					}
					$f['execute'] = (bool) $this->evaluate($this->substitutevars($f['condition']));
					if($this->evaluate_ret === self::EVALUATE_FUNCTION_CALL)
						return self::EXECUTE_PC;
					if($f['execute']) {
						$this->curr('pc', $f['line']);
						return self::EXECUTE_PC;
					}
					else {
						$this->curr('flowctr', '--');
						return self::EXECUTE_NEXT;
					}
				case 'func': // function introduction
					// compile function definition
					if(!preg_match(
						"/^([a-zA-Z]+):\s+(([a-zA-Z]+,\s+)*[a-zA-Z]+)\$/u", 
						$arg, 
						$matches)) {
						echo "Syntax error: In line: $in" . PHP_EOL;
						return self::EXECUTE_SYNTAX_ERROR;
					}
					$name = $matches[1];
					$vars = preg_split("/,\s+/", $matches[2]);
					// functions get their own scope
					$this->curr('currscope', '++');
					$this->curr('vars', array());
					// increment flowcounter so we can edit its variables
					$this->curr('flowctr', '++');
					$f = $this->funcs[$name];
					if($f) {
						// function already exists; call it
						if($this->retline < 0) {
							echo "Syntax error: Redefinition of function $name" . PHP_EOL;
							return self::EXECUTE_SYNTAX_ERROR;
						}
						foreach($f['args'] as $key => $value) {
							$this->setvar($value, $this->funcargs[$key]);
						}
						$flow = array(
							'type' => 'func',
							// we're assuming that we'll only get here when we're actually executing the code
							'execute' => true,
							'ret' => $this->retline,
						);
						// reset variables
						$this->retline = -1;
						$this->funcargs = array();
					}
					else {
						// create new function
						$func = array(
							'name' => $name,
							'args' => $vars,
							'line' => $this->curr('pc'),
							'argcount' => count($vars),
						);
						$this->funcs[$name] = $func;
						$flow = array(
							'type' => 'func',
							// don't execute while we're loading function
							'execute' => false, 
							'line' => $this->curr('pc'),
							'function' => $name,
						);
					}
					// note that flowctr has already been incremented
					$this->curr('flowo', $flow); 
					return self::EXECUTE_NEXT;
				case 'ret':
					// loop through inner control flow structures until we find
					// our function
					$flow = $this->curr('flowctr');
					do {
						$f = $this->flow[$this->currhist][$flow];
						if(!$f['execute'])
							return self::EXECUTE_NEXT;
						$flow--;
						if($flow < 0) {
							echo 'Syntax error: Unexpected "ret"' . PHP_EOL;
							return self::EXECUTE_SYNTAX_ERROR;
						}
					} while($f['type'] !== 'func');
					// return value is the same as the argument; does not get evaluated. If there is no argument, we return NULL.
					$this->eax = $arg;
					$this->curr('pc', $f['ret']);
					$this->funcexecuted = true;
					$this->curr('flowctr', $flow);
					$this->curr('currscope', '--');
					return self::EXECUTE_PC;
				case 'endfunc':
					$f = $this->curr('flowo');
					if($f['type'] !== 'func') {
						echo 'Syntax error: Unexpected "endfunc"' . PHP_EOL;
						return self::EXECUTE_SYNTAX_ERROR;
					}
					if($f['execute']) {
						// we reached end of an executing function, so return NULL
						$this->eax = NULL;
						$this->curr('pc', $f['ret']);
						$this->funcexecuted = true;
						$this->curr('flowctr', '--');
						$this->curr('currscope', '--');
						return self::EXECUTE_PC;
					}
					$this->curr('flowctr', '--');
					$this->curr('currscope', '--');
					return self::EXECUTE_NEXT;
				case 'call':
					// call makes no sense without an argument
					if($arg === NULL) {
						echo 'Syntax error: In line: ' . $in . PHP_EOL;
						return self::EXECUTE_SYNTAX_ERROR;
					}
					// return value gets discarded; just evaluate argument
					$this->evaluate($arg);
					switch($this->evaluate_ret) {
						case self::EVALUATE_FUNCTION_CALL:
							return self::EXECUTE_PC;
						case self::EVALUATE_ERROR:
							return self::EXECUTE_SYNTAX_ERROR;
					}
					return self::EXECUTE_NEXT;
				case 'global':
					if(!preg_match("/^([a-zA-Z]+)\$/u", $arg, $matches)) {
						echo 'Syntax error: In line: ' . $in . PHP_EOL;
						return self::EXECUTE_SYNTAX_ERROR;
					}
					if($this->flowctr == 0) {
						return self::EXECUTE_NEXT;
					}
					$var = $arg;
					if($this->getvar($var) !== NULL) {
						echo 'Notice: attempted global variable ' . $var . ' already exists locally' . PHP_EOL;
						return self::EXECUTE_NEXT;
					}
					if(!isset($this->vars[$this->currhist][0][$var])) {
						echo 'Notice: there is no global variable ' . $var . PHP_EOL;
						$this->setvar($var, NULL);
						return self::EXECUTE_NEXT;
					}
					// alias local variable to global
					$this->vars[$this->currhist][$this->curr('currscope')][$var] =& $this->vars[$this->currhist][0][$var];
					return self::EXECUTE_NEXT;
			}
		}
		// now we're looking only at EH-defined commands, not language constructs
		$cmd = $this->expand_cmd($rawcmd);
		if(!$cmd) {
			echo 'Invalid command: ' . $in . PHP_EOL;
			return self::EXECUTE_NEXT;
		}
		// split command into pieces
		$splitarg = $this->divide_cmd($arg);
		// block of stuff for ease of usage
		{
			// handle output redirection
			$outputredir = $outputredirvar = $returnredir = $returnredirvar = false;
			$next = false;
			$paras = array(); // parameters to be sent to command
			$argument = ''; // argument to be sent to command
			$argarray = array(); // array of argument for docurr commands
			foreach($splitarg as $piece) {
				// handle output redirection
				if($piece === '>' || $piece === '>$' || $piece === '}' || $piece === '}$' ) {
					$next = $piece;
					continue;
				}
				if($next !== false) {
					switch($next) {
						case '>': $outputredir = $piece; break;
						case '>$': $outputredirvar = $piece; break;
						case '}': $returnredir = $piece; break;
						case '}$': $returnredirvar = $piece; break;
					}
					$next = false;
					continue;
				}
				// arguments without initial -
				if($piece[0] !== '-') {
					// allow for escaping
					if(substr($piece, 0, 2) === '\-')
						$piece = substr($piece, 1);
					if($splitcmd['unnamedseparate'])
						$paras[] = $piece;
					else {
						if($argument) $argument .= ' ';
						$argument .= $piece;
					}
					continue;
				}
				// long-form arguments
				if($piece[1] === '-') {
					if(($pos = strpos($piece, '=')) === false)
						$paras[substr($piece, 2)] = true;
					else if($pos === 2) {
						echo 'Invalid argument: ' . $piece . PHP_EOL;
						continue;
					}
					else {
						$key = substr($piece, 2, $pos - 2);
						$value = substr($piece, $pos + 1);
						$paras[$key] = $value;
					}
				}
				// short-form arguments
				else if(($pos = strpos($piece, '=')) === false) {
					$len = strlen($piece);
					for($i = 1; $i < $len; $i++)
						$paras[$piece[$i]] = true;
				}
				else if($pos === 2) {
					$paras[$piece[1]] = substr($piece, 3);
				}
				else {
					echo 'Invalid argument: ' . $piece . PHP_EOL;		
				}
			}
			if($argument) {
				$argument = self::remove_quotes($argument);
				// handle shortcut
				if($argument === '*')
					$argarray = $this->current;
				else
					$argarray = array($argument);
				$paras[0] = $rawarg;
			}
			// cleanup
			foreach($paras as &$text) {
				if($text and is_string($text)) $text = self::remove_quotes($text);
			}		
		}
		// output redirection
		if(($outputredir || $outputredirvar) and $cmd['execute'] !== 'quit') {
			ob_start();
		}
		// return value of executed command
		$ret = NULL;
		// execute it
		switch($cmd['execute']) {
			case 'doallorcurr':
				if($argarray) {
					foreach($argarray as $file)
						if(!($ret = $this->{$cmd['name']}($file, $paras))) break;
				}
				else
					$ret = $this->doall($cmd['name'], $paras);
				break;
			case 'docurr':
				foreach($argarray as $entry) {
					$ret = $this->{$cmd['name']}($entry, $paras);
				}
				break;
			case 'callmethod':
				$ret = $this->{$cmd['name']}($paras);
				break;
			case 'callmethodarg':
				$ret = $this->{$cmd['name']}($argument, $paras);
				break;
			case 'callfunc':
				$ret = $cmd['name']($paras);
				break;
			case 'callfuncarg':
				$ret = $cmd['name']($argument, $paras);
				break;			
			case 'quit':
				return self::EXECUTE_QUIT;
			default:
				trigger_error('Unrecognized execution mode', E_USER_NOTICE); 
				break;
		}
		if($cmd['setcurrent'] and (count($argarray) !== 0))
			$this->current = $argarray;
		if($outputredir) {
			$file = fopen($outputredir, 'w');
			if(!$file) {
				trigger_error('Invalid rediction file: ' . $outputredir, E_USER_NOTICE);
				ob_end_clean();
				return self::EXECUTE_NEXT;
			}
			fwrite($file, ob_get_contents());
			ob_end_clean();
		}
		else if($outputredirvar) {
			$this->setvar($outputredirvar, ob_get_contents());
			ob_end_clean();
		}
		if($returnredir) {
			$file = fopen($returnredir, 'w');
			if(!$file) {
				trigger_error('Invalid rediction file: ' . $outputredir, E_USER_NOTICE);
				ob_end_clean();
				return self::EXECUTE_NEXT;
			}
			fwrite($file, $ret);
		}
		// no else here; having both returnredir and returnredirvar makes sense
		if($returnredirvar) {
			$this->setvar($returnredirvar, $ret);
		}
		// always make return value accessible to script
		$this->setvar('ret', $ret);
		return self::EXECUTE_NEXT;
	}
	private function divide_cmd($in) {
	// divides a string into pieces at each space, and keeps strings in ''/"" together
		$len = strlen($in);
		$out = array();
		$key = 0; // array key, starting at 0
		$insstring = false; // are we in a single-quoted string?
		$indstring = false; // are we in a double-quoted string?
		for($i = 0; $i < $len; $i++) {
			if($in[$i] === ' ' and 
				($i === 0 or $in[$i-1] !== '\\') and 
				!$indstring and 
				!$insstring) {
				$key++;
				continue;
			}
			if($in[$i] === "'" and !$indstring and ($i === 0 or $in[$i-1] !== '\\'))
				$insstring = $insstring ? false : true;
			if($in[$i] === '"' and !$insstring and ($i === 0 or $in[$i-1] !== '\\'))
				$indstring = $indstring ? false : true;
			$out[$key] .= $in[$i];
		}
		return $out;
	}
	private function expand_cmd($in) {
		// substitute variable names in command
		$cmd = $this->substitutevars($cmd);
		// search for dynamic commands
		$cmd = $this->synonyms[$in] ?: ($this->commands[$in] ? $in : false);
		if($cmd) 
			return $this->commands[$cmd];
		// should we try for static commands?
		if(!$this->trystatic) 
			return false;
		$cmd = static::${get_called_class() . '_synonyms'}[$in] ?: (static::${get_called_class() . '_commands'}[$in] ? $in : false);
		return $cmd ? static::${get_called_class() . '_commands'}[$cmd] : false;
	}
	static public function remove_quotes($in) {
	// TODO: replace this and execute()s functionality so we'll be able to do all this in divide_command(). This will fail with weirdly quoted strings.
		return preg_replace("/^['\"]|[\"']$|\\\\(?=['\"])/u", '', $in);
	}
	private function execute_help($in) {	
		// array of functions with info
		if(!$in) {
			echo 'In command line, various options can be used to manipulate the list or its files. The following commands are available:' . PHP_EOL;
			$this->listcommands();
			echo 'Type "help <command>" to get more information about that commmand. Commands will sometimes take an argument, usually a filename. Arguments are always optional; if they are not given, you may be prompted to give a filename or other information.' . PHP_EOL;
			return true;
		}
		$in = $this->expand_cmd($in);
		if($in) {
			echo PHP_EOL . 'Function: ' . $in['name'] . PHP_EOL;
			if($in['aka']) 
				echo 'Aliases: ' . implode(', ', $in['aka']) . PHP_EOL;
			echo 'Description: ' . $in['desc'] . PHP_EOL;
			echo 'Arguments: ' . $in['arg'] . PHP_EOL;
			return true;
		}
		else {
			echo 'No such command' . PHP_EOL;
			return false;
		}
	}
	static protected function expandargs(&$paras, $synonyms) {
	// utility function for EH commands.
	// Deprecated: use process_paras instead
		if(!is_array($synonyms)) return false;
		foreach($synonyms as $key => $result) {
			if(isset($paras[$key]) and !isset($paras[$result]))
				$paras[$result] = $paras[$key];
		}
	}
	static protected function setifneeded(&$paras, $field) {
		if($paras[$field]) 
			return;
		$paras[$field] = getinput_label(ucfirst($field));
	}
	static protected function process_paras(&$paras, $pp_paras = NULL) {
	// processes a function's $paras array, as specified in the $pp_paras parameter
		if(!is_array($paras)) {
			echo 'Error: invalid parameters given' . PHP_EOL;
			return PROCESS_PARAS_ERROR_FOUND;
		}
		if(!is_array($pp_paras)) {
			// this means we only have to check whether $paras is an array
			return 0;
		}
		$founderror = false;
		foreach($pp_paras as $pp_key => $pp_value) {
			switch($pp_key) {
				case 'synonyms':
					if(!is_array($pp_value)) {
						echo 'Error: synonyms parameter is not an array' . PHP_EOL;
						$founderror = true;
						break;
					}
					foreach($pp_value as $key => $result) {
						if(isset($paras[$key]) and !isset($paras[$result]))
							$paras[$result] = $paras[$key];
					}
					break;
				case 'askifempty':
					if(!is_array($pp_value))
						$pp_value = array($pp_value);
					foreach($pp_value as $key) {
						if(!isset($paras[$key])) {
							echo $key . ': ';
							$offset = strlen($key) + 2;
							$paras['key'] = $this->getline(array(
								'offset' => $offset,
							));
						}
					}
					break;
				case 'errorifempty':
					if(!is_array($pp_value))
						$pp_value = array($pp_value);
					foreach($pp_value as $key) {
						if(!isset($paras[$key])) {
							echo 'Error: parameter ' . $key . ' should be set' . PHP_EOL;
							$founderror = true;
						}
					}
					break;
				case 'default':
					if(!is_array($pp_value)) {
						echo 'Error: default parameter is not an array' . PHP_EOL;
						$founderror = true;
						break;
					}
					foreach($pp_value as $key => $result) {
						if(!isset($paras[$key]))
							$paras[$key] = $result;
					}
					break;
				case 'checklist':
					if(!is_array($pp_value)) {
						echo 'Error: list of parameter is not an array' . PHP_EOL;
						$founderror = true;
						break;
					}
					foreach($paras as $key => $result) {
						if(!in_array($key, $pp_value)) {
							echo 'Warning: unrecognized parameter ' . $key . PHP_EOL;
						}
					}
					break;
				default:
					echo 'Error: unrecognized parameter ' . $pp_key . PHP_EOL;
					$founderror = true;
					break;
			}
		}
		if($founderror)
			return PROCESS_PARAS_ERROR_FOUND;
		else
			return 0;
	}
	public function exec_file($file, $paras = '') {
		// open input file
		$in = fopen($file, 'r');
		if(!$in) {
			echo 'Invalid input file' . PHP_EOL;
			return false;
		}
		$this->currhist++;
		// set stuff up
		$this->curr('currscope', 0);
		$this->curr('vars', array());
		$this->curr('history', array());
		$this->curr('histlen', 0);
		$this->curr('pc', 0);
		$this->curr('flowctr', 0);
		$this->curr('flowo', array(
			'type' => 'global',
			'start' => 0,
			'execute' => true,
		));
		while(($line = fgets($in)) !== false) {
			if(!$this->driver($line)) {
				$this->currhist--;
				return false;
			}
		}
		$this->currhist--;
		return true;
	}
	public function setup_commandline($name, $paras = '') {
	// Performs various functions in a pseudo-command line. A main entry point.
	// stty stuff inspired by sfinktah at http://php.net/manual/en/function.fgetc.php
		// perhaps kill this; I never use it
		if($paras['undoable'] and !$this->hascommand('undo')) {
			$this->tmp = clone $this;
			$newcmd['name'] = 'undo';
			$newcmd['desc'] = 'Return to the previous state of the object';
			$newcmd['arg'] = 'None';
			$newcmd['execute'] = 'callmethod';
			$this->addcommand($newcmd, array('ignoreduplicates' => true));
		}
		echo 'Welcome to command line mode. Type "help" for help.' . PHP_EOL;
		// initialize stuff
		$this->curr('currscope', 0);
		$this->curr('vars', array());
		$this->curr('history', array());
		$this->curr('histlen', 0);
		$this->curr('pc', 0);
		$this->curr('flowctr', 0);
		$this->curr('flowo', array(
			'type' => 'global',
			'execute' => true,
			'start' => 0,
		));
		// offset where the cursor should go
		$promptoffset = strlen($name) + 2;
		// loop through commands
		while(true) {
			// print prompt
			echo $name . "> ";
			$cmd = $this->getline(array(
				'lines' => $this->history[$this->currhist], 
				'offset' => $promptoffset)
			);
			if($cmd === false)
				$cmd = 'quit';
			// execute the command
			if(!$this->driver($cmd)) {
				echo 'Goodbye.' . PHP_EOL;
				return;
			}
		}
	}
	private function debugecho($var) {
	// For debugging: adds output to a log file. Useful when debugging methods like fgetc()
		$file = "/Users/jellezijlstra/Dropbox/git/Common/log";
		shell_exec("echo '$var' >> $file");
	}
	private function stty($opt) {
		$cmd = "/bin/stty " . $opt;
		exec($cmd, $output, $return);
		if($return !== 0) {
			trigger_error("Failed to execute " . $cmd);
			return false;
		}
		return implode("\n", $output);
	}
	protected function configset($paras = array()) {
	// sets something in the $this->config array, which configures the EH instance
		foreach($paras as $key => $value) {
			if(array_key_exists($key, $this->config))
				$this->config[$key] = $value;
		}
	}
	abstract public function cli(); // sets up command line
	private function undo() {
		$blacklist = array('tmp', 'commands', 'synonyms', 'p', 'props');
		$vars = get_object_vars($this);
		foreach($vars as $key => $var) {
			if(in_array($key, $blacklist)) continue;
			$this->$key = $this->tmp->$key;
		}
	}
	private function listcommands() {
		echo 'Dynamic commands:' . PHP_EOL;
		foreach($this->commands as $command => $content)
			echo "\t" . $command . PHP_EOL;
		if($this->trystatic) {
			echo 'Static commands:' . PHP_EOL;
			foreach(static::${get_called_class() . '_commands'} as $command => $content) {
				echo "\t" . $command . PHP_EOL;
			}
		}
		echo 'Dynamic synonyms:' . PHP_EOL;
		foreach($this->synonyms as $from => $to)
			echo "\t" . $from . ' -> ' . $to . PHP_EOL;
		if($this->trystatic) {
			echo 'Static synonyms:' . PHP_EOL;
			foreach(static::${get_called_class() . '_synonyms'} as $from => $to)
				echo "\t" . $from . ' -> ' . $to . PHP_EOL; 
		}
	}
	protected function hascommand($cmd) {
		if($this->commands[$cmd])
			return true;
		if($this->synonyms[$cmd])
			return true;
		if($this->trystatic) {
			if(static::${get_called_class() . '_commands'}[$cmd])
				return true;
			if(static::${get_called_class() . '_synonyms'}[$cmd])
				return true;
		}
		return false;
	}
	static protected function testregex($in) {
	// tests whether a regex pattern is valid
		ob_start();
		$t = @preg_match($in, 'test');
		ob_end_clean();
		// if regex was invalid, preg_match returned FALSE
		if($t === false)
			return false;
		else
			return true;
	}
	static private function shell($in) {
		// cd won't actually change the shell until we do some special magic
		if(preg_match('/^cd /', $in)) {
			$dir = substr($in, 3);
			// handle home directory
			if($dir[0] === '~') {
				$home = trim(shell_exec('echo $HOME'));
				$dir = preg_replace('/^~/u', $home, $dir);
			}
			chdir($dir);
		}
		else
			echo shell_exec($in);
	}
	protected function myecho($in) {
		echo $in . PHP_EOL;
	}
	protected function put($in) {
	// like myecho(), but does not put newline
		echo $in;
	}
	private function driver($in) {
	// adds lines to the history array, and handles execution
		// add line to the history array
		$this->history[$this->currhist][$this->histlen[$this->currhist]] = trim($in);
		$this->curr('histlen', '++');
		// continue executing as long as PC is below length of program
		while($this->curr('pc') < $this->curr('histlen')) {
			if($this->config['debug']) 
				echo "Feeding command (" . $this->curr('pc') . "): " . 
					$this->curr('pcres') . PHP_EOL;
			$ret = $this->execute();
			switch($ret) {
				case self::EXECUTE_NEXT: 
					$this->pcinc(); 
					break;
				case self::EXECUTE_PC: 
					break;
				case self::EXECUTE_SYNTAX_ERROR:
				case self::EXECUTE_QUIT:
					return false;
			}
		}
		return true;
	}
	private function &curr($var, $set = NULL) {
		$ret = NULL;
		switch($var) {
			case 'pc': 
				$ret =& $this->pc[$this->currhist];
				break;
			case 'histlen': 
				$ret =& $this->histlen[$this->currhist];
				break;
			case 'history': 
				$ret =& $this->history[$this->currhist];
				break;
			case 'flow': 
				$ret =& $this->flow[$this->currhist];
				break;
			case 'flowo': // object pointing to current control flow block
				$ret =& $this->flow[$this->currhist][$this->flowctr[$this->currhist]];
				break;
			case 'flowctr': 
				$ret =& $this->flowctr[$this->currhist];
				break;
			case 'pco': case 'pcres': 
				$ret =& $this->history[$this->currhist][$this->pc[$this->currhist]];
				break;
			case 'currscope':
				$ret =& $this->currscope[$this->currhist];
				break;
			case 'vars':
				$ret =& $this->vars[$this->currhist][$this->currscope[$this->currhist]];
				break;
		}
		if($set === NULL) return $ret;
		// can't use switch because 0 == '++'
		if($set === '++')
			$ret++;
		else if($set === '--')
			$ret--; 
		else
			$ret = $set;
		return $ret;
	}
	private function pcinc() {
		$this->curr('pc', '++');
	}
	protected function fgetc($infile) {
	// re-implementation of fgetc that allows multi-byte characters
		// internal version of fgetc(), converting number into integer
		$fgetc = function() use($infile) {
			$out = ord(fgetc($infile));
			return $out;
		};
		// use bit mask stuff for 1- to 4-byte character detection
		$test1 = function($in) {
			$bitmasked = $in & 0x80;
			return ($bitmasked === 0x0);
		};
		$test2 = function($in) {
			$bitmasked = $in & 0xe0;
			return ($bitmasked === 0xc0);
		};
		$test3 = function($in) {
			$bitmasked = $in & 0xf0;
			return ($bitmasked === 0xe0);
		};
		$test4 = function($in) {
			$bitmasked = $in & 0xf8;
			return ($bitmasked === 0xf0);
		};
		// test validity of high-order bytes
		$testm = function($in) {
			$bitmasked = $in & 0xc0;
			return ($bitmasked === 0x80);
		};
		// get first character
		$c1 = $fgetc(STDIN);
		if($test1($c1)) {
			// Ctrl+D
			if($c1 === 4)
				return false;
			// special-case KEY_UP etcetera
			if($c1 === 27) {
				$c2 = $fgetc(STDIN);
				if($c2 !== 91)
					return false;
				$c3 = $fgetc(STDIN);
				return (chr($c1) . chr($c2) . chr($c3));
			}
			return chr($c1);
		}
		else if($test2($c1)) {
			$c2 = $fgetc(STDIN);
			if(!$testm($c2))
				return false;
			return (chr($c1) . chr($c2));
		}
		else if($test3($c1)) {
			$c2 = $fgetc(STDIN);
			if(!$testm($c2))
				return false;
			$c3 = $fgetc(STDIN);
			if(!$testm($c3))
				return false;
			return (chr($c1) . chr($c2) . chr($c3));
		}
		else if($test4($c1)) {
			$c2 = $fgetc(STDIN);
			if(!$testm($c2))
				return false;
			$c3 = $fgetc(STDIN);
			if(!$testm($c3))
				return false;
			$c4 = $fgetc(STDIN);
			if(!$testm($c4))
				return false;
			return (chr($c1) . chr($c2) . chr($c3) . chr($c4));
		}
		else
			return false;
	}
	protected function getline($paras) {
	// get a line from stdin, allowing for use of arrow keys, backspace, etc.
	// Return false upon EOF or failure.
		if(self::process_paras($paras, array(
			'checklist' => array(
				'lines', // array of lines accessed upon KEY_UP, KEY_DOWN etcetera
				'offset', // offset where prompt starts. If set to 0, this function will print '> ' as the prompt.
			),
			'default' => array(
				'lines' => array(),
			),
		)) === PROCESS_PARAS_ERROR_FOUND)
			return false;
		$promptoffset = $paras['offset'];
		// default to printing '> '
		if(!$promptoffset) {
			echo '> ';
			$promptoffset = 2;
		}
		// start value of the pointer
		$histptr = count($paras['lines']);
		// lambda function to get string-form command
		$getcmd = function() use(&$cmd, &$cmdlen) {
			// create the command in string form
			// it will sometimes be an array at this point, which will need to be imploded
			if(is_string($cmd))
				return $cmd;
			else if(is_array($cmd))
				return implode($cmd);
			else if(is_null($cmd)) // don't try to execute empty command
				return NULL;
			else {
				trigger_error("Command of unsupported type: $cmd");
				return NULL;
			}
		};
		$showcursor = function() use (&$cmdlen, &$keypos, $getcmd, $promptoffset) {
			// return to saved cursor position, clear line
			$backmove = $promptoffset + $cmdlen + 10;
			echo "\033[" . $backmove . "D\033[" . $promptoffset . "C\033[K";
			// put the command back
			echo $getcmd();
			// put the cursor in the right position
			if($cmdlen > $keypos)
				echo "\033[" . ($cmdlen - $keypos) . "D";	
		};
		// set our settings
		$this->stty('cbreak iutf8');
		// get command
		$cmd = array();
		$cmdlen = 0;
		$keypos = 0;
		while(true) {
			// get input
			$c = $this->fgetc(STDIN);
			if($c === false) {
			// if we encounter EOF or invalid UTF-8, which causes fgetc to 
			// return false, also return false
				echo PHP_EOL;
				return false;
			}
			switch($c) {
				case "\033[A": // KEY_UP
					// decrement pointer
					if($histptr > 0)
						$histptr--;
					// get new command
					$cmd = mb_str_split($paras['lines'][$histptr]);
					$cmdlen = count($cmd);
					$keypos = $cmdlen;
					break;
				case "\033[B": // KEY_DOWN
					// increment pointer
					if($histptr < $this->curr('histlen'))
						$histptr++;
					// get new command
					if($histpr < $this->curr('histlen')) {
						// TODO: get a $this->curr() method for this
						$cmd = mb_str_split($paras['lines'][$histptr]);
						$cmdlen = count($cmd);
						$keypos = $cmdlen;
					}
					else {
						// reset command
						$cmd = array();
						$cmdlen = 0;
						$keypos = 0;
					}
					break;
				case "\033[D": // KEY_LEFT
					if($keypos > 0)
						$keypos--;
					break;
				case "\033[C": // KEY_RIGHT
					if($keypos < $cmdlen)
						$keypos++;
					break;
				case "\177": // KEY_BACKSPACE
					$tmp = array();
					$nchars = $cmdlen - $keypos;
					for($i = $keypos; $i < $cmdlen; $i++) {
						$tmp[] = $cmd[$i];
					}
					$keypos--;
					for($i = 0; $i < $nchars; $i++) {
						$cmd[$keypos + $i] = $tmp[$i];
					}
					// remove killed characters, so we don't need to use substr() in $getcmd()
					for($i = $keypos + $i; $i < $cmdlen; $i++) {
						unset($cmd[$i]);
					}
					$cmdlen--;
					break;
				case "\012": // newline
					$cmd = $getcmd();
					if($cmd === NULL) return false;
					// restore sane stty settings for the duration of command execution
					$this->stty("sane");
					return $cmd;
				// more cases for Ctrl stuff needed
				default: // other characters: add to command
					// temporary array to hold characters to be moved over
					$tmp = array();
					$nchars = $cmdlen - $keypos;
					for($i = $keypos; $i < $cmdlen; $i++) {
						$tmp[] = $cmd[$i];
					}
					// add new character to command
					$cmd[$keypos] = $c;
					$cmdlen++;
					$keypos++;
					// add characters back to command
					for($i = 0; $i < $nchars; $i++) {
						$cmd[$keypos + $i] = $tmp[$i];
					}
					break;
			}
			// show command
			$showcursor();
		}
	}
	protected function menu($paras) {
	// Function that creates a menu and gets input
		if(self::process_paras($paras, array(
			'checklist' => array(
				'head', // Menu heading
				'options', // List of options. Associative array, with option in key and description in value
				'printoptions', // Always print options?
				'helpcommand', // Make help command available? (If set to true, commands beginning with "help" will not get returned.)
				'validfunction', // Function to determine validity of command
			),
			'default' => array(
				'head' => 'MENU',
				'printoptions' => false,
				'helpcommand' => true,
				'validfunction' => function($in) use(&$options) {
					return in_array($in, $options);
				},
			),
			'errorifempty' => array('options'),
		)) === PROCESS_PARAS_ERROR_FOUND) return false;
		// print menu heading
		echo $paras['head'] . PHP_EOL;
		$printoptions = function() use($paras) {
			echo 'Options available:' . PHP_EOL;
			foreach($paras['options'] as $cmd => $desc) {
				echo "-'" . $cmd . "': " . $desc . PHP_EOL;
			}
		};
		if($paras['printoptions'])
			$printoptions();
		$options = array_keys($paras['options']);
		while(true) {
			// get command
			$cmd = $this->getline(array('lines' => $options));
			if($cmd === false)
				return false;
			// provide help if necessary
			if($paras['helpcommand']) {
				// just 'help' prints all options
				if($cmd === 'help') {
					$printoptions();
					continue;
				}
				// help about a specific command
				if(substr($cmd, 0, 5) === 'help ') {
					$option = substr($cmd, 5);
					if($paras['options'][$option])
						echo $option . ': ' . $paras['options'][$option] . PHP_EOL;
					else
						echo 'Option ' . $option . ' does not exist.' . PHP_EOL;
					continue;
				}
			}
			// return command if valid
			if($paras['validfunction']($cmd)) {
				return $cmd;
			}
			echo 'Unrecognized option ' . $cmd . PHP_EOL;
		}
	}
	public function test() {
	// Test function that might do anything I currently want to test
	// Currently, testing the menu() method
		$cmd = $this->menu(array(
			'options' => array('a' => 'Do something', 'b' => 'Do something else'),
			'helpcommand' => false,
		));
		echo $cmd . PHP_EOL;
	}
}
?>
