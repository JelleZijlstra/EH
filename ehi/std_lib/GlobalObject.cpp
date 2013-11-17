/*
 * GlobalObject
 *
 * Contains methods of the global object (i.e., global functions).
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
#include "JSON.hpp"
#include "LoopError.hpp"
#include "MiscellaneousError.hpp"
#include "NameError.hpp"
#include "Node.hpp"
#include "Null.hpp"
#include "Object.hpp"
#include "Random.hpp"
#include "Range.hpp"
#include "String.hpp"
#include "SyntaxError.hpp"
#include "Tuple.hpp"
#include "TypeError.hpp"
#include "UnknownCommandError.hpp"
#include "EmptyIterator.hpp"
#include "Map.hpp"
#include "VisibilityError.hpp"

#include "../eh.bison.hpp"
#include "../eh_files.hpp"

#define GLOBAL_REGISTER_CLASS(name) obj->register_member_class<name>(ehinit_ ## name, #name, attributes_t::make_const(), parent)
#define REGISTER_PURE_CLASS(name) obj->register_member_class(#name, ehinit_ ## name, attributes_t::make_const(), parent)
#define REGISTER_ENUM_CLASS(name) register_enum_class(obj, ehinit_ ## name, #name, attributes_t::make_const(), parent)

EH_INITIALIZER(GlobalObject) {
	cls->parent = nullptr;
	// do nothing; everything is on the global instance
}

void ehinstance_init_GlobalObject(ehobj_t *obj, EHInterpreter *parent) {
	/*
	 * Initialization of base classes.
	 */
	parent->class_object = Class::make(new ehclass_t("Class"), parent);
	parent->base_object = Class::make(new ehclass_t("Object"), parent);
	parent->function_object = Class::make(new ehclass_t("Function"), parent);
	// Must be the first class registered
	obj->register_member_class<Object>(ehinit_Object, "Object", attributes_t::make_const(), parent, parent->base_object);
	// Must be registered before any methods are registered
	obj->register_member_class<Function>(ehinit_Function, "Function", attributes_t::make_const(), parent, parent->function_object);
	obj->register_member_class<Class>(ehinit_Class, "Class", attributes_t::make_const(), parent, parent->class_object);

	// insert reference to global object
	ehmember_p global = ehmember_t::make(attributes_t::make_const(), parent->global_object);
	obj->insert("global", global);

	/*
	 * Initialize top-level classes.
	 */
	GLOBAL_REGISTER_CLASS(Enum);
	REGISTER_ENUM_CLASS(Attribute); // 7
	REGISTER_ENUM_CLASS(Node); // 8
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
	REGISTER_PURE_CLASS(Exception);
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
	REGISTER_PURE_CLASS(JSON);

	/*
	 * Inititalize top-level methods.
	 */
	ehobj_t *cls = obj;
	REGISTER_STATIC_METHOD(GlobalObject, toString);
	REGISTER_STATIC_METHOD(GlobalObject, getinput);
	REGISTER_STATIC_METHOD(GlobalObject, printvar);
	REGISTER_STATIC_METHOD(GlobalObject, include);
	REGISTER_STATIC_METHOD(GlobalObject, pow);
	REGISTER_STATIC_METHOD(GlobalObject, log);
	REGISTER_STATIC_METHOD(GlobalObject, throw);
	REGISTER_STATIC_METHOD(GlobalObject, echo);
	REGISTER_STATIC_METHOD(GlobalObject, put);
	REGISTER_STATIC_METHOD(GlobalObject, handleUncaught);
	REGISTER_STATIC_METHOD(GlobalObject, workingDir);
	REGISTER_STATIC_METHOD(GlobalObject, shell);
	REGISTER_STATIC_METHOD(GlobalObject, exit);
}

/*
 * @description Gives a string representation of the global object
 * @argument None
 * @returns String
 */
EH_METHOD(GlobalObject, toString) {
	return String::make(strdup("(global execution context)"));
}

/*
 * @description Gives a debugging representation of an object
 * @argument Object to represent
 * @returns Null
 */
EH_METHOD(GlobalObject, printvar) {
	printvar_set set;
	args->printvar(set, 0, ehi);
	// this function always returns nullptr
	return nullptr;
}

/*
 * @description Include and execute a file. Largely equivalent to
 * "EH.eval(File.readfile file)", but guards against including the same file
 * twice and updates the current directory.
 * @argument Name of file
 * @returns Return value of the file
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
	const std::string dirname = eh_dirname(full_path);
	EHI parser(end_is_end_e, ehi->get_parent(), obj, dirname, filename);
	return parser.execute_named_file(full_path.c_str());
}

/*
 * @description Raises a number to a given power.
 * @argument Tuple of size 2: base and exponent
 * @returns Result of computation
 */
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

/*
 * @description Returns the natural logarithm of a number.
 * @argument Number
 * @returns Logarithm
 */
EH_METHOD(GlobalObject, log) {
	ehval_p arg = ehi->toFloat(args, obj);
	return Float::make(static_cast<Float::type>(log(arg->get<Float>())));
}

/*
 * @description Reads numeric input from stdin.
 * @argument None
 * @returns Integer
 */
EH_METHOD(GlobalObject, getinput) {
	ASSERT_NULL("getinput");
	// more accurately, getint
	int i = 0;
	fscanf(stdin, "%d", &i);
	ehval_p ret = Integer::make(i);
	return ret;
}

/*
 * @description Throws an exception.
 * @argument Object to throw
 * @returns Never
 */
EH_METHOD(GlobalObject, throw) {
	throw eh_exception(args);
}

/*
 * @description Prints out a variable, followed by a newline. Calls toString
 * on the variable if it is not already a string.
 * @argument Variable to print
 * @returns Null
 */
EH_METHOD(GlobalObject, echo) {
	ehval_p str = ehi->toString(args, obj);
	std::cout << str->get<String>() << std::endl;
	return nullptr;
}

/*
 * @description Similar to echo, but does not print a newline.
 * @argument Anything
 * @returns Null
 */
EH_METHOD(GlobalObject, put) {
	ehval_p str = ehi->toString(args, obj);
	std::cout << str->get<String>();
	return nullptr;
}

/*
 * @description Method that is called when an exception is not caught. By
 * default, it prints a message to stderr including the type of the exception
 * and the exception's message.
 * @argument Exception that was thrown
 * @returns N/A
 */
EH_METHOD(GlobalObject, handleUncaught) {
	const unsigned int type_id = args->get_type_id(ehi->get_parent());
	const std::string &type_string = ehi->get_parent()->repo.get_name(type_id);
	// we're in global context now. Remember this object, because otherwise the string may be freed before we're done with it.
	ehval_p stringval = ehi->toString(args, ehi->global());
	const char *msg = stringval->get<String>();
	std::cerr << "Uncaught exception of type " << type_string << ": " << msg << std::endl;
	return nullptr;
}

/*
 * @description Returns the current working directory. This is the directory
 * that is used by include to resolve file names.
 * @argument None
 * @returns String
 */
EH_METHOD(GlobalObject, workingDir) {
	ASSERT_NULL("workingDir");
	return String::make(strdup(ehi->get_working_dir().c_str()));
}

/*
 * @description Execute a command in the shell.
 * @argument Shell command
 * @returns Shell output
 */
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
