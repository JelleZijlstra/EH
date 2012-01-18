<?php
/*
 * GeneralList - generates a FileList/ListEntry-type set of classes to handle an input CSV file.
 */
require_once(__DIR__ . '/../Common/common.php');
define('GLISTREPO', BPATH . '/GeneralList');
require_once(BPATH . '/GeneralList/ExecuteHandler.php');
class GeneralList extends ExecuteHandler {
	static private $switches = array('bool', 'json', 'serialize', 'normal');
	static $GeneralList_commands = array(
		'create' => array('name' => 'create',
			'desc' => 'Create new GeneralList file',
			'arg' => 'Input file name',
			'execute' => 'callmethodarg'),
	);
	public function __construct() {
		parent::__construct(self::$GeneralList_commands);
		$this->cli();
	}
	public function create($file) {
	// $file: input CSV file
		if(!is_readable($file)) {
			echo 'No such file' . PHP_EOL;
			return false;
		}
		if(substr($file, -4) !== '.csv') {
			echo 'Unsupported file type' . PHP_EOL;
			return false;
		}
		// set name used
		$classname = ucfirst(substr($file, 0, -4));
		$in = fopen($file, 'r');
		if(!$in) return false;
		$firstline = fgetcsv($in);
		if(!$firstline) {
			echo 'Unable to retrieve data labels' . PHP_EOL;
			return false;
		}
		$fields = array();
		foreach($firstline as $i => $field) {
			$pos = strpos($field, '[');
			if($pos !== false) {
				$fieldname = substr($field, 0, $pos);
				$switch = substr($field, $pos + 1);
				if(!in_array($switch, self::$switches)) {
					echo 'Invalid switch for field ' . $fieldname . ': ' . $switch . PHP_EOL;
					// set to default
					$switch = 'normal';
				}
			}
			else {
				$fieldname = $field;
				$switch = 'normal';
			}
			$fieldname = preg_replace('/\s+/u', '_', $fieldname);
			$fields[$i] = array('name' => $fieldname, 'type' => $switch);
		}
		$loadfile = GLISTREPO . '/Load' . $classname . '.php';
		$outload = fopen($loadfile, 'w');
		if(!$outload) return false;
		fwrite($outload, "<?php
require_once(__DIR__ . '/../Common/common.php');
define('GLISTREPO', BPATH . '/Common/GeneralList');
require_once(GLISTREPO . '/$classname.php');
\${$classname}List = new {$classname}List();
\${$classname}List->cli();
");
		fclose($outload);
		$classfile = GLISTREPO . '/' . $classname . '.php';
		$outclass = fopen($classfile, 'w');
		if(!$outclass) return false;
		$realpath = realpath($file);
		fwrite($outclass, "<?php
require_once(__DIR__ . '/../Common/common.php');
require_once(BPATH . '/Common/FileList.php');
require_once(BPATH . '/Common/ListEntry.php');
class {$classname}List extends FileList {
	protected static \$fileloc = '$realpath';
	protected static \$childclass = '{$classname}Entry';
	public function __construct() {
	// all handled adequately in parent
		parent::__construct();
	}
	public function cli() {
		\$this->setup_commandline('$classname');
	}
}
class {$classname}Entry extends ListEntry {
	// stuff that is standard across GeneralList outputs
	protected static \$parentlist = '{$classname}List';
	protected static \$arrays_to_check = array();
	protected static \${$classname}Entry_commands = array(

	);
	protected static \${$classname}Entry_synonyms = array(

	);
	// complex stuff
");
		foreach($fields as $field) {
			fwrite($outclass,
"	public \${$field['name']};
");
		}
		fwrite($outclass,
"	public function __construct(\$in = '', \$code = '') {
		global \${self::\$parentlist};
		if(!\${self::\$parentlist}) \$this->p = \${self::\$parentlist};
		switch(\$code) {
			case 'f': // loading from file
");
		foreach($fields as $i => $field) {
			switch($field['type']) {
				case 'json': fwrite($outclass,
"				if(\$in[$i]) \$this->{$field['name']} = json_decode(\$in[$i], true);
"); break;
				case 'serialize': fwrite($outclass,
"				if(\$in[$i]) \$this->{$field['name']} = unserialize(\$in[$i], true);
"); break;
				default: fwrite($outclass,
"				\$this->{$field['name']} = \$in[$i];
"); break;
			}
		}
		// close __construct(), start toarray()
		fwrite($outclass,
"				break;
			case 'n': // associative array
				if(!\$in['name']) {
					echo 'Error: name must be provided' . PHP_EOL;
					return false;
				}
				foreach(\$in as \$key => \$value) {
					if(self::has_property(\$key))
						\$this->\$key = \$value;
				}
				break;
			default:
				echo 'Invalid code' . PHP_EOL;
				break;
		}
	}
	function toarray() {
		\$out = array();
");
		foreach($fields as $field) {
			switch($field['type']) {
				case 'json': fwrite($outclass,
"		\$out[] = \$this->getarray('{$field['name']}');
"); break;
				case 'serialize': fwrite($outclass,
"		\$out[] = \$this->getarray('{$field['name']}', array('func' => 'serialize'));
"); break;
				default: fwrite($outclass,
"		\$out[] = \$this->{$field['name']};
"); break;
			}
		}
		// close toarray(), start format()
		fwrite($outclass,
"		return \$out;
	}
	function format() {
		\$bools = array();
");
		foreach($fields as $field) {
			if($field['type'] === 'bool') {
				fwrite($outclass,
"		\$bools[] = '{$field['name']}';
");
			}
		}
		// close format(), end the file
		fwrite($outclass,
"		foreach(\$bools as \$bool) {
			\$this->\$bool = \$this->\$bool ? 1 : NULL;
		}
	}
}
");
		fclose($outclass);
		require_once($loadfile);
		return true;
	}
	public function cli() {
		$this->setup_commandline('generallist');
	}
}
?>
