#!/usr/bin/ehi
# Generate a process_paras skeleton
if $argc != 3
	exit
end
var = $argv->1
desc = $argv->2
echo "		if($this->process_paras($paras, array("
echo "			'name' => __FUNCTION__,"
echo "			'synonyms' => array(0 => '" + $var + "'),"
echo "			'checklist' => array('" + $var + "' => '" + $desc + "'),"
echo "			'errorifempty' => array('" + $var + "'),"
echo "		)) === PROCESS_PARAS_ERROR_FOUND) return false;"
