#!/usr/bin/ehi
# Compile an EH file into C++ code

include 'library.eh'

# Lazily build a string. This still has bad performance, since the toString function builds all intermediate strings.
# TODO: provide a native function that takes an iterator and builds a string from everything returned by that iterator.
class StringBuilder
	private pieces

	public initialize = () => (this.pieces = Nil)

	public operator<< = func: str
		this.pieces = str::(this.pieces)
		this
	end

	public toString = () => this.pieces.reduce("", (elt, rest => rest + elt))
end

class Compiler
	private fileName

	private output
	private functions = Nil
	private nFunctions = 0

	const private static header = '#include "../../ehi/eh.hpp"\n'

	public initialize = func: fileName
		this.fileName = fileName
		this.functions = []
	end

	public compile = func: outputFile
		private code = Macro.optimize(EH.parseFile(this.fileName))
		private mainf = StringBuilder.new()
		mainf << "int main(int argc, char *argv[]) {\n"
		# setup code
		mainf << "EHInterpreter interpreter;\n"
		mainf << "interpreter.eh_setarg(argc, argv);\n"
		mainf << 'EHI ehi_obj(end_is_end_e, &interpreter, interpreter.global_object, eh_getcwd(), "' << this.fileName << '");\n'
		mainf << "EHI *ehi = &ehi_obj;\n"
		mainf << "ehcontext_t context = ehi->get_context();\n"
		mainf << "try {\n"
		this.doCompile(mainf, code)
		mainf << ";\n} catch (eh_exception &e) {\n"
		mainf << "ehi->handle_uncaught(e);\n"
		mainf << "return -1;\n"
		mainf << "}\n"
		mainf << "return 0;\n"
		mainf << "}\n"

		# create full source file
		private outputf = StringBuilder.new()
		outputf << header
		for f in this.functions
			outputf << f
		end
		outputf << mainf

		# dirty work because EH's File and String handling is lacking
		shell("rm " + outputFile)
		shell("touch " + outputFile)
		output = File.new outputFile
		private cppcode = outputf.toString()
		output.puts cppcode
		output.close()

		# compile the C++
		echo shell("cd tmp && clang++ -v compile_test.cpp ../../ehi/libeh.a -std=c++11 -stdlib=libc++ -o eh_compiled")
	end

	private doCompile = sb, code => if code.typeId() != Node.typeId()
		match code.type()
			case "String"
				sb << 'String::make("' << code << '")'
			case "Integer"
				sb << "Integer::make(" << code << ")"
			case "Float"
				sb << "Float::make(" << code << ")"
			case "Bool"
				sb << "Bool::make(" << code << ")"
			case "Null"
				sb << "Null::make()"
		end
	else
		match code
			case Node.T_CALL_METHOD(@obj, @method, @args)
				sb << "ehi->call_method("
				this.doCompile(sb, obj)
				sb << ', "' << method << '", '
				this.doCompile(sb, args)
				sb << ", context)"
			case Node.T_CALL(@function, @args)
				sb << "ehi->call_function("
				this.doCompile(sb, function)
				sb << ", "
				this.doCompile(sb, args)
				sb << ", context)"
			case Node.T_VARIABLE(@name)
				sb << 'ehi->eh_op_dollar('
				this.doCompile(sb, name)
				sb << ', context)'
			case Node.T_SEPARATOR(@lhs, @rhs)
				this.doCompile(sb, lhs)
				sb << ";\n"
				this.doCompile(sb, rhs)
		end
	end
end

private co = Compiler.new(argv->1)
co.compile "tmp/compile_test.cpp"
