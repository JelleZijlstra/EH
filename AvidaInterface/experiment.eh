// experiment.eh
// Jelle Zijlstra, November 2011
// Experimental protocol for OEB 192 Avida project
// Usage: php <path>/load.php <path>/experiment.eh
// where <path> is the directory both files reside in

// setconfig --debug=true
$ generaterun = 2000
$ initrun = 5000
$ secondrun = 5000
$ gen = 20
$ smallworld = 15
$ largeworld = 25
// Number of replicates on first run
// set to 4 
$ firstreps = 5
// Hold default configuration (analyze settings)
$ dflt = "--BIRTH_METHOD=4 --COPY_MUT_PROB=0.012"
// Number of input Avidians to be tested
// set to 5
$ inputs = 5

func genevents: n
	global generaterun
	global basefile
	avida_events_start
	avida_events_add --type=Inject --time=begin --arg=default-heads.org
	avida_events_add --type=SavePopulation --time=$generaterun --arg=$basefile-$n
	avida_events_add --type=Exit --time=$generaterun
	avida_events_push
endfunc

func genanalyze: n
	global basefile
	$ orgname = "input$n.org"
	avida_analyze_start
	avida_analyze_add --cmd=LOAD --arg=data/$basefile-$n
	avida_analyze_add --cmd=FIND_GENOTYPE
	avida_analyze_add --cmd=RECALCULATE
	avida_analyze_add --cmd=PRINT --arg='$orgname'
	avida_analyze_push
	avida_analyze $basefile-avida-$n
	avida_get_org $orgname
endfunc

func initevents: n, rep
	global initrun
	global basefile
	avida_events_start
	// Insert organism at beginning
	avida_events_add --type=Inject --time=begin --arg=input${n}.org
	// Exit and save population
	avida_events_add --type=SavePopulation --time=$initrun --arg=${basefile}-avida-$n-rep$rep
	avida_events_add --type=Exit --time=$initrun
	avida_events_push
endfunc

func initanalyze: n, rep
	global basefile
	avida_analyze_start
	$ orgname = "${basefile}-organism-$n-rep$rep.org"
	// Load results from previous analysis
	avida_analyze_add --cmd=LOAD --arg=data/${basefile}-avida-$n-rep$rep
	// Find most common genotype
	avida_analyze_add --cmd=FIND_GENOTYPE
	avida_analyze_add --cmd=RECALCULATE
	// Save data about winning genotype
	avida_analyze_add --cmd=DETAIL --arg=${basefile}-stats-$n-rep$rep
	// Save winning genotype in .org file
	avida_analyze_add --cmd=PRINT --arg='$orgname'
	avida_analyze_push
	avida_analyze ${basefile}-analyze-$n-rep$rep
	avida_get_org $orgname
endfunc

func secevents: n, rep
	global secondrun
	global basefile
	avida_events_start
	// Insert organism at beginning
	avida_events_add --type=Inject --time=begin --arg='init-small-organism-$n-rep$rep.org 0 -1 0'
	avida_events_add --type=Inject --time=begin --arg='init-large-organism-$n-rep$rep.org 1 -1 1'
	// Exit and save population
	avida_events_add --type=SavePopulation --time=$secondrun --arg=${basefile}-avida-$n-rep$rep
	avida_events_add --type=Exit --time=$secondrun
	avida_events_push
endfunc

func secanalyze: n, rep
	global basefile
	echo 'Compiling lineage data...'
	avida_lineages data/${basefile}-avida-$n-rep$rep
endfunc

// Base for filenames used
$ basefile = "generate"
avida_config --WORLD_X=$gen --WORLD_Y=$gen $dflt
echo 'Generating ancestral organisms...'
for $inputs count i
	echo 'Ancestor $i/$inputs...'
	call genevents: $i
	avida_run $basefile-terminal-$i
	call genanalyze: $i
endfor
avida_config_default

$ basefile = "init-small"
avida_config --WORLD_X=$smallworld --WORLD_Y=$smallworld $dflt
echo 'Evolving ancestor in small world...'
for $inputs count i
	echo 'Ancestor $i/$inputs...'
	for $firstreps count rep
		echo 'Replicate $rep/$firstreps...'
		call initevents: $i, $rep
		avida_run ${basefile}-terminal-$i-rep$rep
		// Quit for now after first execution
		// quit
		call initanalyze: $i, $rep
	endfor
endfor

avida_config_default

// Repeat this with different world size
$ basefile = "init-large"
avida_config --WORLD_X=$largeworld --WORLD_Y=$largeworld $dflt
echo 'Evolving ancestor in large world...'
for $inputs count i
	echo 'Ancestor $i/$inputs...'
	for $firstreps count rep
		echo 'Replicate $rep/$firstreps...'
		call initevents: $i, $rep
		avida_run ${basefile}-terminal-$i-rep$rep
		call initanalyze: $i, $rep
	endfor
endfor

avida_config_default

// Run competition experiments
$ basefile = "second"
avida_config --WORLD_X=$smallworld --WORLD_Y=$smallworld --LOG_LINEAGES=1 $dflt
echo 'Running competition between small-world and large-world organisms...'
for $inputs count i
	echo 'Ancestor $i/$inputs...'
	for $firstreps count rep
		echo 'Replicate $rep/$firstreps...'
		call secevents: $i, $rep
		avida_run ${basefile}-terminal-$i-rep$rep
		call secanalyze: $i, $rep
	endfor
endfor
// Find out results
// TODO
avida_config_default
