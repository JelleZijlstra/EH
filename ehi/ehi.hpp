#ifndef EH_EHI_H_
#define EH_EHI_H_
/*
 * ehi.hpp
 * Jelle Zijlstra, December 2011
 */
#include <sstream>
#include <condition_variable>
#include <string.h>

#include "std_lib/Integer.hpp"
#include "std_lib/String.hpp"
#include "std_lib/Float.hpp"
#include "std_lib/Bool.hpp"
#include "std_lib/GlobalObject.hpp"
#include "std_lib/Map.hpp"
#include "std_lib/TypeError.hpp"
#include "std_lib/Hash.hpp"

/*
 * Flex and Bison
 */

typedef enum {
	cli_prompt_e,
	cli_no_prompt_e,
	end_is_end_e
} interactivity_enum;

class ehstack_entry_t;

typedef std::vector<class ehstack_entry_t *> ehstack_t;

// execution stack
class ehstack_entry_t {
public:
	const std::string name;
	ehval_p scope_object;
	ehstack_t &stack;

	ehstack_entry_t(const std::string &nm, ehval_p scope, ehstack_t &stk) : name(nm), scope_object(scope), stack(stk) {
		stack.push_back(this);
	}

	~ehstack_entry_t() {
		stack.pop_back();
	}

	const std::string to_string() const {
		return name;
	}
};

class EHInterpreter {
public:
	Hash::ehhash_t *cmdtable;
	// our GC
	garbage_collector<ehval_t> gc;
	type_repository repo;
	ehval_p global_object;
	ehval_p function_object;
	ehval_p base_object;
	ehval_p class_object;

	struct {
		unsigned int EmptyIterator;
		unsigned int GeneratorExit;
		unsigned int Enum;
		unsigned int Enum_Member;
		unsigned int Enum_Instance;
	} type_ids;

	bool optimize;

	std::set<std::string> included_files;

private:
	void eh_init(void);

	// disallowed operations
	EHInterpreter(const EHInterpreter&);
	EHInterpreter operator=(const EHInterpreter&);

public:
	void eh_setarg(int argc, char **argv);
	EHInterpreter();
	void eh_exit(void);

	virtual ~EHInterpreter();

	// helper functions
	ehval_p get_command(const char *name);
	void insert_command(const char *name, ehval_p cmd);
	void redirect_command(const char *redirect, const char *target);
	ehval_p make_method(ehlibmethod_t in);

	ehval_p instantiate(ehval_p obj);
	ehval_p resource_instantiate(unsigned int type_id, ehval_p obj);
	// stuff for GC'ed ehretval_ts
	template<class T>
	ehval_p allocate(typename T::type val) {
		void *place = gc.get_space();
		T *obj = new(place) T(val);
		return obj;
	}
};

/*
 * The EH parser
 */
class EHI {
public:

	ehval_p eh_execute(ehval_p node, const ehcontext_t context);

	ehval_p optimize(ehval_p node, const ehcontext_t context);
	ehval_p optimize(ehval_p node, ehcontext_t context, bool &is_generator);

	virtual char *eh_getline();
	virtual ehval_p execute_cmd(const char *rawcmd, eh_map_t *paras);

	// simply parse a FILE or string
	void parse_file(FILE *infile);
	void parse_string(const char *cmd);

	// parse and execute
	ehval_p execute_file(FILE *infile);
	ehval_p execute_named_file(const char *name);
	ehval_p execute_string(const char *cmd);

	// parse while spawning a new EHI object
	ehval_p spawning_parse_string(const char *cmd, const ehcontext_t &context);
	ehval_p spawning_parse_file(const char *name, const ehcontext_t &context);

	// execute stuff in global context, without worrying about exceptions
	ehval_p global_parse_file(const char *name) {
		try {
			return spawning_parse_file(name, parent->global_object);
		} catch(eh_exception &) {
			return nullptr;
		}
	}
	ehval_p global_parse_string(const char *cmd) {
		try {
			return spawning_parse_string(cmd, parent->global_object);
		} catch(eh_exception &e) {
			handle_uncaught(e);
			return nullptr;
		}
	}

	// execute the code object of the EHI
	ehval_p execute_code();

