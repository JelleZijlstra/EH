
include 'library.eh'

# Additional Node variants emitted by the optimizer and understood by the compiler
enum ExtendedNode
	T_MIXED_TUPLE_LIST(members)
end

static Node.isNode = (func:
	private node_t = Node.typeId()
	private extended_t = ExtendedNode.typeId()
	func: val
		private type = val.typeId()
		type == node_t || type == extended_t
	end
end)()

class Preprocessor
	public static preprocess = code, filename => listify(Macro.optimize(replace_include(code, File.fullPath filename)))

	class ListifyIterator
		private l

		public initialize = l => (this.l = l)

		public hasNext = _ => this.l != Node.T_END

		private static map_if_node = node => if EH.equalType(node, Node)
			node.map listify
		else
			node
		end

		public next = () => match this.l
			case Node.T_COMMA(@left, @right)
				this.l = right
				map_if_node left
			case Node.T_MIXED_TUPLE(@left, @right)
				this.l = right
				map_if_node left
			case Node.T_END
				throw(EmptyIterator.new())
			case @other
				this.l = Node.T_END
				map_if_node other
		end

		# trickery to get .length() to work
		this.inherit Iterable
		public getIterator = () => this

		# calculate length
		private list_length = l => match l
			case Node.T_COMMA(_, @right) | Node.T_MIXED_TUPLE(_, @right)
				1 + list_length right
			case Node.T_END
				0
			case _
				1
		end

		public length = () => list_length(this.l)
	end

	public listify = code => if code.typeId() == Node.typeId()
		match code
			case Node.T_COMMA(_, _)
				Node.T_LIST(Tuple.initialize(ListifyIterator.new code))
			case Node.T_MIXED_TUPLE(_, _)
				ExtendedNode.T_MIXED_TUPLE_LIST(Tuple.initialize(ListifyIterator.new code))
			case Node.T_ENUM(@name, Node.T_ENUM_WITH_ARGUMENTS(_, _), @enum_code)
				Node.T_ENUM(name, Node.T_LIST(Tuple.initialize((code->1)::Nil)), enum_code)
			case Node.T_ENUM(@name, Node.T_NULLARY_ENUM(_), @enum_code)
				Node.T_ENUM(name, Node.T_LIST(Tuple.initialize((code->1)::Nil)), enum_code)
			case Node.T_END
				Node.T_LIST(Tuple.initialize(Nil))
			case _
				code.map listify
		end
	else
		code
	end

	private expression_map_pattern = f, code => match code
		case Node.T_GROUPING(@inner)
			Node.T_GROUPING(expression_map_pattern(f, inner))
		case Node.T_MATCH_SET(_) | Node.T_ANYTHING
			code
		case Node.T_CALL(@base, Node.T_GROUPING(@args))
			Node.T_CALL(f base, Node.T_GROUPING(expression_map_pattern(f, args)))
		case Node.T_COMMA(@left, @right)
			Node.T_COMMA(expression_map_pattern(f, left), expression_map_pattern(f, right))
		case Node.T_BINARY_OR(@left, @right)
			Node.T_BINARY_OR(expression_map_pattern(f, left), expression_map_pattern(f, right))
		case _
			f code
	end

	private expression_map_match = f, code => match code
		case Node.T_END
			Node.T_END
		case Node.T_COMMA(Node.T_CASE(@pattern, @body), @rest)
			Node.T_COMMA(Node.T_CASE(expression_map_pattern(f, pattern), f body), expression_map_match(f, rest))
	end

	private expression_map_lvalue = f, code => match code
		case Node.T_ARROW(@base, @accessor)
			Node.T_ARROW(f base, f accessor)
		case Node.T_ACCESS(@base, @property)
			Node.T_ACCESS(f base, property)
		case Node.T_COMMA(@left, @right)
			Node.T_COMMA(expression_map_lvalue(f, left), expression_map_lvalue(f, right))
		case Node.T_CLASS_MEMBER(@attributes, @lvalue)
			Node.T_CLASS_MEMBER(attributes, expression_map_lvalue(f, lvalue))
		case Node.T_GROUPING(@internal)
			Node.T_GROUPING(expression_map_lvalue(f, internal))
		case _
			code
	end

	# calls f on all pieces of code in code that are actual executable expressions (not part of lvalues or match statements)
	public expression_map = f, code => if EH.equalType(code, Node)
		match code
			case Node.T_ASSIGN(@lvalue, @rvalue)
				Node.T_ASSIGN(expression_map_lvalue(f, lvalue), f rvalue)
			case Node.T_FUNC(@lvalue, @rvalue)
				Node.T_FUNC(expression_map_lvalue(f, lvalue), f rvalue)
			case Node.T_MATCH(@match_var, @patterns)
				Node.T_MATCH(f match_var, expression_map_match(f, patterns))
			case _
				f code
		end
	else
		code
	end

	public replace_include = code, path => expression_map((code => match code
		case Node.T_CALL(Node.T_VARIABLE("include"), Node.T_LITERAL(@file))
			if file.isA String
				# TODO: make the context update correctly
				private real_name = path + '/' + file
				replace_include(EH.parseFile real_name, File.fullPath real_name)
			else
				Node.T_CALL("include", replace_include(file, path))
			end
		case null
			null
		case _
			code.map(c => replace_include(c, path))
	end), code)
end
