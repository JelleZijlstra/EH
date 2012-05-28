<?php
/*
 * SqlProperty.php
 *
 * A class representing an individual property in a MySQL table.
 */
class SqlProperty extends Property {
	// function that fills a property (won't be using it; retain code that may prove useful elsewhere)
	protected function manualFillerDefault() {
		switch($this->type) {
			case self::JOINT_REFERENCE:
				$this->manualFiller = function(SqlListEntry $file, /* string */ $table) {
					$referred = array();
					while(true) {
						$cmd = $this->menu(array(
							'head' => 
								'Please provide entries for field ' . $name,
							'options' => array('q' => 'Stop adding entries'),
							'processcommand' => function(&$cmd, &$data) {
								
							},
						));
					}
				};
				break;
		}	
	}

	/*
	 * for properties of type SqlProperty::REFERENCE, name of the class they
	 * refer to
	 */
	protected $referredClass = NULL;
	public function getReferredClass() {
		return $this->referredClass;
	}
	protected function referredClassDefault() {
		if($this->getType() === self::REFERENCE) {
			throw new EHException(
				'referredClass not specified for SqlProperty of type REFERENCE');
		}
	}
	
	/*
	 * table name for JOINT_REFERENCEs
	 */
	protected $tableName = NULL;
	public function getTableName() {
		return $this->tableName;
	}
	protected function tableNameDefault() {
		if($this->getType() === self::JOINT_REFERENCE) {
			throw new EHException(
				'tableName not specified for SqlProperty of type JOINT_REFERENCE'
			);
		}
	}
	
	/*
	 * AutomatedFiller: fills in certain properties automatically from the DB.
	 */
	protected $automatedFiller = NULL;
	public function getAutomatedFiller() {
		return $this->automatedFiller;
	}
	protected function automatedFillerDefault() {
		switch($this->getType()) {
			case self::CUSTOM:
				throw new EHException('creator not specified for SqlProperty '
					. 'of type CUSTOM');
			case self::CHILDREN:
				$this->automatedFiller = function(SqlListEntry $file, /* string */ $table) {
					$class = get_class($file);
					$table = strtolower($class);
					if($file->id() === NULL) {
						throw new EHException("Unable to set children array");
					}
					$children = Database::singleton()->select(array(
						'from' => $table,
						'where' => array(
							'parent' => Database::escapeValue($file->id()),
						),
					));
					$out = array();
					foreach($children as $child) {
						$out[] = $class::withId($in['id']);
					}
					return $out;
				};
				break;
			default:
				break;
		}
	}
	
	/*
	 * Constructor.
	 */
	public function __construct($data) {
		parent::__construct($data);
		$this->set($data, 'automatedFiller');
		$this->set($data, 'referredClass');
		$this->set($data, 'tableName');	
	}
}
