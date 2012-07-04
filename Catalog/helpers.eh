#!/usr/bin/ehi
# Functions that are of use in ArticleList

# Tiny finder
f := func: in {
	bfind --title='/' . $in . '/i'
}

# Fix Annals and Magazine of Natural History series and journal name
fixannals := func: dryRun {
	for $(bfind -q --journal='/Journal of Natural History Series \d+/') as file {
		journal := $(getField $file --field=journal)
		series := $(call preg_replace ['/^.*(\d+)$/', '$1', $journal])
		if $dryRun {
			echo $(getField $file --field=name)
			echo $series
		} else {
			setprops $file --journal='Annals and Magazine of Natural History' --series=$series
		}
	}
}

t := func: in {
	printvar: $(edit 'Agathaeromys nov.pdf' -c=$in)
}

t := func: in {
	switch $in
		case $is_string
			echo 'string'
		default
			echo 'not a string'
	end
}

h := func: author, year {
	bfind --year=$year --authors='/' . $author . '/i'
}
