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
		ehval_w args;
		ehval_w code;
		union {
			ehlibmethod_t libmethod_pointer;
			compiled_method compiled_pointer;
		};

		t(functype_enum _type = user_e) : type(_type), args(nullptr), code(nullptr), libmethod_pointer(nullptr) {}

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
		return { value->args, value->code };
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

	Function(type val) : value(val) {}

	static ehval_p exec(ehval_p base_object, ehval_p function_object, ehval_p args, EHI *ehi);

	static ehval_p make(t *val, EHInterpreter *parent);
};

EH_METHOD(Function, operator_colon);
EH_METHOD(Function, toString);
EH_METHOD(Function, decompile);
EH_METHOD(Function, bindTo);

EH_INITIALIZER(Function);

#endif /* EH_FUNCTION_H_ */
