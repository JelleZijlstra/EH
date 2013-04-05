#!/usr/bin/php
<?php
/*
 * tester.php
 * Jelle Zijlstra, December 2011
 *
 * Used to test the EH interpreter.
 * Ultimately, this should be rewritten in EH script itself.
 */
class TestCase {
	public static $executer;
	private $name;
	private $expected;
	public static function make($file) {
		$name = str_replace('.eh', '', $file);
		$tc = new TestCase($name);
		$tc->run();
	}
	public function __construct($name) {
		$this->name = $name;
		$this->getExpected();
	}
	public function run() {
		echo 'Testing file ' . $this->name . '...' . PHP_EOL;
		shell_exec($this->makeTestCommand());
		$this->testOutput('stdout');
		$this->testOutput('stderr');
		$this->cleanUp();
	}
	private function getExpected() {
		$raw = fopen($this->name . '.expected', 'r');
		if($raw === false) {
			echo 'Unable to open test output file for ' . $this->name . PHP_EOL;
			exit(1);
		}
		$expected = array();
		$currentKey = '';
		$currentValue = '';
		while(($line = fgets($raw)) !== false) {
			if(preg_match('/%%(.*)%%\s*/u', $line, $matches)) {
				if($currentKey !== '') {
					$expected[$currentKey] = $currentValue;
				}
				$currentKey = $matches[1];
				$currentValue = '';
			} else {
				$currentValue .= $line;
			}
		}
		$expected[$currentKey] = $currentValue;
		$this->expected = $expected;
	}
	private function makeTestCommand() {
		$command = self::$executer . ' ' . $this->name . '.eh ';
		if(isset($this->expected['arguments'])) {
			$command .= trim($this->expected['arguments']) . ' ';
		}
		$stdoutFile = 'tmp/' . $this->name . '.stdout';
		$stderrFile = 'tmp/' . $this->name . '.stderr';
		$command .= ' > ' . $stdoutFile . ' 2> ' . $stderrFile;
		return $command;
	}
	private function testOutput($stream) {
		$outputFile = 'tmp/' . $this->name . '.' . $stream;
		$output = file_get_contents($outputFile);
		if(isset($this->expected[$stream . '-regex'])) {
			$regex = '/' . $this->expected[$stream . '-regex'] . '/u';
			if(!preg_match($regex, $output)) {
				$this->failTest($stream, $outputFile, $this->expected[$stream . '-regex']);
			}
		} else if(isset($this->expected[$stream])) {
			if($this->expected[$stream] !== $output) {
				$this->failTest($stream, $outputFile, $this->expected[$stream]);
			}
		} else {
			if($output !== '') {
				$this->failTest($stream, $outputFile, '');
			}
		}
	}
	private function failTest($name, $outputFile, $expected) {
		echo 'Failed test for ' . $name . '!' . PHP_EOL;
		$tempFile = 'tmp/' . $this->name . '.tmp';
		file_put_contents($tempFile, $expected);
		echo `diff '$tempFile' '$outputFile'`;
	}
	private function cleanUp() {
		$command = 'rm tmp/' . $this->name . '.*';
		shell_exec($command);
	}
}

// parse arguments
array_shift($argv);
if(isset($argv[0]) && $argv[0] === '--valgrind') {
	array_shift($argv);
	TestCase::$executer = 'valgrind -q --leak-check=full /usr/bin/ehi';
} else if(isset($argv[0]) && $argv[0] === '-O') {
	array_shift($argv);
	TestCase::$executer = '/usr/bin/ehi -O';
} else {
	TestCase::$executer = '/usr/bin/ehi';
}

chdir(__DIR__);
`mkdir -p tmp`;

if(count($argv) === 0) {
	$testfiles = fopen("testfiles", "r");
	if($testfiles === false) {
		echo "Unable to open testfiles\n";
		exit(1);
	}
	while(($file = trim(fgets($testfiles))) !== '') {
		TestCase::make($file);
	}
	fclose($testfiles);
} else {
	foreach($argv as $argument) {
		TestCase::make($argument);
	}
}

exit(0);
