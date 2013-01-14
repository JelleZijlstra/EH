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

class NotImplemented
	this.inherit Exception
end

class Counter
	private counts = {}

	public initialize = () => (this.counts = {})

	public get_id = group => if this.counts.has group
		this.counts->group += 1
		this.counts->group
	else
		this.counts->group = 0
		0
	end
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
	private counter

	const private static header = '#include "../../ehi/eh_compiled.hpp"\n'

	public initialize = func: fileName
		this.fileName = fileName
		this.counter = Counter.new()
	end

	public compile = func: outputFile
		private code = Macro.listify(Macro.optimize(EH.parseFile(this.fileName)))
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
		shell("cd tmp && clang++ -O3 compile_test.cpp ../../ehi/libeh.a -std=c++11 -stdlib=libc++ -o eh_compiled")
	end

	private doCompile = func: sb, code
		private var_name = this.get_var_name "var"
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
				# Basic operations (variables, assignments, calls)
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
				case Node.T_ASSIGN(@lvalue, @rvalue)
					private rvalue_name = this.doCompile(sb, rvalue)
					this.compile_set(sb, lvalue, rvalue_name, null)
					sb << assignment << rvalue_name
				case Node.T_GROUPING(@val)
					private val_name = this.doCompile(sb, val)
					sb << assignment << val_name
				case Node.T_ACCESS(@base, @accessor)
					private base_name = this.doCompile(sb, base)
					sb << assignment << base_name << '->get_property("' << accessor << '", context, ehi);\n'
				# Constants
				case Node.T_NULL
					sb << assignment << "Null::make()"
				case Node.T_THIS
					sb << assignment << "context.object"
				case Node.T_SCOPE
					sb << assignment << "context.scope"
				# Flow control
				case Node.T_RET(@val)
					private ret_name = this.doCompile(sb, val)
					sb << assignment << ret_name << ";\n"
					sb << "return " << ret_name
				case Node.T_WHILE(@condition, @body)
					sb << assignment << "Null::make();\n"
					sb << "while(true) {\n"
					private cond_name = this.doCompile(sb, condition)
					sb << "if(!ehi->toBool(" << cond_name << ", context)->get<Bool>()) {\nbreak;\n}\n"
					private body_name = this.doCompile(sb, body)
					sb << assignment << body_name << ";\n"
					sb << "}"
				case Node.T_FOR(@iteree, @body)
					private iteree_name = this.doCompile(sb, iteree)
					private iterator_name = this.get_var_name "for_iterator"
					sb << "ehval_p " << iterator_name << " = ehi->call_method(" << iteree_name
					sb << ', "getIterator", nullptr, context);\n'
					sb << "while(ehi->call_method_typed<Bool>(" << iterator_name << ', "hasNext", nullptr, context)->get<Bool>()) {\n'
					sb << "ehi->call_method(" << iterator_name << ', "next", nullptr, context);\n'
					this.doCompile(sb, body)
					sb << "}\n"
					sb << assignment << iteree_name
				case Node.T_FOR_IN(@inner_var_name, @iteree, @body)
					private iteree_name = this.doCompile(sb, iteree)
					private iterator_name = this.get_var_name "for_iterator"
					sb << "ehval_p " << iterator_name << " = ehi->call_method(" << iteree_name
					sb << ', "getIterator", nullptr, context);\n'
					sb << "while(ehi->call_method_typed<Bool>(" << iterator_name << ', "hasNext", nullptr, context)->get<Bool>()) {\n'
					# name will not clash, because there won't be another one in this scope
					sb << "ehval_p next = ehi->call_method(" << iterator_name << ', "next", nullptr, context);\n'
					this.compile_set(sb, inner_var_name, "next", Attributes.make_private())
					this.doCompile(sb, body)
					sb << "}\n"
					sb << assignment << iteree_name
				case Node.T_IF(@condition, @if_block, Node.T_LIST(@elsif_blocks))
					this.compile_elsifs(sb, var_name, condition, if_block, elsif_blocks, null)
				case Node.T_IF_ELSE(@condition, @if_block, Node.T_LIST(@elsif_blocks), @else_block)
					this.compile_elsifs(sb, var_name, condition, if_block, elsif_blocks, else_block)
				# Boolean operators
				case Node.T_AND(@left, @right)
					private left_name = this.doCompile(sb, left)
					sb << assignment << "Bool::make(false);\n"
					sb << "if(eh_compiled::boolify(" << left_name << ", context, ehi)) {\n"
					private right_name = this.doCompile(sb, right)
					sb << var_name << " = ehi->toBool(" << right_name << ", context);\n"
					sb << "}\n"
				case Node.T_OR(@left, @right)
					private left_name = this.doCompile(sb, left)
					sb << assignment << "Bool::make(true);\n"
					sb << "if(eh_compiled::boolify(" << left_name << ", context, ehi)) {\n"
					private right_name = this.doCompile(sb, right)
					sb << var_name << " = ehi->toBool(" << right_name << ", context);\n"
					sb << "}\n"
				case Node.T_XOR(@left, @right)
					private left_name = this.doCompile(sb, left)
					sb << "bool " << left_name << "_bool = eh_compiled::boolify(" << left_name << ", context, ehi)) {\n"
					private right_name = this.doCompile(sb, right)
					sb << "bool " << right_name << "_bool = eh_compiled::boolify(" << right_name << ", context, ehi)) {\n"
					sb << assignment << "Bool::make(" << left_name << "_bool != " << right_name << "_bool);\n"
				# Literals
				case Node.T_FUNC(@args, @code)
					private func_name = this.compile_function(args, code)
					sb << assignment << "eh_compiled::make_closure(" << func_name << ", context, ehi)"
				case Node.T_RANGE(@left, @right)
					private left_name = this.doCompile(sb, left)
					private right_name = this.doCompile(sb, right)
					sb << assignment << "eh_compiled::make_range(" << left_name << ", " << right_name << ", ehi)"
				case Node.T_HASH_LITERAL(Node.T_LIST(@hash))
					sb << assignment << "Hash::make(ehi->get_parent());\n"
					private hash_name = this.get_var_name "hash"
					sb << "Hash::ehhash_t *" << hash_name << " = " << var_name << "->get<Hash>();\n"
					for member in hash
						match member
							case Node.T_ARRAY_MEMBER(@key, @value)
								private value_name = this.doCompile(sb, value)
								sb << hash_name << '->set("' << key << '", ' << value_name << ");\n"
						end
					end
				case Node.T_LIST(@items) # Tuple
					private member_names = []
					private size = items.length()
					for i in size
						member_names->i = this.doCompile(sb, items->i)
					end
					sb << assignment << "Tuple::create({"
					for i, name in member_names
						sb << name
						if i < size - 1
							sb << ", "
						end
					end
					sb << "}, ehi->get_parent())"
				case Node.T_CLASS(@body)
					this.compile_class(sb, var_name, "(anonymous class)", body)
				case Node.T_NAMED_CLASS(@name, @body)
					this.compile_class(sb, var_name, name, body)
					sb << 'context.scope->set_member("' << name << '", ehmember_p(attributes_t(), '
					sb << var_name << "), context, ehi)"
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

	private compile_class = func: sb, var_name, class_name, body
		private body_name = this.get_var_name "class"
		private inner_builder = StringBuilder.new()
		this.functions = inner_builder::this.functions

		inner_builder << "void " << body_name << "(const ehcontext_t &context, EHI *ehi) {\n"
		inner_builder << "ehval_p ret;\n" # ignored
		this.doCompile(inner_builder, body)
		inner_builder << "}\n"

		sb << "ehval_p " << var_name << ' = eh_compiled::make_class("' << class_name << '", ' << body_name << ", context, ehi);\n"
	end

	private compile_function = func: args, code
		private func_name = this.get_var_name "function"

		private sb = StringBuilder.new()
		sb << "ehval_p " << func_name << "(ehval_p obj, ehval_p args, EHI *ehi, const ehcontext_t &context) {\n"
		sb << "ehval_p ret;\n"
		this.compile_set(sb, args, "args", Attributes.make_private())

		this.doCompile(sb, code)
		sb << "return ret;\n}\n"

		# add function to list
		this.functions = sb.toString()::this.functions

		# return name of the C++ function created
		func_name
	end

	private compile_elsifs = func: sb, name, condition, if_block, elsifs, else_block
		# pint out if block
		sb << "ehval_p " << name << " = Null::make();\n"
		private condition_name = this.doCompile(sb, condition)
		sb << "if(eh_compiled::boolify(" << condition_name << ", context, ehi)) {\n"
		private if_result = this.doCompile(sb, if_block)
		sb << name << " = " << if_result << ";\n"
		# print out elsif blocks
		for elsif_block in elsifs
			match elsif_block
				case Node.T_ELSIF(@elsif_condition, @elsif_body)
					sb << "} else {\n"
					private condition_name = this.doCompile(sb, elsif_condition)
					sb << "if(eh_compiled::boolify(" << condition_name << ", context, ehi)) {\n"
					private block_result = this.doCompile(sb, elsif_body)
					sb << name << " = " << block_result << ";\n"
			end
		end
		# print out else block, if present
		if else_block != null
			sb << "} else {\n"
			private else_name = this.doCompile(sb, else_block)
			sb << name << " = " << else_name << ";\n"
		end
		# print closing braces
		for elsifs.length() + 1
			sb << "}\n"
		end
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
				private member_name = this.get_var_name "member"
				sb << "ehmember_p " << member_name << ' = context.scope->get<Object>()->get_recursive("'
				sb << var_name << '", context);\n'
				sb << "if(" << member_name << " != nullptr) {\n"
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
		case Node.T_COMMA(@left, @right)
			throw(NotImplemented.new "Need to find a way to handle tuples gracefully")
		case _
			printvar lvalue
			throw(MiscellaneousError.new "Cannot compile this lvalue")
	end

	# get a unique variable name with the given identifying part
	private get_var_name = id => "ehc_" + id + (this.counter.get_id id)
end

private co = Compiler.new(argv->1)
#private co = Compiler.new("/Users/jellezijlstra/code/EH/lib/tmp/test4.eh")
co.compile "tmp/compile_test.cpp"
