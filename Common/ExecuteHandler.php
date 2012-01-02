<?php
/*
 * ExecuteHandler.php
 * Jelle Zijlstra, 2011 – January 2012
 *
 * The core of the ExecuteHandler framework, including functionality to keep 
 * track of commands and pre-process parameters. The actual interpretation of 
 * the EH scripting language is handled by the parent EHI class, which has been
 * implemented both in PHP and more fully in C++.
 */
define('PROCESS_PARAS_ERROR_FOUND', 0x1);
require_once(BPATH . "/Common/EHException.php");
// TODO: more effectively ignore Ctrl+P and stuff like that.
// Fix function definitions in exec_file. Currently, they just do random stuff when executed outside the exec_file context.
abstract class ExecuteHandler extends EHICore {
	/* Private properties */
	protected $commands;
	protected $synonyms;
	protected $config = array(
		'debug' => false,
	);
	// currently handled files
	protected $current;
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
			'execute' => 'callmethodarg'),
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
	// - rawarg: Bool whether the argument can be used "raw" (i.e., unprocessed)
	// - method: String method called by the command
		if(isset($this->commands[$command['name']])) {
			if(!$paras['ignoreduplicates']) trigger_error('Command ' . $command['name'] . ' already exists', E_USER_NOTICE);
			return false;
		}
		if(!self::testcommand($command)) return false;
		if(isset($command['aka'])) {
			if(!is_array($command['aka'])) {
				if(isset($this->synonyms[$command['aka']]))
					trigger_error('Error: ' . $aka . ' already exists as a synonym for ' . $this->synonyms[$aka], E_USER_NOTICE);
				else
					$this->synonyms[$command['aka']] = $command['name'];
			}
			else foreach($command['aka'] as $aka) {
				if(isset($this->synonyms[$aka]))
					trigger_error('Error: ' . $aka . ' already exists as a synonym for ' . $this->synonyms[$aka], E_USER_NOTICE);
				else
					$this->synonyms[$aka] = $command['name'];
			}
		}
		if(!isset($command['setcurrent']))
			$command['setcurrent'] = false;
		if(!isset($command['rawarg']))
			$command['rawarg'] = false;
		$this->commands[$command['name']] = $command;
		return true;
	}
	static private function testcommand($command) {
		/* check and handle input */
		// if we don't have those, little point in proceeding
		if(!isset($command['name'])) {
			trigger_error('No name given for new command', E_USER_NOTICE);
			return false;
		}
		if(!isset($command['execute']) or !array_key_exists($command['execute'], self::$handlers)) {
			trigger_error('No valid execute sequence given for new command ' . $command['name'], E_USER_NOTICE);
			return false;
		}
		// warn if no documentation
		if(!isset($command['desc'])) {
			trigger_error('No documentation given for new command ' . $command['name'], E_USER_NOTICE);
		}
		if(!isset($command['arg'])) {
			trigger_error('No listing of arguments given for new command ' . $command['name'], E_USER_NOTICE);
		}
		return true;
	}
	public function execute_cmd($rawcmd, $paras) {
		// for some reason, accessing rawcmd in ehi mode botches the variable. This works around that.
		$rawcmd2 = $rawcmd;
		$cmd = $this->expand_cmd($rawcmd2);
		if(!$cmd) {
			echo 'Invalid command: ' . $rawcmd2 . PHP_EOL;
			return self::EXECUTE_NEXT;
		}
		$redirection = array(
			'>' => false,
			'>$' => false,
			'}' => false,
			'}$' => false,
		);
		// separate argument from paras
		if(isset($paras[0]))
			$argument = trim($paras[0]);
		else
			$argument = '';
		// separate output redirection and friends
		foreach($redirection as $key => $var) {
			if(isset($paras[$key])) {
				$redirection[$key] = $paras[$key];
				unset($paras[$key]);
			}
		}
		// handle shortcut
		if($argument === '*')
			$argarray = $this->current;
		else
			$argarray = array($argument);
		// output redirection
		if(($redirection['>'] !== false or $redirection['>$'] !== false) and
			$cmd['execute'] !== 'quit') {
			ob_start();
		}
		// return value of executed command
		$ret = NULL;
		// execute it
		switch($cmd['execute']) {
			case 'doallorcurr':
				if(count($argarray) > 0 and $argarray[0] !== '') {
					foreach($argarray as $file)
						if(!($ret = $this->{$cmd['name']}($file, $paras)))
							break;
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
		if($redirection['>'] !== false) {
			$file = fopen($redirection['>'], 'w');
			if(!$file) {
				trigger_error('Invalid rediction file: ' . $outputredir, E_USER_NOTICE);
				ob_end_clean();
				return self::EXECUTE_NEXT;
			}
			fwrite($file, ob_get_contents());
			ob_end_clean();
		}
		else if($redirection['>$'] !== false) {
			$this->setvar($redirection['>$'], ob_get_contents());
			ob_end_clean();
		}
		if($redirection['}']) {
			$file = fopen($redirection['}'], 'w');
			if(!$file) {
				trigger_error('Invalid rediction file: ' . $outputredir, E_USER_NOTICE);
				ob_end_clean();
				return self::EXECUTE_NEXT;
			}
			fwrite($file, $ret);
		}
		// no else here; having both returnredir and returnredirvar makes sense
		if($redirection['}$']) {
			$this->setvar($redirect['}$'], $ret);
		}
		// always make return value accessible to script
		$this->setvar('ret', $ret);
		return self::EXECUTE_NEXT;
	}
	private function expand_cmd($in) {
		// search for commands
		if(isset($this->synonyms[$in]))
			$in = $this->synonyms[$in];
		if(isset($this->commands[$in]))
			return $this->commands[$in];
		else
			return false;
	}
	private function execute_help($in) {
		// array of functions with info
		if(!$in) {
			echo 'In command line, various options can be used to manipulate the list or its files. The following commands are available:' . PHP_EOL;
			$this->listcommands();
			echo 'Type "help <command>" to get more information about that commmand. Commands will sometimes take an argument, often a filename.' . PHP_EOL;
			return true;
		}
		$in = $this->expand_cmd($in);
		if($in) {
			echo PHP_EOL . 'Function: ' . $in['name'] . PHP_EOL;
			if(isset($in['aka'])) {
				if(is_array($in['aka']))
					echo 'Aliases: ' . implode(', ', $in['aka']) . PHP_EOL;
				else if(is_string($in['aka']))
					echo 'Aliases: ' . $in['aka'] . PHP_EOL;
			}
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
	// deprecated; use process_paras instead
		if(isset($paras[$field]))
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
						if(isset($paras[$key]) and !isset($paras[$result])) {
							$paras[$result] = $paras[$key];
							unset($paras[$key]);
						}
					}
					break;
				case 'askifempty':
					if(!is_array($pp_value))
						$pp_value = array($pp_value);
					foreach($pp_value as $key) {
						if(!isset($paras[$key])) {
							$paras['key'] = $this->getline(array(
								'prompt' => $key . ': ',
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
		echo 'Commands:' . PHP_EOL;
		foreach($this->commands as $command => $content)
			echo "\t" . $command . PHP_EOL;
		echo 'Synonyms:' . PHP_EOL;
		foreach($this->synonyms as $from => $to)
			echo "\t" . $from . ' -> ' . $to . PHP_EOL;
	}
	protected function hascommand($cmd) {
		if(isset($this->commands[$cmd]))
			return true;
		if(isset($this->synonyms[$cmd]))
			return true;
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
	protected function getline($paras = array()) {
	// get a line from stdin, allowing for use of arrow keys, backspace, etc.
	// Return false upon EOF or failure.
		// common use case
		if(is_string($paras))
			$paras = array('prompt' => $paras);
		if(self::process_paras($paras, array(
			'checklist' => array(
				'lines', // array of lines accessed upon KEY_UP, KEY_DOWN etcetera
				'prompt', // Prompt to be printed.
			),
			'default' => array(
				'lines' => array(),
				'prompt' => '> ',
			),
		)) === PROCESS_PARAS_ERROR_FOUND)
			return false;
		$promptoffset = strlen($paras['prompt']);
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
			// first move as far west as we can; 200 positions should suffice
			echo "\033[200D\033[" . $promptoffset . "C\033[K";
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
		echo $paras['prompt'];
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
					if(!isset($paras['lines'][$histptr]))
						$cmd = array();
					else
						$cmd = mb_str_split($paras['lines'][$histptr]);
					$cmdlen = count($cmd);
					$keypos = $cmdlen;
					break;
				case "\033[B": // KEY_DOWN
					// increment pointer
					if($histptr < $this->curr('histlen'))
						$histptr++;
					// get new command
					if($histptr < $this->curr('histlen')) {
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
				case "\001": // Ctrl+A
				case "\002": // Ctrl+B
				case "\005": // Ctrl+E
				case "\006": // Ctrl+F
				case "\007": // Ctrl+G
				case "\010": // Ctrl+H
				case "\011": // Ctrl+I
				case "\013": // Ctrl+K
				case "\014": // Ctrl+L
				case "\016": // Ctrl+N
				case "\020": // Ctrl+P
				case "\022": // Ctrl+R
				case "\024": // Ctrl+T
				case "\025": // Ctrl+U
				case "\026": // Ctrl+V
				case "\027": // Ctrl+W
				case "\030": // Ctrl+X
				case "\033": // Ctrl+[
				case "\035": // Ctrl+]
					break;
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
				'process', // Array of callbacks to execute when a given option is called.
				'processcommand', // Function used to process the command after input
			),
			'default' => array(
				'head' => 'MENU',
				'printoptions' => false,
				'helpcommand' => true,
				'validfunction' => function($in, $options) {
					return in_array($in, $options);
				},
				'process' => array(),
				'processcommand' => false,
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
			if($paras['processcommand'])
				$cmd = $paras['processcommand']($cmd);
			// return command if valid
			if($paras['validfunction']($cmd, $options)) {
				if(array_key_exists($cmd, $paras['process'])) {
					$paras['process'][$cmd]();
				}
				else
					return $cmd;
			}
			else
				echo 'Unrecognized option ' . $cmd . PHP_EOL;
		}
	}
	protected function ynmenu($head, $process = NULL) {
	// Make a yes-no menu
		return $this->menu(array(
			'options' => array(
				'y' => 'Yes',
				'n' => 'No',
			),
			'head' => $head,
			'process' => $process,
			'processcommand' => function($in) {
				switch($in) {
					case 'y': case 'yes': case 'Yes': case 'Y': return 'y';
					case 'n': case 'no': case 'No': case 'N': return 'n';
					default: return $in;
				}
			},
		));
	}
	public function test() {
	// Test function that might do anything I currently want to test
	// Currently, testing what arguments it is getting
		$this->stty('cbreak iutf8');
		while(1) {
			$char = ord(fgetc(STDIN));
			echo PHP_EOL;
			echo $char . ' : ' . base_convert($char, 10, 8) . PHP_EOL;
			if($char === 4) break;
		}
		var_dump(func_get_args());
	}
}
?>
