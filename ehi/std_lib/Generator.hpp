#ifndef EH_GENERATOR_H_
#define EH_GENERATOR_H_

#include <thread>

#include "std_lib_includes.hpp"

EH_CLASS(Generator) {
public:
	class t {
	public:
		ehval_p scope;
		EHI *ehi;
		std::thread thread;

		ehval_p yield(ehval_p value);

		template<class Lambda>
		t(ehval_p _scope, EHI *_ehi, Lambda runner) : scope(_scope), ehi(_ehi), thread(runner) {}

		~t();
	};
	typedef t *type;
	type value;

	Generator(type val) : value(val) {}

	virtual ~Generator() {
		delete value;
	}

	virtual bool belongs_in_gc() const override {
		return true;
	}

	virtual std::list<ehval_p> children() const override {
		return {value->scope};
	}

	static ehval_p make(ehval_p base_object, ehval_p function_object, ehval_p args, ehval_p function_scope, EHI *ehi);

};

EH_METHOD(Generator, getIterator);
EH_METHOD(Generator, next);
EH_METHOD(Generator, send);
EH_METHOD(Generator, close);
EH_METHOD(Generator, throw);

EH_INITIALIZER(Generator);

void throw_GeneratorExit(EHI *ehi);

EH_METHOD(GeneratorExit, initialize);

EH_INITIALIZER(GeneratorExit);

#endif /* EH_GENERATOR_H_ */
