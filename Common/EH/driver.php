<?php
// driver.php: Script to compare performance for EH scripting, PHP, and C using the McCarthy function
define(COUNT, 100);

echo 'Testing EH...' . PHP_EOL;
$beforeeh = microtime(true);
for($i = 1; $i < COUNT; $i++)
	shell_exec("echo '$i' > input; /Users/jellezijlstra/Dropbox/git/Common/eh.php mccarthy.eh < input > /dev/null");
$aftereh = microtime(true);
echo "Time for EH: " . ($aftereh - $beforeeh) . PHP_EOL;

echo 'Testing the EH interpreter...' . PHP_EOL;
$beforeehi = microtime(true);
for($i = 1; $i < COUNT; $i++)
	shell_exec("echo '$i' > input; /Users/jellezijlstra/Dropbox/git/Common/parser/ehi mccarthy.eh < input > /dev/null");
$afterehi = microtime(true);
echo "Time for C-interpreted EH: " . ($afterehi - $beforeehi) . PHP_EOL;

echo 'Testing PHP...' . PHP_EOL;
$beforephp = microtime(true);
for($i = 1; $i < COUNT; $i++)
	shell_exec("echo '$i' > input; php mccarthy.php < input > /dev/null");
$afterphp = microtime(true);
echo "Time for PHP: " . ($afterphp - $beforephp) . PHP_EOL;

echo 'Testing C...' . PHP_EOL;
$beforec = microtime(true);
for($i = 1; $i < COUNT; $i++)
	shell_exec("echo '$i' > input; ./mccarthy < input > /dev/null");
$afterc = microtime(true);
echo "Time for C: " . ($afterc - $beforec) . PHP_EOL;

echo 'Testing C+compile...' . PHP_EOL;
$beforecc = microtime(true);
for($i = 1; $i < COUNT; $i++)
	shell_exec("gcc -o mccarthy mccarthy.c; echo '$i' > input; ./mccarthy < input > /dev/null");
$aftercc = microtime(true);
echo "Time for C+compile: " . ($aftercc - $beforecc) . PHP_EOL;

echo 'For comparison: Testing program that does nothing' . PHP_EOL;
$beforenull = microtime(true);
for($i = 1; $i < COUNT; $i++)
	shell_exec("echo '$i' > input; ./fortytwo < input > /dev/null");
$afternull = microtime(true);
echo "Time for program that does nothing: " . ($afternull - $beforenull) . PHP_EOL;

exit(0);
?>
