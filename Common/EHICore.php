<?php
/*
 * EHI.php
 * Jelle Zijlstra, January 2012
 *
 * Implementation of the EH scripting language in PHP, to be used when the C++
 * implementation (ehi, ehphp) is unavailable.
 */

define('IS_PUREPHP', 0);
abstract class EHICore implements EHICoreInterface {
	/* Class constants (partly duplicated) */
	const EXECUTE_NEXT = 0x0; // execute says: go on with next
	const EXECUTE_PC = 0x1; // execute whatever is in the PC now
	const EXECUTE_SYNTAX_ERROR = 0x2; // execute returned syntax error
	const EXECUTE_QUIT = 0x3; // execute asked to quit the program
	const EVALUATE_ERROR = 0x1;
	const EVALUATE_FUNCTION_CALL = 0x2;
	const CHECK_FLOW_IN_IF = 0x1;
	const CHECK_FLOW_IN_FOR = 0x2;
	const CHECK_FLOW_IN_FUNC = 0x3;
	const CHECK_FLOW_IN_WHILE = 0x4;
	const CHECK_FLOW_NOT_EXECUTING_IN_IF = 0x5;
	/* core commands */
	protected static $core_commands = array(
		'myecho' => array('name' => 'myecho',
			'aka' => 'echo',
			'desc' => 'Echo input',
			'arg' => 'Text to be echoed',
			'execute' => 'callmethod'),
		'put' => array('name' => 'put',
			'desc' => 'Print input to terminal',
			'arg' => 'Text to be printed',
			'execute' => 'callmethod'),
		'exec_file' => array('name' => 'exec_file',
			'desc' => 'Execute a series of commands from a file',
			'arg' => 'File path',
			'execute' => 'callmethod'),
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
	// array of functions that shell code is allowed to call
	private static $php_funcs = array(
		'strlen',
		'substr',
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
	public function __construct() {
		return;
	}
	public function setvar($var, $value) {
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
				if(!isset($this->funcs[$funcname])) {
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
				$func = $this->funcs[$funcname];
				$argcount = count($vars);
				if($argcount !== $func['argcount']) {
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
				if(!isset($this->funcs[$funcname])) {
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
				else {
					$func = $this->funcs[$funcname];
					$this->retline = $this->curr('pc');
					$this->evaluate_ret = self::EVALUATE_FUNCTION_CALL;
					$this->curr('pc', $func['line']);
					return NULL;
				}
			}
		}
		// math
		else if(preg_match(
			"/^(?![\"'])([^\s]*)\s*([+\-*\/=><]|!=|!==|>=|<=|==)\s*([^\s]*)(?<![\"'])$/u",
			$in,
			$matches)) {
			$lval = (float) trim($matches[1]);
			$rval = (float) trim($matches[3]);
			switch($matches[2]) {
				case '+': return $lval + $rval;
				case '-': return $lval - $rval;
				case '*': return $lval * $rval;
				case '/': return $lval / $rval;
				case '=':
					// loose comparison is intentional here
					return ($lval == $rval);
				case '==': return ($lval === $rval);
				case '>': return ($lval > $rval);
				case '<': return ($lval < $rval);
				case '!=':
					// loose comparison is intentional here
					return ($lval != $rval);
				case '!==': return ($lval !== $rval);
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
	private function divide_cmd($arg) {
	// transforms an argument string into a $paras array, taking care of quoted strings and output redirection
		$endcmd = function($context) use(&$i, &$len) {
			if($i === $len) {
				throw new EHException(
					"Unexpected end of command while in " . $context,
					EHException::E_RECOVERABLE
				);
			}
		};
		$len = strlen($arg);
		$next = false;
		$key = 0; // array key, either 0 for the argument or a para name
		// initialize
		$paras[0] = '';
		for($i = 0; $i < $len; $i++) {
			if($arg[$i] === ' ' and ($i === 0 or $arg[$i-1] !== '\\')) {
				// add space to separate parts of $argument
				if($key === 0)
					$paras[0] .= ' ';
				$key = 0;
			}
			// handle parameter names
			else if($arg[$i] === '-' and ($i === 0 or $arg[$i-1] === ' ')) {
				$i++;
				$endcmd("parameter definition");
				// short-form or long-form?
				// long form
				if($arg[$i] === '-') {
					$i++;
					$key = '';
					for( ; $arg[$i] !== '='; $i++) {
						$endcmd("parameter name");
						$key .= $arg[$i];
					}
					$paras[$key] = '';
				}
				// short form
				else {
					for( ; $i < $len and $arg[$i] !== ' '; $i++) {
						if(in_array($arg[$i], array('=', '"', "'"))) {
							throw new EHException(
								"Unexpected {$arg[$i]}",
								EHException::E_RECOVERABLE);
						}
						$paras[$arg[$i]] = true;
					}
				}
			}
			// output and return redirection (handled as special key)
			else if(($arg[$i] === '>' or $arg[$i] === '}') and ($i === 0 or $arg[$i-1] === ' ')) {
				$var = $arg[$i];
				if($arg[$i+1] === '$') {
					$i++;
					$key = $var . '$';
				}
				else {
					$key = $var;
				}
				// ignore space
				if($arg[$i+1] === ' ')
					$i++;
			}
			else if($arg[$i] === "'" and ($i === 0 or $arg[$i-1] !== '\\')) {
				// consume characters until end of quoted string
				$i++;
				for( ; ; $i++) {
					$endcmd("single-quoted string");
					if($arg[$i] === "'" and $arg[$i-1] !== '\\')
						break;
					$paras[$key] .= $arg[$i];
				}
			}
			else if($arg[$i] === '"' and ($i === 0 or $arg[$i-1] !== '\\')) {
				// consume characters until end of quoted string
				$i++;
				for( ; ; $i++) {
					$endcmd("double-quoted string");
					if($arg[$i] === '"' and $arg[$i-1] !== '\\')
						break;
					$paras[$key] .= $arg[$i];
				}
			}
			else
				$paras[$key] .= $arg[$i];
		}
		return $paras;
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
	// execute a single command
		// by default, take current value of PC
		if($in === NULL) {
			$in = $this->curr('pcres');
		}
		// handle empty commands and ehi-style comments (do nothing)
		if($in === '' or $in[0] === '#') {
			return self::EXECUTE_NEXT;
		}
		// divide input into keyword and argument
		preg_match("/^\s*([^\s]+)(\s+(.*))?\s*\$/u", $in, $matches);
		$rawcmd = $matches[1];
		$rawarg = isset($matches[3]) ? $matches[3] : '';
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
		if($this->config['debug'])
			echo 'Executing command: ' . $rawcmd . PHP_EOL;
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
				if(isset($this->funcs[$name])) {
					$f = $this->funcs[$name];
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
				// return value is the same as the argument; does not get
				// evaluated. If there is no argument, we return NULL.
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
					// we reached end of an executing function, so return
					// NULL
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
				if($this->flowctr === 0) {
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
		// now we're looking only at EH-defined commands, not language constructs
		// split command into pieces
		$paras = $this->divide_cmd($arg);
		return $this->execute_cmd($rawcmd, $paras);
	}
	protected function &curr($var, $set = NULL) {
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
	protected function init_ehi() {
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
	}
	protected function driver($in) {
	// adds lines to the history array, and handles execution
		// add line to the history array
		$this->history[$this->currhist][$this->histlen[$this->currhist]] = trim($in);
		$this->curr('histlen', '++');
		// continue executing as long as PC is below length of program
		while($this->curr('pc') < $this->curr('histlen')) {
			if($this->config['debug'])
				echo "Feeding command (" . $this->curr('pc') . "): " .
					$this->curr('pcres') . PHP_EOL;
			try {
				$ret = $this->execute();
			}
			catch(EHException $e) {
				echo "Error '" . $e->getMessage() . "' occurred while executing command '" . $in . "'" . PHP_EOL;
				$ret = ExecuteHandler::EXECUTE_NEXT;
			}
			switch($ret) {
				case ExecuteHandler::EXECUTE_NEXT:
					$this->pcinc();
					break;
				case ExecuteHandler::EXECUTE_PC:
					break;
				case ExecuteHandler::EXECUTE_SYNTAX_ERROR:
				case ExecuteHandler::EXECUTE_QUIT:
					return false;
			}
		}
		return true;
	}
	public function setup_commandline($name, array $paras = array()) {
	// Performs various functions in a pseudo-command line. A main entry point.
	// stty stuff inspired by sfinktah at http://php.net/manual/en/function.fgetc.php
		// can't use process_paras now because that is in ExecuteHandler
		if(!is_array($paras)) return false;
		if(!isset($paras['undoable'])) $paras['undoable'] = false;
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
		$this->init_ehi();
		// loop through commands
		while(true) {
			// get input
			$cmd = $this->getline(array(
				'lines' => $this->curr('history'),
				'prompt' => $name . '> ')
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
	/* commands used only in pure-PHP EH */
	public function exec_file(array $paras = array()) {
		if($this->process_paras($paras, array(
			'name' => __FUNCTION__,
			'synonyms' => array(0 => 'file'),
			'checklist' => array('file' => 'File to open'),
			'errorifempty' => array('file'),
		)) === PROCESS_PARAS_ERROR_FOUND) return false;
		// open input file
		$in = fopen($paras['file'], 'r');
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
	protected function myecho(array $paras) {
		$first = true;
		foreach($paras as $value) {
			if(!$first)
				echo ' ';
			$first = false;
			echo $value;
		}
		echo PHP_EOL;
	}
	protected function put(array $paras) {
	// like myecho(), but does not put newline
		$first = true;
		foreach($paras as $value) {
			if(!$first)
				echo ' ';
			$first = false;
			echo $value;
		}
	}
}
