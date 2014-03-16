#include "EmptyIterator.hpp"
#include "Exception.hpp"
#include "Function.hpp"
#include "Generator.hpp"

ehval_p send_and_handle(EHI *ehi, generator_info::message_enum type, ehval_p message, bool is_master) {
    generator_info &gi = ehi->generator_info;
    gi.send(type, message);
    gi.wait(is_master);
    switch(gi.current_message_type) {
        case generator_info::value_e:
            return gi.current_message;
        case generator_info::exception_e:
            throw eh_exception(gi.current_message);
        case generator_info::exit_e:
            if(is_master) {
                throw_EmptyIterator(ehi);
            } else {
                throw_GeneratorExit(ehi);
            }
    }
}
void send_exception_and_exit(generator_info &gi, ehval_p exception) {
    // first send the exception, then close
    gi.send(generator_info::exception_e, exception);
    gi.wait(false);
    gi.send(generator_info::exit_e, nullptr);
}

ehval_p Generator::t::yield(ehval_p value) {
    return send_and_handle(ehi, generator_info::value_e, value, false);
}
Generator::t::~t() {
    ehi->generator_info.send(generator_info::exit_e, nullptr);
    thread.join();
    delete ehi;
}

ehval_p Generator::make(ehval_p base_object, ehval_p function_object, ehval_p args, ehval_p scope, EHI *ehi) {
    EHI *new_ehi = new EHI(end_is_end_e, ehi->get_parent(), scope, ehi->get_working_dir(), ehi->get_context_name());
    ehval_p gen;

    auto run_thread = [base_object, function_object, args, scope, new_ehi, &gen]() {
        printf("X\n");
        new_ehi->generator_info.wait(false);
        printf("Y%d\n", new_ehi->generator_info.current_message_type);
        switch(new_ehi->generator_info.current_message_type) {
            case generator_info::value_e: break; // ignored
            case generator_info::exit_e: return; // we're done
            case generator_info::exception_e:
                send_exception_and_exit(new_ehi->generator_info, new_ehi->generator_info.current_message);
                return;

        }
        // first sent message is ignored. TODO: at least rethrow exceptions and kill the generator if requested.
        ehstack_entry_t stk(function_object->get_full_name(), gen, new_ehi->get_stack());
        ehcontext_t context{base_object, scope};

        // set arguments
        attributes_t attributes(private_e, nonstatic_e, nonconst_e);
        Function::t *f = function_object->get<Function>();
        try {
            new_ehi->set(f->args, args, &attributes, context);

            // execute the function
            new_ehi->eh_execute(f->code, context);
            // we're done
            new_ehi->generator_info.send(generator_info::exit_e, nullptr);
        } catch(eh_exception &e) {
            ehval_p content = e.content;
            const unsigned int type_id = content->get_type_id(new_ehi->get_parent());
            if(type_id == new_ehi->get_parent()->type_ids.GeneratorExit) {
                // swallow it, and send a close in case the caller is still interested
                new_ehi->generator_info.send(generator_info::exit_e, nullptr);
            } else {
                send_exception_and_exit(new_ehi->generator_info, content);
            }
        }
    };

    Generator::type inner_gen = new Generator::t(scope, new_ehi, run_thread);
    gen = ehi->get_parent()->allocate<Generator>(inner_gen);
    return gen;
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
    EHI *inner_ehi = obj->get<Generator>()->ehi;
    return send_and_handle(inner_ehi, generator_info::value_e, args, true);
}
EH_METHOD(Generator, throw) {
    ASSERT_OBJ_TYPE(Generator, "Generator.throw");
    EHI *inner_ehi = obj->get<Generator>()->ehi;
    return send_and_handle(inner_ehi, generator_info::exception_e, args, true);
}
EH_METHOD(Generator, close) {
    ASSERT_NULL_AND_TYPE(Generator, "Generator.close");
    EHI *inner_ehi = obj->get<Generator>()->ehi;
    inner_ehi->generator_info.send(generator_info::exit_e, nullptr);
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
