<?php
class Publisher extends SqlListEntry {
	protected $id;
	
	protected $name;
	
	protected $location;
	
	protected static $Publisher_commands = array(
	);
	
	public function fields() {
		return array(
			new SqlProperty(array(
				'name' => 'id',
				'type' => SqlProperty::ID)),
			new SqlProperty(array(
				'name' => 'name',
				'type' => SqlProperty::STRING)),
			new SqlProperty(array(
				'name' => 'cities',
				'type' => SqlProperty::CUSTOM,
				'creator' => function($id) {
					$entries = Database::singleton()->select(array(
						'fields' => array('city_id'),
						'from' => 'publisher_id',
						'where' => array(
							'publisher_id' => Database::escapeValue($id)
						),
					));
					$out = array();
					foreach($entries as $entry) {
						$out[] = City::withId($author['city_id']);
					}
					return $out;
				})),			
		);
	}
}
