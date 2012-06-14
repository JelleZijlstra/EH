#!/usr/bin/php
<?php
/*
 * Testing the Autocompleter.
 *
 * Currently, OneStepAutocompleter appears to be the best solution, since it 
 * doesn't have the ridiculous load time of TreeAutocompleter and is a bit 
 * faster than NaiveAutocompleter.
 *
 * An improved version might be a "LazyAutocompleter", which is like a
 * TreeAutocompleter but only builds up the parts of the tree that are actually
 * needed. To begin with, it would just do the first letters.
 */
require_once(__DIR__ . '/../Common/common.php');
class AutocompleterTester {
	private static $classes = array(
		'TreeAutocompleter',
		'NaiveAutocompleter',
		'FixedArrayAutocompleter',
		'OneStepAutocompleter',
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
		
		
		$totalTime = 0;
		$tests = array('Agat', 'Oryz', 'Sivalad', 'A');
		foreach($tests as $test) {
			$timeBefore = microtime(true);
			for($i = 0; $i < 100; $i++) {
				$ac->lookup($test);
			}
			$timeAfter = microtime(true);
			$timeTaken = $timeAfter - $timeBefore;
			echo 'Time taken for lookup of "' . $test . '": '
				. round($timeTaken, 6) . ' s' . PHP_EOL;
			$totalTime += $timeTaken;
		}
		echo 'Total time taken for lookup: '
				. round($totalTime, 6) . ' s' . PHP_EOL;
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
