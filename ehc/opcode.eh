# Definition for the operations in EH bytecode.
# There are four registers (#0, #1, #2, #3) and a stack
# Code is organized in code objects, corresponding to .eh files. Each code
# object has a global registry of strings used in the file. Instructions
# with a string argument instead use an index into this registry.

# Code evaluation will proceed using frame object, essentially consisting of a reference
# to a code object and the location of the instruction pointer.

const CODE_OBJECT_HEADER_SIZE = 20
const SIZEOF_OFFSET = 4
const SIZEOF_OPCODE = 8

enum Opcode
    JUMP(target), # unconditionally jumps to position target in the current code object
    JUMP_TRUE(target), # jumps to target if the current value in #0 is true
    JUMP_FALSE(target), # jumps to target if the current value in #0 is false
    MOVE(register_a, register_b), # moves a value from register a to register b
    LOAD(name), # looks up the value associated with the name and puts it in #0
    SET(attributes, name), # sets name to the value in #0 with the attributes given
    PUSH(register), # pushes the value in register n to the stack
    POP(register), # pops the value in register n off the stack
    CALL, # calls the object in #1 with the arguments in #0, putting the result in #0
    RETURN, # returns from the function with the value in #0
    LOAD_PROPERTY(name), # loads the property of #1 into #0
    SET_PROPERTY(attributes, name), # sets the property of #1 to #0
    LOAD_INSTANCE_PROPERTY(name), # loads the instance property of #1 to #0
    SET_INSTANCE_PROPERTY(attributes, name), # sets the instance property of #1 to #0
    CALL_METHOD(name), # calls the method named name of #1 with the arguments in #0
    LOAD_STRING(value), # load a constant with the given value into #0
    LOAD_INTEGER(value),
    LOAD_FLOAT(value),
    LOAD_TRUE(n), # load this value into register n
    LOAD_FALSE(n),
    LOAD_NULL(n),
    CREATE_TUPLE(n), # creates an empty tuple of size n and puts it in #2
    SET_TUPLE(n), # sets value n in a tuple literal in #2 to the value in #0
    CREATE_ARRAY(n), # creates an empty array of size n and puts it in #2
    SET_ARRAY(n), # sets value n in an array literal in #2 to the value in #0
    CREATE_MAP(n), # creates an empty map of size n and puts it in #2 (implementation will likely ignore n)
    SET_MAP(n), # sets value n in a tuple literal in #2 to the key in #0 and value in #1
    CREATE_RANGE, # creates a range object out of the objects in #0 (right end) and #1 (left end) and puts it in #2
    CREATE_FUNCTION(target), # creates a function object with code starting at target and puts it in #0
    LOAD_CLASS(target), # loads the class defined starting at target into #0. This will create a special frame terminated by an END_CLASS instruction
    LOAD_ENUM(target), # loads an enum defined at target into #0
    ENUM_INIT(n, name), # sets the number of elements in the enum to n and the name to name. Enum size is limited to 65535 and each member can have up to 255 arguments.
    SET_ENUM_MEMBER(size, n, name), # sets enum member n to a member with name and size as given
    SET_ENUM_ARGUMENT(m, n, name), # sets argument m of member n to be named name
    HALT, # ends a class, enum, or module definition
    ASSERT_NULL(n), # throws an exception if the value in n is not null
    LABEL(n) # pseudo-opcode for a jump target (keep this last)
    # TODO: try/catch, special opcodes for match?
end

enum CAttributes
    Null, Set(is_private, is_const)

    public toInteger() = match this
        case Null
            0
        case Set(@is_private, @is_const)
            (is_private.toInteger() << 2) | (is_const.toInteger() << 1) | 1
    end
end

# round_up_to_multiple 15 4 = 16, 16 4 = 16
round_up_to_multiple m n = do
    whole = m / n
    mod = m % n
    if mod != 0
        whole += 1
    end
    whole * n
end

assert expr message = do
    if !expr
        throw(Exception message)
    end
end

