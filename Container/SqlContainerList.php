<?php
/*
 * SqlContainerList.php
 *
 * SQL-based implementation of a ContainerList.
 *
 * This class works together with the SqlListEntry class. It makes a number of
 * assumptions about the layout of the underlying database:
 *	- Records have a numeric field `id` and a string `name` that are both 
 *		unique. (Possibly the uniqueness of `name` will not be necessary.)
 *	- Fields with names of the form X_id are references to records in the table
 *		X with id corresponding to X_id. These are represented as objects of 
 *		class X, part of an XList container class.
 */
require_once(__DIR__ . '/../Common/common.php');
require_once(BPATH . '/MySQL/Database.php');
require_once(BPATH . '/Container/ContainerList.interface.php');
require_once(BPATH . '/Container/SqlListEntry.php');

abstract class SqlContainerList extends ContainerList {
	private $c = array();
	private $cByName = array();
	
	// name of table corresponding to this object
	abstract protected function table();

	protected function _addEntry(ListEntry $file, array $paras) {
		Database::singleton()->insert(array(
			'into' => $this->table(),
			'values' => $file->toArray(),
		));
		$this->c[$file->id()] = $file;
		$this->cByName[$file->name()] = $file;
		return true;
	}
	
	protected function _removeEntry(/* string */ $file, array $paras) {
		Database::singleton()->delete(array(
			'from' => $this->table(),
			'where' => array('id' => $file->id()),
		));
		unset($this->c[$file->id()]);
		unset($this->cByName[$file->name()]);
		return true;
	}
	
	/*
	 * Ensure an entry is in memory here.
	 */
	private function grabEntry(/* string */ $name) {
		if(isset($this->cByName[$name])) {
			return $this->cByName[$name];
		}
		$res = Database::singleton()->select(array(
			'from' => $this->table(),
			'where' => array('name' => $name),
		));
		if(count($res) === 0) {
			return false;
		} elseif(count($res) === 1) {
			$obj = new static::$childClass($res[0], SqlListEntry::CONSTR_FULL);
			$this->c[$obj->id()] = $obj;
			$this->cByName[$name] = $obj;
		} else {
			throw new EHException("Duplicate objects in table " .
				$this->table() . " with name " . $name,
				EHException::E_RECOVERABLE);
		}
	}
	
	/*
	 * Put all entries in the DB in memory. This is really beside the point of
	 * the SqlContainerList, and should be avoided.
	 */
	private function grabAllEntries() {
		$entries = Database::singleton()->select(array(
			'from' => $this->table()
		));
		foreach($entries as $entry) {
			if(!isset($this->c[$entry['id']])) {
				$obj = new static::$childClass(
					$entry, SqlListEntry::CONSTR_FULL);
				$this->c[$obj->id()] = $obj;
				$this->cByName[$obj->name()] = $obj;
			}
		}
	}

	/*
	 * Move an entry.
	 */
	protected function _moveEntry(/* string */ $file, /* string */ $newName, array $paras) {
		$obj = $this->grabEntry($file);
		$this->cByName[$newname] = $obj;
		unset($this->cByName[$file]);
		return true;
	}
	
	/*
	 * Whether we have a child of this name.
	 */
	public function has(/* string */ $file) {
		if($this->grabEntry($file) === false) {
			return false;
		} else {
			return true;
		}
	}
	
	/*
	 * Get a particular child, or a field of that child.
	 */
	public function get(/* string */ $file) {
		$obj = $this->grabEntry($file);
		if($obj === false) {
			echo 'Invalid file: ' . $file;
			return false;
		}
		return $obj;
	}
	
	/*
	 * Return an object on the basis of ID.
	 */
	public function fromId(/* int */ $id) {
		if(isset($this->c[$id])) {
			return $this->c[$id];
		} else {
			$obj = new static::$childclass($id, SqlListEntry::CONSTR_ID, $list);
			$this->addEntry($obj);
			return $obj;
		}
	}

	/*
	 * Save anything necessary.
	 */
	public function saveIfNeeded() {
		$this->each(function($file) {
			$file->save();
		});
	}
	
	/*
	 * Tell the object that a save is needed.
	 */
	function needsave() {
		// do nothing; is kept track of by children
	}
	
	/*
	 * Go through all children and check validity, make minor corrections, 
	 * etcetera.
	 */
	protected function _formatAll(array $paras) {
		$this->each(function($file) {
			$file->format();
		});
	}

	/*
	 * Perform a function on each member.
	 */
	public function each($f) {
		$this->grabAllEntries();
		foreach($this->c as $entry) {
			$f($entry);
		}
	}

	/*
	 * List all children.
	 */
	protected function _listMembers(array $paras) {
		$fields = Database::singleton()->select(array(
			'fields' => array('name'),
			'from' => $this->table()
		));
		foreach($fields as $entry) {
			echo $entry['name'];
		}
		return true;
	}
	
	/*
	 * Provide statistics.
	 */
	protected function _stats(array $paras = array()) {
		// TODO: provide statistics
		return true;
	}
	
	/*
	 * Count members.
	 */
	public function count() {
		return Database::singleton()->count(array(
			'from' => $this->table(),
		));
	}
	
	/*
	 * Make a list of the possible values of a field.
	 */
	protected function _mlist(array $paras) {
		$childclass = static::$childclass;
		// test whether we need to do a WHERE
		$dowhere = false;
		$whereparas = array();
		foreach($paras as $para => $content) {
			if($childclass::hasproperty($para)) {
				$dowhere = true;
				$whereparas[$para] = $content;
			}
		}
		$query = 'SELECT ' . Database::escapeField($paras['field']) 
			. ', COUNT(*) FROM '
			. Database::escapeField($this->table()) . ' GROUP BY '
			. Database::escapeField($paras['field']);
		if($dowhere) {
			$query .= ' WHERE ' . Database::where($whereparas);
		}
		$values = Database::singleton()->query($query);
		$out = array();
		foreach($values as $value) {
			// TODO: does this work?
			$out[$value[$paras['field']]] = $value['count'];
		}
		return $out;
	}
	
	/*
	 * Queries.
	 */
	protected function _bfind(array $queries, array $paras) {
		// TODO: make this work.
		// We will perhaps make an extended version of Database::where() that
		// can handle bfind's complicated needs.
		return array();
	}

}
