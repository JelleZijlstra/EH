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
define('PROCESS_PARAS_ERROR_FOUND', false);
require_once(BPATH . "/Common/EHException.php");
class ExecuteHandler extends EHICore {
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
	// currently handled files. Why is this in EH and not in ContainerList?
	protected $current;
	private static $ExecuteHandler_commands = array(
		'execute_help' => array('name' => 'execute_help',
			'aka' => array('h', 'help', 'man'),
			'desc' => 'Get help information about the command line or a specific command',
			'arg' => 'Command name'),
		'listcommands' => array('name' => 'listcommands',
			'desc' => 'List legal commands',
			'arg' => 'None',
			'execute' => 'callmethod'),
		'shell' => array('name' => 'shell',
			'aka' => 'exec_catch',
			'desc' => 'Execute a command from the shell',
			'arg' => 'Shell command'),
		'configset' => array('name' => 'configset',
			'aka' => 'setconfig',
			'desc' => 'Set a configuration variable',
			'arg' => 'Variables to be set'),
		'switchcli' => array('name' => 'switchcli',
			'aka' => array('switch', 'cli'),
			'desc' => 'Switch to a different command line',
			'arg' => 'Name of command line to switch to'),
		'print_paras' => array('name' => 'print_paras',
			'desc' => 'Print its arguments',
			'arg' => 'As many as you want'),
		'return_para' => array('name' => 'return_para',
			'desc' => 'Returns its first argument',
			'arg' => 'One; everything else is ignored'),
		'test' => array('name' => 'test',
			'desc' => 'Do something random',
			'arg' => 'None'),
		'menu' => array('name' => 'menu',
			'desc' => 'Create a menu'),
		'call' => array('name' => 'call',
			'aka' => array('phpcall'),
			'desc' => 'Call an arbitrary PHP function'),
	);
	/* Setting up the EH interface */
	protected function __construct(array $commands = array()) {
		parent::__construct();
		$this->setup_ExecuteHandler($commands);
	}
	public function setup_ExecuteHandler(array $commands = array()) {
	// sets up a handler.
	// @param commands Array of commands to be added to the object's library
		$this->current = array();
		foreach(self::$ExecuteHandler_commands as $command) {
			$this->addcommand($command);
		}	
		foreach(parent::$core_commands as $command) {
			$this->addcommand($command);
		}
		foreach($commands as $command) {
			$this->addcommand($command);
		}
	}
	public function cli() { 
	// sets up command line
		$this->setup_commandline(get_called_class());
	}
	/* Handling commands and execution */
	public function addcommand($command, array $paras = array()) {
	// adds a command to the object's library
	// @param command Array of data forming a command
	// command can have the following components
	// - name: String basic name of the command
	// - aka: Array synonyms
	// - desc: String description of what the command does
	// - arg: String description of the arguments the command takes
		if(isset($this->commands[$command['name']])) {
			throw new EHException(
				'Command ' . $command['name'] . ' already exists'
			);
		}
		self::testcommand($command);
		if(isset($command['aka'])) {
			if(!is_array($command['aka'])) {
				$command['aka'] = array($command['aka']);
			}
			foreach($command['aka'] as $aka) {
				if(isset($this->synonyms[$aka])) {
					throw new EHException(
						$aka . ' already exists as a synonym for ' 
						. $this->synonyms[$aka]
					);
				} else {
					$this->synonyms[$aka] = $command['name'];
				}
			}
		}
		$this->commands[$command['name']] = $command;
		return true;
	}
	static private function testcommand($command) {
		/* check and handle input */
		// if we don't have those, little point in proceeding
		if(!isset($command['name'])) {
			throw new EHException('No name given for new command');
		}
		// warn if no documentation
		if(!isset($command['desc'])) {
			throw new EHException(
				'No documentation given for new command ' . $command['name']
			);
		}
		return true;
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
		return isset($this->commands[$cmd]) || isset($this->synonyms[$cmd]);
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
			if(defined('IS_EHPHP')) {
				return NULL;
			} else {
				return self::EXECUTE_NEXT;
			}
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
			foreach($this->current as $file) {
				$paras[] = $file;
			}
		}
		// output redirection
		if($redirection['>'] !== false or $redirection['>$'] !== false) {
			ob_start();
		}
		try {
			// execute the command
			$ret = $this->{$cmd['name']}($paras);
		} catch(EHException $e) {
			$e->handle();
			echo "Error '" . $e->getMessage() . "' occurred while executing command '" . $cmd['name'] . "'" . PHP_EOL;
			if(defined('IS_EHPHP')) {
				return NULL;
			} else {
				return self::EXECUTE_NEXT;
			}
		} catch(StopException $e) {
			echo 'Stopped ' . $cmd['name'] . ' (' . $e->getMessage() . ')' . PHP_EOL;
			if(defined('IS_EHPHP')) {
				return NULL;
			} else {
				return self::EXECUTE_NEXT;
			}
		}
		// always make return value accessible to script
		$this->setvar('ret', $ret);

