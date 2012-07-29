/*
 * eh_error.c
 * Jelle Zijlstra, December 2011
 *
 * Code file for the EH error handling system.
 */
#include "eh.h"
#include <stdio.h>
#include <stdlib.h>
#include <sstream>

void eh_error(const char *message, errlevel_e level) {
	if(message)
		fputs(message, stderr);
	switch(level) {
		case efatal_e:
			fprintf(stderr, ": EH fatal error\n");
			throw quit_exception();
		case eparsing_e:
			fprintf(stderr, ": EH parsing error\n");
			// TODO: figure out whether we're interactive
			break;
		case eerror_e:
			fprintf(stderr, ": EH runtime error\n");
			break;
		case enotice_e:
			fprintf(stderr, ": EH notice\n");
			break;
	}
}

void eh_error_type(const char *context, type_enum type, errlevel_e level) {
	fprintf(stderr, "Unsupported type %s for %s", get_typestring(type), context);
	eh_error(NULL, level);
}
void eh_error_types(const char *context, type_enum type1, type_enum type2, errlevel_e level) {
	fprintf(stderr, "Unsupported types %s and %s for operator %s", get_typestring(type1), get_typestring(type2), context);
	eh_error(NULL, level);
}
void eh_error_looplevels(const char *context, int levels) {
	fprintf(stderr, "%s for %d levels", context, levels);
	eh_error(NULL, eerror_e);
}
void eh_error_unknown(const char *kind, const char *name, errlevel_e level) {
	fprintf(stderr, "No such %s: %s", kind, name);
	eh_error(NULL, level);
}
void eh_error_redefine(const char *kind, const char *name, errlevel_e level) {
	fprintf(stderr, "Attempt to redefine %s: %s", kind, name);
	eh_error(NULL, level);
}
void eh_error_int(const char *message, int opcode, errlevel_e level) {
	fprintf(stderr, "%s: %d", message, opcode);
	eh_error(NULL, level);
}
void eh_error_argcount(int expected, int received) {
	fprintf(stderr, "Incorrect argument count for function: expected %d, got %d", expected, received);
	eh_error(NULL, eerror_e);
}
void eh_error_argcount_lib(const char *name, int expected, int received) {
	fprintf(stderr, "Incorrect argument count for function %s: expected %d, got %d", name, expected, received);
	eh_error(NULL, eerror_e);
}
void eh_error_line(int line, const char *msg) {
	fprintf(stderr, "In line %d: ", line);
	eh_error(msg, eparsing_e);
}
void eh_error_invalid_argument(const char *function, int n) {
  fprintf(stderr, "Invalid value for argument %d to %s", n, function);
  eh_error(NULL, enotice_e);
}

/*
 * Exception classes
 */
void throw_error(const char *class_name, ehretval_p args, EHI *ehi) {
	ehmember_p class_member = ehi->global_object->get_objectval()->operator[](class_name);
	if(class_member == NULL) {
		throw eh_exception(ehretval_t::make_string(strdup("Attempt to throw exception of non-existent type")));
	}
	ehretval_p e = ehi->call_method(class_member->value, "new", args, ehi->global_object);
	throw eh_exception(e);
}

void throw_UnknownCommandError(const char *msg, EHI *ehi) {
	throw_error("UnknownCommandError", ehretval_t::make_string(strdup(msg)), ehi);
}

START_EHLC(UnknownCommandError)
EHLC_ENTRY(UnknownCommandError, initialize)
EHLC_ENTRY(UnknownCommandError, toString)
END_EHLC()

EH_METHOD(UnknownCommandError, initialize) {
	ASSERT_TYPE(args, string_e, "UnknownCommandError.initialize");
	context->get_objectval()->set("command", args);
	std::string msg = std::string("Unknown command: ") + args->get_stringval();
	return ehretval_t::make_resource(new Exception(strdup(msg.c_str())));
}
EH_METHOD(UnknownCommandError, toString) {
	Exception *e = reinterpret_cast<Exception *>(obj->get_resourceval());
	return ehretval_t::make_string(strdup(e->msg));
}

void throw_TypeError(const char *msg, int type, EHI *ehi) {
	ehretval_p args[2];
	args[0] = ehretval_t::make_string(strdup(msg));
	args[1] = ehretval_t::make_int(type);
	throw_error("TypeError", ehi->make_tuple(new ehtuple_t(2, args)), ehi);
}

START_EHLC(TypeError)
EHLC_ENTRY(TypeError, initialize)
EHLC_ENTRY(TypeError, toString)
END_EHLC()

EH_METHOD(TypeError, initialize) {
	ASSERT_TYPE(args, tuple_e, "TypeError.initialize");
	ehretval_p msg = args->get_tupleval()->get(0);
	ASSERT_TYPE(msg, string_e, "TypeError.initialize");
	ehretval_p id = args->get_tupleval()->get(1);
	ASSERT_TYPE(id, int_e, "TypeError.initialize");
	context->get_objectval()->set("message", msg);
	std::string type_str = ehi->repo.get_name(id->get_intval());
	context->get_objectval()->set("type", id);
	std::string exception_msg = std::string(msg->get_stringval()) + ": " + type_str;
	return ehretval_t::make_resource(new Exception(strdup(exception_msg.c_str())));
}
EH_METHOD(TypeError, toString) {
	Exception *e = reinterpret_cast<Exception *>(obj->get_resourceval());
	return ehretval_t::make_string(strdup(e->msg));
}

void throw_LoopError(const char *msg, int level, EHI *ehi) {
	ehretval_p args[2];
	args[0] = ehretval_t::make_string(strdup(msg));
	args[1] = ehretval_t::make_int(level);
	throw_error("LoopError", ehi->make_tuple(new ehtuple_t(2, args)), ehi);	
}

START_EHLC(LoopError)
EHLC_ENTRY(LoopError, initialize)
EHLC_ENTRY(LoopError, toString)
END_EHLC()

EH_METHOD(LoopError, initialize) {
	ASSERT_TYPE(args, tuple_e, "LoopError.initialize");
	ehretval_p msg = args->get_tupleval()->get(0);
	ASSERT_TYPE(msg, string_e, "LoopError.initialize");
	ehretval_p level = args->get_tupleval()->get(1);
	ASSERT_TYPE(level, int_e, "LoopError.initialize");
	context->get_objectval()->set("message", msg);
	context->get_objectval()->set("level", level);
	std::ostringstream exception_msg;
	exception_msg << "Cannot " << msg->get_stringval() << " " << level->get_intval() << " levels";
	return ehretval_t::make_resource(new Exception(strdup(exception_msg.str().c_str())));
}
EH_METHOD(LoopError, toString) {
	Exception *e = reinterpret_cast<Exception *>(obj->get_resourceval());
	return ehretval_t::make_string(strdup(e->msg));
}
