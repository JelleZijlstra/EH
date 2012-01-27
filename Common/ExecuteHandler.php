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
	/* Class constants */
	const EXECUTE_NEXT = 0x0; // execute says: go on with next
	const EXECUTE_PC = 0x1; // execute whatever is in the PC now
	const EXECUTE_SYNTAX_ERROR = 0x2; // execute returned syntax error
	const EXECUTE_QUIT = 0x3; // execute asked to quit the program
	/* Private properties */
	protected $commands;
	protected $synonyms;
	protected $config = array(
		'debug' => false,
	);
	// currently handled files
	protected $current;
	private static $ExecuteHandler_commands = array(
		'execute_help' => array('name' => 'execute_help',
			'aka' => array('h', 'help', 'man'),
			'desc' => 'Get help information about the command line or a specific command',
			'arg' => 'Command name',
			'execute' => 'callmethod'),
		'listcommands' => array('name' => 'listcommands',
			'desc' => 'List legal commands',
			'arg' => 'None',
			'execute' => 'callmethod'),
		'shell' => array('name' => 'shell',
			'aka' => 'exec_catch',
			'desc' => 'Execute a command from the shell',
			'arg' => 'Shell command',
			'execute' => 'callmethod'),
		'configset' => array('name' => 'configset',
			'aka' => 'setconfig',
			'desc' => 'Set a configuration variable',
			'arg' => 'Variables to be set',
			'execute' => 'callmethod'),
		'switchcli' => array('name' => 'switchcli',
			'aka' => array('switch'),
			'desc' => 'Switch to a different command line',
			'arg' => 'Name of command line to switch to',
			'execute' => 'callmethod'),
		'print_paras' => array('name' => 'print_paras',
			'desc' => 'Print its arguments',
			'arg' => 'As many as you want',
			'execute' => 'callmethod'),
		'test' => array('name' => 'test',
			'desc' => 'Do something random',
			'arg' => 'None',
			'execute' => 'callmethod'),
	);
	public function __construct($commands) {
		parent::__construct();
		$this->setup_ExecuteHandler($commands);
	}
	public function setup_ExecuteHandler($commands = NULL) {
	// sets up a handler.
	// @param commands Array of commands to be added to the object's library
		$this->current = array();
		foreach(self::$ExecuteHandler_commands as $command)
			$this->addcommand($command);
		foreach(parent::$core_commands as $command)
			$this->addcommand($command);
		if($commands) foreach($commands as $command)
			$this->addcommand($command);
	}
	public function addcommand($command, array $paras = array()) {
	// adds a command to the object's library
	// @param command Array of data forming a command
	// command can have the following components
	// - name: String basic name of the command
	// - aka: Array synonyms
	// - desc: String description of what the command does
	// - arg: String description of the arguments the command takes
		if(isset($this->commands[$command['name']])) {
			if(!isset($paras['ignoreduplicates']))
				trigger_error('Command ' . $command['name'] . ' already exists', E_USER_NOTICE);
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
		if($rawcmd2 === 'quit') {
			// execute "quit" immediately. No need to account for ehphp here,
			// because "quit" is built in to the language.
			return self::EXECUTE_QUIT;
		}
		$cmd = $this->expand_cmd($rawcmd2);
		if(!$cmd) {
			echo 'Invalid command: ' . $rawcmd2 . PHP_EOL;
			if(defined('IS_EHPHP'))
				return NULL;
			else
				return self::EXECUTE_NEXT;
		}
		$redirection = array(
			'>' => false,
			'>$' => false,
			'}' => false,
			'}$' => false,
		);
		// separate output redirection and friends
		foreach($redirection as $key => $var) {
			if(isset($paras[$key])) {
				$redirection[$key] = $paras[$key];
				unset($paras[$key]);
			}
		}
		// handle shortcut
		if(isset($paras[0]) and $paras[0] === '*') {
			unset($paras[0]);
			foreach($this->current as $file)
				$paras[] = $file;
		}
		// output redirection
		if($redirection['>'] !== false or $redirection['>$'] !== false) {
			ob_start();
		}
		// execute the command
		$ret = $this->{$cmd['name']}($paras);
		// always make return value accessible to script
		$this->setvar('ret', $ret);

		// from here, return $ret if we're in ehphp, but EXECUTE_NEXT if we're in the pure-PHP implementation
		if(defined('IS_EHPHP'))
			$returnvalue = $ret;
		else
			$returnvalue = self::EXECUTE_NEXT;
		if($redirection['>'] !== false) {
			$file = fopen($redirection['>'], 'w');
			if(!$file) {
				trigger_error('Invalid rediction file: ' . $outputredir, E_USER_NOTICE);
				ob_end_clean();
				return $returnvalue;
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
				return $returnvalue;
			}
			fwrite($file, $ret);
		}
		// no else here; having both returnredir and returnredirvar makes sense
		if($redirection['}$']) {
			$this->setvar($redirect['}$'], $ret);
		}
		return $returnvalue;
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
	private function execute_help(array $paras = array()) {
		if($this->process_paras($paras, array(
			'name' => __FUNCTION__,
			'synonyms' => array(0 => 'cmd'),
			'checklist' => array('cmd' => 'Command to explain'),
			'default' => array('cmd' => ''),
		)) === PROCESS_PARAS_ERROR_FOUND) return false;
		// array of functions with info
		if($paras['cmd'] === '') {
			echo 'In command line, various options can be used to manipulate the list or its files. The following commands are available:' . PHP_EOL;
			$this->listcommands();
			echo 'Type "help <command>" to get more information about that commmand. Commands will sometimes take an argument, often a filename.' . PHP_EOL;
			return true;
		}
		$cmd = $this->expand_cmd($paras['cmd']);
		if($cmd) {
			echo PHP_EOL . 'Function: ' . $cmd['name'] . PHP_EOL;
			if(isset($cmd['aka'])) {
				if(is_array($cmd['aka']))
					echo 'Aliases: ' . implode(', ', $cmd['aka']) . PHP_EOL;
				else if(is_string($cmd['aka']))
					echo 'Aliases: ' . $cmd['aka'] . PHP_EOL;
			}
			echo 'Description: ' . $cmd['desc'] . PHP_EOL;
			echo 'Arguments: ' . $cmd['arg'] . PHP_EOL;
			return true;
		}
		else {
			echo 'No such command' . PHP_EOL;
			return false;
		}
	}
	static protected function setifneeded(&$paras, $field) {
	// deprecated; use process_paras instead
		if(isset($paras[$field]))
			return;
		$paras[$field] = getinput_label(ucfirst($field));
	}
	static protected function printvar($in) {
		// print a variable in human-readable form.
		if($in === true) {
			echo '(true)';
		} elseif($in === false) {
			echo '(false)';
		} elseif($in === NULL) {
			echo '(null)';
		} elseif($in === '') {
			echo '(empty string)';
		} else {
			print_r($in);
		}
	}
	protected function process_paras(&$paras, $pp_paras, &$split = NULL) {
		/*
		 *    int ExecuteHandler::process_paras(array &$paras, array $pp_paras,
		 *      array &$split = NULL);
		 *
		 * This method processes a function's $paras array in the way specified
		 * in the $pp_paras parameter. In addition, it will print a summary of
		 * the method's usage if $paras['help'] is set. process_paras() will
		 * return 0 if it is successful and PROCESS_PARAS_ERROR_FOUND if it has
		 * detected an error in its input.
		 *
		 * $pp_paras is an associative array with the following members:
		 *   'name': Name of the calling function. This is used to generate the
		 *      information printed by $paras['help'].
		 *   'toarray': If $paras is not an array, convert it to an array with
		 *      the non-array value associated with the key given by
		 *      $pp_paras['toarray'].
		 *   'synonyms': An associative array where the keys denote synonyms
		 *      and the values the normalized names for parameters in $paras.
		 *      For example, if 'synonyms' contains array('f' => 'force'), the
		 *      value in $paras['f'] will be moved to $paras['force'].
		 *   'checklist': An associative array with the names of parameters in
		 *      the keys and a description of their usage in the values. Any
		 *      parameter in $paras that is not a key in 'checklist' will
		 *      generate an error (except if 'checkfunc' is set; see below). The
		 *      descriptions are used by $paras['help'].
		 *   'checkfunc': A function that is called if process_paras()
		 *      encounters a key in $paras that is not in 'checklist'. If the
		 *      functions returns true, the key is accepted; if not, an error
		 *      is thrown.
		 *   'default': An associative array where the key is a $paras key and
		 *      the value is a default value. If the key is not set in $paras,
		 *      the default value is inserted.
		 *   'errorifempty': An array of keys. If any of the keys in this array
		 *      is not set in $paras, an error is thrown.
		 *   'askifempty': Similar to 'errorifempty', but instead of throwing
		 *      an error, process_paras() asks the user to provide a value for
		 *      the parameter.
		 *   'checkparas': An associative array where the key is $paras key and
		 *      the value is a function. process_paras() will call the function
		 *      with the value for the key in $paras as its argument, and will
		 *      throw an error if the function returns false. The default
		 *      value of a parameter is always accepted.
		 *    'split': An array of paras that are to be split off into a
		 *      separate array, the $split argument. This is useful when a
		 *      method passes some of its parameters to another method, but also
		 *      takes some parameters itself. If just set to "true", this
		 *      parameter will split off all paras listed in 'checklist'.
		 *
		 * The order of the $pp_paras members is significant, because they will
		 * be executed sequentially. For example, if 'checklist' is placed
		 * before 'synonyms', 'checklist' will throw an error for synonyms
		 * that are not listed separately in 'checklist'.
		 */
		if(!is_array($pp_paras)) {
			// bogus input
			echo 'process_paras: error: invalid parameters' . PHP_EOL;
			return PROCESS_PARAS_ERROR_FOUND;
		}
		if(!is_array($paras)) {
			// apply 'toarray'
			if(isset($pp_paras['toarray'])) {
				$paras = array($pp_paras['toarray'] => $paras);
			} else {
				echo 'process_paras: error: $paras is not an array' . PHP_EOL;
				return PROCESS_PARAS_ERROR_FOUND;
			}
		}
		// special parameter in all cases: help
		if(isset($paras['help'])) {
			if(isset($pp_paras['name'])) {
				$this->execute_help(array($pp_paras['name']));
				unset($pp_paras['name']);
			}
			// without checklist, we can't do much
			if(!isset($pp_paras['checklist']))
				return PROCESS_PARAS_ERROR_FOUND;
			echo 'Parameters:' . PHP_EOL;
			foreach($pp_paras['checklist'] as $name => $description) {
				echo '- ' . $name . PHP_EOL;
				echo $description . PHP_EOL;
				if(isset($pp_paras['default'][$name])) {
					echo 'Default: ';
					self::printvar($pp_paras['default'][$name]);
					echo PHP_EOL;
				}
				if(isset($pp_paras['errorifempty']) and in_array($name, $pp_paras['errorifempty'])) {
					echo 'This parameter is required.' . PHP_EOL;
				}
				if(isset($pp_paras['askifempty']) and in_array($name, $pp_paras['askifempty'])) {
					echo 'This parameter is required; if it is not set, the user will be asked to provide a value.' . PHP_EOL;
				}
				echo PHP_EOL;
			}
			if(isset($pp_paras['synonyms'])) {
				echo 'Synonyms:' . PHP_EOL;
				foreach($pp_paras['synonyms'] as $key => $value)
					echo $key . ' -> ' . $value . PHP_EOL;
			}
			// return "false": caller should stop after process_paras call
			return PROCESS_PARAS_ERROR_FOUND;
		}
		$founderror = false;
		$showerror = function($msg) use($pp_paras, &$founderror) {
			if(isset($pp_paras['name'])) {
				echo $pp_paras['name'] . ': ';
			}
			echo 'error: ' . $msg . PHP_EOL;
			$founderror = true;
		};
		foreach($pp_paras as $pp_key => $pp_value) {
			switch($pp_key) {
				case 'synonyms': // rename paras
					if(!is_array($pp_value)) {
						$showerror('synonyms parameter is not an array');
						break;
					}
					foreach($pp_value as $key => $result) {
						if(isset($paras[$key]) and !isset($paras[$result])) {
							$paras[$result] = $paras[$key];
							unset($paras[$key]);
						}
					}
					break;
				case 'askifempty': // if a para is empty, ask user for input
					if(!is_array($pp_value)) {
						$pp_value = array($pp_value);
					}
					foreach($pp_value as $key) {
						if(!isset($paras[$key])) {
							// paras for the menu() call
							$menu_paras = array(
								'head' => $key . ': ',
								'options' => array('q'),
							);
							// use checkparas validation if possible
							if(isset($pp_paras['checkparas'][$key])) {
								$menu_paras['validfunction'] = $pp_paras['checkparas'][$key];
							}
							// else accept anything
							else {
								$menu_paras['validfunc'] = function($in) {
									return true;
								};
							}
							$paras[$key] = $this->menu($menu_paras);
							if($paras[$key] === 'q')
								$founderror = true;
						}
					}
					break;
				case 'errorifempty': // if a para is empty, throw an error
					if(!is_array($pp_value)) {
						$pp_value = array($pp_value);
					}
					foreach($pp_value as $key) {
						if(!isset($paras[$key])) {
							$showerror('parameter "' . $key . '" should be set');
						}
					}
					break;
				case 'default': // set default values for paras
					if(!is_array($pp_value)) {
						$showerror('default parameter is not an array');
						break;
					}
					foreach($pp_value as $key => $result) {
						if(!isset($paras[$key]))
							$paras[$key] = $result;
					}
					break;
				case 'checklist': // check that all paras given are legal
					if(!is_array($pp_value)) {
						$showerror('checklist parameter is not an array');
						break;
					}
					foreach($paras as $key => $result) {
						if(!array_key_exists($key, $pp_value)) {
							// if the check function returns true, do not warn
							if(isset($pp_paras['checkfunc']) and $pp_paras['checkfunc']($key)) {
								continue;
							}
							// for now, ignore para 0, which is automatically set
							if($key === 0) {
								continue;
							}
							$showerror('unrecognized parameter "' . $key . '"');
						}
					}
					break;
				case 'checkparas': // functions used to check the validity of input for a given para
					if(!is_array($pp_value)) {
						$showerror('checkparas parameter is not an array');
						break;
					}
					foreach($pp_value as $para => $func) {
						// errorifempty may already have yelled
						if(!isset($paras[$para])) {
							continue;
						}
						// always accept default value
						if(isset($pp_paras['default'][$para]) and ($paras[$para] === $pp_paras['default'][$para])) {
							continue;
						}
						if(!$func($paras[$para], $paras)) {
							$showerror('invalid value "' . $paras[$para] . '" for parameter "' . $para . '"');
						}
					}
					break;
				case 'split': // transfer paras
					if(!is_array($split)) {
						$showerror('split argument is not an array');
						break;
					}
					$handlepara = function($para) use(&$paras, &$split) {
						if(isset($paras[$para])) {
							$split[$para] = $paras[$para];
							unset($paras[$para]);
						} else {
							// with good pp_paras set, this will happen only if
							// errorifempty has already yelled.
							$split[$para] = NULL;
						}
					};
					if($pp_value === true) {
						foreach($pp_paras['checklist'] as $para => $desc) {
							$handlepara($para);
						}
					} elseif(!is_array($pp_value)) {
						$showerror('split parameter is not an array');
					} else {
						foreach($pp_value as $para) {
							$handlepara($para);
						}
					}
					break;
				case 'checkfunc': // function used to check paras that are not matched by checklist
				case 'name': // name of method calling process_paras
				case 'toarray':
					// ignore, used internally in other places
					break;
				default:
					$showerror('unrecognized process_paras parameter ' . $pp_key);
					break;
			}
		}
		if($founderror)
			return PROCESS_PARAS_ERROR_FOUND;
		else
			return 0;
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
	protected function configset(array $paras) {
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
		// sort commands and synonyms first
		ksort($this->commands);
		ksort($this->synonyms);
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
	private function shell(array $paras) {
		if($this->process_paras($paras, array(
			'name' => __FUNCTION__,
			'synonyms' => array(0 => 'cmd'),
			'errorifempty' => array('cmd'),
		)) === PROCESS_PARAS_ERROR_FOUND) return false;
		// cd won't actually change the shell until we do some special magic
		if(preg_match('/^cd /', $paras['cmd'])) {
			$dir = substr($paras['cmd'], 3);
			// handle home directory
			if($dir[0] === '~') {
				$home = trim(shell_exec('echo $HOME'));
				$dir = preg_replace('/^~/u', $home, $dir);
			}
			chdir($dir);
		}
		else
			echo shell_exec($paras['cmd']);
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
		if($this->process_paras($paras, array(
			'name' => __FUNCTION__,
			'toarray' => 'prompt',
			'checklist' => array(
				'lines' =>
					'Array of lines that can be accessed when KEY_UP and KEY_DOWN are pressed',
				'prompt' =>
					'Prompt to be printed',
				'includenewlines' =>
					'Whether to include newlines in the line returned',
			),
			'default' => array(
				'lines' => array(),
				'prompt' => '> ',
				'includenewlines' => false,
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
						$cmd = mb_str_split(trim($paras['lines'][$histptr]));
					$cmdlen = count($cmd);
					$keypos = $cmdlen;
					break;
				case "\033[B": // KEY_DOWN
					// increment pointer
					if(isset($paras['lines'][$histptr]))
						$histptr++;
					// get new command
					if(isset($paras['lines'][$histptr])) {
						// TODO: get a $this->curr() method for this
						$cmd = mb_str_split(trim($paras['lines'][$histptr]));
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
					if($paras['includenewlines'])
						$cmd[$cmdlen++] = "\n";
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
	protected function menu(array $paras) {
	// Function that creates a menu and gets input
		if($this->process_paras($paras, array(
			'name' => __FUNCTION__,
			'checklist' => array(
				'head' =>
					'Menu heading',
				'options' =>
					'List of options. Associative array, with option in key and description in value',
				'printoptions' =>
					'Whether options should always be printed',
				'helpcommand' =>
					'Whether to make the help command available. (If set to true, commands beginning with "help" will not get returned.)',
				'validfunction' =>
					'Function to determine validity of command',
				'process' =>
					'Array of callbacks to execute when a given option is called',
				'processcommand' =>
					'Function used to process the command after input',
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
				echo 'Invalid value ' . $cmd . PHP_EOL;
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
	public function switchcli(array $paras) {
		if($this->process_paras($paras, array(
			'name' => __FUNCTION__,
			'synonyms' => array(0 => 'to'),
			'checklist' => array('to' => 'CLI to switch to'),
			'errorifempty' => array('to'),
		)) === PROCESS_PARAS_ERROR_FOUND) return false;
		$to = $paras['to'];
		global ${$to};
		if(!${$to} or !is_object(${$to}) or !method_exists(${$to}, 'cli')) {
			echo 'No such variable or CLI' . PHP_EOL;
			return false;
		}
		return ${$to}->cli();
	}
	public function print_paras() {
	// dump the arguments it gets, useful for debugging ehphp
		var_dump(func_get_args());
		return true;
	}
	public function test($paras) {
	// Test function that might do anything I currently want to test
	// Currently, returning its argument
		return $paras[0];
	}
}
?>
