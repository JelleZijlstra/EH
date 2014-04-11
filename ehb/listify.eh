class ListifyIterator
    private l

    public initialize l = (this.l = l)

    public hasNext() = this.l != Node.T_END

    private static map_if_node node = if node.isA Node
        node.map listify
    else
        node
    end

    public next() = match this.l
        case Node.T_COMMA(@left, @right)
            this.l = right
            return map_if_node left
        case Node.T_MIXED_TUPLE(@left, @right)
            throw(CompileError "The compiler does not support T_MIXED_TUPLE")
        case Node.T_END
            throw(EmptyIterator())
        case @other
            this.l = Node.T_END
            return map_if_node other
    end

    public getIterator() = this

    # calculate length
    private static list_length l = match l
        case Node.T_COMMA(_, @right) | Node.T_MIXED_TUPLE(_, @right)
            1 + list_length right
        case Node.T_END
            0
        case _
            1
    end

    public length() = list_length(this.l)
end

public listify code = if code.isA Node
    match code
        case Node.T_COMMA(_, _)
            Node.T_LIST(Tuple(ListifyIterator code))
        case Node.T_MIXED_TUPLE(_, _)
            throw(CompileError "The compiler does not support T_MIXED_TUPLE")
        case Node.T_ENUM(@name, Node.T_ENUM_WITH_ARGUMENTS(_, _), @enum_code)
            Node.T_ENUM(name, Node.T_LIST(Tuple [code->1]), enum_code)
        case Node.T_ENUM(@name, Node.T_NULLARY_ENUM(_), @enum_code)
            Node.T_ENUM(name, Node.T_LIST(Tuple [code->1]), enum_code)
        case Node.T_END
            Node.T_LIST(Tuple [])
        case _
            code.map listify
    end
else
    code
end
