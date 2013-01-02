#ifndef EH_EHI_H_
#define EH_EHI_H_
/*
 * ehi.h
 * Jelle Zijlstra, December 2011
 */
#include "eh.bison.hpp"
#include "eh_error.hpp"
#include "concurrency.hpp"
#include "eh_files.hpp"
#include "std_lib/Function.hpp"
#include "std_lib/Integer.hpp"
#include "std_lib/String.hpp"
#include "std_lib/Float.hpp"
#include "std_lib/Bool.hpp"
#include "std_lib/GlobalObject.hpp"

#include <sstream>

/*
 * Flex and Bison
 */
int yylex(YYSTYPE *, void *);
int yylex_init_extra(class EHI *, void **);
int yylex_destroy(void *);
struct yy_buffer_state *yy_scan_string (const char *str);

typedef enum {
	cli_prompt_e,
	cli_no_prompt_e,
	end_is_end_e
} interactivity_enum;

void *gc_thread(void *arg);

class EHInterpreter {
public:
	Hash::ehhash_t *cmdtable;

private:
	void eh_init(void);

	// disallowed operations
	EHInterpreter(const EHInterpreter&);
	EHInterpreter operator=(const EHInterpreter&);
public:
	// our GC
	garbage_collector<ehval_t> gc;
	type_repository repo;
	ehval_p global_object;
	ehval_p function_object;
	ehval_p base_object;

	int enum_id;
	int enum_member_id;
	int enum_instance_id;

	bool optimize;

	// for some reason I haven't been able to figure out, I get a compiler error
	// inside glibc when I declare this as std::set<const std::string>.
	std::set<std::string> included_files;

	ehval_p get_primitive_class(ehval_p obj) {
		return this->repo.get_object(obj);
	}

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
	ehval_p resource_instantiate(int type_id, ehval_p obj);
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

	virtual char *eh_getline();
	virtual ehval_p execute_cmd(const char *rawcmd, Array::t *paras);

	ehval_p parse_file(FILE *infile);
	ehval_p parse_string(const char *cmd);

	int eh_interactive(interactivity_enum interactivity = cli_prompt_e);
	ehval_p global_parse_file(const char *name) {
		try {
			return parse_file(name, parent->global_object);
		} catch(eh_exception &) {
			return nullptr;
		}
	}
	ehval_p global_parse_string(const char *cmd) {
		try {
			return parse_string(cmd, parent->global_object);
		} catch(eh_exception &e) {
			handle_uncaught(e);
			return nullptr;
		}
	}
	ehval_p parse_string(const char *cmd, ehcontext_t context);
	ehval_p parse_file(const char *name, ehcontext_t context);
	ehval_p parse_interactive();

	ehval_p call_method(ehval_p in, const char *name, ehval_p args, ehcontext_t context);

	template<class T>
	ehval_p call_method_typed(ehval_p in, const char *name, ehval_p args, ehcontext_t context) {
		ehval_p out = call_method(in, name, args, context);
		if(!out->is_a<T>()) {
			std::ostringstream message;
			message << "Method " << name << " must return a value of type ";
			message << ehval_t::name<T>();
			throw_TypeError(message.str().c_str(), out, this);
		}
		return out;
	}

	ehval_p set(ehval_p lvalue, ehval_p rvalue, attributes_t *attributes, ehcontext_t context);
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
	const std::string &get_context_name() const {
		return context_name;
	}
	const std::string &get_working_dir() const {
		return working_dir;
	}

