/*
 * GlobalObject.cpp
 * Jelle Zijlstra, September 2012
 *
 * Contains methods of the global object (i.e., global functions)
 */

#include <cmath>

#include "ArgumentError.hpp"
#include "Array.hpp"
#include "Attribute.hpp"
#include "Binding.hpp"
#include "Bool.hpp"
#include "ConstError.hpp"
#include "EH.hpp"
#include "Enum.hpp"
#include "Exception.hpp"
#include "File.hpp"
#include "FixedArray.hpp"
#include "Float.hpp"
#include "Function.hpp"
#include "GarbageCollector.hpp"
#include "GlobalObject.hpp"
#include "Hash.hpp"
#include "Integer.hpp"
#include "LoopError.hpp"
#include "MiscellaneousError.hpp"
#include "NameError.hpp"
#include "Node.hpp"
#include "Null.hpp"
#include "Object.hpp"
#include "Random.hpp"
#include "Range.hpp"
#include "String.hpp"
#include "SuperClass.hpp"
#include "SyntaxError.hpp"
#include "Tuple.hpp"
#include "TypeError.hpp"
#include "UnknownCommandError.hpp"
#include "EmptyIterator.hpp"
#include "Map.hpp"
#include "VisibilityError.hpp"

#include "../eh.bison.hpp"

#define GLOBAL_REGISTER_CLASS(name) obj->register_member_class<name>(ehinit_ ## name, #name, attributes_t::make_const(), parent)
#define REGISTER_PURE_CLASS(name) obj->register_member_class(#name, ehinit_ ## name, attributes_t::make_const(), parent)
#define REGISTER_ENUM_CLASS(name) obj->register_enum_class(ehinit_ ## name, #name, attributes_t::make_const(), parent)

EH_INITIALIZER(GlobalObject) {
	/*
	 * Initialization of base classes.
	 */
	parent->base_object = Object::make(new ehobj_t(), parent);
	parent->function_object = Object::make(new ehobj_t(), parent);
	// Must be the first class registered
	obj->register_member_class<Object>(ehinit_Object, "Object", attributes_t::make_const(), parent, parent->base_object);
	// Must be registered before any methods are registered
	obj->register_member_class<Function>(ehinit_Function, "Function", attributes_t::make_const(), parent, parent->function_object);

	// insert reference to global object
	ehmember_p global = ehmember_t::make(attributes_t::make_const(), parent->global_object);
	obj->insert("global", global);

	/*
	 * Initialize top-level classes.
	 */
	GLOBAL_REGISTER_CLASS(Enum);
	REGISTER_ENUM_CLASS(Attribute); // 5
	REGISTER_ENUM_CLASS(Node); // 6
	GLOBAL_REGISTER_CLASS(Binding);
	GLOBAL_REGISTER_CLASS(File);
	GLOBAL_REGISTER_CLASS(Integer);
	GLOBAL_REGISTER_CLASS(String);
	GLOBAL_REGISTER_CLASS(Array);
	GLOBAL_REGISTER_CLASS(Float);
	GLOBAL_REGISTER_CLASS(Bool);
	GLOBAL_REGISTER_CLASS(Null);
	GLOBAL_REGISTER_CLASS(Range);
	GLOBAL_REGISTER_CLASS(Hash);
	GLOBAL_REGISTER_CLASS(Tuple);
	GLOBAL_REGISTER_CLASS(SuperClass);
	GLOBAL_REGISTER_CLASS(Exception);
	REGISTER_PURE_CLASS(UnknownCommandError);
	REGISTER_PURE_CLASS(TypeError);
	REGISTER_PURE_CLASS(LoopError);
	REGISTER_PURE_CLASS(NameError);
	REGISTER_PURE_CLASS(ConstError);
	REGISTER_PURE_CLASS(ArgumentError);
	REGISTER_PURE_CLASS(SyntaxError);
	REGISTER_PURE_CLASS(MiscellaneousError);
	REGISTER_PURE_CLASS(GarbageCollector);
	REGISTER_PURE_CLASS(EmptyIterator);
	GLOBAL_REGISTER_CLASS(FixedArray);
	REGISTER_PURE_CLASS(Random);
	GLOBAL_REGISTER_CLASS(Map);
	REGISTER_PURE_CLASS(VisibilityError);
	REGISTER_PURE_CLASS(EH);

	/*
	 * Inititalize top-level methods.
	 */
	REGISTER_METHOD(GlobalObject, toString);
	REGISTER_METHOD(GlobalObject, getinput);
	REGISTER_METHOD(GlobalObject, printvar);
	REGISTER_METHOD(GlobalObject, include);
	REGISTER_METHOD(GlobalObject, pow);
	REGISTER_METHOD(GlobalObject, log);
	REGISTER_METHOD(GlobalObject, throw);
	REGISTER_METHOD(GlobalObject, echo);
	REGISTER_METHOD(GlobalObject, put);
	REGISTER_METHOD(GlobalObject, handleUncaught);
	REGISTER_METHOD(GlobalObject, workingDir);
	REGISTER_METHOD(GlobalObject, shell);
	REGISTER_METHOD(GlobalObject, exit);
}

