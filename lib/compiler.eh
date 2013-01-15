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

	# implement this non-recursively until the compiler can optimize tail recursion
	# elegant version: () => this.pieces.reduce("", (elt, rest => rest + elt))
	public toString = func:
		private out = ''
		for piece in this.pieces
			out = piece.toString() + out
		end
		out
	end
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
		case Node.T_LIST(_) | Node.T_END
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

	public static preprocess = code => Macro.listify(Macro.optimize(code))

	public compile = func: outputFile
		private code = preprocess(EH.parseFile(this.fileName))
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
		private var_name = this.get_var_name "var"
		private assignment = "ehval_p " + var_name + " = "
		if code.typeId() != Node.typeId()
			sb << assignment
			match code.type()
				case "String"
					sb << 'String::make("' << code.replace('\\', '\\\\').replace('"', '\\"').replace("\n", "\\n") << '")'
				case "Integer"
					sb << "Integer::make(" << code << ")"
				case "Float"
					sb << "Float::make(" << code << ")"
				case "Bool"
					sb << "Bool::make(" << code << ")"
				case "Null"
					sb << "Null::make()"
				case _
					printvar code
					throw(NotImplemented.new "Cannot compile this kind of literal")
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
				case Node.T_CLASS_MEMBER(@attributes, @lvalue)
					this.compile_set(sb, lvalue, "Null::make()", Attributes.parse attributes)
					sb << assignment << "Null::make();\n"
				# Constants
				case Node.T_NULL
					sb << assignment << "Null::make()"
				case Node.T_THIS
					sb << assignment << "context.object"
				case Node.T_SCOPE
					sb << assignment << "context.scope"
				# Control flow
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
				case Node.T_MATCH(@match_var, Node.T_LIST(@cases))
					private match_var_name = this.doCompile(sb, match_var)
					sb << assignment << "Null::make();\n"
					# for each match case, generate: ... code ... if(it matches) { ... more code ... } else {
					# after the last one, generate a throw_MiscellaneousError followed by cases.length() closing braces
					private cases_length = cases.length()
					for cse in cases
						match cse
							case Node.T_CASE(@pattern, @body)
								private match_bool = this.get_var_name "match_bool"
								sb << "bool " << match_bool << " = true;\n"
								# perform the match
								this.compile_match(sb, match_var_name, match_bool, pattern)
								# apply code
								sb << "if(" << match_bool << ") {\n"
								private body_name = this.doCompile(sb, body)
								sb << var_name << " = " << body_name << ";\n"
								sb << "} else {\n"
						end
					end
					# throw error if nothing was matched
					sb << 'throw_MiscellaneousError("No matching case in match statement", ehi);\n'
					for cases_length
						sb << "}\n"
					end
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
					sb << "{\nHash::ehhash_t *new_hash = " << var_name << "->get<Hash>();\n"
					for member in hash
						match member
							case Node.T_ARRAY_MEMBER(@key, @value)
								private value_name = this.doCompile(sb, value)
								sb << 'new_hash->set("' << key << '", ' << value_name << ");\n"
						end
					end
					sb << "}\n"
				case Node.T_ARRAY_LITERAL(Node.T_LIST(@array))
					# reverse, because the parser produces them in reverse order
					private members = array.reverse()
					sb << assignment << "Array::make(ehi->get_parent());\n"
					sb << "{\nauto new_array = " << var_name << "->get<Array>();\n"
					private index = 0
					for member in members
						match member
							case Node.T_ARRAY_MEMBER_NO_KEY(@value)
								private value_name = this.doCompile(sb, value)
								sb << "new_array->int_indices[" << index << "] = " << value_name << ";\n"
							case Node.T_ARRAY_MEMBER(@key, @value)
								private key_name = this.doCompile(sb, key)
								private value_name = this.doCompile(sb, value)
								sb << "if(" << key_name << "->is_a<Integer>()) {\n"
								sb << "new_array->int_indices[" << key_name << "->get<Integer>()] = " << value_name << ";\n"
								sb << "} else if(" << key_name << "->is_a<String>()) {\n"
								sb << "new_array->string_indices[" << key_name << "->get<String>()] = " << value_name << ";\n"
								sb << "} else {\n"
								sb << "throw_TypeError_Array_key(" << key_name << ", ehi);\n}\n"
						end
						index++
					end
					sb << "}\n"
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
				case Node.T_ENUM(@enum_name, Node.T_LIST(@members), @body)
					private body_name = this.get_var_name "enum"
					private inner_builder = StringBuilder.new()
					this.functions = inner_builder::this.functions

					inner_builder << "void " << body_name << "(const ehcontext_t &context, EHI *ehi) {\n"
					inner_builder << "ehval_p ret;\n" # ignored
					this.doCompile(inner_builder, body)
					inner_builder << "}\n"

					sb << assignment << 'Enum::make_enum_class("' << enum_name << '", context.scope, ehi->get_parent());\n{\n'
					sb << "ehobj_t *enum_obj = " << var_name << "->get<Object>();\n"
					for member in members
						sb << 'enum_obj->add_enum_member("' << member->0 << '", {'
						match member
							case Node.T_NULLARY_ENUM(@name)
								# ignore
							case Node.T_ENUM_WITH_ARGUMENTS(@name, Node.T_LIST(@args))
								private nargs = args.length()
								for i in nargs
									sb << '"' << args->i << '"'
									if i < nargs - 1
										sb << ", "
									end
								end
							case Node.T_ENUM_WITH_ARGUMENTS(@name, @arg)
								sb << '"' << arg << '"'
							case _
								printvar member
						end
						sb << '}, ehi->get_parent());\n'
					end
					# execute inner code
					sb << body_name << "(" << var_name << ", ehi);\n"
					sb << 'context.scope->set_member("' << enum_name << '", ehmember_p(attributes_t(), '
					sb << var_name << "), context, ehi);\n"
					sb << "}\n"
				case Node.T_CLASS(@body)
					this.compile_class(sb, var_name, "(anonymous class)", body)
				case Node.T_NAMED_CLASS(@name, @body)
					this.compile_class(sb, var_name, name, body)
					sb << 'context.scope->set_member("' << name << '", ehmember_p(attributes_t(), '
					sb << var_name << "), context, ehi)"
				case _
					printvar code
					throw(NotImplemented.new("Cannot compile this expression"))
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
				case _
					printvar elsif_block
					throw()
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
		case Node.T_LIST(@vars)
			for i in vars.length()
				# create scope
				sb << "{\n"
				sb << "ehval_p rvalue = ehi->call_method(" << name << ', "operator->", Integer::make('
				sb << i << "), context);\n"
				this.compile_set(sb, vars->i, "rvalue", attributes)
				sb << "}\n"
			end
		case _
			printvar lvalue
			throw(NotImplemented.new "Cannot compile this lvalue")
	end

	private compile_match = sb, match_var_name, match_bool, pattern => match pattern
		case Node.T_ANYTHING
			# do nothing, it's always true
		case Node.T_GROUPING(@inner)
			this.compile_match(sb, match_var_name, match_bool, inner)
		case Node.T_MATCH_SET(@name)
			sb << 'context.scope->set_member("' << name << '", ehmember_p(attributes_t::make_private(), '
			sb << match_var_name << "), context, ehi);\n"
		case Node.T_BINARY_OR(@left, @right)
			this.compile_match(sb, match_var_name, match_bool, left)
			# if that one did not succeed, try the other one
			sb << "if(!" << match_bool << ") {\n"
			sb << match_bool << " = true;\n"
			this.compile_match(sb, match_var_name, match_bool, right)
			sb << "}\n"
		case Node.T_LIST(@members)
			private members_size = members.length()
			sb << "if(!" << match_var_name << "->is_a<Tuple>()) {\n"
			sb << match_bool << " = false;\n"
			sb << "} else {\n"
			sb << "Tuple::t *tuple = " << match_var_name << "->get<Tuple>();\n"
			sb << "if(tuple->size() != " << members_size << ") {\n"
			sb << match_bool << " = false;\n"
			sb << "} else {"
			for i in members_size
				sb << "if(" << match_bool << ") {\n"
				sb << "ehval_p tuple_member = tuple->get(" << i << ");\n"
				this.compile_match(sb, "tuple_member", match_bool, members->i)
				sb << "}\n"
			end
			sb << "}\n}\n"
		case Node.T_CALL(@base, Node.T_GROUPING(@args))
			private base_name = this.doCompile(sb, base)
			# create scope
			sb << "{\n"
			sb << "if(!" << base_name << "->is_a<Enum_Instance>()) {\n"
			sb << 'throw_TypeError("match case is not an Enum.Member", ' << base_name << ", ehi);\n}\n"
			sb << "const auto em = " << base_name << "->get<Enum_Instance>();\n"
			sb << "if(em->members != nullptr) {\n"
			sb << 'throw_MiscellaneousError("Invalid argument in Enum.Member match", ehi);\n}\n'
			sb << "if(!" << match_var_name << "->is_a<Enum_Instance>()) {\n"
			sb << match_bool << " = false;\n} else {\n"
			sb << "const auto var_ei = " << match_var_name << "->get<Enum_Instance>();"
			sb << "if(var_ei->members == nullptr || em->type_compare(var_ei) != 0) {"
			sb << match_bool << " = false;\n} else {\n"
			private args_size = match args
				case Node.T_LIST(@args_list); args_list.length()
				case _; 1
			end
			sb << "if(em->nmembers != " << args_size << ") {\n"
			sb << 'throw_MiscellaneousError("Invalid argument number in Enum.Member match", ehi);\n}\n'
			match args
				case Node.T_LIST(@args_list)
					for i in args_size
						sb << "if(" << match_bool << ") {\n"
						sb << "ehval_p ei_member = var_ei->get(" << i << ");\n"
						this.compile_match(sb, "ei_member", match_bool, args_list->i)
						sb << "}\n"
					end
				case _
					sb << "ehval_p single_arg = var_ei->get(0);\n"
					this.compile_match(sb, "single_arg", match_bool, args)
			end
			sb << "}\n}\n}\n"
		case _
			private compared_name = this.doCompile(sb, pattern)
			sb << match_bool << " = ehi->call_method_typed<Bool>(" << compared_name << ', "operator==", '
			sb << match_var_name << ", context)->get<Bool>();\n"
	end

	# get a unique variable name with the given identifying part
	private get_var_name = id => "ehc_" + id + (this.counter.get_id id)
end

private co = Compiler.new(argv->1)
#private co = Compiler.new("/Users/jellezijlstra/code/EH/lib/tmp/test5.eh")
co.compile "tmp/compile_test.cpp"
