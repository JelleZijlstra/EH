/*
 * A code object is a sequence of binary data. It consists of the following components:
 * - A header, as defined below.
 * - A string registry, consisting of a sequence of (size, string) pairs. For example,
 *   if the header's string_offset is 16, then load_string 16 will load a string starting
 *   at offset 20 with size given in bytes 16-19.
 * - Code for all non-global functions defined in the code object.
 * - Global code (e.g., module initialization).
 *
 * Code consists of eight-byte opcodes. The first byte gives the instruction, the others
 * the arguments (many of which may be unused).
 */

#ifndef EH_VM_H_
#define EH_VM_H_

const static int SIZEOF_OFFSET = 4;
const static int SIZEOF_OPCODE = 8;

enum eh_opcode_t {
    JUMP,
    JUMP_TRUE,
    JUMP_FALSE,
    JUMP_EQUAL,
    MOVE,
    LOAD,
    SET,
    PUSH,
    POP,
    CALL,
    RETURN,
    LOAD_PROPERTY,
    SET_PROPERTY,
    LOAD_INSTANCE_PROPERTY,
    SET_INSTANCE_PROPERTY,
    CALL_METHOD,
    LOAD_STRING,
    LOAD_INTEGER,
    LOAD_FLOAT,
    LOAD_TRUE,
    LOAD_FALSE,
    LOAD_NULL,
    LOAD_THIS,
    LOAD_SCOPE,
    CREATE_TUPLE,
    SET_TUPLE,
    GET_TUPLE,
    CREATE_ARRAY,
    SET_ARRAY,
    CREATE_MAP,
    SET_MAP,
    CREATE_RANGE,
    CREATE_FUNCTION,
    LOAD_CLASS,
    CLASS_INIT,
    LOAD_ENUM,
    ENUM_INIT,
    SET_ENUM_MEMBER,
    SET_ENUM_ARGUMENT,
    GET_ENUM_ARGUMENT,
    HALT,
    THROW_EXCEPTION,
    LOAD_RAW_INTEGER,
    GET_RAW_TYPE,
    RAW_TUPLE_SIZE,
    MATCH_ENUM_INSTANCE,
    BEGIN_TRY_FINALLY,
    BEGIN_FINALLY,
    END_TRY_FINALLY,
    BEGIN_TRY_CATCH,
    BEGIN_CATCH,
    END_TRY_CATCH,
};

struct code_object_header {
    uint8_t magic[2]; // always "EH"
    uint8_t version[2]; // for now, always 00
    uint32_t size; // total size of the code object in bytes
    uint32_t string_offset; // offset of the string registry
    uint32_t code_offset; // offset of the actual code
    uint32_t entry_point; // offset of first piece of code to be executed
    static_assert(SIZEOF_OFFSET == sizeof(entry_point), "");
};

// hardcoded for easy reference from the EH code
const static int CODE_OBJECT_HEADER_SIZE = 20;
static_assert(CODE_OBJECT_HEADER_SIZE == sizeof(code_object_header), "needs to be kept in sync");

class code_object {
public:
    const uint8_t *data;
    const code_object_header *header;

    code_object(const uint8_t *);
    ~code_object();

    void validate_header(EHI *ehi);
};

// a tagged union for register values
// integer if LSB == 1, pointer if LSB == 0
class register_value {
private:
    union {
        long integer_value;
        void *pointer_value;
    };
    static_assert(sizeof(long) == sizeof(void *), "value must be the size of a single pointer");

    // dec_rc's the pointer if there is one
    void destruct_pointer() {
        if(this->is_pointer()) {
            // rely on destructor
            ehval_p to_destruct = ehval_p(static_cast<ehval_t *>(this->pointer_value), false);
        }
    }

    // disallowed operations
    register_value(const register_value&);
    register_value operator=(const register_value&);

public:
    // integer_value(1) == 0
    register_value() : integer_value(1) {
        assert(this->is_integer());
        assert(!this->is_pointer());
    }

    ~register_value() {
        this->destruct_pointer();
    }

    bool is_pointer() {
        return ((~(this->integer_value)) & 1) == 1;
    }

    ehval_p get_pointer() {
        assert(this->is_pointer());
        return ehval_p(static_cast<ehval_t *>(this->pointer_value));
    }

    void set_pointer(ehval_p new_value) {
        this->destruct_pointer();
        // we're keeping a reference around
        new_value->inc_rc();
        this->pointer_value = static_cast<void *>(new_value.operator->());
        assert(this->is_pointer());
    }

    bool is_integer() {
        return (this->integer_value & 1) == 1;
    }

    long get_integer() {
        assert(this->is_integer());
        return this->integer_value >> 1;
    }

    void set_integer(long new_value) {
        this->destruct_pointer();
        this->integer_value = (new_value << 1) | 1;
        assert(this->is_integer());
    }

    void move(const register_value &other) {
        this->destruct_pointer();
        this->integer_value = other.integer_value;
        if(this->is_pointer()) {
            this->get_pointer()->inc_rc();
        }
    }

    bool equal(const register_value &other) {
        return this->integer_value == other.integer_value;
    }
};


class eh_frame_t {
public:
    enum type {
        class_e, enum_e, function_e, module_e
    };

    type typ;
    code_object *co;
    uint32_t current_offset;
    ehcontext_t context;
    register_value registers[4];

    eh_frame_t(type typ_, code_object *co_, uint32_t current_offset_, ehcontext_t context_, ehval_p argument_ = nullptr) : typ(typ_), co(co_), current_offset(current_offset_), context(context_), registers() {
        this->registers[0].set_pointer(argument_);
    }
};

void eh_execute_bytecode(const uint8_t *data, EHI *ehi);
ehval_p eh_execute_frame(eh_frame_t *frame, EHI *ehi);

#endif /* EH_VM_H_ */
