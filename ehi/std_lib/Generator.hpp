/*
 * Generators are implemented by saving the frame object, which contains
 * the current code offset. When a YIELD opcode is executed, the eh_execute_frame
 * function returns the value yielded to the caller. Subsequently, a value is sent
 * to the generator (using its .next() or .send() method), which then executes
 * a POST_YIELD instruction processing the response.
 *
 * Known issues:
 * - yielding from inside a try-catch will likely break
 * - returning can't be distinguished from yielding
 */

#ifndef EH_GENERATOR_H_
#define EH_GENERATOR_H_

#include "std_lib_includes.hpp"

EH_CLASS(Generator) {
public:
	class t {
	public:
		ehval_p function_object;
		eh_frame_t *frame;

		ehval_p run(EHI *ehi);

		t(ehval_p function_object_, eh_frame_t *frame_) : function_object(function_object_), frame(frame_) {}

		~t() {
			delete frame;
		}
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
		// TODO probably also need to retrieve local variables from the frame
		//return {value->frame->context.object, value->frame->context.scope, value->function_object};
		return std::list<ehval_p>{};
	}

	static ehval_p make(ehval_p function_object, eh_frame_t *frame, EHI *ehi);

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
