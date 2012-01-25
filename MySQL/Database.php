<?php
require_once(__DIR__ . "/../Common/common.php");
require_once(BPATH . "/Common/ExecuteHandler.php");
require_once(BPATH . "/MySQL/Database.settings.php");
class Database extends ExecuteHandler {
	private $err = false;
	private $table; // table we're currently working on
	private $columns; // columns in that table
	private $verbose = true;
	public $errmessage = '';
	public function __construct() {
		parent::__construct(self::$Database_commands);
		if(mysql_connect(DB_SERVER, DB_USERNAME, DB_PASSWORD) === false) {
			$this->err = true;
			$this->errmessage = mysql_error();
			return;
		}
		// go to the right database
		$this->query("USE " . mysql_real_escape_string(DB_NAME));
	}
	protected static $Database_commands = array(
		'count' => array('name' => 'count',
			'aka' => 'dbcount',
			'desc' => 'Count from a table',
			'arg' => 'None',
			'execute' => 'callmethod'),
		'settable' => array('name' => 'settable',
			'desc' => 'Set the default table to work with',
			'arg' => 'Table name',
			'execute' => 'callmethod'),
	);
	public function cli() {
		return $this->setup_commandline('Database');
	}
	private function query($sql) {
		$result = mysql_query($sql);
		if($result === false) {
			$this->err = true;
			$this->errmessage = mysql_error();
			if($this->verbose) {
				echo 'Query failed: ' . $sql . PHP_EOL;
				echo 'Error message: ' . $this->errmessage . PHP_EOL;
			}
		}
		else {
			$this->err = false;
		}
		return $result;
	}
	public function settable(array $paras) {
		if($this->process_paras($paras, array(
			'name' => __FUNCTION__,
			'synonyms' => array(0 => 'table'),
			'checklist' => array('table' => 'Table to set to'),
			'errorifempty' => array('table'),
		)) === PROCESS_PARAS_ERROR_FOUND) return false;
		$table = mysql_real_escape_string($paras['table']);
		$columns = $this->query("SHOW COLUMNS FROM " . $table);
		if(!$columns) {
			echo 'Failed to set table' . PHP_EOL;
			return false;
		}
		$this->table = $table;
		$this->columns = array();
		while($column = mysql_fetch_array($columns)) {
			$this->columns[] = $column['Field'];
		}
		return true;
	}
	public function count(array $paras) {
		$query = 'SELECT COUNT(*) FROM ' . $this->table . ' ' . $this->where($paras);
		$result = $this->query($query);
		if(!$result)
			return false;
		$row = mysql_fetch_array($result);
		return $row[0];
	}
	private function where($paras) {
	// assemble where clause
		$out = 'WHERE ';
		foreach($paras as $key => $value) {
			if(in_array($key, $this->columns)) {
				$out .= mysql_real_escape_string($key) .
					' = ' .
					mysql_real_escape_string($value) .
					' AND ';
			}
		}
		$out = preg_replace('/ AND $/u', '', $out);
		return $out;
	}
	public function showcolumns() {
	// show columns in default table
		if(!$this->table)
			return false;
		echo 'Columns in table ' . $this->table . PHP_EOL;
		foreach($this->columns as $column) {
			echo $column . PHP_EOL;
		}
		return true;
	}
}
