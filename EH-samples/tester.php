#!/usr/bin/php
<?php
/*
 * tester.php
 * Jelle Zijlstra, December 2011
 *
 * Used to test the EH interpreter.
 * Ultimately, this should be rewritten in EH script itself.
 */
// parse arguments
if(isset($argv[1]) && $argv[1] === '--valgrind') {
	$executer = 'valgrind -q --leak-check=full /usr/bin/ehi';
} else {
	$executer = '/usr/bin/ehi';
}

$testfiles = fopen("testfiles", "r");
if($testfiles === false) {
	echo "Unable to open testfiles\n";
	exit(1);
}
while(($file = trim(fgets($testfiles))) !== '') {
	echo "Testing $file...\n";
	$expected = str_replace('.eh', '.expected', $file);
	$output = str_replace('.eh', '.output', $file);
	`$executer $file &> '$output'`;
	echo `diff '$expected' '$output'`;
	`rm '$output'`;
}
fclose($testfiles);
exit(0);
