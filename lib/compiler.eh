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
	private counter = 0

	const private static header = '#include "../../ehi/eh_compiled.hpp"\n'

	public initialize = func: fileName
		this.fileName = fileName
		this.functions = Nil
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
		mainf << "eh_main(ehi, ehi->get_context());\n"
		mainf << "} catch (eh_exception &e) {\n"
		mainf << "ehi->handle_uncaught(e);\n"
		mainf << "return -1;\n"
		mainf << "}\n"
		mainf << "return 0;\n"
		mainf << "}\n"

		private eh_mainf = StringBuilder.new()
		eh_mainf << "ehval_p eh_main(EHI *ehi, const ehcontext_t &context) {\nehval_p ret;\n"
		this.doCompile(eh_mainf, code)
		eh_mainf << "return ret;\n}\n"

		# create full source file
		private outputf = StringBuilder.new()
		outputf << header
		for f in this.functions
			outputf << f
		end
		outputf << eh_mainf
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

	private doCompile = func: sb, code
		private var_name = "eh_var" + this.counter
		this.counter++
		private assignment = "ehval_p " + var_name + " = "
		if code.typeId() != Node.typeId()
			sb << assignment
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
					private obj_name = this.doCompile(sb, obj)
					private args_name = this.doCompile(sb, args)
					sb << assignment << "ehi->call_method(" << obj_name
					sb << ', "' << method << '", ' << args_name << ", context)"
				case Node.T_CALL(@function, @args)
					private func_name = this.doCompile(sb, function)
					private args_name = this.doCompile(sb, args)
					sb << assignment << "ehi->call_function(" << func_name << ", " << args_name << ", context)"
				case Node.T_VARIABLE(@name)
					sb << assignment << 'ehi->eh_op_dollar(String::make("' << name << '"), context)'
				case Node.T_SEPARATOR(@lhs, @rhs)
					this.doCompile(sb, lhs)
					private returning_name = this.doCompile(sb, rhs)
					sb << assignment << returning_name
				case Node.T_NULL
					sb << assignment << "Null::make()"
				case Node.T_THIS
					sb << assignment << "context.object"
				case Node.T_SCOPE
					sb << assignment << "context.scope"
				case Node.T_RET(@val)
					private ret_name = this.doCompile(sb, val)
					sb << assignment << ret_name << ";\n"
					sb << "return " << ret_name
				case Node.T_FUNC(@args, @code)
					private func_name = this.compile_function(args, code)
					sb << assignment << "eh_compiled::make_closure(" << func_name << ", context, ehi)"
			end
		end
		sb << ";\n"
		# to enable implicit return
		sb << "ret = " << var_name << ";\n"
		# return name of variable created
		var_name
	end

	private compile_function = func: args, code
		private func_name = "ehc_function" + this.counter
		this.counter++

		private sb = StringBuilder.new()
		sb << "ehval_p " << func_name << "(ehval_p obj, ehval_p args, EHI *ehi) {\n"
		sb << "ehval_p ret;\n"
		this.compile_set(sb, args, "args")

		this.doCompile(sb, code)
		sb << "return ret;\n}\n"

		# add function to list
		this.functions = sb.toString()::this.functions

		# return name of the C++ function created
		func_name
	end

	private compile_set = func: sb, lvalue, name

	end
end

private co = Compiler.new(argv->1)
co.compile "tmp/compile_test.cpp"