	/*
	 * Constructors and destructors.
	 */
	EHI(interactivity_enum _inter, EHInterpreter *_parent, ehcontext_t _context, const std::string &dir, const std::string &name) : scanner(), interactivity(_inter), yy_buffer(), buffer(), parent(_parent), interpreter_context(_context), inloop(0), breaking(0), continuing(0), returning(false), working_dir(dir), context_name(name) {
		yylex_init_extra(this, &scanner);
	}
	EHI() : scanner(), interactivity(cli_prompt_e), yy_buffer(), buffer(), parent(nullptr), inloop(0), breaking(0), continuing(0), returning(false), working_dir(eh_getcwd()), context_name("(none)") {
		yylex_init_extra(this, &scanner);
		parent = new EHInterpreter();
		interpreter_context = parent->global_object;
	}
	virtual ~EHI(void) {
		yylex_destroy(scanner);
		delete[] this->buffer;
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
	EHInterpreter *parent;
	ehcontext_t interpreter_context;

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
	ehval_p call_function(ehval_p function, ehval_p args, ehcontext_t context);
	ehval_p eh_always_execute(ehval_p code, ehcontext_t context);
	ehval_p eh_op_anonclass(ehval_p node, ehcontext_t context);
	ehval_p eh_op_array(ehval_p node, ehcontext_t context);
	ehval_p eh_op_colon(ehval_p *paras, ehcontext_t context);
	ehval_p eh_op_customop(ehval_p *paras, ehcontext_t context);
	ehval_p eh_op_command(const char *name, ehval_p node, ehcontext_t context);
	ehval_p eh_op_enum(Node::t *op, ehcontext_t context);
	ehval_p eh_op_declareclass(Node::t *op, ehcontext_t context);
	ehval_p eh_op_declareclosure(ehval_p *paras, ehcontext_t context);
	ehval_p eh_op_dollar(ehval_p node, ehcontext_t context);
	ehval_p eh_op_dot(ehval_p *paras, ehcontext_t context);
	ehval_p eh_op_match(ehval_p *paras, ehcontext_t context);
	bool match(ehval_p node, ehval_p var, ehcontext_t context);
	ehval_p eh_op_given(ehval_p *paras, ehcontext_t context);
	ehval_p eh_op_if(Node::t *op, ehcontext_t context);
	ehval_p eh_op_in(Node::t *op, ehcontext_t context);
	ehval_p eh_op_set(ehval_p *paras, ehcontext_t context);
	ehval_p eh_op_switch(ehval_p *paras, ehcontext_t context);
	ehval_p eh_op_try(Node::t *op, ehcontext_t context);
	ehval_p eh_op_tuple(ehval_p node, ehcontext_t context);
	ehval_p eh_op_while(ehval_p *paras, ehcontext_t context);
	ehval_p eh_try_catch(ehval_p try_block, ehval_p catch_blocks, ehcontext_t context);
	ehval_p eh_xtoarray(ehval_p in);
	ehval_p perform_op(const char *name, int nargs, ehval_p *paras, ehcontext_t context);
	void array_insert(Array::t *array, ehval_p in, int place, ehcontext_t context);
	void eh_op_break(Node::t *op, ehcontext_t context);
	ehval_p eh_op_classmember(Node::t *op, ehcontext_t context);
	void eh_op_continue(Node::t *op, ehcontext_t context);

	ehval_p promote(ehval_p in, ehcontext_t context);
	bool eh_floatequals(float infloat, ehval_p operand2, ehcontext_t context) {
		ehval_p operand = this->toInteger(operand2, context);
		// checks whether a float equals an int. C++ handles this correctly.
		return (infloat == static_cast<float>(operand->get<Integer>()));
	}

public:
	// conversion methods, guaranteed to return the type they're supposed to return
#define CASTER(class_name) ehval_p to ## class_name(ehval_p in, ehcontext_t context) { \
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

	int compare(ehval_p lhs, ehval_p rhs, ehcontext_t context) {
		return call_method_typed<Integer>(lhs, "operator<=>", rhs, context)->get<Integer>();
	}
};

template<class T>
inline int ehobj_t::register_member_class(const ehobj_t::initializer init_func, const char *name, const attributes_t attributes, class EHInterpreter *interpreter_parent, ehval_p the_class) {
	ehobj_t *newclass;
	ehval_p new_value;
	if(the_class == nullptr) {
		newclass = new ehobj_t();
		new_value = Object::make(newclass, interpreter_parent);
	} else {
		newclass = the_class->get<Object>();
		new_value = the_class;
	}

	// register class
	newclass->type_id = interpreter_parent->repo.register_inbuilt_class<T>(new_value);

	// inherit from Object, except in Object itself
	if(typeid(T) != typeid(GlobalObject)) {
		if(typeid(T) != typeid(Object)) {
			newclass->inherit(interpreter_parent->base_object);
		}
		newclass->parent = interpreter_parent->global_object;
	}
	init_func(newclass, interpreter_parent);
	// inherit from Object, except in Object itself
	ehmember_p member = ehmember_t::make(attributes, new_value);
	this->insert(name, member);
	return newclass->type_id;
}

#include "std_lib/Null.hpp"

template<>
inline bool ehval_t::is_a<Null>() const {
	return (this == nullptr) || typeid(*this) == typeid(Null);
}

inline bool ehval_t::equal_type(ehval_p rhs) const {
	bool lhs_null = this->is_a<Null>();
	bool rhs_null = rhs->is_a<Null>();
	if(lhs_null != rhs_null) {
		return false;
	} else if(lhs_null) {
		return true;
	} else {
		return typeid(*this) == typeid(rhs.operator*());
	}
}

inline std::type_index ehval_t::type_index() const {
	return (this == nullptr) ? std::type_index(typeid(Null)) : std::type_index(typeid(*this));
}

inline int ehval_t::get_type_id(class EHInterpreter *parent) {
	if(this == nullptr) {
		return parent->repo.get_primitive_id<Null>();
	} else if(this->is_a<Object>()) {
		return get<Object>()->type_id;
	} else {
		return parent->repo.get_type_id(this);
	}
}

inline ehval_t *ehval_t::null_object() {
	return Null::make().operator->();
}


#endif /* EH_EHI_H_ */

