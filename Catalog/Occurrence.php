<?php
class Occurrence extends SqlListEntry {
	protected $id;
	
	protected $taxon;
	
	protected $location;
	
	protected $age;
	
	protected $articles;
	
	protected static $Occurrence_commands = array(
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
			'location' => new SqlProperty(array(
				'name' => 'location',
				'type' => SqlProperty::REFERENCE,
				'referredClass' => 'Location')),
			'age' => new SqlProperty(array(
				'name' => 'age',
				'type' => SqlProperty::REFERENCE,
				'referredClass' => 'age')),
			'articles' => new SqlProperty(array(
				'name' => 'articles',
				'type' => SqlProperty::JOINT_REFERENCE,
				'tableName' => 'occurrence_article',
				'referredClass' => 'article')),
		);
	}
}
