/*
 * ByteArray
 */
#include "ByteArray.hpp"

#include "ArgumentError.hpp"

EH_INITIALIZER(ByteArray) {
    REGISTER_CONSTRUCTOR(ByteArray);
    REGISTER_METHOD(ByteArray, length);
    REGISTER_METHOD(ByteArray, resize);
    REGISTER_METHOD_RENAME(ByteArray, operator_arrow, "operator->");
    REGISTER_METHOD_RENAME(ByteArray, operator_arrow_equals, "operator->=");
}

EH_METHOD(ByteArray, operator_colon) {
    ASSERT_TYPE(args, Integer, "ByteArray()");
    int size = args->get<Integer>();
    if(size < 0) {
        throw_ArgumentError("size must be non-negative", "ByteArray()", args, ehi);
    }
    return ByteArray::make(size);
}

EH_METHOD(ByteArray, length) {
    ASSERT_OBJ_TYPE(ByteArray, "ByteArray.length");
    return Integer::make(obj->get<ByteArray>()->size);
}

EH_METHOD(ByteArray, resize) {
    ASSERT_OBJ_TYPE(ByteArray, "ByteArray.resize");
    ASSERT_TYPE(args, Integer, "ByteArray.resize");
    int new_size = args->get<Integer>();
    if(new_size < 0) {
        throw_ArgumentError("new size must be non-negative", "ByteArray.resize", args, ehi);
    }
    obj->get<ByteArray>()->resize(new_size);
    return nullptr;
}

EH_METHOD(ByteArray, operator_arrow) {
    ASSERT_OBJ_TYPE(ByteArray, "ByteArray.operator->");
    ASSERT_TYPE(args, Integer, "ByteArray.operator->");
    int index = args->get<Integer>();
    auto ba = obj->get<ByteArray>();
    if(!ba->has(index)) {
        throw_ArgumentError_out_of_range("ByteArray.operator->", args, ehi);
    }
    return Integer::make(ba->content[index]);
}

EH_METHOD(ByteArray, operator_arrow_equals) {
    ASSERT_NARGS_AND_TYPE(2, ByteArray, "ByteArray.operator->=");
    ehval_p index_v = args->get<Tuple>()->get(0);
    ASSERT_TYPE(index_v, Integer, "ByteArray.operator->=");
    int index = index_v->get<Integer>();
    auto ba = obj->get<ByteArray>();
    if(!ba->has(index)) {
        throw_ArgumentError_out_of_range("ByteArray.operator->=", index_v, ehi);
    }
    ehval_p rvalue = args->get<Tuple>()->get(1);
    ASSERT_TYPE(rvalue, Integer, "ByteArray.operator->=");
    int value = rvalue->get<Integer>();
    if(value < 0 || value >= 256) {
        throw_ArgumentError_out_of_range("ByteArray.operator->=", rvalue, ehi);
    }
    ba->content[index] = value;
    return rvalue;
}
