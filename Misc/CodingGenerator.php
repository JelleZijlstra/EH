<?php
/*
 * CodingGenerator.php
 *
 * Generate a character coding for a taxon in standard format.
 */
class CodingGenerator extends ExecuteHandler {
	private static $CodingGenerator_commands = array(
		'generate' => array(
			'name' => 'generate',
			'desc' => 'Generate coding for data in a file',
		),
	);
	public function __construct() {
		parent::__construct(self::${__CLASS__ . '_commands'});
	}
	
	public function generate(array $paras) {
		if(!$this->process_paras($paras, array(
			'name' => __FUNCTION__,
			'synonyms' => array(
				0 => 'file',
				'q' => 'quiet',
			),
			'checklist' => array(
				'file' => 'File to get data from',
				'quiet' => 'Whether to be quiet and not print the output',
				'spacing' => 'Whether to put spaces between characters at regular intervals',
			),
			'askifempty' => array(
				'file',
			),
			'default' => array(
				'quiet' => false,
				'spacing' => 10,
			),
		))) return false;
		$input = fopen($paras['file'], 'r');
		if($input === false) {
			throw new EHException("Could not open input file");
		}
		$total = -1;
		$taxon = '';
		$codings = array();
		while(($line = fgets($input)) !== false) {
			if(preg_match('/^(\d+)\.([\d?])(\s+\(.*\))?$/u', $line, $matches)) {
				$codings[$matches[1]] = $matches[2];
			} elseif(preg_match('/^Codings for (.*)$/u', $line, $matches)) {
				$taxon = $matches[1];
			} elseif(preg_match('/^Total (\d+)$/u', $line, $matches)) {
				$total = (int) $matches[1];
			} elseif($line !== '') {
				throw new EHException('Invalid input line: ' . $line);
			}
		}
		if($total === -1) {
			throw new EHException('Total number of characters not given');
		}
		if($taxon === '') {
			throw new EHException('Taxon not given');
		}
		$out = $taxon . ' ';
		for($i = 1; $i <= $total; $i++) {
			if(isset($codings[$i])) {
				$out .= $codings[$i];
			} else {
				$out .= '?';
			}
			if($paras['spacing'] !== 0 && ($i % 10) === 0) {
				$out .= ' ';
			}
		}
		if(!$paras['quiet']) {
			echo $out . PHP_EOL;
		}
		return $out;
	}
}
