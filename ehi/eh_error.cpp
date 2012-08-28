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
	ehretval_p class_member = ehi->get_property(ehi->global_object, NULL, class_name, ehcontext_t(ehi->global_object, ehi->global_object));
	ehretval_p e = ehi->call_method(class_member, "new", args, ehcontext_t(ehi->global_object, ehi->global_object));
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
	ehi->set_property(obj, "command", args, ehi->global_object);
	std::string msg = std::string("Unknown command: ") + args->get_stringval();
	return ehretval_t::make_resource(new Exception(strdup(msg.c_str())));
}
EH_METHOD(UnknownCommandError, toString) {
	ASSERT_OBJ_TYPE(resource_e, "UnknownCommandError.toString");
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
	ehi->set_property(obj, "message", msg, ehi->global_object);
	std::string type_str = ehi->repo.get_name(id->get_intval());
	ehi->set_property(obj, "type", id, ehi->global_object);
	std::string exception_msg = std::string(msg->get_stringval()) + ": " + type_str;
	return ehretval_t::make_resource(new Exception(strdup(exception_msg.c_str())));
}
EH_METHOD(TypeError, toString) {
	ASSERT_OBJ_TYPE(resource_e, "TypeError.toString");
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
	ehi->set_property(obj, "message", msg, ehi->global_object);
	ehi->set_property(obj, "level", level, ehi->global_object);
	std::ostringstream exception_msg;
	exception_msg << "Cannot " << msg->get_stringval() << " " << level->get_intval() << " levels";
	return ehretval_t::make_resource(new Exception(strdup(exception_msg.str().c_str())));
}
EH_METHOD(LoopError, toString) {
	ASSERT_OBJ_TYPE(resource_e, "LoopError.toString");
	Exception *e = reinterpret_cast<Exception *>(obj->get_resourceval());
	return ehretval_t::make_string(strdup(e->msg));
}

void throw_NameError(ehretval_p object, const char *name, EHI *ehi) {
	ehretval_p args[2];
	args[0] = object;
	args[1] = ehretval_t::make_string(strdup(name));
	throw_error("NameError", ehi->make_tuple(new ehtuple_t(2, args)), ehi);
}

START_EHLC(NameError)
EHLC_ENTRY(NameError, initialize)
EHLC_ENTRY(NameError, toString)
END_EHLC()

EH_METHOD(NameError, initialize) {
	ASSERT_NARGS(2, "NameError.initialize");
	ehretval_p object = args->get_tupleval()->get(0);
	ehretval_p name = args->get_tupleval()->get(1);
	ASSERT_TYPE(name, string_e, "NameError.initialize");
	ehi->set_property(obj, "object", object, ehi->global_object);
	ehi->set_property(obj, "name", name, ehi->global_object);
	std::ostringstream exception_msg;
	exception_msg << "Unknown member " << name->get_stringval() << " in object of type " << object->type_string(ehi);
	exception_msg << ": " << ehi->to_string(object, ehi->global_object)->get_stringval();
	return ehretval_t::make_resource(new Exception(strdup(exception_msg.str().c_str())));
}
EH_METHOD(NameError, toString) {
	ASSERT_OBJ_TYPE(resource_e, "NameError.toString");
	Exception *e = reinterpret_cast<Exception *>(obj->get_resourceval());
	return ehretval_t::make_string(strdup(e->msg));
}

void throw_ConstError(ehretval_p object, const char *name, EHI *ehi) {
	ehretval_p args[2] = {object, ehretval_t::make_string(strdup(name))};
	throw_error("ConstError", ehi->make_tuple(new ehtuple_t(2, args)), ehi);
}

START_EHLC(ConstError)
EHLC_ENTRY(ConstError, initialize)
EHLC_ENTRY(ConstError, toString)
END_EHLC()

EH_METHOD(ConstError, initialize) {
	ASSERT_NARGS(2, "ConstError.initialize");
	ehretval_p object = args->get_tupleval()->get(0);
	ehretval_p name = args->get_tupleval()->get(1);
	ASSERT_TYPE(name, string_e, "ConstError.initialize");
	ehi->set_property(obj, "object", object, ehi->global_object);
	ehi->set_property(obj, "name", name, ehi->global_object);
	std::ostringstream exception_msg;
	exception_msg << "Cannot set constant member " << name->get_stringval() << " in object of type " << object->type_string(ehi);
	exception_msg << ": " << ehi->to_string(object, ehi->global_object)->get_stringval();
	return ehretval_t::make_resource(new Exception(strdup(exception_msg.str().c_str())));
}
EH_METHOD(ConstError, toString) {
	ASSERT_OBJ_TYPE(resource_e, "ConstError.toString");
	Exception *e = reinterpret_cast<Exception *>(obj->get_resourceval());
	return ehretval_t::make_string(strdup(e->msg));
}

