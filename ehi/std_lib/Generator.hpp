#ifndef EH_GENERATOR_H_
#define EH_GENERATOR_H_

#include "std_lib_includes.hpp"

EH_CLASS(Generator) {
	class t {
		ehval_p scope;
		EHI *ehi;
	};
	typedef t *type;
	type value;

	virtual ~Generator() {
		delete value;
	}

	virtual bool belongs_in_gc() const override {
		return true;
	}

	virtual std::list<ehval_p> children() const override {
		return {value->scope};
	}

}

EH_METHOD(Generator, getIterator);
EH_METHOD(Generator, next);
EH_METHOD(Generator, hasNext);
EH_METHOD(Generator, send);
EH_METHOD(Generator, close);
EH_METHOD(Generator, throw);

EH_INITIALIZER(Generator);
