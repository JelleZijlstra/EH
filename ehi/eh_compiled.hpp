// Functionality to ease the duties of the EH compiler

#include "eh.hpp"

static const char *get_filename();
ehval_p eh_main(EHI *ehi, const ehcontext_t &context);

namespace eh_compiled {

static inline ehval_p make_closure(ehlibmethod_t method, const ehcontext_t &context, EHI *ehi) {
	ehval_p ret = ehi->get_parent()->make_method(method);
	ret->get<Object>()->parent = context.scope;
	return ret;
}

};

int main(int argc, char *argv[]) {
	EHInterpreter interpreter;
	interpreter.eh_setarg(argc, argv);
	EHI ehi(end_is_end_e, &interpreter, interpreter.global_object, eh_getcwd(), get_filename());
	try {
		eh_main(&ehi, ehi.get_context());
	} catch (eh_exception &e) {
		ehi.handle_uncaught(e);
		return -1;
	}
	return 0;
}
