<?php
class Occurrence extends SqlListEntry {
	protected $id;
	
	protected $taxon;
	
	protected $location;
	
	protected $age;
	
	protected $articles;
	
	protected static $Occurrence_commands = array(
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
				'name' => 'location',
				'type' => SqlProperty::REFERENCE,
				'referredClass' => 'Location')),
			new SqlProperty(array(
				'name' => 'age',
				'type' => SqlProperty::REFERENCE,
				'referredClass' => 'age')),
			new SqlProperty(array(
				'name' => 'articles',
				'type' => SqlProperty::JOINT_REFERENCE,
				'tableName' => 'occurrence_article',
				'referredClass' => 'article')),
		);
	}
}
