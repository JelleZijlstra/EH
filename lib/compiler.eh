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

class Attributes
	private is_private = false
	private is_static = false
	private is_const = false

	public initialize = func: is_private, is_static, is_const
		this.is_private = is_private
		this.is_static = is_static
		this.is_const = is_const
	end

	public static make_private = () => this.new(true, false, false)

	public static make_static = () => this.new(false, true, false)

	public static make_const = () => this.new(false, false, true)

	public static make = () => this.new(false, false, false)

	public toString = func:
		private sb = StringBuilder.new()
		sb << "attributes_t("
		sb << if this.is_private; "private_e"; else "public_e"; end << ", "
		sb << if this.is_static; "static_e"; else "nonstatic_e"; end << ", "
		sb << if this.is_const; "const_e"; else "nonconst_e"; end << ")"
		sb.toString()
	end

	public static parse = code => match code
		case Node.T_END
			this.make()
		case Node.T_ATTRIBUTE(Attribute.constAttribute, @tail)
			private out = this.parse tail
			out.is_const = true
			out
		case Node.T_ATTRIBUTE(Attribute.staticAttribute, @tail)
			private out = this.parse tail
			out.is_static = true
			out
		case Node.T_ATTRIBUTE(Attribute.publicAttribute, @tail)
			private out = this.parse tail
			out.is_private = false
			out
		case Node.T_ATTRIBUTE(Attribute.privateAttribute, @tail)
			private out = this.parse tail
			out.is_private = true
			out
	end
end

class Compiler
	private fileName

	private output
	private functions = Nil
	private counter = 0

	const private static header = '#include "../../ehi/eh_compiled.hpp"\n'

	public initialize = func: fileName
		this.fileName = fileName
	end

	public compile = func: outputFile
		private code = Macro.optimize(EH.parseFile(this.fileName))
		private mainf = StringBuilder.new()
		mainf << 'const char *get_filename() { return "' << this.fileName << '"; }\n'

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
		shell("cd tmp && clang++ compile_test.cpp ../../ehi/libeh.a -std=c++11 -stdlib=libc++ -o eh_compiled")
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
					sb << assignment << 'eh_compiled::get_variable("' << name << '", context, ehi)'
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
				case Node.T_ASSIGN(@lvalue, @rvalue)
					private rvalue_name = this.doCompile(sb, rvalue)
					this.compile_set(sb, lvalue, rvalue_name, null)
					sb << assignment << rvalue_name << ";\n"
				case _
					printvar code
					throw(MiscellaneousError.new("Cannot compile this expression"))
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
		sb << "ehval_p " << func_name << "(ehval_p obj, ehval_p args, EHI *ehi, const ehcontext_t &context) {\n"
		sb << "ehval_p ret;\n"
		this.compile_set(sb, args, "args", Attribute.make_private())

		this.doCompile(sb, code)
		sb << "return ret;\n}\n"

		# add function to list
		this.functions = sb.toString()::this.functions

		# return name of the C++ function created
		func_name
	end

	private compile_set = sb, lvalue, name, attributes => match lvalue
		case Node.T_NULL
			# assert that rvalue is null
			sb << "if(!" << name << "->is_a<Null>()) {\n"
			sb << 'throw_MiscellaneousError("Non-null value assigned to null", ehi);\n}\n'
		case Node.T_ANYTHING
			# ignore
		case Node.T_ARROW(@base, @accessor)
			private base_name = this.doCompile(sb, base)
			private accessor_name = this.doCompile(sb, accessor)
			sb << "ehi->call_method(" << base_name << ', "operator->=", Tuple::create({'
			sb << accessor_name << ", " << name << "}, ehi->get_parent()), context);\n"
		case Node.T_ACCESS(@base, @accessor)
			private base_name = this.doCompile(sb, base)
			sb << "if(" << base_name << "->is_a<SuperClass>()) {\n"
			sb << 'throw_TypeError("Cannot set member on parent class", ' << base_name << ", ehi);\n}\n"
			sb << "if(!" << base_name << "->is_a<Object>()) {\n"
			sb << 'throw_TypeError("Cannot set member on primitive", ' << base_name << ", ehi);\n}\n"
			if attributes == null
				sb << base_name << '->set_property("' << accessor << '", ' << name << ", context, ehi);\n"
			else
				sb << base_name << '->set_member("' << accessor << '", ehmember_p('
				sb << attributes << ", " << name << "), context, ehi);\n"
			end
		case Node.T_VARIABLE(@var_name)
			if attributes == null
				private member_name = "ehc_member" + this.counter
				this.counter++
				sb << "ehmember_p " << member_name << ' = context.scope->get<Object>()->get_recursive("'
				sb << var_name << '", context);\n'
				sb << "if(" << member_name << " == nullptr) {\n"
				sb << "if(" << member_name << "->isconst()) {\n"
				sb << 'throw_ConstError(context.scope, "' << var_name << '", ehi);\n}\n'
				sb << member_name << "->value = " << name << ";\n"
				sb << "} else {\n"
				sb << 'context.scope->set_member("' << var_name << '", ehmember_p(attributes_t(), '
				sb << name << "), context, ehi);\n}\n"
			else
				sb << 'context.scope->set_member("' << var_name << '", ehmember_p(' << attributes << ", "
				sb << name << "), context, ehi);\n"
			end
		case Node.T_CLASS_MEMBER(@attributes_code, @lval)
			private inner_attributes = Attributes.parse attributes_code
			this.compile_set(sb, lval, name, inner_attributes)
		case Node.T_GROUPING(@lval)
			this.compile_set(sb, lval, name, attributes)
		case _
			printvar lvalue
			throw(MiscellaneousError.new "Cannot compile this lvalue")
	end
end

private co = Compiler.new(argv->1)
co.compile "tmp/compile_test.cpp"
