<?php
interface ArticleInterface {
	/*
	 * Types.
	 */
	const JOURNAL = 9; // no zero because that value may signify an error
	const CHAPTER = 1;
	const BOOK = 2;
	const THESIS = 3; // kind of degree in "series", university in "publisher"
	const WEB = 5;
	const MISCELLANEOUS = 6;
	const REDIRECT = 7; // ID of target in "parent"
	const SUPPLEMENT = 8; // ID of target in "parent", kind of supplement in "title"
	/*
	 * Constructors
	 */
	public static function makeNewNoFile(/* string */ $handle, ContainerList $parent);
	public static function makeNewRedirect(/* string */ $handle, $target, ContainerList $parent);
}
