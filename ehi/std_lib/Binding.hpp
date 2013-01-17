#ifndef EH_BINDING_H_
#define EH_BINDING_H_

#include "std_lib_includes.hpp"

// method binding
EH_CLASS(Binding) {
public:
	class t {
	public:
		ehval_w object_data;
		ehval_w method;

		t(ehval_p _object_data, ehval_p _method) : object_data(_object_data),  method(_method) {}
	};

	typedef t *type;
	type value;

	Binding(t *val) : value(val) {}

	virtual bool belongs_in_gc() const override {
		return true;
	}

	virtual std::list<ehval_p> children() const override {
		return { value->object_data, value->method };
	}

	virtual std::string decompile(int level) const override {
		return value->method->decompile(level);
	}

	virtual void printvar(printvar_set &set, int level, EHI *ehi) override {
		value->method->printvar(set, level, ehi);
	}

	virtual ~Binding() {
		delete value;
	}

	static ehval_p make(ehval_p obj, ehval_p method, EHInterpreter *parent);
};

EH_METHOD(Binding, operator_colon);
EH_METHOD(Binding, toString);
EH_METHOD(Binding, decompile);
EH_METHOD(Binding, bindTo);
EH_METHOD(Binding, new);
EH_METHOD(Binding, object);
EH_METHOD(Binding, method);

EH_INITIALIZER(Binding);

#endif /* EH_BINDING_H_ */
