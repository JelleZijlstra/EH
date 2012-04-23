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
			'desc' => 'Count from a table'),
		'query' => array('name' => 'query',
			'desc' => 'Perform an arbitrary query'),
		'select' => array('name' => 'select',
			'desc' => 'Perform a SELECT query'),
		'insert' => array('name' => 'insert',
			'desc' => 'Perform an INSERT query'),
		'delete' => array('name' => 'delete',
			'desc' => 'Perform a DELETE query'),
		'update' => array('name' => 'update',
			'desc' => 'Perform an UPDATE query'),
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
		$this->rawQuery("USE " . mysql_real_escape_string(DB_NAME));
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
	 * Perform a query, without additional protections. Callers outside this
	 * class should use Database::query() instead.
	 */
	private function rawQuery(/* string */ $sql) {
		$result = mysql_query($sql, $this->connection);
		if($result === false) {
			throw new DatabaseException(mysql_error(), $sql);
		}
		return $result;
	}
	
	/*
	 * Wrapper for query to use as a command.
	 */
	public function query(array $paras) {
		if($this->process_paras($paras, array(
			'name' => __FUNCTION__,
			'toarray' => 'query',
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
		$result = $this->rawQuery($paras['query']);
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
		$result = $this->rawQuery($query);
		$row = mysql_fetch_array($result);
		return $row[0];
	}
	
	/*
	 * Select.
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
				'order_by' => 'ORDER BY clause',
			),
			'errorifempty' => array(
				'from',
			),
			'default' => array(
				'fields' => '*',
				'where' => false,
				'order_by' => false,
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
				'order_by' => function($val, $paras) {
					return is_string($val);
				},
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
		if($paras['order_by'] !== false) {
			$sql .= ' ORDER BY ' . self::escapeField($paras['order_by']);
		}
		$result = $this->rawQuery($sql);
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
		return $this->rawQuery($sql);
	}
	
	/*
	 * Deletion
	 */
	public function delete(array $paras) {
		if($this->process_paras($paras, array(
			'name' => __FUNCTION__,
			'synonyms' => array(
				0 => 'from',
				1 => 'where',
			),
			'checklist' => array(
				'from' => 'Table to delete from',
				'where' => 'Where clauses',
			),
			'errorifempty' => array(
				'from', 'where',
			),
			'checkparas' => array(
				'from' => function($val, $paras) {
					return is_string($val);
				},
				'where' => function($val, $paras) {
					return is_array($val);
				},
			),
		)) === PROCESS_PARAS_ERROR_FOUND) return false;
		$sql = 'DELETE FROM ' . self::escapeField($paras['from']) . ' WHERE '
			. self::where($paras['where']);
		return $this->rawQuery($sql);
	}
	
	/*
	 * Peform an update.
	 */
	public function update(array $paras) {
		if($this->process_paras($paras, array(
			'name' => __FUNCTION__,
			'synonyms' => array(
				0 => 'table',
				1 => 'set',
				2 => 'where',
			),
			'checklist' => array(
				'table' => 'Table to update',
				'set' => 'Fields to set',
				'where' => 'Where clauses',
			),
			'errorifempty' => array(
				'table', 'set', 'where',
			),
			'checkparas' => array(
				'table' => function($val, $paras) {
					return is_string($val);
				},
				'set' => function($val, $paras) {
					return is_array($val);
				},
				'where' => function($val, $paras) {
					return is_array($val);
				},
			),
		)) === PROCESS_PARAS_ERROR_FOUND) return false;
		$sql = 'UPDATE ' . self::escapeField($paras['table']) . ' SET '
			. self::assembleSet($paras['set'])
			. ' WHERE ' . self::where($paras['where']);
		return $this->rawQuery($sql);		
	}
	
	/*
	 * Static functions used to assemble part of a query.
	 */
	 
	/*
	 * Assemble a WHERE ... AND ... clause.
	 */
	public static function where(array $conditions) {
	// assemble where clause
		return implode(' AND ', array_map(function($key, $value) {
			return Database::escapeField($key) . ' = ' 
				. Database::escapeValue($value);
		}, array_keys($conditions), $conditions));
	}
	
	/*
	 * Assemble a list of fields like `name`, `id`.
	 */
	private static function assembleFieldList(array $fields) {
		return implode(', ', 
			array_map(array('Database', 'escapeField'), $fields)
		);
	}
	
	/*
	 * Assemble something of the form `id` = 5, `name` = Jelle.
	 */
	public static function assembleSet(array $fields) {
		return implode(', ', array_map(function($key, $value) {
			return Database::escapeField($key) . ' = ' 
				. Database::escapeValue($value);
		}, array_keys($fields), $fields));
	}
	
	/*
	 * Escape a field name like name -> `name`. Throws an exception if
	 * it is not a valid field name. Accepts only a subset of what MySQL
	 * actually accepts.
	 */
	public static function escapeField(/* string */ $in) {
		if(is_string($in) and preg_match('/^[a-z_][a-z0-9_]+$/', $in)) {
			return '`' . $in . '`';
		} else {
			throw new DatabaseException('Invalid field "' . $in . '"');
		}
	}
	
	/*
	 * Convert a MySQL response resource into an array. 
	 * Private since callers outside the class should never get such a 
	 * resource.
	 */
	private static function arrayFromSql(/* resource */ $in) {
		$out = array();
		while(($row = mysql_fetch_assoc($in)) !== false) {
			$out[] = $row;
		}
		return $out;
	}
	
	/*
	 * Convert a value into something that can go into a MySQL query.
	 */
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
