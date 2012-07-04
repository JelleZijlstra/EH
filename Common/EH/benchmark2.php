<?php
// driver.php: Script to compare performance for EH scripting, PHP, and C using the McCarthy function
require_once(__DIR__ . '/../common.php');
define('COUNT', 100000);

echo 'Testing ehi...' . PHP_EOL;
$beforeehi = microtime(true);
shell_exec(BPATH . "/EH-samples/mccarthyloop.eh " . COUNT . " > /dev/null");
$afterehi = microtime(true);
echo "Time for ehi: " . ($afterehi - $beforeehi) . PHP_EOL;

echo 'Testing PHP...' . PHP_EOL;
$beforephp = microtime(true);
shell_exec(BPATH . "/Common/EH/mccarthyloop.php " . COUNT . " > /dev/null");
$afterphp = microtime(true);
echo "Time for PHP: " . ($afterphp - $beforephp) . PHP_EOL;

echo 'Testing C...' . PHP_EOL;
$beforec = microtime(true);
shell_exec(BPATH . "/Common/EH/mccarthyloop " . COUNT . " > /dev/null");
$afterc = microtime(true);
echo "Time for C: " . ($afterc - $beforec) . PHP_EOL;

exit(0);
?>
