<?php
/*
 * Autoloader for the EH framework.
 */

abstract class AutoLoader {
	public static $BPATH;
	private static $locations = array(
		// Common
		'ExecuteHandler' => '/Common/ExecuteHandler.php',
		'Sanitizer' => '/Common/Sanitizer.php',

		// Container
		'ContainerList' => '/Container/ContainerList.php',
		'CsvContainerList' => '/Container/CsvContainerList.php',
		'CsvListEntry' => '/Container/CsvListEntry.php',
		'ListEntry' => '/Container/ListEntry.php',
		'SqlContainerList' => '/Container/SqlContainerList.php',
		'SqlListEntry' => '/Container/SqlListEntry.php',
		'SqlProperty' => '/Container/SqlProperty.php',
		
		// Catalog
		'Age' => '/Catalog/Age.php',
		'AgeList' => '/Catalog/AgeList.php',
		'Article' => '/Catalog/Article.php',
		'ArticleList' => '/Catalog/ArticleList.php',
		'Author' => '/Catalog/Author.php',
		'AuthorList' => '/Catalog/AuthorList.php',
		'City' => '/Catalog/City.php',
		'CityList' => '/Catalog/CityList.php',
		'Folder' => '/Catalog/Folder.php',
		'FolderList' => '/Catalog/FolderList.php',
		'Journal' => '/Catalog/Journal.php',
		'JournalList' => '/Catalog/JournalList.php',
		'Location' => '/Catalog/Location.php',
		'LocationList' => '/Catalog/LocationList.php',
		'Name' => '/Catalog/Name.php',
		'NameList' => '/Catalog/NameList.php',
		'Publisher' => '/Catalog/Publisher.php',
		'PublisherList' => '/Catalog/PublisherList.php',
		
		// List
		'Country' => '/List/Country.php',
		'Taxon' => '/List/Taxon.php',
		'TaxonList' => '/List/TaxonList.php',

		// GeneralList
		'GeneralList' => '/GeneralList/GeneralList.php',
		
		// MySQL
		'Database' => '/MySQL/Database.php',
		
		// Parse
		'Citation' => '/Parse/parse.php',
		'Parser' => '/Parse/parser.php',
		
		// UcuchaBot
		'Bot' => '/UcuchaBot/Bot.php',
		'FacsList' => '/UcuchaBot/Facs.php',
		'FacsEntry' => '/UcuchaBot/Facs.php',
	);
	public static function load($class) {
		if(isset(self::$locations[$class])) {
			require_once(self::$BPATH . self::$locations[$class]);
			return true;
		} else {
			return false;
		}
	}
}

AutoLoader::$BPATH = __DIR__ . '/..';
function __autoload($class) {
	return AutoLoader::load($class);
}