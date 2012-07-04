-- The main catalog.
DROP TABLE IF EXISTS `article`;
CREATE TABLE `article` (
	`id` INT UNSIGNED AUTO_INCREMENT,
	`name` VARCHAR(255) NOT NULL,
	-- 0 for nofile
	`folder_id` INT UNSIGNED NOT NULL,
	`added` DATETIME NOT NULL,
	-- Journal article, chapter, book, web, thesis, misc, redirect, supplement
	`type` INT NOT NULL,
	-- Authors in article_authors table
	`year` VARCHAR(255) DEFAULT NULL,
	`title` VARCHAR(512) DEFAULT NULL,
	`journal_id` INT DEFAULT NULL,
	`series` VARCHAR(255) DEFAULT NULL,
	`volume` VARCHAR(255) DEFAULT NULL,
	`issue` VARCHAR(255) DEFAULT NULL,
	`start_page` VARCHAR(255) DEFAULT NULL,
	`end_page` VARCHAR(255) DEFAULT NULL,
	`pages` VARCHAR(255) DEFAULT NULL,
	-- Book for book chapter
	`parent` INT UNSIGNED DEFAULT NULL,
	`publisher_id` INT UNSIGNED DEFAULT NULL,
	-- Any other data we wish to preserve
	`misc_data` VARCHAR(4096) DEFAULT NULL,
	PRIMARY KEY(`id`),
	INDEX(`name`)
) ENGINE=INNODB DEFAULT CHARSET=utf8;

-- Publishers and their locations
DROP TABLE IF EXISTS `publisher`;
CREATE TABLE `publisher` (
	`id` INT UNSIGNED AUTO_INCREMENT,
	`name` VARCHAR(512),
	PRIMARY KEY(`id`),
	INDEX(`name`)
) ENGINE=INNODB DEFAULT CHARSET=utf8;

DROP TABLE IF EXISTS `city`;
CREATE TABLE `city` (
	`id` INT UNSIGNED AUTO_INCREMENT,
	`name` VARCHAR(255) NOT NULL,
	`state` VARCHAR(255) DEFAULT NULL,
	`country` VARCHAR(255) NOT NULL,
	PRIMARY KEY(`id`),
	INDEX(`name`)
) ENGINE=INNODB DEFAULT CHARSET=utf8;

DROP TABLE IF EXISTS `publisher_city`;
CREATE TABLE `publisher_city` (
	`publisher_id` INT UNSIGNED,
	`city_id` INT UNSIGNED,
	INDEX(`publisher_id`),
	INDEX(`city_id`)
) ENGINE=INNODB DEFAULT CHARSET=utf8;

-- How do we organize folders?
DROP TABLE IF EXISTS `folder`;
CREATE TABLE `folder` (
	`id` INT UNSIGNED AUTO_INCREMENT,
	-- Parent folder. May be 0 for the global folder.
	`parent` INT UNSIGNED NOT NULL,
	`name` VARCHAR(255),
	PRIMARY KEY(`id`),
	INDEX(`name`)
) ENGINE=INNODB DEFAULT CHARSET=utf8;

-- Authors
DROP TABLE IF EXISTS `author`;
CREATE TABLE `author` (
	`id` INT UNSIGNED AUTO_INCREMENT,
	-- In form "J.S." or "Jelle Sjoerd"
	`first_names` VARCHAR(255) DEFAULT NULL,
	`name` VARCHAR(255) NOT NULL,
	-- E.g. "Jr." or "Sr."
	`suffix` VARCHAR(255) DEFAULT NULL,
	PRIMARY KEY(`id`),
	INDEX(`name`)
) ENGINE=INNODB DEFAULT CHARSET=utf8;

-- Authors for each article
DROP TABLE IF EXISTS `article_author`;
CREATE TABLE `article_author` (
	`article_id` INT UNSIGNED NOT NULL,
	`author_id` INT UNSIGNED NOT NULL,
	-- 0 for first author, 1 for second, etc.
	`position` INT UNSIGNED NOT NULL,
	INDEX(`article_id`),
	INDEX(`author_id`)
) ENGINE=INNODB DEFAULT CHARSET=utf8;

-- Journals
DROP TABLE IF EXISTS `journal`;
CREATE TABLE `journal` (
	`id` INT UNSIGNED AUTO_INCREMENT,
	`name` VARCHAR(255) NOT NULL,
	`issn` VARCHAR(255) DEFAULT NULL,
	-- Whether this is a journal that has no page numbers
	`nopagenumber` BOOL DEFAULT FALSE,
	PRIMARY KEY(`id`),
	INDEX(`name`)
) ENGINE=INNODB DEFAULT CHARSET=utf8;

