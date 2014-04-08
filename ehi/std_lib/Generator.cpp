#include "EmptyIterator.hpp"
#include "Exception.hpp"
#include "Function.hpp"
#include "Generator.hpp"

ehval_p Generator::t::run(EHI *ehi) {
    ehval_p ret = eh_execute_frame(this->frame, ehi);
    if(this->frame->response != eh_frame_t::yielding_e) {
        throw_EmptyIterator(ehi);
    }
    return ret;
}

ehval_p Generator::make(ehval_p function_object, eh_frame_t *frame, EHI *ehi) {
    auto inner_gen = new Generator::t(function_object, frame);
    return ehi->get_parent()->allocate<Generator>(inner_gen);
}

EH_METHOD(Generator, getIterator) {
    ASSERT_OBJ_TYPE(Generator, "Generator.getIterator");
    return obj;
}
EH_METHOD(Generator, next) {
    ASSERT_NULL_AND_TYPE(Generator, "Generator.next");
    return ehlm_Generator_send(obj, nullptr, ehi);
}
EH_METHOD(Generator, send) {
    ASSERT_OBJ_TYPE(Generator, "Generator.send");
    auto gen = obj->get<Generator>();
    gen->frame->response = eh_frame_t::value_e;
    gen->frame->registers[0].set_pointer(args);
    return gen->run(ehi);
}
EH_METHOD(Generator, throw) {
    ASSERT_OBJ_TYPE(Generator, "Generator.throw");
    auto gen = obj->get<Generator>();
    gen->frame->response = eh_frame_t::exception_e;
    gen->frame->registers[0].set_pointer(args);
    return gen->run(ehi);
}
EH_METHOD(Generator, close) {
    ASSERT_NULL_AND_TYPE(Generator, "Generator.close");
    auto gen = obj->get<Generator>();
    gen->frame->response = eh_frame_t::closing_e;
    gen->run(ehi);
    return nullptr;
}
EH_INITIALIZER(Generator) {
    REGISTER_METHOD(Generator, getIterator);
    REGISTER_METHOD(Generator, next);
    REGISTER_METHOD(Generator, send);
    REGISTER_METHOD(Generator, throw);
    REGISTER_METHOD(Generator, close);
}

void throw_GeneratorExit(EHI *ehi) {
    throw_error("GeneratorExit", nullptr, ehi);
}

EH_INITIALIZER(GeneratorExit) {
    REGISTER_METHOD(GeneratorExit, initialize);
    INHERIT_PURE_CLASS(Exception);
}

/*
 * @description Initializer. Takes no arguments.
 * @argument None
 * @returns N/A
 */
EH_METHOD(GeneratorExit, initialize) {
    obj->set_property("message", String::make(strdup("Generator closing")), ehi->global(), ehi);
    return nullptr;
}