		// from here, return $ret if we're in ehphp, but EXECUTE_NEXT if we're in the pure-PHP implementation
		if(defined('IS_EHPHP')) {
			$returnvalue = $ret;
		} else {
			$returnvalue = self::EXECUTE_NEXT;
		}
		if($redirection['>'] !== false) {
			$file = fopen($redirection['>'], 'w');
			if(!$file) {
				trigger_error('Invalid rediction file: ' . $outputredir, E_USER_NOTICE);
				ob_end_clean();
				return $returnvalue;
			}
			fwrite($file, ob_get_contents());
			ob_end_clean();
		} elseif($redirection['>$'] !== false) {
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
		if(isset($this->synonyms[$in])) {
			$in = $this->synonyms[$in];
		} 
		if(isset($this->commands[$in])) {
			return $this->commands[$in];
		} else {
			return false;
		}
	}
	/* Help functions */
	private function execute_help(array $paras = array()) {
		if(!$this->process_paras($paras, array(
			'name' => __FUNCTION__,
			'synonyms' => array(0 => 'cmd'),
			'checklist' => array('cmd' => 'Command to explain'),
			'default' => array('cmd' => ''),
		))) return false;
		// array of functions with info
		if($paras['cmd'] === '') {
			echo 'The following commands are available in this ExecuteHandler interface:' . PHP_EOL;
			$this->listcommands();
			echo 'Type "help <command>" or "<command> --help" to get more information about that commmand.' . PHP_EOL;
			return true;
		}
		$cmd = $this->expand_cmd($paras['cmd']);
		if($cmd) {
			echo PHP_EOL . 'Function: ' . $cmd['name'] . PHP_EOL;
			if(isset($cmd['aka'])) {
				if(is_array($cmd['aka'])) {
					echo 'Aliases: ' . implode(', ', $cmd['aka']) . PHP_EOL;
				} elseif(is_string($cmd['aka'])) {
					echo 'Aliases: ' . $cmd['aka'] . PHP_EOL;
				}
			}
			echo 'Description: ' . $cmd['desc'] . PHP_EOL;
			return true;
		}
		else {
			echo 'No such command' . PHP_EOL;
			return false;
		}
	}
	private function debugecho($var) {
	// For debugging: adds output to a log file. Useful when debugging methods like fgetc()
		$file = "/Users/jellezijlstra/Dropbox/git/Common/log";
		shell_exec("echo '$var' >> $file");
	}
	/* Input validation */
	// paras that process_paras's checklist ignores
	private static $pp_checklist_ignore = array('_ehphp');
	protected function process_paras(&$paras, $pp_paras, &$split = NULL) {
		/*
		 *    int ExecuteHandler::process_paras(array &$paras, array $pp_paras,
		 *      array &$split = NULL);
		 *
		 * This method processes a function's $paras array in the way specified
		 * in the $pp_paras parameter. In addition, it will print a summary of
		 * the method's usage if $paras['help'] is set. process_paras() will
		 * return true if it is successful and false if it has
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
		 *   'defaultfunc': Similar to 'default', but the value in the array is
		 *		a function instead of a raw value. This function is executed 
		 *		and its return value is put in $paras.
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
		 *    'listoptions': An associative array, similar to 'checkparas', but
		 *      instead of a function, each para has an array associated with 
		 *      it. The code throws an error if the value for a para is not in 
		 *      the array for that para.
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
			echo 'process_paras: error: invalid pp_paras' . PHP_EOL;
			return false;
		}
		if(!is_array($paras)) {
			// apply 'toarray'
			if(isset($pp_paras['toarray'])) {
				$paras = array($pp_paras['toarray'] => $paras);
			} else {
				echo 'process_paras: error: $paras is not an array' . PHP_EOL;
				return false;
			}
		}
		// special parameter in all cases: help
		if(isset($paras['help'])) {
			if(isset($pp_paras['name'])) {
				$this->execute_help(array($pp_paras['name']));
			}
			// without checklist, we can't do much
			if(!isset($pp_paras['checklist'])) {
				return false;
			}
			echo 'Parameters:' . PHP_EOL;
			foreach($pp_paras['checklist'] as $name => $description) {
				echo '- ' . $name . PHP_EOL;
				echo $description . PHP_EOL;
				if(isset($pp_paras['default'][$name])) {
					echo 'Default: ';
					Sanitizer::printVar($pp_paras['default'][$name]);
					echo PHP_EOL;
				}
				if(isset($pp_paras['errorifempty']) and in_array($name, $pp_paras['errorifempty'], true)) {
					echo 'This parameter is required.' . PHP_EOL;
				}
				if(isset($pp_paras['askifempty']) and in_array($name, $pp_paras['askifempty'], true)) {
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
			return false;
		}
		// variable used in checking input
		$founderror = false;
		$showerror = function($msg) use($pp_paras, &$founderror) {
			if(isset($pp_paras['name'])) {
				echo $pp_paras['name'] . ': ';
			}
			echo 'error: ' . $msg . PHP_EOL;
			$founderror = true;
		};
		// perform the checks
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
								'prompt' => $key . ': ',
								'options' => array(),
							);
							// use checkparas validation if possible
							if(isset($pp_paras['checkparas'][$key])) {
								$menu_paras['validfunction'] = 
									$pp_paras['checkparas'][$key];
							}
							// else accept anything
							else {
								$menu_paras['validfunction'] = function($in) {
									return true;
								};
							}
							if(isset($pp_paras['checklist'][$key])) {
								$menu_paras['helpinfo'] =
									$pp_paras['checklist'][$key];
							}
							try {
								$paras[$key] = $this->menu($menu_paras);
							} catch(StopException $e) {
								return false;
							}
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
						if(!isset($paras[$key])) {
							$paras[$key] = $result;
						}
					}
					break;
				case 'defaultfunc':
					// set default values for paras using information in the 
					// rest of the paras array
					if(!is_array($pp_value)) {
						$showerror('defaultfunc parameter is not an array');
						break;
					}
					foreach($pp_value as $key => $result) {
						if(!isset($paras[$key])) {
							$paras[$key] = $result($paras);
						}
					}
					break;
				case 'checklist': // check that all paras given are legal
					if(!is_array($pp_value)) {
						$showerror('checklist parameter is not an array');
						break;
					}
					foreach($paras as $key => $result) {
						if(!array_key_exists($key, $pp_value)) {
							// ignore some
							if(in_array($key, self::$pp_checklist_ignore, true)) {
								continue;
							}
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
							$showerror('invalid value "' 
								. Sanitizer::varToString($paras[$para]) 
								. '" for parameter "' . $para . '"');
						}
					}
					break;
				case 'listoptions': // list options for a para
					if(!is_array($pp_value)) {
						$showerror('listoptions parameter is not an array');
						break;
					}
					foreach($pp_value as $para => $options) {
						if(!isset($paras[$para])) {
							continue;
						}
						// always accept default value
						if(isset($pp_paras['default'][$para]) and ($paras[$para] === $pp_paras['default'][$para])) {
							continue;
						}
						if(!in_array($paras[$para], $options, true)) {
							$showerror('invalid value "' 
								. Sanitizer::varToString($paras[$para]) 
								. '" for parameter "' . $para . '" (allowed ' 
								. 'options: "' . implode('", "', $options) 
								. '")'
							);
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
				case 'checkfunc':
				case 'name':
				case 'toarray':
					// ignore, used internally in other places
					break;
				default:
					$showerror('unrecognized process_paras parameter ' . $pp_key);
					break;
			}
		}
		if($founderror) {
			return false;
		} else {
			return true;
		}
	}
	/* Input for EH methods */
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
				return chr($c1);
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
		if(!$this->process_paras($paras, array(
			'name' => __FUNCTION__,
			'toarray' => 'prompt',
			'checklist' => array(
				'lines' =>
					'Array of lines that can be accessed when KEY_UP and KEY_DOWN are pressed',
				'prompt' =>
					'Prompt to be printed',
				'includenewlines' =>
					'Whether to include newlines in the line returned',
				'initialtext' =>
					'Initial suggested text',
				'autocompletion' =>
					'Array of strings for autocompletion',
			),
			'default' => array(
				'lines' => array(),
				'prompt' => '> ',
				'includenewlines' => false,
				'initialtext' => false,
				'autocompletion' => false,
			),
			'checkparas' => array(
				'lines' => function($in) {
					return is_array($in);
				},
				'autocompletion' => function($in) {
					return is_array($in);
				},
			),
		))) return false;
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
		$showcursor = function() use(&$cmdlen, &$keypos, $getcmd, $promptoffset) {
			// return to saved cursor position, clear line
			// first move as far west as we can; 200 positions should suffice
			echo "\033[200D\033[" . $promptoffset . "C\033[K";
			// put the command back
			echo $getcmd();
			// put the cursor in the right position
			if($cmdlen > $keypos)
				echo "\033[" . ($cmdlen - $keypos) . "D";
		};
		$addCharacter = function($c) use(&$cmdlen, &$keypos, &$cmd) {
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
		};
		// prepare autocompletion
		if($paras['autocompletion'] !== false) {
			$autocompleter = new Autocompleter($paras['autocompletion']);
		}
		// set our settings
		$this->stty('cbreak iutf8');
		// always put cursor at beginning of line, and print prompt
		echo "\033[200D" . $paras['prompt'];
		if($paras['initialtext'] === false) {
			// get command
			$cmd = array();
			$cmdlen = 0;
			$keypos = 0;
		} else {
			echo $paras['initialtext'];
			$cmd = mb_str_split($paras['initialtext']);
			$cmdlen = $keypos = count($cmd);
		}
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
					if($keypos === 0) {
						break;
					}
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
				case "\011": // Tab and Ctrl+I
					$command = $getcmd();
					$offset = strrpos($command, "'", $keypos);
					if(isset($autocompleter) && $offset !== false) {
						$autocompleted = substr($command, $offset + 1);
						$addition = $autocompleter->lookup($autocompleted);
						if($addition !== false) {
							for($i = 0, $len = strlen($addition); $i < $len; $i++) {
								$addCharacter($addition[$i]);
							}
						}
					}
					break;
				case "\001": // Ctrl+A
				case "\002": // Ctrl+B
				case "\005": // Ctrl+E
				case "\006": // Ctrl+F
				case "\007": // Ctrl+G
				case "\010": // Ctrl+H
				case "\013": // Ctrl+K
				case "\016": // Ctrl+N
				case "\020": // Ctrl+P
				case "\024": // Ctrl+T
				case "\025": // Ctrl+U
				case "\026": // Ctrl+V
				case "\027": // Ctrl+W
				case "\030": // Ctrl+X
				case "\033": // Ctrl+[
				case "\035": // Ctrl+]
					break;
				case "\014": // Ctrl+L(eft): go to beginning of line
					$keypos = 0;
					break;
				case "\022": // Ctrl+R(ight): go to end of line
					$keypos = $cmdlen;
					break;
				case "\004": // Ctrl+D: stop
					echo PHP_EOL; // make newline
					throw new StopException("fgetc");
				default: // other characters: add to command
					$addCharacter($c);
					break;
			}
			// show command
			$showcursor();
		}
	}
	private function stty($opt) {
		$cmd = "/bin/stty " . $opt . ' 2> /dev/null';
		exec($cmd, $output, $return);
		return implode("\n", $output);
	}
	public function menu(array $paras) {
	// Function that creates a menu and gets input
		if(!$this->process_paras($paras, array(
			'name' => __FUNCTION__,
			'checklist' => array(
				'head' =>
					'Menu heading',
				'prompt' =>
					'Prompt to be shown',
				'headasprompt' =>
					'Whether to show the heading as the prompt for input',
				'options' =>
					'List of options. Associative array, with option in key and description in value',
				'printoptions' =>
					'Whether options should always be printed',
				'helpcommand' =>
					'Whether to make the help command available. (If set to true, commands beginning with "help" will not get returned.)',
				'helpinfo' =>
					'Information that gets shown to the user when they type "help"',
				'validfunction' =>
					'Function to determine validity of command',
				'process' =>
					'Array of callbacks to execute when a given option is'
						. ' called. These function take the command given and'
						. ' the data produced by processcommand as arguments,'
						. ' and they'
						. ' should return either true (indicating that menu'
						. ' should continue) or false (indicating that menu'
						. ' should return).',
				'processcommand' =>
					'Function used to process the command after input. This'
						. ' function may take a second reference argument of'
						. ' data that is given to processcommand or to the'
						. ' caller. This function may return false if the'
						. ' command is invalid.',
			),
			'default' => array(
				'head' => false,
				'prompt' => '> ',
				'printoptions' => false,
				'helpcommand' => true,
				'options' => array(),
				'validfunction' => function($in, $options) {
					return array_key_exists($in, $options);
				},
				'process' => array(),
				'processcommand' => false,
				'headasprompt' => false,
				'helpinfo' => false,
			),
		))) return false;
		// print menu heading
		if(!$paras['headasprompt']) {
			if($paras['head'] !== false) {
				echo $paras['head'] . PHP_EOL;
			}
		}
		$printoptions = function() use($paras) {
			if(count($paras['options']) === 0) {
				return;
			}
			echo 'Options available:' . PHP_EOL;
			foreach($paras['options'] as $cmd => $desc) {
				echo "-'" . $cmd . "': " . $desc . PHP_EOL;
			}
		};
		if($paras['printoptions']) {
			$printoptions();
		}
		$getlineparas = array('lines' => array_keys($paras['options']));
		if($paras['headasprompt']) {
			$getlineparas['prompt'] = $paras['head'];
		} else {
			$getlineparas['prompt'] = $paras['prompt'];
		}
		while(true) {
			// get command
			$cmd = $this->getline($getlineparas);
			$data = NULL;
			if($cmd === false) {
				return false;
			}
			// remember command in history
			$getlineparas['lines'][] = $cmd;
			// provide help if necessary
			if($paras['helpcommand']) {
				// just 'help' prints all options
				if($cmd === 'help') {
					if($paras['helpinfo'] !== false) {
						echo $paras['helpinfo'] . PHP_EOL;
					}
					$printoptions();
					continue;
				}
				// help about a specific command
				if(substr($cmd, 0, 5) === 'help ') {
					$option = substr($cmd, 5);
					if(isset($paras['options'][$option])) {
						echo $option . ': ' . $paras['options'][$option] . PHP_EOL;
					} else {
						echo 'Option ' . $option . ' does not exist.' . PHP_EOL;
					}
					continue;
				}
			}
			if($paras['processcommand'] && !array_key_exists($cmd, $paras['options'])) {
				$cmd = $paras['processcommand']($cmd, $data);
			}
			// return command if valid
			if($cmd !== false && ((is_string($cmd) 
				&& array_key_exists($cmd, $paras['options'])) 
				|| $paras['validfunction']($cmd, $paras['options'], $data))) {
				if(isset($paras['process'][$cmd])) {
					if($paras['process'][$cmd]($cmd, $data) === false) {
						break;
					}
				} else {
					break;
				}
			} else {
				echo 'Invalid value ' . $cmd . PHP_EOL;
			}
		}
		if($data === NULL) {
			return $cmd;
		} else {
			return array($cmd, $data);
		}
	}
	/*
	 * A specialization of ExecuteHandler::menu() to create a yes/no menu.
	 */
	public /* bool */ function ynmenu(/* string */ $head, /* callable */ $process = NULL) {
		switch($this->menu(array(
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
		))) {
			case 'y': return true;
			case 'n': return false;
		}
		throw new EHException("Execution should never reach here");
	}
	/* Testing the EH framework */
	public function print_paras(array $paras) {
	// dump the arguments it gets, useful for debugging ehphp
		var_dump($paras);
		return true;
	}
	public function return_para(array $paras) {
	// return its first para
		if(!isset($paras[0])) {
			return NULL;
		} else {
			return $paras[0];
		}
	}
	public function test($paras) {
	// Test function that might do anything I currently want to test
		$c = $this->fgetc(STDIN);
		var_dump($c);
		var_dump(ord($c), chr($c));
		return;
	
	// Currently, returning its argument
		// and telling us what functions etcetera we have defined
		eval($paras[0]);
		return;
		var_dump(array_keys($GLOBALS));
		$funcs = get_defined_functions();
		var_dump($funcs['user']);
		var_dump(array_keys(get_defined_constants()));
		var_dump(get_declared_classes());
		return $paras[0];
	}
	/* Miscellaneous stuff */
	public function switchcli(array $paras) {
		if(!$this->process_paras($paras, array(
			'name' => __FUNCTION__,
			'synonyms' => array(0 => 'to'),
			'checklist' => array('to' => 'CLI to switch to'),
			'errorifempty' => array('to'),
			'checkparas' => array(
				'to' => function($to) {
					// return false if class fails to load
					try {
						return (class_exists($to) 
							&& is_subclass_of($to, 'ContainerList'));
					} catch(EHException $e) {
						return false;
					}
				}
			),
		))) return false;
		return $paras['to']::singleton()->cli();
	}
	protected function configset(array $paras) {
	// sets something in the $this->config array, which configures the EH instance
		foreach($paras as $key => $value) {
			if(array_key_exists($key, $this->config))
				$this->config[$key] = $value;
		}
	}
	public function shell($paras) {
		// TODO: set up our own shell process with a persistent pipe, so we can
		// keep state in the shell.
		if(!$this->process_paras($paras, array(
			'name' => __FUNCTION__,
			'toarray' => 'cmd',
			'synonyms' => array(
				0 => 'cmd',
				1 => 'arg',
				'o' => 'stdout',
				'e' => 'stderr',
				'v' => 'printcmd',
			),
			'checklist' => array(
				'cmd' => 'Command to be executed',
				'arg' => 'Array of arguments to the command. These arguments are escaped by this command.',
				'stdout' => 'Place to send stdout to',
				'append-out' => 'Whether to append to the stdout file',
				'stderr' => 'Place to send stderr to',
				'append-err' => 'Whether to append to the stderr file',
				'input' => 'Place to get input from',
				'input-string' => 'String to send as stdin input',
				'return' => 'What to return. Options are "success" (whether the command returned exit status 0), "output" (the stdout output), "outputlines" (the output as an array of lines), and "exitvalue" (the exit code of the command).',
				'printcmd' => 'Print the command as it is executed',
				'printout' => 'Whether to print the output',
				'exceptiononerror' => 'Whether to throw an exception when an error occurs',
			),
			'default' => array(
				'arg' => false,
				'stdout' => false,
				'append-out' => true,
				'stderr' => isset($paras['_ehphp']) ? false : '/dev/null',
				'append-err' => true,
				'input' => false,
				'input-string' => false,
				'return' => 'success',
				'printcmd' => false,
				'printout' => true,
				'exceptiononerror' => true,
			),
			'listoptions' => array(
				'return' => array('success', 'output', 'exitvalue', 'outputlines'),
			),
			'checkparas' => array(
				'arg' => function($in) {
					return is_array($in);
				},
			),
			'errorifempty' => array('cmd'),
		))) return false;
		$cmd = $paras['cmd'];
		if($paras['arg']) {
			$args = array_map('escapeshellarg', $paras['arg']);
			$cmd .= ' ' . implode(' ', $args);
		}
		// cd won't actually change the shell until we do some special magic
		if(substr($cmd, 0, 3) === 'cd ') {
			$dir = substr($cmd, 3);
			// more hack
			if($dir[0] === "'") {
				$dir = substr($dir, 1, -1);
			}
			// handle home directory
			if($dir[0] === '~') {
				$home = trim(shell_exec('echo $HOME'));
				$dir = preg_replace('/^~/u', $home, $dir);
			}
			if($paras['printcmd']) {
				echo 'cd ' . $dir . PHP_EOL;
			}
			return chdir($dir);
		} else {
			if($paras['stdout'] !== false) {
				$cmd .= $paras['append-out'] ? ' >> ' : ' > ';
				$cmd .= $paras['stdout'];
			}
			if($paras['stderr'] !== false) {
				$cmd .= $paras['append-err'] ? ' 2>> ' : ' 2> ';
				$cmd .= $paras['stderr'];
			}
			if($paras['input'] !== false) {
				$cmd .= ' < ' . $paras['input'];
			}
			if($paras['input-string'] !== false) {
				$cmd .= " <<INPUT\n" . $paras['input-string'] . "\nINPUT";
			}
			if($paras['printcmd']) {
				echo $cmd . PHP_EOL;
			}
			exec($cmd, $output, $exitval);
			if($paras['printout'] and count($output) > 0) {
				echo implode(PHP_EOL, $output) . PHP_EOL;
			}
			if($paras['exceptiononerror'] && ($exitval !== 0)) {
				throw new EHException(
					'Error (exit value = ' . $exitval . ') occurred while '
						. 'executing command ' . $cmd
				);
			}
			switch($paras['return']) {
				case 'success':
					return ($exitval === 0);
				case 'outputlines':
					return $output;
				case 'output':
					return implode(PHP_EOL, $output);
				case 'exitvalue':
					return $exitval;
			}
		}
	}
	public function call($paras) {
		if(!$this->process_paras($paras, array(
			'name' => __FUNCTION__,
			'synonyms' => array(
				0 => 'function',
				1 => 'arguments',
				'p' => 'print',
			),
			'checklist' => array(
				'function' => 'Function to call',
				'arguments' => 'Arguments for the function call',
				'print' => 'Whether to print the return value',
			),
			'errorifempty' => array('function'),
			'default' => array(
				'arguments' => array(),
				'print' => false,
			),
			'checkparas' => array(
				'function' => function($in) {
					return function_exists($in);
				},
				'arguments' => function($in) {
					return is_array($in);
				}
			),
		))) return false;
		$ret = call_user_func_array($paras['function'], $paras['arguments']);
		if($paras['print']) {
			Sanitizer::printVar($ret);
			echo PHP_EOL;
		}
		return $ret;
	}
}
