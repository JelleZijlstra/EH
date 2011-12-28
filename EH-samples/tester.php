<?php
/*
 * tester.php
 * Jelle Zijlstra, December 2011
 *
 * Used to test the EH interpreter.
 * Ultimately, this should be rewritten in EH script itself.
 */
chdir(__DIR__);
$testfiles = fopen("testfiles", "r");
while($file = trim(fgets($testfiles))) {
	echo "Testing $file...\n";
	$expected = str_replace('.eh', '.expected', $file);
	$output = str_replace('.eh', '.output', $file);
	`/usr/bin/ehi '$file' > '$output' 2>> 'stderr.output'`;
	`diff '$expected' '$output'`;
	`rm '$output'`;
}
echo "Testing stderr output:\n";
`diff stderr.expected stderr.output`;
`rm stderr.output`;
fclose($testfiles);