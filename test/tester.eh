#!/usr/bin/ehi

include '../lib/library.eh'
include '../lib/argument_parser.eh'

class TestCase
	public static executer
	private name
	private expected

	public static make = func: file
		private name = file.replace("\.eh$", "")
		TestCase.new name.run()
	end

	public initialize(this.name) = this.getExpected()

	public run = func:
		echo('Testing file ' + this.name + '...')
		shell(this.makeTestCommand())
		this.testOutput 'stdout'
		this.testOutput 'stderr'
		this.cleanUp()
	end

	private getExpected = func:
		private expected_f = File.new(this.name + '.expected')
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

	private makeTestCommand = func:
		private command = this.executer + ' ' + this.name + '.eh'
		if this.expected.has 'arguments'
			command += ' ' + this.expected->'arguments'.trim() + ' '
		end
		private stdoutFile = 'tmp/' + this.name + '.stdout'
		private stderrFile = 'tmp/' + this.name + '.stderr'
		command += ' > ' + stdoutFile + ' 2> ' + stderrFile
		command
	end

	private testOutput = func: stream
		private outputFile = 'tmp/' + this.name + '.' + stream
		private output = File.readFile outputFile
		private regex_stream = stream + '-regex'
		if this.expected.has regex_stream
			if !output.doesMatch(this.expected->regex_stream)
				this.failTest(stream, outputFile, this.expected->regex_stream)
			end
		elsif this.expected.has stream
			if output != this.expected->stream
				this.failTest(stream, outputFile, this.expected->stream)
			end
		elsif output != ''
			this.failTest(stream, outputFile, '')
		end
	end

	private failTest = func: name, outputFile, expected
		echo('Failed test for ' + name + "!")
		private tempFile = "tmp/" + this.name + ".tmp"
		shell("touch " + tempFile)
		File.new tempFile.puts expected
		echo(shell("diff '" + tempFile + "' '" + outputFile + "'").trim())
	end

	private cleanUp() = shell("rm tmp/" + this.name + ".*")
end

private ap = ArgumentParser.new("Test case runner", ( \
	{name: '--valgrind', desc: 'Whether to use Valgrind', type: Bool, dflt: false}, \
	{name: '--optimize', synonyms: ['-O'], desc: "Whether to use the optimizing interpreter", type: Bool, dflt: false}, \
	{name: 'file', desc: "File to test", nargs: '+'} \
))
private args = ap.parse argv
private executer = ''
if args->'valgrind'
	executer += 'valgrind -q --leak-check=full '
end
executer += '/usr/bin/ehi'
if args->'optimize'
	executer += ' -O'
end
TestCase.executer = executer

shell "mkdir -p tmp"

if args->'file'.length() == 0
	private testfiles = File.new "testfiles"
	while((private file = testfiles.gets()) != null)
		TestCase.make(file.trim())
	end
	testfiles.close()
else
	args->'file'.each(_, v => TestCase.make v)
end

