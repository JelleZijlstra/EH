#!/usr/bin/ehi

include '../lib/library.eh'
include '../lib/argument_parser.eh'

if !(String.isRegexAvailable())
	echo(argv->0 + ": error: regex support is required for this script to run")
	exit 1
end

class TestCase
	public static executer
	private name
	private expected

	public static make file = do
		TestCase.new file.run()
	end

	public initialize(this.name) = do
		this.short_name = this.name.replace('\\.[a-z]+$', '')
		this.getExpected()
	end

	public run() = do
		echo('Testing file ' + this.name + '...')
		shell(this.makeTestCommand())
		private result = this.testOutput 'stdout' && this.testOutput 'stderr'
		this.cleanUp()
		result
	end

	private getExpected() = do
		private expected_f = File.new(this.short_name + '.expected')
		if expected_f == false
			throw(ArgumentError.new("Unable to open test output file for " + this.name))
		end
		private expected = {}
		private currentKey = ''
		private currentValue = ''
		while (private line = expected_f.gets()) != null
			if line.doesMatch("^%%(.*)%%\\s*$")
				if currentKey != ''
					expected->currentKey = currentValue
				end
				currentKey = line.replace("(^%%|%%\\s*$)", '')
				currentValue = ''
			else
				currentValue += line
			end
		end
		expected->currentKey = currentValue
		this.expected = expected
		expected_f.close()
	end

	private makeTestCommand() = do
		private command = executer + ' ' + this.name
		if this.expected.has 'arguments'
			command += ' ' + this.expected->'arguments'.trim() + ' '
		end
		private stdoutFile = 'tmp/' + this.short_name + '.stdout'
		private stderrFile = 'tmp/' + this.short_name + '.stderr'
		command += ' > ' + stdoutFile + ' 2> ' + stderrFile
		command
	end

	private testOutput stream = do
		private outputFile = 'tmp/' + this.short_name + '.' + stream
		private output = File.readFile outputFile
		private regex_stream = stream + '-regex'
		if this.expected.has regex_stream
			if !(output.doesMatch(this.expected->regex_stream))
				this.failTest(stream, outputFile, this.expected->regex_stream)
				false
			else
				true
			end
		elsif this.expected.has stream
			if output != this.expected->stream
				this.failTest(stream, outputFile, this.expected->stream)
				false
			else
				true
			end
		elsif output != ''
			this.failTest(stream, outputFile, '')
			false
		else
			true
		end
	end

	private failTest(name, outputFile, expected) = do
		echo('Failed test for ' + name + "!")
		private tempFile = "tmp/" + this.short_name + ".tmp"
		shell("touch " + tempFile)
		private tf = File.new tempFile
		tf.puts expected
		tf.close()
		put(shell("diff '" + tempFile + "' '" + outputFile + "'"))
	end

	private cleanUp() = shell("rm tmp/" + this.short_name + ".*")
end

private ap = ArgumentParser.new("Test case runner", (
	{name: '--valgrind', desc: 'Whether to use Valgrind', type: Bool, dflt: false},
	{name: '--optimize', synonyms: ['-O'], desc: "Whether to use the optimizing interpreter", type: Bool, dflt: false},
	{name: '--program', synonyms: ['-p'], desc: "Program to run tests on", type: String, dflt: "/usr/bin/ehi", nargs: 1},
	{name: 'file', desc: "File to test", nargs: '+'}
))
private args = ap.parse argv
private executer = args->'program'
if args->'valgrind'
	executer = 'valgrind -q --leak-check=full ' + executer
end
if args->'optimize'
	executer += ' -O'
end
TestCase.executer = executer

shell "mkdir -p tmp"

if args->'file'.length() == 0
	private testfiles = File.new "testfiles"
	private total = 0
	private passed = 0
	while((private file = testfiles.gets()) != null)
		if TestCase.make(file.trim())
			passed++
		end
		total++
	end
	testfiles.close()
	pct = if total == 0; 100; else (passed * 100.0 / total).round(); end
	echo("Passed " + passed + " of " + total + " tests (" + pct + "%)")
else
	args->'file'.each(TestCase.make)
end