class Label
    public initialize() = do
        private this.actual_location = null
    end

    public get_location() = this.actual_location

    public set_location location = do
        if this.actual_location != null
            throw(Exception "location already set")
        end
        this.actual_location = location
    end
end

class CodeObject
    public initialize() = do
        this.string_registry = {}
        this.next_string_index = CODE_OBJECT_HEADER_SIZE
        this.functions = []
        this.main_code = []
    end

    public register_string string = do
        if this.string_registry.has string
            this.string_registry->string
        else
            index = this.next_string_index
            len = string.length()
            # align to 4 bytes
            this.next_string_index += SIZEOF_OFFSET + round_up_to_multiple len SIZEOF_OFFSET
            this.string_registry->string = index
            index
        end
    end

    public register_function() = do
        private label = Label()
        private nco = NestedCodeObject this
        nco.append(Opcode.LABEL label)
        this.functions.append(nco.function_code)
        label, nco
    end

    public append opcode = this.main_code.append opcode

    # loads the code object into a binary string that can be passed to the bytecode interpreter
    public serialize() = do
        # first calculate the required size and the actual position of all labels created
        private code_offset = this.next_string_index
        private size = code_offset
        for fn in this.functions
            size = this.sizeof_code fn size
        end
        private entry_point = size
        size = this.sizeof_code (this.main_code) size

        # initialize the header
        private ba = ByteArray size
        echo size
        printvar ba
        echo(ba.length())
        ba->0 = 'E'.charAtPosition 0
        ba->1 = 'H'.charAtPosition 0
        ba->2 = 0
        ba->3 = 0
        ba.setInteger(4, size)
        ba.setInteger(8, CODE_OBJECT_HEADER_SIZE)
        ba.setInteger(12, code_offset)
        ba.setInteger(16, entry_point)

        # write the string registry
        for string, index in this.string_registry
            ba.setInteger(index, string.length())
            content_index = index + SIZEOF_OFFSET
            for i in string.length()
                ba->(content_index + i) = string.charAtPosition i
            end
        end

        # write the code
        private offset = code_offset
        for fn in this.functions
            printvar fn
            offset = this.write_code ba offset fn
        end
        this.write_code ba offset (this.main_code)

        return ba
    end

    private write_code ba offset code = do
        for opcode in code
            match opcode
                case Opcode.LABEL(@label)
                    assert (label.get_location() != null) "label's location was not set"
                    continue
                case _
                    ()
            end
            ba->offset = opcode.numericValue()
            match opcode
                case Opcode.JUMP(@target) | Opcode.JUMP_TRUE(@target) | Opcode.JUMP_FALSE(@target) | Opcode.CREATE_FUNCTION(@target) | Opcode.LOAD_CLASS(@target) | Opcode.LOAD_ENUM(@target)
                    ba.setInteger(offset + SIZEOF_OFFSET, target.get_location())
                case Opcode.MOVE(register_a, register_b)
                    # offset + 1 is unused
                    ba->(offset + 2) = register_a
                    ba->(offset + 3) = register_b
                case Opcode.LOAD(@name) | Opcode.LOAD_PROPERTY(@name) | Opcode.LOAD_INSTANCE_PROPERTY(@name) | Opcode.CALL_METHOD(@name) | Opcode.LOAD_STRING(@name) | Opcode.LOAD_INTEGER(@name)
                    ba.setInteger(offset + SIZEOF_OFFSET, name)
                case Opcode.SET(@attributes, @name) | Opcode.SET_PROPERTY(@attributes, @name) | Opcode.SET_INSTANCE_PROPERTY(@attributes, @name)
                    # TODO: implement an attributes class that can be represented in bytecode
                    ba->(offset + 2) = attributes.toInteger()
                    ba.setInteger(offset + SIZEOF_OFFSET, name)
                case Opcode.PUSH(@register) | Opcode.POP(@register) | Opcode.LOAD_TRUE(@register) | Opcode.LOAD_FALSE(@register) | Opcode.LOAD_NULL(@register) | Opcode.ASSERT_NULL(@register)
                    ba->(offset + 2) = register
                case Opcode.CALL | Opcode.RETURN | Opcode.CREATE_RANGE | Opcode.HALT
                    ()
                case Opcode.LOAD_FLOAT(@value)
                    # TODO once we switch to double: use a separate float registry
                    float_bytes = value.toBytes()
                    for i in SIZEOF_OFFSET
                        ba->(offset + SIZEOF_OFFSET + i) = float_bytes->i
                    end
                case Opcode.CREATE_TUPLE(@n) | Opcode.SET_TUPLE(@n) | Opcode.CREATE_ARRAY(@n) | Opcode.SET_ARRAY(@n) | Opcode.CREATE_MAP(@n) | Opcode.SET_MAP(@n)
                    ba->(offset + 2), ba->(offset + 3) = this.bytes_of_short n
                case Opcode.ENUM_INIT(@n, @name)
                    ba->(offset + 2), ba->(offset + 3) = this.bytes_of_short n
                    ba.setInteger(offset + SIZEOF_OFFSET, name)
                case Opcode.SET_ENUM_MEMBER(@m, @n, @name) | Opcode.SET_ENUM_ARGUMENT(@m, @n, @name)
                    ba->(offset + 1) = m
                    ba->(offset + 2), ba->(offset + 3) = this.bytes_of_short n
                    ba.setInteger(offset + SIZEOF_OFFSET, name)
            end
            offset += SIZEOF_OPCODE
        end
        offset
    end

    private bytes_of_short short = do
        assert (n < 65536) "Cannot create literal with 65536 or more elements"
        private last_byte = n & 127
        private first_byte = (n >> 8) & 127
        first_byte, last_byte
    end

    private sizeof_code code initial_size = do
        private size = initial_size
        for opcode in code
            match opcode
                case Opcode.LABEL(@label)
                    label.set_location size
                case _
                    size += SIZEOF_OPCODE
            end
        end
        size
    end
