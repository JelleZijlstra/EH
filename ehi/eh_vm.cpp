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

const bool verbose = false;

static void dump(ehval_p object, EHI *ehi) {
    printvar_set s;
    object->printvar(s, 0, ehi);
}

static attributes_t parse_attributes(int attr) {
    visibility_enum v = (attr & 4) ? private_e : public_e;
    const_enum c = (attr & 2) ? const_e : nonconst_e;
    return attributes_t(v, nonstatic_e, c);
}

code_object::code_object(const uint8_t *data_) : data(data_) {
    header = static_cast<const code_object_header *>(static_cast<const void *>(data));
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

void eh_execute_bytecode(const uint8_t *data, EHI *ehi) {
    code_object co(data);
    co.validate_header(ehi);

    // create blob-global frame object
    ehval_p global_object = ehi->get_parent()->global_object;
    ehcontext_t context(global_object, global_object);
    eh_frame_t frame(eh_frame_t::module_e, &co, co.header->entry_point, context);
    eh_execute_frame(&frame, ehi);
}

template<class T>
T get_bytes(const uint8_t *const data, size_t offset) {
    T val;
    memcpy(&val, &data[offset], sizeof(T));
    return val;
}

static char *load_string(const uint8_t *const code, const uint8_t *const current_op) {
    uint32_t string_offset = get_bytes<uint32_t>(current_op, SIZEOF_OFFSET);
    uint32_t len = get_bytes<uint32_t>(code, string_offset);
    const uint8_t *const bytes = &code[string_offset + SIZEOF_OFFSET];
    char *str = new char[len + 1];
    memcpy(str, bytes, len);
    str[len] = '\0';
    return str;
}

ehval_p eh_execute_frame(eh_frame_t *frame, EHI *ehi) {
    const uint8_t *const code = frame->co->data;
    ehcontext_t context = frame->context;
    register_value *registers = frame->registers;
    std::vector<ehval_p> &stack = frame->stack;
    while(true) {
        const uint8_t *const current_op = &code[frame->current_offset];

        if(verbose) {
            printf("Executing opcode (position = %d): %d (%d, %d, %d; %d)\n", frame->current_offset, current_op[0], current_op[1], current_op[2], current_op[3], get_bytes<uint32_t>(current_op, SIZEOF_OFFSET));
        }

        frame->current_offset += SIZEOF_OPCODE;
        switch(current_op[0]) {
            case JUMP:
                frame->current_offset = get_bytes<uint32_t>(current_op, SIZEOF_OFFSET);
                break;
            case JUMP_TRUE: {
                bool val = eh_compiled::boolify(registers[0].get_pointer(), context, ehi);
                if(val) {
                    frame->current_offset = get_bytes<uint32_t>(current_op, SIZEOF_OFFSET);
                }
                break;
            }
            case JUMP_FALSE: {
                bool val = eh_compiled::boolify(registers[0].get_pointer(), context, ehi);
                if(!val) {
                    frame->current_offset = get_bytes<uint32_t>(current_op, SIZEOF_OFFSET);
                }
                break;
            }
            case JUMP_EQUAL:
                if(registers[current_op[2]].equal(registers[current_op[3]])) {
                    frame->current_offset = get_bytes<uint32_t>(current_op, SIZEOF_OFFSET);
                }
                break;
            case MOVE:
                registers[current_op[3]].move(registers[current_op[2]]);
                break;
            case LOAD: {
                // TODO fix memory leak. Does unique_ptr work with new[]?
                char *name = load_string(code, current_op);
                registers[0].set_pointer(eh_compiled::get_variable(name, context, ehi));
                break;
            }
            case SET: {
                uint8_t attr = current_op[2];
                char *name = load_string(code, current_op);
                if(attr == 0) {
                    ehi->set_bare_variable(name, registers[0].get_pointer(), context, nullptr);
                } else {
                    attributes_t attributes = parse_attributes(attr);
                    ehi->set_bare_variable(name, registers[0].get_pointer(), context, &attributes);
                }
                break;
            }
            case SET_PROPERTY: {
                uint8_t attr = current_op[2];
                char *accessor = load_string(code, current_op);
                ehval_p obj = registers[1].get_pointer();
                ehval_p rvalue = registers[0].get_pointer();
                if(attr == 0) {
                    obj->set_property(accessor, rvalue, context, ehi);
                } else {
                    ehmember_p new_member = ehmember_t::make(parse_attributes(attr), rvalue);
                    obj->set_member(accessor, new_member, context, ehi);
                }
                break;
            }
            case SET_INSTANCE_PROPERTY: {
                uint8_t attr = current_op[2];
                char *accessor = load_string(code, current_op);
                ehval_p obj = registers[1].get_pointer();
                ehval_p rvalue = registers[0].get_pointer();
                if(attr == 0) {
                    obj->set_instance_property(accessor, rvalue, context, ehi);
                } else {
                    ehmember_p new_member = ehmember_t::make(parse_attributes(attr), rvalue);
                    obj->set_instance_member(accessor, new_member, context, ehi);
                }
                break;
            }
            case PUSH:
                stack.push_back(registers[current_op[2]].get_pointer());
                break;
            case POP:
                registers[current_op[2]].set_pointer(stack.back());
                stack.pop_back();
                break;
            case CALL: {
                auto tmp = ehi->call_function(registers[1].get_pointer(), registers[0].get_pointer(), context);
                registers[0].set_pointer(tmp);
                break;
            }
            case LOAD_PROPERTY: {
                char *name = load_string(code, current_op);
                registers[0].set_pointer(registers[1].get_pointer()->get_property(name, context, ehi));
                break;
            }
            case LOAD_INSTANCE_PROPERTY: {
                char *name = load_string(code, current_op);
                registers[0].set_pointer(registers[1].get_pointer()->get_instance_member_throwing(name, context, ehi));
                break;
            }
            case CALL_METHOD: {
                char *name = load_string(code, current_op);
                registers[0].set_pointer(ehi->call_method(registers[1].get_pointer(), name, registers[0].get_pointer(), context));
                break;
            }
            case LOAD_STRING: {
                char *value = load_string(code, current_op);
                registers[0].set_pointer(String::make(strdup(value)));
                break;
            }
            case LOAD_INTEGER: {
                registers[0].set_pointer(Integer::make(get_bytes<uint32_t>(current_op, SIZEOF_OFFSET)));
                break;
            }
            case LOAD_FLOAT: {
                registers[0].set_pointer(Float::make(get_bytes<float>(current_op, SIZEOF_OFFSET)));
                break;
            }
            case LOAD_TRUE:
                registers[current_op[2]].set_pointer(Bool::make(true));
                break;
            case LOAD_FALSE:
                registers[current_op[2]].set_pointer(Bool::make(false));
                break;
            case LOAD_NULL:
                registers[current_op[2]].set_pointer(Null::make());
                break;
            case LOAD_THIS:
                registers[current_op[2]].set_pointer(context.object);
                break;
            case LOAD_SCOPE:
                registers[current_op[2]].set_pointer(context.scope);
                break;
            case CREATE_TUPLE: {
                uint16_t size = get_bytes<uint16_t>(current_op, 2);
                registers[2].set_pointer(Tuple::make(size, ehi));
                break;
            }
            case SET_TUPLE: {
                uint16_t index = get_bytes<uint16_t>(current_op, 2);
                registers[2].get_pointer()->get<Tuple>()->set(index, registers[0].get_pointer());
                break;
            }
            case GET_TUPLE: {
                uint32_t index = get_bytes<uint32_t>(current_op, SIZEOF_OFFSET);
                registers[current_op[3]].set_pointer(registers[current_op[2]].get_pointer()->get<Tuple>()->get(index));
            }
            case CREATE_ARRAY: {
                uint16_t size = get_bytes<uint16_t>(current_op, 2);
                registers[2].set_pointer(Array::make(ehi->get_parent(), size));
                break;
            }
            case SET_ARRAY: {
                uint16_t index = get_bytes<uint16_t>(current_op, 2);
                registers[2].get_pointer()->get<Array>()->v[index] = registers[0].get_pointer();
                break;
            }
            case CREATE_MAP: {
                // the size given here is currently unused; a future optimization may be to use it to allocate
                // the underlying object in a more efficient manner.
                registers[2].set_pointer(Map::make(ehi));
                break;
            }
            case SET_MAP: {
                eh_map_t *map = registers[2].get_pointer()->get<Map>();
                map->set(registers[0].get_pointer(), registers[1].get_pointer(), ehi);
                break;
            }
            case CREATE_RANGE: {
                registers[2].set_pointer(Range::make(registers[1].get_pointer(), registers[0].get_pointer(), ehi->get_parent()));
                break;
            }
            case CREATE_FUNCTION: {
                uint32_t target = get_bytes<uint32_t>(current_op, SIZEOF_OFFSET);
                Function::t *f = new Function::t(Function::bytecode_e);
                f->bytecode.co = frame->co;
                f->bytecode.offset = target;
                f->parent = context.scope;
                registers[0].set_pointer(Function::make(f, ehi->get_parent()));
                break;
            }
            case CREATE_GENERATOR: {
                uint32_t target = get_bytes<uint32_t>(current_op, SIZEOF_OFFSET);
                Function::t *f = new Function::t(Function::bytecode_e);
                f->bytecode.co = frame->co;
                f->bytecode.offset = target;
                f->parent = context.scope;
                f->is_generator = true;
                registers[0].set_pointer(Function::make(f, ehi->get_parent()));
                break;
            }
            case POST_ARGUMENTS:
                if(frame->typ == eh_frame_t::generator_e) {
                    frame->response = eh_frame_t::yielding_e;
                    return Null::make();
                } else {
                    break;
                }
            case YIELD:
                frame->response = eh_frame_t::yielding_e;
                return registers[0].get_pointer();
            case POST_YIELD: {
                switch(frame->response) {
                    case eh_frame_t::none_e:
                        assert(false); // must have a response
                    case eh_frame_t::value_e:
                        break; // we put the value in #0 but it's already there
                    case eh_frame_t::exception_e:
                        throw eh_exception(registers[0].get_pointer());
                    case eh_frame_t::closing_e:
                        throw_GeneratorExit(ehi);
                }
                break;
            }
            case LOAD_CLASS: {
                uint32_t target = get_bytes<uint32_t>(current_op, SIZEOF_OFFSET);

                // execute the code within the class
                eh_frame_t nested_frame(eh_frame_t::class_e, frame->co, target, context);
                registers[0].set_pointer(eh_execute_frame(&nested_frame, ehi));
                break;
            }
            case CLASS_INIT: {
                char *name = load_string(code, current_op);
                EHInterpreter *parent = ehi->get_parent();
                ehclass_t *new_obj = new ehclass_t(name);
                ehval_p ret = Class::make(new_obj, parent);

                new_obj->type_id = parent->repo.register_class(name, ret);
                new_obj->parent = context.scope;
                frame->context = context = ret;
                break;
            }
            case LOAD_ENUM: {
                uint32_t target = get_bytes<uint32_t>(current_op, SIZEOF_OFFSET);

                // execute the code within the enum
                eh_frame_t nested_frame(eh_frame_t::enum_e, frame->co, target, context);
                registers[0].set_pointer(eh_execute_frame(&nested_frame, ehi));
                break;
            }
            case ENUM_INIT: {
                char *name = load_string(code, current_op);
                // not actually used at the moment
                // uint16_t size = get_bytes<uint16_t>(current_op, 2);
                ehval_p ret = Enum::make_enum_class(strdup(name), context.scope, ehi->get_parent());

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
            case GET_ENUM_ARGUMENT: {
                uint32_t index = get_bytes<uint32_t>(current_op, SIZEOF_OFFSET);
                ehval_p value = registers[current_op[2]].get_pointer()->get<Enum_Instance>()->get(index);
                registers[current_op[3]].set_pointer(value);
                break;
            }
            case THROW_EXCEPTION: {
                uint16_t type_id = get_bytes<uint16_t>(current_op, 2);
                ehval_p args = registers[0].get_pointer();
                ehval_p class_object = ehi->get_parent()->repo.get_object(type_id);
                ehval_p e = ehi->call_method(class_object, "operator()", args, ehi->global());
                throw eh_exception(e);
            }
            case THROW_VARIABLE: {
                throw eh_exception(registers[current_op[2]].get_pointer());
            }
            case RETURN:
                frame->current_offset = 0;
                return registers[0].get_pointer();
            case HALT:
                // HALT returns the context object; this makes classes and enums work
                return context.object;
            case LOAD_RAW_INTEGER:
                registers[current_op[1]].set_integer(get_bytes<uint32_t>(current_op, SIZEOF_OFFSET));
                break;
            case GET_RAW_TYPE:
                registers[current_op[3]].set_integer(registers[current_op[2]].get_pointer()->get_type_id(ehi->get_parent()));
                break;
            case RAW_TUPLE_SIZE:
                registers[current_op[3]].set_integer(registers[current_op[2]].get_pointer()->get<Tuple>()->size());
                break;
            case MATCH_ENUM_INSTANCE: {
                ehval_p pattern = registers[current_op[2]].get_pointer();
                ehval_p to_match = registers[current_op[3]].get_pointer();
                uint32_t next_label = get_bytes<uint32_t>(current_op, SIZEOF_OFFSET);
                if(!pattern->is_a<Enum_Instance>()) {
                    throw_TypeError("match case is not an Enum constructor", pattern, ehi);
                }
                const auto em = pattern->get<Enum_Instance>();
                if(em->members != nullptr) {
                    throw_TypeError("match case is not an Enum constructor", pattern, ehi);
                }
                if(!to_match->is_a<Enum_Instance>()) {
                    frame->current_offset = next_label;
                    break;
                }
                const auto to_match_ei = to_match->get<Enum_Instance>();
                if(to_match_ei->members == nullptr || em->type_compare(to_match_ei) != 0 || em->member_id != to_match_ei->member_id) {
                    frame->current_offset = next_label;
                    break;
                }
                registers[3].set_integer(em->nmembers);
                break;
            }
            case BEGIN_TRY_FINALLY: {
                uint32_t start_finally = get_bytes<uint32_t>(current_op, SIZEOF_OFFSET);

                ehval_p ret;
                try {
                    ret = eh_execute_frame(frame, ehi);
                } catch(...) {
                    // execute finally block, then re-throw the exception
                    frame->current_offset = start_finally;
                    eh_execute_frame(frame, ehi);
                    throw;
                }

                bool should_return = (frame->current_offset == 0);
                frame->current_offset = start_finally;
                eh_execute_frame(frame, ehi);

                if(should_return) {
                    return ret;
                }
                break;
            }
            case BEGIN_FINALLY:
            case END_TRY_FINALLY:
            case END_TRY_BLOCK:
                return registers[0].get_pointer();
            case BEGIN_CATCH:
            case END_TRY_CATCH:
                break;
            case BEGIN_TRY_CATCH: {
                uint32_t start_catch = get_bytes<uint32_t>(current_op, SIZEOF_OFFSET);
                uint32_t end_catch = get_bytes<uint32_t>(current_op + SIZEOF_OPCODE, SIZEOF_OFFSET);
                frame->current_offset += SIZEOF_OPCODE;

                ehval_p ret;
                try {
                    ret = eh_execute_frame(frame, ehi);
                } catch(eh_exception &e) {
                    frame->current_offset = start_catch;
                    attributes_t attributes(public_e, nonstatic_e, nonconst_e);
                    registers[3].set_pointer(e.content);
                    ehi->set_bare_variable("exception", e.content, frame->context, &attributes);
                    break;
                }
                frame->current_offset = end_catch;
                break;
            }
            default:
                throw_RuntimeError("Unrecognized opcode", ehi);
        }
    }
}
