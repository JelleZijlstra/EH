// Main function for compiled EH programs
#include "eh_compiled.hpp"

static const char *get_filename();
ehval_p eh_main(EHI *ehi, const ehcontext_t &context);

int main(int argc, char *argv[]) {
    EHInterpreter interpreter;
    interpreter.eh_setarg(argc + 1, argv - 1);
    EHI ehi(end_is_end_e, &interpreter, interpreter.global_object, eh_getcwd(), get_filename());
    try {
        eh_main(&ehi, ehi.get_context());
    } catch(eh_exception &e) {
        ehi.handle_uncaught(e);
        return 1;
    } catch(quit_exception &) {
        return 1;
    }
    return 0;
}