end

public disassemble ba = do
    private sb = String.Builder()
    private size = ba.getInteger 4
    private string_offset = ba.getInteger 8
    private code_offset = ba.getInteger 12
    private entry_point = ba.getInteger 16

    sb << "Header:\n"
    # TODO implement sprintf
    sb << "\tmagic=" << (ba->0).toChar() << (ba->1).toChar() << "\n"
    sb << "\tversion=" << ba->2 << "." << ba->3 << "\n"
    sb << "\tsize=" << size << "\n"
    sb << "\tstring_offset=" << string_offset << "\n"
    sb << "\tcode_offset=" << code_offset << "\n"
    sb << "\tentry_point=" << entry_point << "\n"
    sb << "Strings:\n"
    private offset = string_offset
    while offset < code_offset
        private string_length = ba.getInteger offset
        sb << "\t" << offset << " (" << string_length << ")"
        offset += SIZEOF_OFFSET
        for i in string_length
            sb << (ba->(offset + i)).toChar()
        end
        sb << "\n"
        offset += round_up_to_multiple string_length SIZEOF_OFFSET
    end
    sb << "Code:\n"
    while offset < size
        try
            opcode = Opcode.ofNumeric(ba->offset)
        catch if exception.isA ArgumentError
            opcode = '(unrecognized opcode: ' + String(ba->offset) + ')'
        end
        sb << "\t" << offset << " " << opcode << ": "
        sb << ba->(offset + 1) << ", " << ba->(offset + 2) << ", " << ba->(offset + 3) << "; "
        sb << ba.getInteger(offset + 4) << "\n"
        offset += SIZEOF_OPCODE
    end

    sb.toString()
end

class NestedCodeObject
    public initialize(this.co) = do
        this.function_code = []
    end

    public append opcode = this.function_code.append opcode

    public register_string string = this.co.register_string string
    public register_function() = this.co.register_function()
end

public compile code = do
    co = CodeObject()
    compile_rec code co
    co.append(Opcode.HALT)
    co.serialize()
end

