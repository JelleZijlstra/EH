<?php
class Name extends SqlListEntry {
	protected $id;
	
	protected $taxon;
	
	protected $name;
	
	protected $group;
	
	protected $status;
	
	protected $original_name;
	
	protected $base_name;
	
	protected $year;
	
	protected $page_described;
	
	protected $article;
	
	protected $verbatim_citation;
	
	protected $type;
	
	protected $verbatim_type;
	
	protected $location;
	
	protected $type_locality;
	
	protected $nomenclature_comments;
	
	protected $taxonomy_comments;
	
	protected $other_comments;
	
	protected static $Name_commands = array(
	);
	
	protected static function fillFields() {
		return array(
			'id' => new SqlProperty(array(
				'name' => 'id',
				'type' => SqlProperty::ID)),
			'taxon' => new SqlProperty(array(
				'name' => 'taxon',
				'type' => SqlProperty::REFERENCE,
				'referredClass' => 'Taxon')),
			'group' => new SqlProperty(array(
				'name' => 'group',
				'type' => SqlProperty::INT)),
			'status' => new SqlProperty(array(
				'name' => 'status',
				'type' => SqlProperty::INT)),
			'original_name' => new SqlProperty(array(
				'name' => 'original_name',
				'type' => SqlProperty::STRING)),
			'base_name' => new SqlProperty(array(
				'name' => 'base_name',
				'type' => SqlProperty::STRING)),
			'year' => new SqlProperty(array(
				'name' => 'year',
				'type' => SqlProperty::STRING,
				'validator' => function($in) {
					return preg_match('/^\d{4}(–\d{4})?\??$/', $in);
				})),
			'page_described' => new SqlProperty(array(
				'name' => 'page_described',
				'type' => SqlProperty::STRING)),
			'type' => new SqlProperty(array(
				'name' => 'type',
				'type' => SqlProperty::REFERENCE,
				'referredClass' => 'Name')),
			'verbatim_type' => new SqlProperty(array(
				'name' => 'verbatim_type',
				'type' => SqlProperty::STRING)),
			'article' => new SqlProperty(array(
				'name' => 'article',
				'type' => SqlProperty::REFERENCE,
				'referredClass' => 'Article')),
			'verbatim_citation' => new SqlProperty(array(
				'name' => 'verbatim_citation',
				'type' => SqlProperty::STRING)),
			'location' => new SqlProperty(array(
				'name' => 'location',
				'type' => SqlProperty::REFERENCE,
				'referredClass' => 'Location')),
			'type_locality' => new SqlProperty(array(
				'name' => 'type_locality',
				'type' => SqlProperty::STRING)),
			'nomenclature_comments' => new SqlProperty(array(
				'name' => 'nomenclature_comments',
				'type' => SqlProperty::STRING)),
			'taxonomy_comments' => new SqlProperty(array(
				'name' => 'taxonomy_comments',
				'type' => SqlProperty::STRING)),
			'other_comments' => new SqlProperty(array(
				'name' => 'other_comments',
				'type' => SqlProperty::STRING)),
		);
	}
}