	// interactive parsing
	int eh_interactive(interactivity_enum interactivity = cli_prompt_e);
	ehval_p parse_interactive();

	/*
	 * EH API: operations exposed to the standard library.
	 */
	ehval_p call_method(ehval_p in, const char *name, ehval_p args, const ehcontext_t &context);
	ehval_p call_function(ehval_p function, ehval_p args, const ehcontext_t &context);

	template<class T>
	ehval_p call_method_typed(ehval_p in, const char *name, ehval_p args, const ehcontext_t &context) {
		ehval_p out = call_method(in, name, args, context);
		if(!out->is_a<T>()) {
			std::ostringstream message;
			message << "Method " << name << " must return a value of type ";
			message << ehval_t::name<T>();
			throw_TypeError(strdup(message.str().c_str()), out, this);
		}
		return out;
	}

	ehval_p set(ehval_p lvalue, ehval_p rvalue, attributes_t *attributes, const ehcontext_t &context);
	ehval_p set_bare_variable(const char *name, ehval_p rvalue, ehcontext_t context, attributes_t *attributes);
	void handle_uncaught(eh_exception &e);

	/*
	 * Accessors
	 */
	interactivity_enum get_interactivity() const {
		return interactivity;
	}
	bool get_returning() const {
		return returning;
	}
	void not_returning() {
		this->returning = false;
	}
	EHInterpreter *get_parent() const {
		return parent;
	}
	ehcontext_t get_context() const {
		return interpreter_context;
	}
	ehval_p global() const {
		return parent->global_object;
	}
	ehval_p get_code() const {
		return program_code;
	}
	void set_code(ehval_p c) {
		program_code = c;
	}
	const std::string &get_context_name() const {
		return context_name;
	}
	const std::string &get_working_dir() const {
		return working_dir;
	}
	ehstack_t &get_stack() {
		return stack;
	}

	/*
	 * Constructors and destructors.
	 */
	EHI(interactivity_enum _inter, EHInterpreter *_parent, ehcontext_t _context, const std::string &dir, const std::string &name);
	EHI();
	virtual ~EHI();

	/*
	 * Operations exposed for use in code generated by the compiler
	 */
	ehval_p eh_op_dollar(ehval_p node, const ehcontext_t &context);

	/*
	 * Lexer operations
	 */
	void reset_string() {
		str_str.str("");
	}

	template<class T>
	void add_to_string(T addition) {
		str_str << addition;
	}

	char *get_string() const {
		return strdup(str_str.str().c_str());
	}

private:

	/*
	 * Properties
	 */
	void *scanner;
	interactivity_enum interactivity;
	struct yy_buffer_state *yy_buffer;
	// buffer for interactive prompt
	char *buffer;
	// buffer for string lexing
	std::ostringstream str_str;

	EHInterpreter *parent;
	ehcontext_t interpreter_context;
	ehval_p program_code;
	ehstack_t stack;
	ehstack_entry_t stack_entry;

	// number of loops we're currently in
	int inloop;
	int breaking;
	int continuing;
	bool returning;

	// execution context
	std::string working_dir;
	const std::string context_name;

	/*
	 * Disallowed operations
	 */
	EHI(const EHI&);
	EHI operator=(const EHI&);