-- Identifiers (e.g., URLs, DOIs, HDLs, JSTOR IDs)
DROP TABLE IF EXISTS `article_identifier`;
CREATE TABLE `article_identifier` (
	`article_id` INT UNSIGNED NOT NULL,
	-- Indicate that an identifier does not have a 1-to-1 relationship to this work.
	`identifier_is_shared` BOOL DEFAULT FALSE,
	-- E.g., 'jstor' (constants defined in Article.php)
	`identifier` INT NOT NULL,
	`data` VARCHAR(255) NOT NULL,
	INDEX(`article_id`),
	INDEX(`data`)
) ENGINE=INNODB DEFAULT CHARSET=utf8;

--
-- TAXONOMY
-- 

DROP TABLE IF EXISTS `taxon`;
CREATE TABLE `taxon` (
	`id` INT UNSIGNED AUTO_INCREMENT,
	`name` VARCHAR(512) NOT NULL,
	`rank` INT NOT NULL,
	`comments` VARCHAR(4096) DEFAULT NULL,
	PRIMARY KEY(`id`),
	INDEX(`name`)
) ENGINE=INNODB DEFAULT CHARSET=utf8;

DROP TABLE IF EXISTS `name`;
CREATE TABLE `name` (
	`id` INT UNSIGNED AUTO_INCREMENT,
	-- Name of valid taxon this belongs to
	`taxon_id` INT UNSIGNED,
	-- Species, genus, family, or higher
	`group` INT NOT NULL,
	-- Valid, synonym, or species inquirenda
	`status` INT NOT NULL,
	`original_name` VARCHAR(512) DEFAULT NULL,
	`base_name` VARCHAR(512) DEFAULT NULL,
	`year` VARCHAR(255) DEFAULT NULL,
	`page_described` VARCHAR(255) DEFAULT NULL,
	`article_id` INT UNSIGNED DEFAULT NULL,
	`verbatim_citation` VARCHAR(1024) DEFAULT NULL,
	-- ID of type genus/species for family, genus group
	`type_id` INT UNSIGNED DEFAULT NULL,
	`verbatim_type` VARCHAR(512) DEFAULT NULL,
	-- ID representing place (i.e., state/country) where type locality is
	`location_id` INT UNSIGNED DEFAULT NULL,
	`typelocality` VARCHAR(1024) DEFAULT NULL,
	`nomenclature_comments` VARCHAR(4096) DEFAULT NULL,
	`taxonomy_comments` VARCHAR(4096) DEFAULT NULL,
	`other_comments` VARCHAR(4096) DEFAULT NULL,
	PRIMARY KEY(`id`),
	INDEX(`original_name`),
	INDEX(`base_name`),
	INDEX(`taxon_id`)
) ENGINE=INNODB DEFAULT CHARSET=utf8;


--
-- GEOGRAPHY
--
DROP TABLE IF EXISTS `location`;
CREATE TABLE `location` (
	`id` INT UNSIGNED AUTO_INCREMENT,
	`parent` INT UNSIGNED,
	`name` VARCHAR(255) NOT NULL,
	PRIMARY KEY(`id`),
	INDEX(`name`)
) ENGINE=INNODB DEFAULT CHARSET=utf8;

DROP TABLE IF EXISTS `age`;
CREATE TABLE `age` (
	`id` INT UNSIGNED AUTO_INCREMENT,
	`parent` INT UNSIGNED,
	-- In thousands of years
	`end_time` INT UNSIGNED,
	`start_time` INT UNSIGNED,
	`name` VARCHAR(255) NOT NULL,
	PRIMARY KEY(`id`),
	INDEX(`name`)
) ENGINE=INNODB DEFAULT CHARSET=utf8;

DROP TABLE IF EXISTS `occurrence`;
CREATE TABLE `occurrence` (
	`id` INT UNSIGNED AUTO_INCREMENT,
	`taxon_id` INT UNSIGNED,
	`location_id` INT UNSIGNED,
	`age_id` INT UNSIGNED,
	`comments` VARCHAR(4096) DEFAULT NULL,
	PRIMARY KEY(`id`),
	INDEX(`taxon_id`)
) ENGINE=INNODB DEFAULT CHARSET=utf8;

DROP TABLE IF EXISTS `occurrence_article`;
CREATE TABLE `occurrence_article` (
	`occurrence_id` INT UNSIGNED,
	`article_id` INT UNSIGNED,
	INDEX(`occurrence_id`),
	INDEX(`article_id`)
);
