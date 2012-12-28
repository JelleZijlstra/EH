#include "Tuple.hpp"

EH_INITIALIZER(Tuple) {
	REGISTER_METHOD(Tuple, initialize);
	REGISTER_METHOD_RENAME(Tuple, operator_arrow, "operator->");
	REGISTER_METHOD(Tuple, length);
	REGISTER_METHOD(Tuple, toTuple);
	REGISTER_METHOD(Tuple, getIterator);
	REGISTER_CLASS(Tuple, Iterator);
	REGISTER_METHOD(Tuple, compare);
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
	ehretval_p length = ehi->call_method(args, "length", NULL, obj);
	ASSERT_TYPE(length, int_e, "length method must return an int");
	const int len = length->get_intval();

	ehretval_a values(len);
	ehtuple_t *t = new ehtuple_t(len, values);

	ehretval_p iterator = ehi->call_method(args, "getIterator", NULL, obj);
	for(int i = 0; i < len; i++) {
		ehretval_p next = ehi->call_method(iterator, "next", NULL, obj);
		t->content[i] = next;
	}
	return ehi->get_parent()->make_tuple(t);
}
EH_METHOD(Tuple, operator_arrow) {
	ASSERT_OBJ_TYPE(tuple_e, "Tuple.operator->");
	ASSERT_TYPE(args, int_e, "Tuple.operator->");
	int index = args->get_intval();
	if(index < 0 || index >= obj->get_tupleval()->size()) {
    	throw_ArgumentError_out_of_range("Tuple.operator->", args, ehi);
	}
  	return obj->get_tupleval()->get(index);
}
EH_METHOD(Tuple, length) {
  	ASSERT_NULL_AND_TYPE(tuple_e, "Tuple.length");
  	return ehretval_t::make_int(obj->get_tupleval()->size());
}
EH_METHOD(Tuple, toTuple) {
	ASSERT_OBJ_TYPE(tuple_e, "Tuple.toTuple");
	return obj;
}
EH_METHOD(Tuple, compare) {
	ASSERT_OBJ_TYPE(tuple_e, "Tuple.compare");
	ASSERT_TYPE(args, tuple_e, "Tuple.compare");
	ehtuple_t *lhs = obj->get_tupleval();
	ehtuple_t *rhs = args->get_tupleval();
	int size_cmp = intcmp(lhs->size(), rhs->size());
	if(size_cmp != 0) {
		return ehretval_t::make_int(size_cmp);
	}
	int size = lhs->size();
	ehretval_p lhs_val, rhs_val;
	for(int i = 0; i < size; i++) {
		int comparison = ehi->compare(lhs->get(i), rhs->get(i), obj);
		if(comparison != 0) {
			return ehretval_t::make_int(comparison);
		}
	}
	return ehretval_t::make_int(0);
}
EH_METHOD(Tuple, getIterator) {
	ASSERT_NULL_AND_TYPE(tuple_e, "Tuple.getIterator");
	ehretval_p class_member = ehi->get_property(obj, "Iterator", obj);
	return ehi->call_method(class_member, "new", obj, obj);
}

bool Tuple_Iterator::has_next() {
	ehtuple_t *the_tuple = this->tuple->get_tupleval();
	return this->position < the_tuple->size();
}
ehretval_p Tuple_Iterator::next() {
	ehtuple_t *the_tuple = this->tuple->get_tupleval();
	return the_tuple->get(this->position++);
}

EH_INITIALIZER(Tuple_Iterator) {
	REGISTER_METHOD(Tuple_Iterator, initialize);
	REGISTER_METHOD(Tuple_Iterator, hasNext);
	REGISTER_METHOD(Tuple_Iterator, next);
}

EH_METHOD(Tuple_Iterator, initialize) {
	ASSERT_TYPE(args, tuple_e, "Tuple.Iterator.initialize");
	Tuple_Iterator *it = new Tuple_Iterator(args);
	return ehretval_t::make_resource(obj->get_full_type(), static_cast<LibraryBaseClass *>(it));
}
EH_METHOD(Tuple_Iterator, hasNext) {
	ASSERT_TYPE(args, null_e, "Tuple.Iterator.hasNext");
	ASSERT_RESOURCE(Tuple_Iterator, "Tuple.Iterator.hasNext");
	return ehretval_t::make_bool(data->has_next());
}
EH_METHOD(Tuple_Iterator, next) {
	ASSERT_TYPE(args, null_e, "Tuple.Iterator.next");
	ASSERT_RESOURCE(Tuple_Iterator, "Tuple.Iterator.next");
	if(!data->has_next()) {
		throw_EmptyIterator(ehi);
	}
	return data->next();
}
