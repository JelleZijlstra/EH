#!/usr/bin/php
<?php
/*
 * Testing the Autocompleter.
 */
require_once(__DIR__ . '/../Common/common.php');
class AutocompleterTester {
	private static $classes = array(
		'TreeAutocompleter',
		'NaiveAutocompleter',
		'FixedArrayAutocompleter'
	);

	private static function test(/* string */ $class) {
		$data = array(
			'Hello',
			'Hellish',
			'Goodbye',
		);
		$ac = new $class($data);
		assert($ac->lookup('') === '');
		assert($ac->lookup('H') === 'ell');
		assert($ac->lookup('G') === 'oodbye');
		assert($ac->lookup('Helli') === 'sh');
	}
	
	public static function runTests() {
		foreach(self::$classes as $class) {
			self::test($class);
		}
	}
	
	private static function profile(/* string */ $class, array $data) {
		echo 'Profiling class: ' . $class . PHP_EOL;
		$memoryBefore = memory_get_usage();
		$timeBefore = microtime(true);
		$ac = new $class($data);
		$timeAfter = microtime(true);
		$memoryAfter = memory_get_usage();
		echo 'Time taken for object creation: ' 
			. round($timeAfter - $timeBefore, 6) . ' s' . PHP_EOL;
		echo 'Memory taken: ' . 
			Sanitizer::intToBytes($memoryAfter - $memoryBefore) . PHP_EOL;
		
		$timeBefore = microtime(true);
		for($i = 0; $i < 100; $i++) {
			$ac->lookup('Agat');
			$ac->lookup('Oryz');
			$ac->lookup('Sivalad');
			$ac->lookup('A');
		}
		$timeAfter = microtime(true);
		echo 'Time taken for lookup: '
			. round($timeAfter - $timeBefore, 6) . ' s' . PHP_EOL;
		echo PHP_EOL;
	}
	
	public static function runProfiling() {
		$names = CsvArticleList::singleton()->getNameArray();
		foreach(self::$classes as $class) {
			self::profile($class, $names);
		}		
	}
}
AutocompleterTester::runTests();
AutocompleterTester::runProfiling();
