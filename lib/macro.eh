# Useful functions for EH macros

Node.Context.toString = () => "(context: " + this.getObject() + ", " + this.getScope() + ")"

class Macro
	public decorate = func: f, (code, context)
		private processedCode = f code
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
		case (Node.T_VARIABLE(_) | Node.T_ANYTHING | Node.T_NULL | Node.T_COMMA(_, _))
			# ignore T_COMMA for now; not sure internal implementation is currently correct
			code
		case Node.T_CLASS_MEMBER(@attributes, @code)
			Node.T_CLASS_MEMBER(attributes, optimize_lvalue(code))
		case Node.T_GROUPING(Node.T_COMMA(_, _))
			code
		case Node.T_GROUPING(@internal)
			optimize_lvalue(internal)
		case _
			throw(MiscellaneousError.new "Invalid lvalue")
	end

	public optimize = code => if code.typeId() == Node.typeId()
		match code
			case Node.T_LITERAL(@val); val
			case Node.T_NULL; null
			case Node.T_ASSIGN(@lvalue, @rvalue)
				Node.T_ASSIGN(optimize_lvalue(lvalue), optimize(rvalue))
			case Node.T_FUNC(@args, @code)
				Node.T_FUNC(optimize_lvalue(args), optimize(code))
			case Node.T_CASE(@case_code, @code)
				Node.T_CASE(case_code, optimize(code))
			case Node.T_GROUPING(Node.T_COMMA(@left, @right))
				Node.T_GROUPING(Node.T_COMMA(optimize left, optimize right))
			case Node.T_GROUPING(@internal)
				optimize internal
			case Node.T_CALL(Node.T_ACCESS(@base, @accessor), @argument)
				Node.T_CALL_METHOD(optimize base, accessor, optimize argument)
			case Node.T_ADD(@lval, @rval)
				Node.T_CALL_METHOD(optimize lval, "operator+", optimize rval)
			case Node.T_SUBTRACT(@l, @r)
				Node.T_CALL_METHOD(optimize l, "operator-", optimize r)
			case Node.T_EQ(@l, @r)
				Node.T_CALL_METHOD(optimize l, "operator==", optimize r)
			case (Node.T_ANYTHING | Node.T_MATCH_SET(_))
				throw(MiscellaneousError.new("Cannot use T_ANYTHING or T_MATCH_SET outside of match expression"))
			case _; code.map optimize
		end
	else
		code
	end
end
