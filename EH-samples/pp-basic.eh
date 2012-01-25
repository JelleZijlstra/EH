#!/usr/bin/ehi
# Generate a process_paras skeleton
if $argc != 3
	exit
end
set var = $argv->1
set desc = $argv->2
echo "	if($this->process_paras($paras, array("
echo "		'name' => __FUNCTION__,"
echo "		'synonyms' => array(0 => '" + $var + "'),"
echo "		'checklist' => array('" + $var + "' => '" + $desc + "'),"
echo "		'errorifempty' => array('" + $var + "'),"
echo "	);"
