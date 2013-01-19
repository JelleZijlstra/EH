#include "Tuple.hpp"

EH_INITIALIZER(Tuple) {
	REGISTER_METHOD(Tuple, initialize);
	REGISTER_METHOD_RENAME(Tuple, operator_arrow, "operator->");
	REGISTER_METHOD(Tuple, length);
	REGISTER_METHOD(Tuple, toTuple);
	REGISTER_METHOD(Tuple, getIterator);
	REGISTER_METHOD(Tuple, compare);
	REGISTER_METHOD(Tuple, has);

	REGISTER_CLASS(Tuple, Iterator);
	REGISTER_CLASS(Tuple, WithStringKeys);
}

ehval_p Tuple::make(unsigned int size, ehval_p *in, EHInterpreter *parent) {
	return parent->allocate<Tuple>(new t(size, in));
}

/*
 * @description Initializer. Converts an arbitrary object to a tuple by the
 * following algorithm: call the object's .length() method; allocate a tuple of
 * that length; call the object's .getIterator() method and add members to the
 * tuple while iterating.
 * @argument Object that should be converted to a tuple.
 * @returns N/A
 */
EH_METHOD(Tuple, initialize) {
	ehval_p length = ehi->call_method_typed<Integer>(args, "length", nullptr, obj);
	const int raw_length = length->get<Integer>();
	if(raw_length < 0) {
		throw_ArgumentError("Tuple cannot have negative length", "Tuple.initialize", args, ehi);
	}
	unsigned int len = static_cast<unsigned int>(raw_length);

	ehretval_a values(len);

	ehval_p iterator = ehi->call_method(args, "getIterator", nullptr, obj);
	for(unsigned int i = 0; i < len; i++) {
		values[i] = ehi->call_method(iterator, "next", nullptr, obj);
	}
	return Tuple::make(len, values, ehi->get_parent());
}
EH_METHOD(Tuple, operator_arrow) {
	ASSERT_RESOURCE(Tuple, "Tuple.operator->");
	args->assert_type<Integer>("Tuple.operator->", ehi);
	int index = args->get<Integer>();
	if(!data->is_in_range(index)) {
    	throw_ArgumentError_out_of_range("Tuple.operator->", args, ehi);
	}
  	return data->get(index);
}
EH_METHOD(Tuple, has) {
	ASSERT_RESOURCE(Tuple, "Tuple.has");
	if(args->is_a<Integer>()) {
		return Bool::make(data->is_in_range(args->get<Integer>()));
	} else {
		return Bool::make(false);
	}
}
EH_METHOD(Tuple, length) {
  	ASSERT_NULL_AND_TYPE(Tuple, "Tuple.length");
  	return Integer::make(static_cast<Integer::type>(obj->get<Tuple>()->size()));
}
EH_METHOD(Tuple, toTuple) {
	ASSERT_OBJ_TYPE(Tuple, "Tuple.toTuple");
	return obj;
}
EH_METHOD(Tuple, compare) {
	ASSERT_OBJ_TYPE(Tuple, "Tuple.compare");
	args = args->data();
	args->assert_type<Tuple>("Tuple.compare", ehi);
	Tuple::t *lhs = obj->get<Tuple>();
	Tuple::t *rhs = args->get<Tuple>();
	int size_cmp = intcmp(lhs->size(), rhs->size());
	if(size_cmp != 0) {
		return Integer::make(size_cmp);
	}
	int size = lhs->size();
	ehval_p lhs_val, rhs_val;
	for(int i = 0; i < size; i++) {
		int comparison = ehi->compare(lhs->get(i), rhs->get(i), obj);
		if(comparison != 0) {
			return Integer::make(comparison);
		}
	}
	return Integer::make(0);
}
EH_METHOD(Tuple, getIterator) {
	ASSERT_NULL_AND_TYPE(Tuple, "Tuple.getIterator");
	ehval_p class_member = obj->get_property("Iterator", obj, ehi);
	return ehi->call_method(class_member, "new", obj, obj);
}

ehval_p Tuple_WithStringKeys::make(type in, EHInterpreter *parent) {
	return parent->allocate<Tuple_WithStringKeys>(in);
}

EH_INITIALIZER(Tuple_WithStringKeys) {
	REGISTER_METHOD_RENAME(Tuple_WithStringKeys, operator_arrow, "operator->");
	REGISTER_METHOD(Tuple_WithStringKeys, has);
}

/*
 * @description Checks whether a given key is valid.
 * @argument String or integer key
 * @returns Bool
 */
EH_METHOD(Tuple_WithStringKeys, has) {
	ASSERT_RESOURCE(Tuple_WithStringKeys, "Tuple.WithStringKeys.has");
	if(args->is_a<Integer>()) {
		return Bool::make(data->is_in_range(args->get<Integer>()));
	} else if(args->is_a<String>()) {
		return Bool::make(data->has(args->get<String>()));
	} else {
		return Bool::make(false);
	}
}

/*
 * @description Returns a value in the object given an integer or string key.
 * @argument String or integer key
 * @returns Value corresponding to the given key
 */
EH_METHOD(Tuple_WithStringKeys, operator_arrow) {
	ASSERT_RESOURCE(Tuple_WithStringKeys, "Tuple.WithStringKeys.operator->");
	if(args->is_a<Integer>()) {
		const int index = args->get<Integer>();
		if(index < 0 || index >= static_cast<Integer::type>(data->size())) {
    		throw_ArgumentError_out_of_range("Tuple.WithStringKeys.operator->", args, ehi);
		}
		return data->get(index);
	} else if(args->is_a<String>()) {
		const std::string name = args->get<String>();
		if(!data->has(name)) {
			throw_ArgumentError("Invalid key", "Tuple.WithStringKeys.operator->", args, ehi);
		}
		return data->get_string(name);
	} else {
		throw_TypeError("Tuple.WithStringKeys key must be an Integer or String", args, ehi);
		return nullptr;
	}
}

ehval_p Tuple_Iterator::make(ehval_p tuple, EHInterpreter *parent) {
	return parent->allocate<Tuple_Iterator>(new t(tuple));
}

bool Tuple_Iterator::t::has_next() {
	Tuple::t *the_tuple = this->tuple->get<Tuple>();
	return this->position < the_tuple->size();
}
ehval_p Tuple_Iterator::t::next() {
	Tuple::t *the_tuple = this->tuple->get<Tuple>();
	return the_tuple->get(this->position++);
}

EH_INITIALIZER(Tuple_Iterator) {
	REGISTER_METHOD(Tuple_Iterator, initialize);
	REGISTER_METHOD(Tuple_Iterator, hasNext);
	REGISTER_METHOD(Tuple_Iterator, next);
}

EH_METHOD(Tuple_Iterator, initialize) {
	args->assert_type<Tuple>("Tuple.Iterator.initialize", ehi);
	return Tuple_Iterator::make(args, ehi->get_parent());
}
EH_METHOD(Tuple_Iterator, hasNext) {
	args->assert_type<Null>("Tuple.Iterator.hasNext", ehi);
	ASSERT_RESOURCE(Tuple_Iterator, "Tuple.Iterator.hasNext");
	return Bool::make(data->has_next());
}
EH_METHOD(Tuple_Iterator, next) {
	args->assert_type<Null>("Tuple.Iterator.next", ehi);
	ASSERT_RESOURCE(Tuple_Iterator, "Tuple.Iterator.next");
	if(!data->has_next()) {
		throw_EmptyIterator(ehi);
	}
	return data->next();
}
