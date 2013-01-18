# Useful functions for EH macros

Node.Context.toString = () => "(context: " + this.getObject() + ", " + this.getScope() + ")"

# Additional Node variants emitted by the optimizer and understood by the compiler
enum ExtendedNode
	T_MIXED_TUPLE_LIST(members)
end

Node.isNode = (func:
	private node_t = Node.typeId()
	private extended_t = ExtendedNode.typeId()
	func: val
		private type = val.typeId()
		type == node_t || type == extended_t
	end
end)()

class Macro
	public decorate = func: f, (code, context)
		private processedCode = f code
		#echo processedCode
		processedCode.execute context
	end

	private privateAttrs = Node.T_ATTRIBUTE(Attribute.privateAttribute, Node.T_END)

	public privatize = code => match code
		case Node.T_ASSIGN(@lvalue, @rvalue)
			Node.T_ASSIGN(Node.T_CLASS_MEMBER(privateAttrs, lvalue), rvalue)
	end

	public identity = code => code

	private optimize_lvalue = code => match code
		case Node.T_ARROW(@base, @accessor)
			Node.T_ARROW(optimize(base), optimize(accessor))
		case Node.T_ACCESS(@base, @prop)
			Node.T_ACCESS(optimize(base), prop)
		case Node.T_VARIABLE(_) | Node.T_ANYTHING | Node.T_NULL
			code
		case Node.T_COMMA(@left, @right)
			Node.T_COMMA(optimize_lvalue left, optimize_lvalue right)
		case Node.T_CLASS_MEMBER(@attributes, @code)
			Node.T_CLASS_MEMBER(attributes, optimize_lvalue(code))
		case Node.T_GROUPING(Node.T_COMMA(@left, @right))
			Node.T_GROUPING(Node.T_COMMA(optimize_lvalue left, optimize_lvalue right))
		case Node.T_GROUPING(@internal)
			optimize_lvalue(internal)
		case _
			throw(MiscellaneousError.new("Invalid lvalue: " + code))
	end

	private optimize_match_cases = code => match code
		case Node.T_END
			Node.T_END
		case Node.T_COMMA(Node.T_CASE(@pattern, @body), @rest)
			Node.T_COMMA(Node.T_CASE(optimize_match_pattern pattern, optimize body), optimize_match_cases rest)
		case _
			printvar code
			throw()
	end

	private optimize_match_pattern = code => match code
		case Node.T_GROUPING(Node.T_COMMA(@left, @right))
			Node.T_GROUPING(Node.T_COMMA(optimize_match_pattern left, optimize_match_pattern right))
		case Node.T_GROUPING(@inner)
			optimize_match_pattern inner
		case Node.T_MATCH_SET(_)
			code
		case Node.T_CALL(@base, Node.T_GROUPING(@args))
			Node.T_CALL(optimize base, Node.T_GROUPING(optimize_match_pattern args))
		case Node.T_CALL(_, _)
			throw(MiscellaneousError.new("Invalid match pattern: " + code.decompile()))
		case Node.T_COMMA(_, _) | Node.T_BINARY_OR(_, _) | Node.T_ANYTHING
			code.map optimize_match_pattern
		case _
			optimize code
	end

	public optimize = code => if code.typeId() == Node.typeId()
		match code
			case Node.T_LITERAL(@val); val
			case Node.T_NULL; null
			case Node.T_ASSIGN(@lvalue, @rvalue)
				Node.T_ASSIGN(optimize_lvalue(lvalue), optimize(rvalue))
			case Node.T_FUNC(@args, @code)
				Node.T_FUNC(optimize_lvalue(args), optimize(code))
			case Node.T_GROUPING(Node.T_COMMA(@left, @right))
				Node.T_GROUPING(Node.T_COMMA(optimize left, optimize right))
			case Node.T_GROUPING(@internal)
				optimize internal
			case Node.T_CALL(Node.T_ACCESS(@base, @accessor), @argument)
				Node.T_CALL_METHOD(optimize base, accessor, optimize argument)
			case Node.T_EQ(@l, @r)
				Node.T_CALL_METHOD(optimize l, "operator==", optimize r)
			case Node.T_ARROW(@l, @r)
				Node.T_CALL_METHOD(optimize l, "operator->", optimize r)
			case Node.T_NE(@l, @r)
				Node.T_CALL_METHOD(optimize l, "operator!=", optimize r)
			case Node.T_GREATER(@l, @r)
				Node.T_CALL_METHOD(optimize l, "operator>", optimize r)
			case Node.T_GE(@l, @r)
				Node.T_CALL_METHOD(optimize l, "operator>=", optimize r)
			case Node.T_LESSER(@l, @r)
				Node.T_CALL_METHOD(optimize l, "operator<", optimize r)
			case Node.T_LE(@l, @r)
				Node.T_CALL_METHOD(optimize l, "operator<=", optimize r)
			case Node.T_COMPARE(@l, @r)
				Node.T_CALL_METHOD(optimize l, "operator<=>", optimize r)
			case Node.T_ADD(@l, @r)
				Node.T_CALL_METHOD(optimize l, "operator+", optimize r)
			case Node.T_SUBTRACT(@l, @r)
				Node.T_CALL_METHOD(optimize l, "operator-", optimize r)
			case Node.T_MULTIPLY(@l, @r)
				Node.T_CALL_METHOD(optimize l, "operator*", optimize r)
			case Node.T_DIVIDE(@l, @r)
				Node.T_CALL_METHOD(optimize l, "operator/", optimize r)
			case Node.T_MODULO(@l, @r)
				Node.T_CALL_METHOD(optimize l, "operator%", optimize r)
			case Node.T_BINARY_AND(@l, @r)
				Node.T_CALL_METHOD(optimize l, "operator&", optimize r)
			case Node.T_BINARY_XOR(@l, @r)
				Node.T_CALL_METHOD(optimize l, "operator^", optimize r)
			case Node.T_BINARY_OR(@l, @r)
				Node.T_CALL_METHOD(optimize l, "operator|", optimize r)
			case Node.T_LEFTSHIFT(@l, @r)
				Node.T_CALL_METHOD(optimize l, "operator<<", optimize r)
			case Node.T_RIGHTSHIFT(@l, @r)
				Node.T_CALL_METHOD(optimize l, "operator>>", optimize r)
			case Node.T_CUSTOMOP(@l, @op, @r)
				Node.T_CALL_METHOD(optimize l, "operator" + op, optimize r)
			case Node.T_BINARY_COMPLEMENT(@arg)
				Node.T_CALL_METHOD(optimize arg, "operator~", null)
			case Node.T_NOT(@arg)
				Node.T_CALL_METHOD(optimize arg, "operator!", null)
			case Node.T_MATCH(@match_var, @cases)
				Node.T_MATCH(optimize match_var, optimize_match_cases cases)
			case (Node.T_ANYTHING | Node.T_MATCH_SET(_))
				throw(MiscellaneousError.new("Cannot use T_ANYTHING or T_MATCH_SET outside of match expression: " + code))
			case Node.T_SEPARATOR((), @rhs)
				optimize rhs
			case Node.T_SEPARATOR(@lhs, ())
				optimize lhs
			case _
				code.map optimize
		end
	else
		code
	end

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
		public length = () => ListifyIterator.new(this.l).iterableLength()
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

	public replace_include = code => expression_map((code => match code
		case Node.T_CALL(Node.T_VARIABLE("include"), Node.T_LITERAL(@file))
			if file.isA String
				# TODO: make the context update correctly
				replace_include(EH.parseFile file)
			else
				Node.T_CALL("include", replace_include file)
			end
		case null
			null
		case _
			code.map replace_include
	end), code)
end
