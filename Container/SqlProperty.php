<?php
/*
 * SqlProperty.php
 *
 * A class representing an individual property in a MySQL table.
 */
class SqlProperty extends Property {
	// function that fills a property
	private $manualFiller;
	public function getManualFiller() {
		return $this->manualFiller;	
	}
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
			case SqlProperty::REFERENCE:
			case SqlProperty::STRING:
			case SqlProperty::INT:
			case SqlProperty::BOOL:
				$fname = $this->getName();
				$validator = $this->getValidator();
				$processor = $this->getProcessor();
				$this->manualFiller = function(SqlListEntry $file, $table) use($fname, $validator, $processor) {
					return $file->menu(array(
						'head' => $fname . ': ',
						'headasprompt' => true,
						'options' => array(
							'e' => "Enter the file's command-line interface",
						),
						'validfunction' => $validator,
						'process' => array(
							'e' => function() use($file) {
								$file->edit();
								return true;
							},
						),
						'processcommand' => function($in) use($processor) {
							$out = $in;
							// enable having 'e' or 's' as field by typing '\e'
							if(isset($in[0]) and ($in[0] === '\\')) {
								$out = substr($in, 1);
							}
							return $processor($out);
						},
					));				
				};
				break;
			default:
				$this->manualFiller = function() {
					throw new EHException("Use of unspecified manualFiller");
				};
				break;
		}
	
	}

	/*
	 * for properties of type SqlProperty::REFERENCE, name of the class they
	 * refer to
	 */
	private $referredClass = NULL;
	public function getReferredClass() {
		return $this->referredClass;
	}
	protected function referredClassDefault() {
		if($this->type === self::REFERENCE) {
			throw new EHException(
				'referredClass not specified for SqlProperty of type REFERENCE');
		}
	}
	
	/*
	 * table name for JOINT_REFERENCEs
	 */
	private $tableName = NULL;
	public function getTableName() {
		return $this->tableName;
	}
	protected function tableNameDefault() {
		if($this->type === self::JOINT_REFERENCE) {
			throw new EHException(
				'tableName not specified for SqlProperty of type JOINT_REFERENCE'
			);
		}
	}
	
	/*
	 * AutomatedFiller: fills in certain properties automatically from the DB.
	 */
	private $automatedFiller = NULL;
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
