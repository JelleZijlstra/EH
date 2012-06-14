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
}
AutocompleterTester::runTests();