private compile_rec code co = do
    if !(Node.isNode code)
        match code.type()
            case String
                string_id = co.register_string code
                co.append(Opcode.LOAD_STRING string_id)
            case Integer
                co.append(Opcode.LOAD_INTEGER code)
            case Float
                co.append(Opcode.LOAD_FLOAT code)
            case Bool
                private opcode = if code
                    Opcode.LOAD_TRUE
                else
                    Opcode.LOAD_FALSE
                end
                co.append(opcode 0)
            case Null
                co.append(Opcode.LOAD_NULL 0)
        end
        return
    end
    match code
        # Basic operations (variables, assignments, calls)
        case Node.T_CALL_METHOD(@obj, @method, @args)
            compile_rec obj co
            co.append(Opcode.PUSH 0)
            compile_rec args co
            co.append(Opcode.POP 1)
            method_id = co.register_string method
            co.append(Opcode.CALL_METHOD method_id)
        case Node.T_CALL(@function, @args)
            compile_rec function co
            co.append(Opcode.PUSH 0)
            compile_rec args co
            co.append(Opcode.POP 1)
            co.append(Opcode.CALL)
        case Node.T_VARIABLE(@name)
            name_id = co.register_string name
            co.append(Opcode.LOAD name_id)
        case Node.T_SEPARATOR(@lhs, @rhs)
            compile_rec lhs co
            compile_rec rhs co
        case Node.T_ASSIGN(@lvalue, @rvalue)
            compile_rec rvalue co
            compile_set lvalue co (CAttributes.Null)
        case Node.T_GROUPING(@val)
            compile_rec val co
        case Node.T_ACCESS(@base, @accessor)
            compile_rec base co
            co.append(Opcode.LOAD_PROPERTY(co.register_string accessor))
        case Node.T_INSTANCE_ACCESS(@base, @accessor)
            compile_rec base co
            co.append(Opcode.LOAD_INSTANCE_PROPERTY(co.register_string accessor))
        case Node.T_CLASS_MEMBER(@attributes, @lvalue)
            # TODO
            # this.compile_set(sb, lvalue, "Null::make()", Attributes.parse attributes)
            # sb << assignment << "Null::make();\n"
        # Constants
        case Node.T_NULL
            co.append(Opcode.LOAD_NULL 0)
        case Node.T_THIS
            # TODO
            # sb << assignment << "context.object"
        case Node.T_SCOPE
            # TODO
            # sb << assignment << "context.scope"
        # Control flow
        case Node.T_RET(@val)
            compile_rec val co
            co.append(Opcode.RETURN)
        case Node.T_BREAK(1)
            # TODO
            # sb << "ehval_p " << var_name << ";\nbreak"
        case Node.T_CONTINUE(1)
            # TODO
            # sb << "ehval_p " << var_name << ";\ncontinue"
        case Node.T_WHILE(@condition, @body)
            private begin_label = Label()
            private end_label = Label()
            co.append(Node.JUMP end_label)
            co.append(Node.LABEL begin_label)
            compile_rec body co
            co.append(Node.LABEL end_label)
            compile_rec condition co
            co.append(Node.JUMP_TRUE begin_label)
            # make sure while loop always returns null. Perhaps we can do without this.
            co.append(Node.LOAD_NULL 0)
        case Node.T_FOR(@iteree, @body)
            # TODO
            private iteree_name = this.doCompile(sb, iteree)
            private iterator_name = this.get_var_name "for_iterator"
            sb << "ehval_p " << iterator_name << " = ehi->call_method(" << iteree_name
            sb << ", \"getIterator\", nullptr, context);\n"
            sb << "while(ehi->call_method_typed<Bool>(" << iterator_name << ", \"hasNext\", nullptr, context)->get<Bool>()) {\n"
            sb << "ehi->call_method(" << iterator_name << ", \"next\", nullptr, context);\n"
            this.doCompile(sb, body)
            sb << "}\n"
            sb << assignment << iteree_name
        case Node.T_FOR_IN(@inner_var_name, @iteree, @body)
            # TODO
            private iteree_name = this.doCompile(sb, iteree)
            private iterator_name = this.get_var_name "for_iterator"
            sb << "ehval_p " << iterator_name << " = ehi->call_method(" << iteree_name
            sb << ", \"getIterator\", nullptr, context);\n"
            sb << "while(ehi->call_method_typed<Bool>(" << iterator_name << ", \"hasNext\", nullptr, context)->get<Bool>()) {\n"
            # name will not clash, because there won't be another one in this scope
            sb << "ehval_p next = ehi->call_method(" << iterator_name << ", \"next\", nullptr, context);\n"
            this.compile_set(sb, inner_var_name, "next", Attributes.make_private())
            this.doCompile(sb, body)
            sb << "}\n"
            sb << assignment << iteree_name
        case Node.T_IF(@condition, @if_block, Node.T_LIST(@elsif_blocks))
            this.compile_elsifs(sb, var_name, condition, if_block, elsif_blocks, null)
        case Node.T_IF_ELSE(@condition, @if_block, Node.T_LIST(@elsif_blocks), @else_block)
            this.compile_elsifs(sb, var_name, condition, if_block, elsif_blocks, else_block)
        case Node.T_GIVEN(@given_var, Node.T_LIST(@cases))
            private given_var_name = this.doCompile(sb, given_var)
            private cases_length = cases.length()
            sb << assignment << "Null::make();\n"
            for cse in cases
                match cse
                    case Node.T_DEFAULT(@body)
                        sb << "if(true) {\n"
                        private default_name = this.doCompile(sb, body)
                        sb << var_name << " = " << default_name << ";\n"
                    case Node.T_CASE(@pattern, @body)
                        private case_var_name = this.doCompile(sb, pattern)
                        # use an indicator variable that is set by the code to decide whether the case matches
                        private does_match = this.get_var_name "given_does_match"
                        sb << "bool " << does_match << " = true;\n"
                        sb << "if(" << case_var_name << "->is_a<Function>() || " << case_var_name
                        sb << "->is_a<Binding>()) {\n"
                        sb << does_match << " = eh_compiled::call_function_typed<Bool>(" << case_var_name
                        sb << ", " << given_var_name << ", context, ehi);\n"
                        sb << "} else {\n"
                        sb << does_match << " = ehi->call_method_typed<Bool>(" << given_var_name
                        sb << ', "operator==", ' << case_var_name << ", context)->get<Bool>();\n"
                        sb << "}\n"
                        sb << "if(" << does_match << ") {\n"
                        private body_name = this.doCompile(sb, body)
                        sb << var_name << " = " << body_name << ";\n"
                end
                sb << "} else {\n"
            end
            sb << "throw_MiscellaneousError(\"No matching case in given statement\", ehi);\n"
            for cases_length
                sb << "}\n"
            end
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
                    case Node.T_WHEN(@pattern, @guard, @body)
                        private match_bool = this.get_var_name "match_bool"
                        sb << "bool " << match_bool << " = true;\n"
                        # perform the match
                        this.compile_match(sb, match_var_name, match_bool, pattern)
                        sb << "if(" << match_bool << ") {\n"
                        # apply guard
                        private guard_name = this.doCompile(sb, guard)
                        sb << match_bool << " = eh_compiled::boolify(" << guard_name << ", context, ehi);\n}\n"
                        # apply code
                        sb << "if(" << match_bool << ") {\n"
                        private body_name = this.doCompile(sb, body)
                        sb << var_name << " = " << body_name << ";\n"
                        sb << "} else {\n"
                end
            end
            # throw error if nothing was matched
            sb << "throw_MiscellaneousError(\"No matching case in match statement\", ehi);\n"
            for cases_length
                sb << "}\n"
            end
        # Exceptions
        case Node.T_TRY(@try_block, Node.T_LIST(@catch_blocks))
            sb << assignment << "Null::make();\n"
            this.compile_try_catch(sb, try_block, catch_blocks, var_name)
        case Node.T_TRY_FINALLY(@try_block, Node.T_LIST(@catch_blocks), @finally_block)
            sb << assignment << "Null::make();\n"
            # wrap finally block in a function, so we can call it twice
            private finally_builder = String.Builder.new()
            private finally_name = this.get_var_name "finally_function"
            finally_builder << "void " << finally_name << "(const ehcontext_t &context, EHI *ehi) {\n"
            finally_builder << "ehval_p ret;\n"
            this.doCompile(finally_builder, finally_block)
            finally_builder << "}\n"
            this.add_function finally_builder
            sb << "try {\n"
            this.compile_try_catch(sb, try_block, catch_blocks, var_name)
            sb << "} catch(...) {\n"
            sb << finally_name << "(context, ehi);\n"
            sb << "throw;\n"
            sb << "}\n"
            sb << finally_name << "(context, ehi)"
        # Boolean operators
        case Node.T_AND(@left, @right)
            compile_rec left co
            private end_label = Label()
            co.append(Opcode.JUMP_FALSE end_label)
            compile_rec right co
            co.append(Opcode.LABEL end_label)
        case Node.T_OR(@left, @right)
            compile_rec left co
            private end_label = Label()
            co.append(Opcode.JUMP_TRUE end_label)
            compile_rec right co
            co.append(Opcode.LABEL end_label)
        case Node.T_XOR(@left, @right)
            # why is this an operator rather than a method on Object?
            private left_name = this.doCompile(sb, left)
            sb << "bool " << left_name << "_bool = eh_compiled::boolify(" << left_name << ", context, ehi)) {\n"
            private right_name = this.doCompile(sb, right)
            sb << "bool " << right_name << "_bool = eh_compiled::boolify(" << right_name << ", context, ehi)) {\n"
            sb << assignment << "Bool::make(" << left_name << "_bool != " << right_name << "_bool);\n"
        # Literals
        case Node.T_FUNC(@args, @code)
            label, nco = co.register_function()
            compile_set args nco (CAttributes.Null)
            compile_rec code nco
            nco.append(Opcode.RETURN)
            co.append(Opcode.CREATE_FUNCTION label)
        case Node.T_RANGE(@left, @right)
            compile_rec left co
            co.append(Opcode.PUSH 0)
            compile_rec right co
            co.append(Opcode.POP 1)
            co.append(Opcode.CREATE_RANGE)
            co.append(Opcode.MOVE(2, 0))
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
            private size = members.length()
            co.append(Opcode.CREATE_ARRAY size)
            for i in size
                co.append(Opcode.PUSH 2)
                compile_rec (members->i) co
                co.append(Opcode.POP 2)
                co.append(Opcode.SET_ARRAY i)
            end
            co.append(Opcode.MOVE(2, 0))
        case Node.T_LIST(@items) # Tuple
            private size = items.length()
            co.append(Opcode.CREATE_TUPLE size)
            for i in size
                co.append(Opcode.PUSH 2)
                compile_rec (items->i) co
                co.append(Opcode.POP 2)
                co.append(Opcode.SET_TUPLE i)
            end
            co.append(Opcode.MOVE(2, 0))
        case ExtendedNode.T_MIXED_TUPLE_LIST(@items)
            private member_names = []
            private size = items.countWithPredicate(elt => match elt
                case Node.T_NAMED_ARGUMENT(_, _); false
                case _; true
            end)
            private twsk_name = this.get_var_name "twsk"
            sb << "auto " << twsk_name << " = new Tuple_WithStringKeys::t(" << size << ");\n"
            sb << assignment << "Tuple_WithStringKeys::make(" << twsk_name << ", ehi->get_parent());\n"
            private i = 0
            for item in items
                match item
                    case Node.T_NAMED_ARGUMENT(@name, @code)
                        private arg_name = this.doCompile(sb, code)
                        sb << twsk_name << '->set("' << name << '", ' << arg_name << ");\n"
                    case _
                        private arg_name = this.doCompile(sb, item)
                        sb << twsk_name << "->set(" << i << ", " << arg_name << ");\n"
                        i += 1
                end
            end
        case Node.T_ENUM(@enum_name, Node.T_LIST(@members), @body)
            private body_name = this.get_var_name "enum"
            private inner_builder = String.Builder.new()
            this.add_function inner_builder

            inner_builder << "void " << body_name << "(const ehcontext_t &context, EHI *ehi) {\n"
            inner_builder << "ehval_p ret;\n" # ignored
            this.doCompile(inner_builder, body)
            inner_builder << "}\n"

            sb << assignment << 'Enum::make_enum_class("' << enum_name << "\", context.scope, ehi->get_parent());\n{\n"
            sb << "Enum::t *enum_obj = " << var_name << "->get<Enum>();\n"
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
                sb << "}, ehi->get_parent());\n"
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

