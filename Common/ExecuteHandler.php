<?php
//TODO: implement some sort of stuff that enables KEY_UP and KEY_DOWN to work
//Look at PHP's pcntl_signal()
define(PROCESS_PARAS_ERROR_FOUND, 0x1);
abstract class ExecuteHandler {
	private $commands;
	private $synonyms;
	private $config = array(
		'debug' => false,
	);
	/* command history */
	// holds all executed commands
	private $history = array();
	// length of history array
	private $histlen = 0;
	// pointer to where in the array we currently are
	private $histptr = 0;
	
	protected $current;
	protected $trystatic; // set to true to try additional stuff in expand_cmd
	// array of codes that can be given in the 'execute' field of a command, and descriptions
	protected static $handlers = array(
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
	public function execute($in) {
	// functions as an interpreter of the byfile() "command line"
		$splitcmd = $this->divide_cmd($in);
		$rawcmd = array_shift($splitcmd);
		$cmd = $this->expand_cmd($rawcmd);
		// handle output redirection
		$outputredir = false;
		$nextoutput = false;
		if(!$cmd) {
			echo 'Invalid command: ' . $in . PHP_EOL;
			return true;
		}
		$paras = array();
		foreach($splitcmd as $piece) {
			// handle output redirection
			if($piece === '>') {
				$nextoutput = true;
				continue;
			}
			if($nextoutput) {
				$outputredir = $piece;
				$nextoutput = false;
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
					if($rawarg) $rawarg .= ' ';
					$rawarg .= $piece;
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
		if($rawarg) {
			$rawarg = self::remove_quotes($rawarg);
			// handle shortcut
			if($rawarg === '*')
				$arg = $this->current;
			else {
				$arg = array($rawarg);
				if(method_exists($this, 'has') and $this->has($rawarg))
					$this->current = $arg;
			}
			$paras[0] = $rawarg;
		}
		else {
			$arg = array();
			$rawarg = '';
		}
		// cleanup
		foreach($paras as &$text) {
			if($text and is_string($text)) $text = self::remove_quotes($text);
		}
		// output redirection
		if($outputredir and $cmd['execute'] !== 'quit') {
			ob_start();
		}
		// execute it
		switch($cmd['execute']) {
			case 'doallorcurr':
				if($arg and is_array($arg)) {
					foreach($arg as $file)
						if(!$this->{$cmd['name']}($file, $paras)) break;
				}
				else
					$this->doall($cmd['name'], $paras);
				break;
			case 'docurr':
				foreach($arg as $entry) {
					$this->{$cmd['name']}($entry, $paras);
				}
				break;
			case 'callmethod':
				$this->{$cmd['name']}($paras);
				break;
			case 'callmethodarg':
				$this->{$cmd['name']}($rawarg, $paras);
				break;
			case 'callfunc':
				$cmd['name']($paras);
				break;
			case 'callfuncarg':
				$cmd['name']($rawarg, $paras);
				break;			
			case 'quit':
				return false;
			default:
				trigger_error('Unrecognized execution mode', E_USER_NOTICE); 
				break;
		}
		if($cmd['setcurrent'] and $arg)
			$this->current = $arg;
		if($outputredir) {
			$file = fopen($outputredir, 'w');
			if(!$file) {
				trigger_error('Invalid rediction file: ' . $outputredir, E_USER_NOTICE);
				ob_end_clean();
				return true;
			}
			fwrite($file, ob_get_contents());
			ob_end_clean();
		}
		return true;
	}
	private function divide_cmd($in) {
	// divides a string into pieces at each space, and keeps strings in '' together
		$len = strlen($in);
		$out = array();
		$key = 0;
		$instring = false;
		for($i = 0; $i < $len; $i++) {
			if($in[$i] === ' ' and ($i === 0 or $in[$i-1] !== '\\') and !$instring) {
				$key++;
				continue;
			}
			if($in[$i] === "'" and ($i === 0 or $in[$i-1] !== '\\'))
				$instring = $instring ? false : true;
			$out[$key] .= $in[$i];
		}
		return $out;
	}
	private function expand_cmd($in) {
		$cmd = $this->synonyms[$in] ?: ($this->commands[$in] ? $in : false);
		if($cmd) 
			return $this->commands[$cmd];
		if(!$this->trystatic) return false;
		$cmd = static::${get_called_class() . '_synonyms'}[$in] ?: (static::${get_called_class() . '_commands'}[$in] ? $in : false);
		return $cmd ? static::${get_called_class() . '_commands'}[$cmd] : false;
	}
	static public function remove_quotes($in) {
		return preg_replace("/^'|'$|\\\\(?=')/u", '', $in);
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
			return PROCESS_PARA_ERROR_FOUND;
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
							$paras['key'] = getinput();
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
			return PROCESS_PARA_ERROR_FOUND;
		else
			return 0;
	}
	public function setup_commandline($name, $paras = '') {
	// Performs various functions in a pseudo-command line. A main entry point.
	// stty stuff inspired by sfinktah at http://php.net/manual/en/function.fgetc.php
		if($paras['undoable'] and !$this->hascommand('undo')) {
			$this->tmp = clone $this;
			$newcmd['name'] = 'undo';
			$newcmd['desc'] = 'Return to the previous state of the object';
			$newcmd['arg'] = 'None';
			$newcmd['execute'] = 'callmethod';
			$this->addcommand($newcmd, array('ignoreduplicates' => true));
		}
		echo 'Welcome to command line mode. Type "help" for help.' . PHP_EOL;
		// save stty settings
		$saved_stty = preg_replace("/.*; ?/s", "", $this->stty("-a"));
		// set our settings
		$this->stty('cbreak');
		// loop through commands
		while(true) {
			// save current cursor position
			echo $name . "> \033[s";
			// get command
			$cmd = '';
			$cmdlen = 0;
			while(true) {
				// get input
				$c = fgetc(STDIN);
				if($this->config['debug']) {
					echo ' '. ord($c) . ' ';
				}
				switch(ord($c)) {
					case 27: // arrow keys
						$c2 = fgetc(STDIN);
						if(ord($c2) == 91) {
							$c3 = fgetc(STDIN);
							// put back cursor
							echo "\033[4D\033[K";
							switch(ord($c3)) {
								case 65: // KEY_UP
									// decrement pointer
									if($this->histptr > 0)
										$this->histptr--;
									// go back to saved cursor position; clear line
									if($cmdlen > 0)
										echo "\033[" . $cmdlen . "D\033[K"; 
									// get new command
									$cmd = $this->history[$this->histptr];
									$cmdlen = strlen($cmd);
									echo $cmd;
									break;
								case 66: // KEY_DOWN
									// increment pointer
									if($this->histptr < $this->histlen)
										$this->histptr++;
									// go back to saved cursor position; clear line
									if($cmdlen > 0)
										echo "\033[" . $cmdlen . "D\033[K"; 
									// get new command
									if($this->histpr < $this->histlen) {
										$cmd = $this->history[$this->histptr];
										$cmdlen = strlen($cmd);
										echo $cmd;
									}
									else {
										// reset command
										$cmd = '';
										$cmdlen = 0;
									}
									break;
								case 68: // KEY_LEFT
									echo "\033[1D";
									break;
								case 67: // KEY_RIGHT
									echo "\033[1C";
									break;
							}
						}
						break;
					case 127: //backspace
						if($cmdlen > 0) {
							echo "\033[3D\033[K"; // move cursor back three spots (because it puts stuff there for backspace), and erase to end of line
							$cmdlen--;
						}
						else {
							// remove junk
							echo "\033[2D\033[K";
						}
						break; 
					case 10: //newline
						break 2; 
					default: 
						$cmd[$cmdlen] = $c;
						$cmdlen++;
						break; 
					// it looks like KEY_UP etcetera may have more than one... need to see how to handle those
				}
			}
			// create the command in string form
			// it will sometimes be an array at this point, which will need to be imploded
			if(is_string($cmd))
				$tmpcmd = $cmd;
			else if(is_array($cmd))
				$tmpcmd = implode($cmd);
			else if(is_null($cmd)) // don't try to execute empty command
				continue;
			else {
				var_dump($cmd);
			}
			$cmd = substr($tmpcmd, 0, $cmdlen);
			unset($c);
			// save to history
			$this->history[$this->histlen] = $cmd;
			$this->histlen++;
			$this->histptr = $this->histlen;
			// execute the command
			if(!$this->execute($cmd)) {
				echo 'Goodbye.' . PHP_EOL;
				return;
			}
		}
		// restore stty settings
		$this->stty($saved_stty);
	}
	private function stty($opt) {
		$cmd = "/bin/stty " . $opt;
		exec($cmd, $output, $return);
		if($return !== 0) {
			trigger_error("Failed to execute " . $cmd);
			return false;
		}
		return implode(' ', $output);
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
		if($this->trystatic and static::${get_called_class() . '_commands'}[$cmd])
			return true;
		if($this->trystatic and static::${get_called_class() . '_synonyms'}[$cmd])
			return true;
		return false;
	}
	protected function exec_file($file, $paras = '') {
		$in = fopen($file, 'r');
		if(!$in) {
			echo 'Invalid input file' . PHP_EOL;
			return false;
		}
		while(($line = fgets($in)) !== false)
			$this->execute(trim($line));
		return true;
	}
	static protected function testregex($in) {
	// tests whether a regex pattern is valid
		ob_start();
		$t = @preg_match($in, 'test');
		ob_end_clean();
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
}
?>
