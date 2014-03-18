#include "eh.hpp"
#include "eh_compiled.hpp"
#include "eh_libclasses.hpp"
#include "eh_libcmds.hpp"
#include "eh_vm.hpp"

#include "std_lib/Array.hpp"
#include "std_lib/Attribute.hpp"
#include "std_lib/Binding.hpp"
#include "std_lib/ByteArray.hpp"
#include "std_lib/ConstError.hpp"
#include "std_lib/Exception.hpp"
#include "std_lib/Enum.hpp"
#include "std_lib/Function.hpp"
#include "std_lib/Generator.hpp"
#include "std_lib/GlobalObject.hpp"
#include "std_lib/LoopError.hpp"
#include "std_lib/Map.hpp"
#include "std_lib/MiscellaneousError.hpp"
#include "std_lib/NameError.hpp"
#include "std_lib/Node.hpp"
#include "std_lib/Object.hpp"
#include "std_lib/Range.hpp"
#include "std_lib/Tuple.hpp"
#include "std_lib/VisibilityError.hpp"

// Needs to come after inclusion of Attribute
#include "eh.bison.hpp"

code_object::code_object(uint8_t *data_) : data(data_) {
    header = static_cast<code_object_header *>(static_cast<void *>(data));
}

code_object::~code_object() {
}

void code_object::validate_header(EHI *ehi) {
    if(header->magic[0] != 'E' || header->magic[1] != 'H') {
        throw_CompileError("First two bytes in code object must be 'EH'", ehi);
    }
    if(header->version[0] != CURRENT_MAJOR_VERSION || header->version[1] != CURRENT_MINOR_VERSION) {
        std::ostringstream stream;
        stream << "This code object was compiled for version " << int(header->version[0]) << "." << int(header->version[1]);
        stream << ", but the current version is " << int(CURRENT_MAJOR_VERSION) << "." << int(CURRENT_MINOR_VERSION);
        throw_CompileError(stream.str().c_str(), ehi);
    }
    if(header->size < CODE_OBJECT_HEADER_SIZE) {
        throw_CompileError("Code object is too small", ehi);
    }
    if(header->string_offset != CODE_OBJECT_HEADER_SIZE) {
        throw_CompileError("String offset must be immediately after header", ehi);
    }
    if(header->entry_point < header->string_offset) {
        throw_CompileError("Entry point is too early", ehi);
    }
}

void eh_execute_bytecode(uint8_t *data, EHI *ehi) {
    code_object co(data);
    co.validate_header(ehi);

    // create blob-global frame object
    ehval_p global_object = ehi->get_parent()->global_object;
    ehcontext_t context(global_object, global_object);
    eh_frame_t frame(eh_frame_t::module_e, &co, co.header->entry_point, context);
    eh_execute_frame(&frame, ehi);
}

template<class T>
T get_bytes(uint8_t *data, size_t offset) {
    T val;
    memcpy(&val, &data[offset], sizeof(T));
    return val;
}

static char *load_string(uint8_t *code, uint8_t *current_op) {
    uint32_t string_offset = get_bytes<uint32_t>(current_op, SIZEOF_OFFSET);
    uint32_t len = get_bytes<uint32_t>(code, string_offset);
    uint8_t *bytes = &code[string_offset + SIZEOF_OFFSET];
    char *str = new char[len + 1];
    memcpy(str, bytes, len);
    str[len] = '\0';
    return str;
}

