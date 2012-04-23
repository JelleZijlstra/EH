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
	
	public function fields() {
		return array(
			new SqlProperty(array(
				'name' => 'id',
				'type' => SqlProperty::ID)),
			new SqlProperty(array(
				'name' => 'taxon',
				'type' => SqlProperty::REFERENCE,
				'referredClass' => 'Taxon')),
			new SqlProperty(array(
				'name' => 'group',
				'type' => SqlProperty::INT)),
			new SqlProperty(array(
				'name' => 'status',
				'type' => SqlProperty::INT)),
			new SqlProperty(array(
				'name' => 'original_name',
				'type' => SqlProperty::STRING)),
			new SqlProperty(array(
				'name' => 'base_name',
				'type' => SqlProperty::STRING)),
			new SqlProperty(array(
				'name' => 'year',
				'type' => SqlProperty::STRING,
				'validator' => function($in) {
					return preg_match('/^\d{4}(–\d{4})?\??$/', $in);
				})),
			new SqlProperty(array(
				'name' => 'page_described',
				'type' => SqlProperty::STRING)),
			new SqlProperty(array(
				'name' => 'type',
				'type' => SqlProperty::REFERENCE,
				'referredClass' => 'Name')),
			new SqlProperty(array(
				'name' => 'verbatim_type',
				'type' => SqlProperty::STRING)),
			new SqlProperty(array(
				'name' => 'article',
				'type' => SqlProperty::REFERENCE,
				'referredClass' => 'Article')),
			new SqlProperty(array(
				'name' => 'verbatim_citation',
				'type' => SqlProperty::STRING)),
			new SqlProperty(array(
				'name' => 'location',
				'type' => SqlProperty::REFERENCE,
				'referredClass' => 'Location')),
			new SqlProperty(array(
				'name' => 'type_locality',
				'type' => SqlProperty::STRING)),
			new SqlProperty(array(
				'name' => 'nomenclature_comments',
				'type' => SqlProperty::STRING)),
			new SqlProperty(array(
				'name' => 'taxonomy_comments',
				'type' => SqlProperty::STRING)),
			new SqlProperty(array(
				'name' => 'other_comments',
				'type' => SqlProperty::STRING)),
		);
	}
}
