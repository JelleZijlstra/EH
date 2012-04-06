<?php
require_once(__DIR__ . "/../Common/common.php");
require_once(BPATH . "/Common/ExecuteHandler.php");
require_once(BPATH . "/MySQL/Database.settings.php");
class DatabaseException extends EHException {
	public $query;
	public function __construct($msg, $query = '') {
		$msg = 'Database exception: ' . $msg;
		if($query !== '') {
			$msg = $msg . ' (query: "' . $query . '")';
		}
		parent::__construct($msg, EHException::E_RECOVERABLE);
	}
}

class Database extends ExecuteHandler {
	private $connection;
	protected static $Database_commands = array(
		'count' => array('name' => 'count',
			'aka' => 'dbcount',
			'desc' => 'Count from a table',
			'arg' => 'None',
			'execute' => 'callmethod'),
		'doQuery' => array('name' => 'doQuery',
			'aka' => 'query',
			'desc' => 'Perform an arbitrary query'),
		'select' => array('name' => 'select',
			'desc' => 'Perform a SELECT query'),
		'insert' => array('name' => 'insert',
			'desc' => 'Perform an INSERT query'),
	);
	/*
	 * Constructor: does EH stuff and connects to DB
	 */
	public function __construct() {
		parent::__construct(self::$Database_commands);
		$connection = mysql_connect(DB_SERVER, DB_USERNAME, DB_PASSWORD);
		if($connection === false) {
			throw new DatabaseException(mysql_error());
		}
		$this->connection = $connection;
		// go to the right database
		$this->query("USE " . mysql_real_escape_string(DB_NAME));
	}
	
	/*
	 * Get a DB instance.
	 */
	public static function singleton() {
		static $db = false;
		if($db === false) {
			$db = new Database();
		}
		return $db;
	}
	
	/*
	 * Sets up EH CLI. Not sure we'd need it much.
	 */
	public function cli() {
		return $this->setup_commandline('Database');
	}
	
	/*
	 * Perform a query, without additional protections.
	 */
	public function query(/* string */ $sql) {
		$result = mysql_query($sql, $this->connection);
		if($result === false) {
			throw new DatabaseException(mysql_error(), $sql);
		}
		return $result;
	}
	
	/*
	 * Wrapper for query to use as a command.
	 */
	public function doQuery(array $paras) {
		if($this->process_paras($paras, array(
			'name' => 'doQuery',
			'synonyms' => array(
				0 => 'query',
			),
			'checklist' => array(
				'query' => 'Query to perform',
			),
			'errorifempty' => array(
				'query',
			),
		)) === PROCESS_PARAS_ERROR_FOUND) return false;
		$result = $this->query($paras['query']);
		if(is_resource($result)) {
			$result = self::arrayFromSql($result);
		}
		return $result;
	}
	
	/*
	 * Counting.
	 */
	public function count(array $paras) {
		if($this->process_paras($paras, array(
			'name' => __FUNCTION__,
			'synonyms' => array(
				0 => 'from',
				1 => 'where',
			),
			'checklist' => array(
				'from' => 'Table to use',
				'where' => 'Array of where clauses',
			),
			'errorifempty' => array(
				'from',
			),
			'default' => array(
				'where' => false,
			),
		)) === PROCESS_PARAS_ERROR_FOUND) return false;
		$query = 'SELECT COUNT(*) FROM ' . $paras['from'];
		if($paras['where'] !== false) {
			$query .= ' ' . self::where($paras['where']);
		}
		$result = $this->query($query);
		$row = mysql_fetch_array($result);
		return $row[0];
	}
	
	/*
	 * 
	 */
	public function select(array $paras) {
		if($this->process_paras($paras, array(
			'name' => __FUNCTION__,
			'synonyms' => array(
				0 => 'fields',
				1 => 'from',
				2 => 'where',
			),
			'checklist' => array(
				'fields' => 'Array of fields',
				'from' => 'Table to select from',
				'where' => 'Where clauses',
			),
			'errorifempty' => array(
				'from',
			),
			'default' => array(
				'fields' => '*',
				'where' => false,
			),
			'checkparas' => array(
				'fields' => function($val, $paras) {
					return is_array($val);
				},
				'from' => function($val, $paras) {
					return is_string($val);
				},
				'where' => function($val, $paras) {
					return is_array($val);
				}
			),
		)) === PROCESS_PARAS_ERROR_FOUND) return false;
		$sql = 'SELECT ' 
			. ($paras['fields'] === '*' 
				? '*' 
				: self::assembleFieldList($paras['fields']))
			. ' FROM ' . self::escapeField($paras['from']);
		if($paras['where'] !== false) {
			$sql .= ' WHERE ' . self::where($paras['where']);
		}
		$result = $this->query($sql);
		return self::arrayFromSql($result);
	}
	
	/*
	 * Insertion
	 */
	public function insert(array $paras) {
		if($this->process_paras($paras, array(
			'name' => __FUNCTION__,
			'synonyms' => array(
				0 => 'into',
				1 => 'values',
			),
			'checklist' => array(
				'into' => 'Table to insert into',
				'values' => 'Associative array of values',
				'onduplicate' => 'What to do with a duplicate',
			),
			'errorifempty' => array(
				'into', 'values',
			),
			'checkparas' => array(
				'into' => function($val, $paras) {
					return is_string($val);
				},
				'values' => function($val, $paras) {
					return is_array($val);
				},
			),
			'default' => array(
				'onduplicate' => false,
			),
		)) === PROCESS_PARAS_ERROR_FOUND) return false;
		$sql = 'INSERT INTO ' . self::escapeField($paras['into']) . '('
			. implode(', ', array_map(
				array('Database', 'escapeField'), array_keys($paras['values'])))
			. ') VALUES(' 
			. implode(', ', array_map(
				array('Database', 'escapeValue'), $paras['values']))
			. ')';
		// TODO: handle onduplicate
		return $this->query($sql);
	}
	
	/*
	 * Private functions.
	 */
	private static function where(array $conditions) {
	// assemble where clause
		return implode(' AND ', array_map(function($key, $value) {
			return Database::escapeField($key) . ' = ' 
				. Database::escapeValue($value);
		}, array_keys($conditions), $conditions));
	}
	private static function assembleFieldList(array $fields) {
		return implode(', ', array_map(array($this, 'escapeField'), $fields));
	}
	public static function escapeField(/* string */ $in) {
		if(is_string($in) and preg_match('/^[a-z_]+$/', $in)) {
			return '`' . $in . '`';
		} else {
			throw new DatabaseException('Invalid field "' . $in . '"');
		}
	}
	private static function arrayFromSql(/* resource */ $in) {
		$out = array();
		while(($row = mysql_fetch_assoc($in)) !== false) {
			$out[] = $row;
		}
		return $out;
	}
	public static function escapeValue(/* mixed */ $in) {
		if(is_integer($in) || is_float($in) || is_object($in)) {
			return (string) $in;
		} elseif(is_string($in)) {
			return "'" . mysql_real_escape_string($in) . "'";
		} elseif(is_bool($in)) {
			return $in ? 'true' : 'false';
		} else {
			throw new DatabaseException('Invalid type ' . gettype($in));
		}
	}
}