# compiles a set-expression lvalue, assuming the rvalue is in #0
private compile_set code co attributes = match code
    case Node.T_NULL
        co.append(Opcode.ASSERT_NULL 0)
    case Node.T_ANYTHING
        # ignore
    case Node.T_ARROW(@base, @accessor)
        co.append(Opcode.PUSH 0)
        compile_rec base co
        co.append(Opcode.PUSH 0)
        compile_rec accessor co
        co.append(Opcode.CREATE_TUPLE)
        co.append(Opcode.SET_TUPLE 0)
        co.append(Opcode.POP 1)
        co.append(Opcode.POP 0)
        co.append(Opcode.SET_TUPLE 1)
        co.append(Opcode.MOVE(2, 0))
        co.append(Opcode.CALL_METHOD(co.register_string("operator->=")))
    case Node.T_ACCESS(@base, @accessor)
        co.append(Opcode.PUSH 0)
        compile_rec base co
        co.append(Opcode.MOVE(0, 1))
        co.append(Opcode.POP 0)
        co.append(Opcode.SET_PROPERTY(attributes, co.register_string accessor))
    case Node.T_INSTANCE_ACCESS(@base, @accessor)
        co.append(Opcode.PUSH 0)
        compile_rec base co
        co.append(Opcode.MOVE(0, 1))
        co.append(Opcode.POP 0)
        co.append(Opcode.SET_INSTANCE_PROPERTY(attributes, co.register_string accessor))
    case Node.T_VARIABLE(@name)
        co.append(Opcode.SET(attributes, co.register_string name))
    case Node.T_CLASS_MEMBER(@attributes_code, @lval)
        private inner_attributes = Attributes.parse attributes_code
        this.compile_set(sb, lval, name, inner_attributes)
    case Node.T_GROUPING(@lval)
        compile_set lval co
    case Node.T_LIST(@vars)
        for i in vars.length()
            this.compile_set_list_member(sb, vars->i, i, name, attributes)
        end
    case ExtendedNode.T_MIXED_TUPLE_LIST(@vars)
        private i = 0
        for var in vars
            match var
                case Node.T_NAMED_ARGUMENT(@var_name, @dflt)
                    private na_var_name = this.get_var_name "na_var"
                    sb << "ehval_p " << na_var_name << ";\n"
                    sb << "if(ehi->call_method_typed<Bool>(" << name << ', "has", String::make(strdup("' << var_name << "\")), context)->get<Bool>()) {\n"
                    sb << na_var_name << " = ehi->call_method(" << name << ', "operator->", String::make(strdup("' << var_name << "\")), context);\n"
                    sb << "} else {\n"
                    private dflt_name = this.doCompile(sb, dflt)
                    sb << na_var_name << " = " << dflt_name << ";\n}\n"
                    sb << 'context.scope->set_member("' << var_name << '", ehmember_p('
                    if attributes == null
                        sb << "attributes_t::make_private()"
                    else
                        sb << attributes
                    end
                    sb << ", " << na_var_name << "), context, ehi);\n"
                case _
                    this.compile_set_list_member(sb, var, i, name, attributes)
                    i++
            end
        end
    case _
        printvar lvalue
        throw(NotImplemented "Cannot compile this lvalue")
end
