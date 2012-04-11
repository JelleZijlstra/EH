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
			'id', 'taxon', 'name', 'group', 'status', 'original_name', 
			'base_name', 'year', 'page_described', 'article', 
			'verbatim_citation', 'type', 'verbatim_type', 'location',
			'type_locality', 'nomenclature', 'taxonomy', 'other'
		);
	}
}
