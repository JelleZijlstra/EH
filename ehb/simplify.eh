private optimize_lvalue code = match code
    case Node.T_ARROW(@base, @accessor)
        Node.T_ARROW(optimize(base), optimize(accessor))
    case Node.T_ACCESS(@base, @prop)
        Node.T_ACCESS(optimize(base), prop)
    case Node.T_INSTANCE_ACCESS(@base, @prop)
        Node.T_INSTANCE_ACCESS(optimize(base), prop)
    case Node.T_VARIABLE(_) | Node.T_ANYTHING | Node.T_NULL
        code
    case Node.T_COMMA(@left, @right)
        Node.T_COMMA(optimize_lvalue left, optimize_lvalue right)
    case Node.T_MIXED_TUPLE(@left, @right)
        Node.T_MIXED_TUPLE(optimize_lvalue left, optimize_lvalue right)
    case Node.T_NAMED_ARGUMENT(@name, @dflt)
        Node.T_NAMED_ARGUMENT(name, optimize dflt)
    case Node.T_CLASS_MEMBER(@attributes, @code)
        Node.T_CLASS_MEMBER(attributes, optimize_lvalue(code))
    case Node.T_GROUPING(Node.T_COMMA(@left, @right))
        Node.T_GROUPING(Node.T_COMMA(optimize_lvalue left, optimize_lvalue right))
    case Node.T_GROUPING(Node.T_MIXED_TUPLE(@left, @right))
        Node.T_GROUPING(Node.T_MIXED_TUPLE(optimize_lvalue left, optimize_lvalue right))
    case Node.T_GROUPING(@internal)
        optimize_lvalue(internal)
    case _
        throw(CompileError("Invalid lvalue: " + code))
end

private optimize_match_cases code = match code
    case Node.T_END
        Node.T_END
    case Node.T_COMMA(Node.T_CASE(@pattern, @body), @rest)
        Node.T_COMMA(Node.T_CASE(optimize_match_pattern pattern, optimize body), optimize_match_cases rest)
    case Node.T_COMMA(Node.T_WHEN(@pattern, @guard, @body), @rest)
        Node.T_COMMA(Node.T_WHEN(optimize_match_pattern pattern, optimize guard, optimize body), optimize_match_cases rest)
    case _
        throw(CompileError("Invalid match pattern: " + code))
end

private optimize_match_pattern code = match code
    case Node.T_GROUPING(Node.T_COMMA(@left, @right))
        Node.T_GROUPING(Node.T_COMMA(optimize_match_pattern left, optimize_match_pattern right))
    case Node.T_GROUPING(@inner)
        optimize_match_pattern inner
    case Node.T_MATCH_SET(_)
        code
    case Node.T_AS(@code, @name)
        Node.T_AS(optimize_match_pattern code, name)
    case Node.T_CALL(@base, Node.T_GROUPING(@args))
        Node.T_CALL(optimize base, Node.T_GROUPING(optimize_match_pattern args))
    case Node.T_CALL(_, _)
        throw(MiscellaneousError.new("Invalid match pattern: " + code.decompile()))
    case Node.T_COMMA(_, _) | Node.T_BAR(_, _) | Node.T_ANYTHING
        code.map optimize_match_pattern
    case _
        optimize code
end

public optimize code = if code.isA Node
    match code
        case Node.T_LITERAL(@val); val
        case Node.T_NULL; null
        # Special cases for function declarations
        case Node.T_ASSIGN(Node.T_CALL(@fname, @args), @body)
            optimize(Node.T_ASSIGN(fname, Node.T_FUNC(args, body)))
        case Node.T_ASSIGN(Node.T_CLASS_MEMBER(@attributes, Node.T_CALL(@fname, @args)), @body)
            optimize(Node.T_ASSIGN(Node.T_CLASS_MEMBER(attributes, fname), Node.T_FUNC(args, body)))
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
        case Node.T_MATCH(@match_var, @cases)
            Node.T_MATCH(optimize match_var, optimize_match_cases cases)
        case Node.T_BAR(@lhs, @rhs)
            Node.T_CALL_METHOD(optimize lhs, "operator|", optimize rhs)
        case Node.T_ARROW(@base, @accessor)
            Node.T_CALL_METHOD(optimize base, "operator->", optimize accessor)
        case Node.T_BINARY_COMPLEMENT(@arg)
            Node.T_CALL_METHOD(optimize arg, "operator~", ())
        case Node.T_NOT(@arg)
            Node.T_CALL_METHOD(optimize arg, "operator!", ())
        case Node.T_ANYTHING | Node.T_MATCH_SET(_) | Node.T_AS(_, _)
            throw(MiscellaneousError.new("Cannot use T_ANYTHING, T_AS, or T_MATCH_SET outside of match expression: " + code))
        case Node.T_SEPARATOR((), @rhs)
            optimize rhs
        case Node.T_SEPARATOR(@lhs, ())
            optimize lhs
        case Node.T_FOR_IN(@iter_var, @iteree, @body)
            Node.T_FOR_IN(optimize_lvalue iter_var, optimize iteree, optimize body)
        case _
            code.map optimize
    end
else
    code
end
