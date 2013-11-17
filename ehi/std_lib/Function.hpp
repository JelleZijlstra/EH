/*
 * Function class
 */
#ifndef EH_FUNCTION_H_
#define EH_FUNCTION_H_

#include "std_lib_includes.hpp"

#include <sstream>

/*
 * EH functions. Unlike other primitive types, functions must always be wrapped
 * in objects in order to preserve scope.
 */
EH_CLASS(Function) {
public:
	enum functype_enum {
		user_e,
		lib_e,
		compiled_e
	};

	typedef ehval_p (*compiled_method)(ehval_p, ehval_p, class EHI *, const ehcontext_t &context);

	class t {
	public:
		functype_enum type;
		ehval_p args;
		ehval_p code;
		union {
			ehlibmethod_t libmethod_pointer;
			compiled_method compiled_pointer;
		};
		ehval_p parent;
		std::string name;

		t(functype_enum _type = user_e) : type(_type), args(nullptr), code(nullptr), libmethod_pointer(nullptr) {}

		t(ehlibmethod_t pointer) : type(lib_e), args(), code(), libmethod_pointer(pointer) {}

		~t() {}

	private:
		t(const t&);
		t operator=(const t&);
	};
	typedef t *type;
	type value;

	virtual bool belongs_in_gc() const override {
		return true;
	}

	virtual std::list<ehval_p> children() const override {
		return { value->args, value->code, value->parent };
	}

	virtual std::string decompile(int level) const override {
		if(value->type == lib_e || value->type == compiled_e) {
			return "(args) => (native code)";
		} else {
			std::ostringstream out;
			out << "func: " << value->args->decompile(level) << "\n";
			for(int i = 0; i < level + 1; i++) {
				out << "\t";
			}
			out << value->code->decompile(level + 1) << "\n";
			for(int i = 0; i < level; i++) {
				out << "\t";
			}
			out << "end";
			return out.str();
		}
	}

	virtual const std::string get_name() const override {
		return value->name;
	}

	virtual void printvar(printvar_set &set, int level, EHI *ehi) override {
		std::cout << "@function <";
		switch(value->type) {
			case user_e:
				std::cout << "user>: ";
				std::cout << value->args->decompile(0);
				break;
			case lib_e:
				std::cout << "library>: ";
				break;
			case compiled_e:
				std::cout << "compiled>: ";
				break;
		}
		std::cout << std::endl;
	}

	virtual ~Function() {
		delete value;
	}

	Function(type val) : value(val) {}

	static ehval_p exec(ehval_p base_object, ehval_p function_object, ehval_p args, EHI *ehi);

	static ehval_p make(t *val, EHInterpreter *parent);
};

EH_CHILD_CLASS(Function, Scope) {
public:
	class t {
	public:
		obj_map members;
		ehval_p parent_scope;

		t(ehval_p parent) : members(), parent_scope(parent) {}

		bool has(const std::string &key) {
			return members.count(key) != 0;
		}

		void insert(const std::string &key, ehmember_p value) {
			members[key] = value;
		}

		ehmember_p get(const std::string &key) {
			return members.at(key);
		}
	};

	typedef t *type;
	type value;

	Function_Scope(type val) : value(val) {}

	virtual ~Function_Scope() {
		delete value;
	}

	virtual bool belongs_in_gc() const override {
		return true;
	}

	virtual std::list<ehval_p> children() const override {
		std::list<ehval_p> out;
		for(auto &pair : value->members) {
			out.push_back(pair.second->value);
		}
		out.push_back(value->parent_scope);
		return out;
	}

	virtual bool can_access_private(ehcontext_t context, class EHI *ehi) override {
		return ehval_p(this) == context.scope;
	}

	virtual void set_member_directly(const char *name, ehmember_p value, ehcontext_t context, class EHI *ehi);

	virtual ehmember_p get_property_current_object(const char *name, ehcontext_t context, class EHI *ehi);

    virtual ehval_p get_parent_scope();

	static ehval_p make(ehval_p parent, EHInterpreter *interpreter_parent);
};

EH_METHOD(Function, operator_colon);
EH_METHOD(Function, toString);
EH_METHOD(Function, decompile);
EH_METHOD(Function, bindTo);
EH_METHOD(Function, args);
EH_METHOD(Function, code);

EH_INITIALIZER(Function);

EH_METHOD(Function_Scope, toString);

EH_INITIALIZER(Function_Scope);

#endif /* EH_FUNCTION_H_ */