ehval_p eh_execute_frame(eh_frame_t *frame, EHI *ehi) {
    uint8_t *const code = frame->co->data;
    ehcontext_t context = frame->context;
    register ehval_p registers[4];
    registers[0] = frame->argument;
    std::vector<ehval_p> stack;
    while(true) {
        uint8_t *current_op = &code[frame->current_offset];
        frame->current_offset += SIZEOF_OPCODE;
        switch(current_op[0]) {
            case JUMP:
                frame->current_offset = get_bytes<uint32_t>(current_op, SIZEOF_OFFSET);
                break;
            case JUMP_TRUE: {
                bool val = eh_compiled::boolify(registers[0], context, ehi);
                if(val) {
                    frame->current_offset = get_bytes<uint32_t>(current_op, SIZEOF_OFFSET);
                }
                break;
            }
            case JUMP_FALSE: {
                bool val = eh_compiled::boolify(registers[0], context, ehi);
                if(!val) {
                    frame->current_offset = get_bytes<uint32_t>(current_op, SIZEOF_OFFSET);
                }
                break;
            }
            case MOVE:
                registers[3] = registers[2];
                break;
            case LOAD: {
                // TODO fix memory leak. Does unique_ptr work with new[]?
                char *name = load_string(code, current_op);
                registers[0] = eh_compiled::get_variable(name, context, ehi);
                break;
            }
            case SET: {
                uint8_t attr = current_op[2];
                char *name = load_string(code, current_op);
                if(attr == 0) {
                    ehi->set_bare_variable(name, registers[0], context, nullptr);
                } else {
                    visibility_enum v = (attr & 4) ? private_e : public_e;
                    const_enum c = (attr & 2) ? const_e : nonconst_e;
                    attributes_t attributes(v, nonstatic_e, c);
                    ehi->set_bare_variable(name, registers[0], context, &attributes);
                }
                break;
            }
            case SET_PROPERTY:
            case SET_INSTANCE_PROPERTY:
                // TODO (fix attributes)
                break;
            case PUSH:
                stack.push_back(registers[current_op[2]]);
                break;
            case POP:
                // TODO: does vector have a special operation for this? pop_back returns void
                registers[current_op[2]] = stack.at(stack.size() - 1);
                stack.pop_back();
                break;
            case CALL: {
                auto tmp = ehi->call_function(registers[1], registers[0], context);
                registers[0] = tmp;
                break;
            }
            case LOAD_PROPERTY: {
                char *name = load_string(code, current_op);
                registers[0] = registers[1]->get_property(name, context, ehi);
                break;
            }
            case LOAD_INSTANCE_PROPERTY: {
                char *name = load_string(code, current_op);
                registers[0] = registers[1]->get_instance_member_throwing(name, context, ehi);
                break;
            }
            case CALL_METHOD: {
                char *name = load_string(code, current_op);
                registers[0] = ehi->call_method(registers[1], name, registers[0], context);
                break;
            }
            case LOAD_STRING: {
                char *value = load_string(code, current_op);
                registers[0] = String::make(strdup(value));
                break;
            }
            case LOAD_INTEGER: {
                registers[0] = Integer::make(get_bytes<uint32_t>(current_op, SIZEOF_OFFSET));
                break;
            }
            case LOAD_FLOAT: {
                registers[0] = Float::make(get_bytes<float>(current_op, SIZEOF_OFFSET));
                break;
            }
            case LOAD_TRUE:
                registers[current_op[2]] = Bool::make(true);
                break;
            case LOAD_FALSE:
                registers[current_op[2]] = Bool::make(false);
                break;
            case LOAD_NULL:
                registers[current_op[2]] = Null::make();
                break;
            case CREATE_TUPLE: {
                uint16_t size = get_bytes<uint16_t>(current_op, 2);
                registers[2] = Tuple::make(size, ehi);
                break;
            }
            case SET_TUPLE: {
                uint16_t index = get_bytes<uint16_t>(current_op, 2);
                registers[2]->get<Tuple>()->set(index, registers[0]);
                break;
            }
            case CREATE_ARRAY: {
                uint16_t size = get_bytes<uint16_t>(current_op, 2);
                registers[2] = Array::make(ehi->get_parent(), size);
                break;
            }
            case SET_ARRAY: {
                uint16_t index = get_bytes<uint16_t>(current_op, 2);
                registers[2]->get<Array>()->v[index] = registers[0];
                break;
            }
            case CREATE_MAP: {
                // the size given here is currently unused; a future optimization may be to use it to allocate
                // the underlying object in a more efficient manner.
                registers[2] = Map::make(ehi);
                break;
            }
            case SET_MAP: {
                Map::t *map = registers[2]->get<Map>();
                map->set(registers[0], registers[1]);
                break;
            }
            case CREATE_RANGE: {
                registers[2] = Range::make(registers[1], registers[0], ehi->get_parent());
                break;
            }
            case CREATE_FUNCTION: {
                uint32_t target = get_bytes<uint32_t>(current_op, SIZEOF_OFFSET);
                Function::t *f = new Function::t(Function::bytecode_e);
                f->bytecode.code_object = frame->co;
                f->bytecode.offset = target;
                f->parent = context.scope;
                registers[0] = Function::make(f, ehi->get_parent());
                break;
            }
            case LOAD_CLASS: {
                uint32_t target = get_bytes<uint32_t>(current_op, SIZEOF_OFFSET);
                EHInterpreter *parent = ehi->get_parent();
                ehclass_t *new_obj = new ehclass_t("");
                ehval_p ret = Class::make(new_obj, parent);

                // TODO: named class
                new_obj->type_id = parent->repo.register_class("", ret);
                new_obj->parent = context.scope;

                // execute the code within the class
                eh_frame_t nested_frame(eh_frame_t::class_e, frame->co, target, ret);
                eh_execute_frame(&nested_frame, ehi);
                registers[0] = ret;
                break;
            }
            case LOAD_ENUM: {
                uint32_t target = get_bytes<uint32_t>(current_op, SIZEOF_OFFSET);
                // execute the code within the enum
                eh_frame_t nested_frame(eh_frame_t::enum_e, frame->co, target, context);
                eh_execute_frame(&nested_frame, ehi);
                // NB: the code within the enum frame should leave r1 to ultimately contain the enum
                break;
            }
            case ENUM_INIT: {
                char *name = load_string(code, current_op);
                // not actually used at the moment
                // uint16_t size = get_bytes<uint16_t>(current_op, 2);
                ehval_p ret = Enum::make_enum_class(strdup(name), context.scope, ehi->get_parent());
                // NB: this introduces a small change in semantics; in the previous implementation the
                // enum class is only set as a variable after it is defined. Should not normally matter,
                // but ideally the contradiction should be resolved one way or the other.
                context.scope->set_member(strdup(name), ehmember_p(attributes_t(), ret), context, ehi);

                // overwrite the frame's context to be the enum class
                frame->context = context = ret;
                break;
            }
            case SET_ENUM_MEMBER: {
                uint8_t size = current_op[1];
                uint16_t index = get_bytes<uint16_t>(current_op, 2);
                char *name = load_string(code, current_op);
                context.object->get<Enum>()->add_enum_member(strdup(name), std::vector<std::string>(size), ehi->get_parent(), index);
                break;
            }
            case SET_ENUM_ARGUMENT: {
                uint8_t inner_index = current_op[1];
                uint16_t outer_index = get_bytes<uint16_t>(current_op, 2);
                char *name = load_string(code, current_op);
                context.object->get<Enum>()->enum_members[outer_index].members[inner_index] = name;
                break;
            }
            case ASSERT_NULL: {
                ehval_p val = registers[current_op[2]];
                if(!val.null()) {
                    throw_RuntimeError("Expected a non-null value", ehi);
                }
                break;
            }
            case RETURN:
            case HALT:
                return registers[0];
            default:
                throw_RuntimeError("Unrecognized opcode", ehi);
        }
    }
}
