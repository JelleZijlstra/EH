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

include 'library.eh'
include 'preprocessor.eh'
include 'compiler.eh'

private co = Compiler.new(argv->1)
#private co = Compiler.new("/Users/jellezijlstra/code/EH/lib/tmp/test5.eh")
co.compile "tmp/compile_test.cpp"