EH_METHOD(GlobalObject, toString) {
	return String::make(strdup("(global execution context)"));
}

/*
 * printvar
 */

EH_METHOD(GlobalObject, printvar) {
	printvar_set set;
	args->printvar(set, 0, ehi);
	// this function always returns nullptr
	return nullptr;
}

/*
 * Including files
 */
EH_METHOD(GlobalObject, include) {
	args->assert_type<String>("include", ehi);
	// do the work
	const char *filename = args->get<String>();
	std::string full_path;
	if(filename[0] == '/') {
		full_path = filename;
	} else {
		full_path = ehi->get_working_dir() + "/" + filename;
	}
	// prevent including the same file more than once
	std::set<std::string> &included = ehi->get_parent()->included_files;
	if(included.count(full_path) > 0) {
		return nullptr;
	}
	included.insert(full_path);

	const std::string dirname = eh_dirname(full_path);
	EHI parser(end_is_end_e, ehi->get_parent(), obj, dirname, filename);
	return parser.execute_named_file(full_path.c_str());
}

// power
EH_METHOD(GlobalObject, pow) {
	ASSERT_NARGS(2, "pow");
	ehval_p rhs = args->get<Tuple>()->get(0);
	ehval_p lhs = args->get<Tuple>()->get(1);
	if(rhs->is_a<Integer>()) {
		if(lhs->is_a<Integer>()) {
			return Integer::make(static_cast<Integer::type>(pow((float) rhs->get<Integer>(), (float) lhs->get<Integer>())));
		} else if(lhs->is_a<Float>()) {
			return Float::make(static_cast<Float::type>(pow((float) rhs->get<Integer>(), lhs->get<Float>())));
		} else {
			throw_TypeError("Invalid type for argument to pow", lhs, ehi);
		}
	} else if(rhs->is_a<Float>()) {
		if(lhs->is_a<Integer>()) {
			return Float::make(static_cast<Float::type>(pow(rhs->get<Float>(), (float) lhs->get<Integer>())));
		} else if(lhs->is_a<Float>()) {
			return Float::make(static_cast<Float::type>(pow(rhs->get<Float>(), lhs->get<Float>())));
		} else {
			throw_TypeError("Invalid type for argument to pow", lhs, ehi);
		}
	} else {
		throw_TypeError("Invalid type for argument to pow", rhs, ehi);
	}
	return nullptr;
}

EH_METHOD(GlobalObject, log) {
	ehval_p arg = ehi->toFloat(args, obj);
	return Float::make(static_cast<Float::type>(log(arg->get<Float>())));
}

EH_METHOD(GlobalObject, getinput) {
	ASSERT_NULL("getinput");
	// more accurately, getint
	int i = 0;
	fscanf(stdin, "%d", &i);
	ehval_p ret = Integer::make(i);
	return ret;
}

EH_METHOD(GlobalObject, throw) {
	throw eh_exception(args);
}

EH_METHOD(GlobalObject, echo) {
	ehval_p str = ehi->toString(args, obj);
	std::cout << str->get<String>() << std::endl;
	return nullptr;
}

EH_METHOD(GlobalObject, put) {
	ehval_p str = ehi->toString(args, obj);
	std::cout << str->get<String>();
	return nullptr;
}

EH_METHOD(GlobalObject, handleUncaught) {
	const std::string &type_string = ehi->get_parent()->repo.get_name(args);
	// we're in global context now. Remember this object, because otherwise the string may be freed before we're done with it.
	ehval_p stringval = ehi->toString(args, ehi->global());
	const char *msg = stringval->get<String>();
	std::cerr << "Uncaught exception of type " << type_string << ": " << msg << std::endl;
	return nullptr;
}

EH_METHOD(GlobalObject, workingDir) {
	ASSERT_NULL("workingDir");
	return String::make(strdup(ehi->get_working_dir().c_str()));
}

EH_METHOD(GlobalObject, shell) {
	args->assert_type<String>("shell", ehi);
	std::string output = eh_shell_exec(args->get<String>());
	return String::make(strdup(output.c_str()));
}

/*
 * @description Exits the program.
 * @argument Return code (this is currently ignored)
 * @returns Never
 */
EH_METHOD(GlobalObject, exit) {
	ASSERT_TYPE(args, Integer, "exit");
	throw quit_exception();
}