	/*
	 * Private methods
	 */
	ehval_p eh_always_execute(ehval_p code, const ehcontext_t &context);
	ehval_p eh_op_anonclass(ehval_p node, const ehcontext_t &context);
	ehval_p eh_op_array(ehval_p node, const ehcontext_t &context);
	ehval_p eh_op_colon(ehval_p *paras, const ehcontext_t &context);
	ehval_p eh_op_customop(ehval_p *paras, const ehcontext_t &context);
	ehval_p eh_op_command(const char *name, ehval_p node, const ehcontext_t &context);
	ehval_p eh_op_enum(ehval_p *paras, const ehcontext_t &context);
	ehval_p eh_op_class(ehval_p *paras, const ehcontext_t &context);
	ehval_p eh_op_named_class(ehval_p *paras, const ehcontext_t &context);
	ehval_p declare_class(const char *name, ehval_p code, const ehcontext_t &context);
	ehval_p eh_op_declareclosure(ehval_p *paras, const ehcontext_t &context, bool is_generator);
	ehval_p eh_op_dot(ehval_p *paras, const ehcontext_t &context);
	ehval_p eh_op_instance_access(ehval_p *paras, const ehcontext_t &context);
	ehval_p eh_op_match(ehval_p *paras, const ehcontext_t &context);
	bool match(ehval_p node, ehval_p var, const ehcontext_t &context);
	ehval_p eh_op_given(ehval_p *paras, const ehcontext_t &context);
	ehval_p eh_op_if(int token, ehval_p *paras, const ehcontext_t &context);
	ehval_p do_for_loop(ehval_p iteree_block, ehval_p body_block, int op, ehval_p set_block, const ehcontext_t &context);
	ehval_p eh_op_for(ehval_p *paras, const ehcontext_t &context);
	ehval_p eh_op_for_in(ehval_p *paras, const ehcontext_t &context);
	ehval_p eh_op_set(ehval_p *paras, const ehcontext_t &context);
	ehval_p eh_op_switch(ehval_p *paras, const ehcontext_t &context);
	ehval_p eh_op_try(ehval_p *paras, const ehcontext_t &context);
	ehval_p eh_op_try_finally(ehval_p *paras, const ehcontext_t &context);
	ehval_p eh_op_tuple(ehval_p node, const ehcontext_t &context);
	ehval_p eh_op_mixed_tuple(ehval_p node, const ehcontext_t &context);
	ehval_p eh_op_while(ehval_p *paras, const ehcontext_t &context);
	ehval_p eh_op_yield(ehval_p para, const ehcontext_t &context);
	ehval_p eh_try_catch(ehval_p try_block, ehval_p catch_blocks, const ehcontext_t &context);
	ehval_p eh_xtoarray(ehval_p in);
	ehval_p perform_op(const char *name, unsigned int nargs, ehval_p *paras, const ehcontext_t &context);
	void eh_op_break(ehval_p *paras, const ehcontext_t &context);
	ehval_p eh_op_classmember(ehval_p *paras, const ehcontext_t &context);
	void eh_op_continue(ehval_p *paras, const ehcontext_t &context);

	ehval_p promote(ehval_p in, const ehcontext_t &context);
	bool eh_floatequals(float infloat, ehval_p operand2, const ehcontext_t &context) {
		ehval_p operand = this->toInteger(operand2, context);
		// checks whether a float equals an int. C++ handles this correctly.
		return (infloat == static_cast<float>(operand->get<Integer>()));
	}

	void do_parse();

public:
	// conversion methods, guaranteed to return the type they're supposed to return
#define CASTER(class_name) ehval_p to ## class_name(ehval_p in, const ehcontext_t &context) { \
	if(in->is_a<class_name>()) { \
		return in; \
	} \
	return call_method_typed<class_name>(in, "to" #class_name, nullptr, context); \
}
	CASTER(String)
	CASTER(Integer)
	CASTER(Float)
	CASTER(Bool)
#undef CASTER

	int compare(ehval_p lhs, ehval_p rhs, const ehcontext_t &context) {
		return call_method_typed<Integer>(lhs, "operator<=>", rhs, context)->get<Integer>();
	}

	void printvar(ehval_p val) {
		printvar_set set;
		val->printvar(set, 0, this);
	}
};

#include "std_lib/Class.hpp"

template<class T>
inline unsigned int ehobj_t::register_member_class(const initializer init_func, const char *name, const attributes_t attributes, EHInterpreter *interpreter_parent, ehval_p the_class) {
	ehclass_t *newclass;
	ehval_p new_value;
	if(the_class == nullptr) {
		newclass = new ehclass_t(name);
		new_value = Class::make(newclass, interpreter_parent);
	} else {
		newclass = the_class->get<Class>();
		new_value = the_class;
	}

	// register class
	newclass->type_id = interpreter_parent->repo.register_inbuilt_class<T>(new_value);

	init_func(newclass, interpreter_parent);
	// inherit from Object, except in Object itself
	ehmember_p member = ehmember_t::make(attributes, new_value);
	this->insert(name, member);
	return newclass->type_id;
}

#include "std_lib/Null.hpp"

inline ehval_p ehval_t::null_object() {
	return Null::make();
}

#endif /* EH_EHI_H_ */
