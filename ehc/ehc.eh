#!/usr/bin/ehi
#
# ehc, the EH compiler
#
# Compiles an EH file into C++ code
#
# This compiler does not produce much of an improvement over the interpreter, since it works by hooking into the
# same C++ methods that the interpreter calls to execute a program. Nevertheless, it may offer some performance
# improvement.
#
# The compiler is still matching the following language features:
# - switch
# - multi-level break and continue
# - raw
# - commands
# - named arguments
#
# There are some minor behavioral differences between the interpreter and this compiler:
# - When multiple values in an array have the same key, the compiler chooses the last one and the interpreter the first one
#   For example, the code "printvar [3 => 4, 3 => 5]" will evaluate differently.
# - When a pattern match gives the wrong number of constructor arguments, the interpreter will match against the given
#   arguments before throwing an error, while the compiler will immediately throw an error before performing any match.
#   For example (asssuming A.B takes two arguments) "case A.B(@var1, @var2, @var3)" will set var1 and var2 in the
#   interpreter, but not in the compiler.
# - Return, break, and continue in unusual places (e.g., within class definitions or finally blocks) may be executed
#   differently by the interpreter and compiler.
# All of these only affect code that is poorly written anyway, and hopefully I'll be able to solve them with better
# static analysis in the future.
#
# Compilation currently follows these steps:
# - EH.parseFile is called to get the AST representation of the file to be compiled.
# - Preprocessor.replace_include replaces all include statements with the actual code to be included. (This will fail
#   when the include method is overwritten or aliased.)
# - Macro.optimize performs some small optimization and abstract operators like + into method calls.
# - Preprocessor.listify replaces some of the more awkward parts of the parser outputs (e.g., tuples formed like
#   T_COMMA(foo, T_COMMA(bar, T_COMMA(baz, quux))))) with more manageable T_LIST constructs.
# - compiler.eh generates C++ code from the AST emitted by the previous phases.
# - clang++ compiles the generated C++ code and links it to libeh.a.

include '../lib/library.eh'
include 'preprocessor.eh'
include 'compiler.eh'
include '../lib/argument_parser.eh'

private replace_extension(file, extension) = do
	private len = file.length()
	if len > 3 and file.slice(-3, max: null) == '.eh'
		private base = file.slice(0, max: len - 3)
		base + extension
	else
		file + '.ehc' + extension
	end
end

private main(argc, argv) = do
	private ap = ArgumentParser.new("Compiler for the EH language", (
		{name: "--output", synonyms: ['-o'], desc: "Output file to use", nargs: 1, dflt: null},
		{name: "input", desc: "Input file"},
		{name: "--to-cpp", synonyms: ['-c'], desc: "Output C++ code; do not compile to machine code", type: Bool, dflt: false},
		{name: "--verbose", synonyms: ['-v'], desc: "Give verbose output", type: Bool, dflt: false},
		{name: "--optimize", synonyms: ['-O'], desc: "Turn on optimizations", type: Bool, dflt: false}
	))
	private args = ap.parse argv
	private verbose = args->'verbose'
	if verbose
		echo "Parsing input file..."
	end
	private code = EH.parseFile(args->'input')

	private preprocessed_code = Preprocessor.preprocess(code, args->'input', verbose: args->'verbose')

	if args->'output' == null
		if args->'to-cpp'
			args->'output' = replace_extension(args->'input', '.cpp')
		else
			args->'output' = replace_extension(args->'input', '')
		end
	end
	args->'output' = EH.escapeShellArgument(args->'output')

	private tmp_name = EH.escapeShellArgument(Compiler.new(fileName: args->'input', code: preprocessed_code, verbose: verbose).compile())

	if args->'to-cpp'
		private cmd = "mv " + tmp_name + " " + args->'output'
		if verbose
			echo cmd
		end
		shell cmd
	else
		# invoke clang
		private cmd = "clang++ "
		if args->'optimize'
			cmd += "-O3 "
		end
		cmd += tmp_name + " " + workingDir() + "/../ehi/libeh.a -std=c++11 -stdlib=libc++ -o " + args->'output'
		if verbose
			echo cmd
		end
		shell(cmd)
		shell("rm " + tmp_name)
	end
end

main(argc, argv)