void throw_ArgumentError(const char *message, const char *method, ehretval_p value, EHI *ehi) {
	ehretval_p args[3];
	args[0] = ehretval_t::make_string(strdup(message));
	args[1] = ehretval_t::make_string(strdup(method));
	args[2] = value;
	ehretval_p the_tuple = ehi->make_tuple(new ehtuple_t(3, args));
	throw_error("ArgumentError", the_tuple, ehi);
}

START_EHLC(ArgumentError)
EHLC_ENTRY(ArgumentError, initialize)
EHLC_ENTRY(ArgumentError, toString)
END_EHLC()

EH_METHOD(ArgumentError, initialize) {
	ASSERT_NARGS(3, "ArgumentError.initialize");
	ehretval_p message = args->get_tupleval()->get(0);
	ASSERT_TYPE(message, string_e, "ArgumentError.initialize");
	ehi->set_property(obj, "message", message, ehi->global_object);

	ehretval_p method = args->get_tupleval()->get(1);
	ASSERT_TYPE(method, string_e, "ArgumentError.initialize");
	ehi->set_property(obj, "method", method, ehi->global_object);

	ehretval_p value = args->get_tupleval()->get(2);
	ehi->set_property(obj, "value", value, ehi->global_object);

	std::ostringstream exception_msg;
	exception_msg << message->get_stringval() << " (method " << method->get_stringval() << "): ";
	exception_msg << ehi->to_string(value, ehi->global_object)->get_stringval();
	return ehretval_t::make_resource(new Exception(strdup(exception_msg.str().c_str())));	
}
EH_METHOD(ArgumentError, toString) {
	ASSERT_OBJ_TYPE(resource_e, "ArgumentError.toString");
	Exception *e = reinterpret_cast<Exception *>(obj->get_resourceval());
	return ehretval_t::make_string(strdup(e->msg));
}

void throw_MiscellaneousError(const char *message, EHI *ehi) {
	throw_error("MiscellaneousError", ehretval_t::make_string(strdup(message)), ehi);
}

START_EHLC(MiscellaneousError)
EHLC_ENTRY(MiscellaneousError, initialize)
EHLC_ENTRY(MiscellaneousError, toString)
END_EHLC()

EH_METHOD(MiscellaneousError, initialize) {
	ASSERT_TYPE(args, string_e, "MiscellaneousError.initialize");
	ehi->set_property(obj, "message", args, ehi->global_object);
	return ehretval_t::make_resource(new Exception(strdup(args->get_stringval())));
}
EH_METHOD(MiscellaneousError, toString) {
	ASSERT_OBJ_TYPE(resource_e, "MiscellaneousError.toString");
	Exception *e = reinterpret_cast<Exception *>(obj->get_resourceval());
	return ehretval_t::make_string(strdup(e->msg));
}

void throw_SyntaxError(const char *message, int line, EHI *ehi) {
	ehretval_p args[2];
	args[0] = ehretval_t::make_string(strdup(message));
	args[1] = ehretval_t::make_int(line);
	throw_error("SyntaxError", ehi->make_tuple(new ehtuple_t(2, args)), ehi);
}

START_EHLC(SyntaxError)
EHLC_ENTRY(SyntaxError, initialize)
EHLC_ENTRY(SyntaxError, toString)
END_EHLC()

EH_METHOD(SyntaxError, initialize) {
	ASSERT_NARGS(2, "SyntaxError.initialize");
	ehretval_p message = args->get_tupleval()->get(0);
	ASSERT_TYPE(message, string_e, "SyntaxError.initialize");
	ehi->set_property(obj, "errorMessage", message, ehi->global_object);

	ehretval_p line = args->get_tupleval()->get(1);
	ASSERT_TYPE(line, int_e, "SyntaxError.initialize");
	ehi->set_property(obj, "line", line, ehi->global_object);

	std::ostringstream exception_msg;
	exception_msg << message->get_stringval() << " at line ";
	exception_msg << line->get_intval();
	return ehretval_t::make_resource(new Exception(strdup(exception_msg.str().c_str())));	
}
EH_METHOD(SyntaxError, toString) {
	ASSERT_OBJ_TYPE(resource_e, "SyntaxError.toString");
	Exception *e = reinterpret_cast<Exception *>(obj->get_resourceval());
	return ehretval_t::make_string(strdup(e->msg));	
}
