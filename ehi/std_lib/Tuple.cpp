#include "Tuple.h"

START_EHLC(Tuple)
	EHLC_ENTRY(Tuple, initialize)
	EHLC_ENTRY_RENAME(Tuple, operator_arrow, "operator->")
	EHLC_ENTRY(Tuple, length)
	EHLC_ENTRY(Tuple, toTuple)
	EHLC_ENTRY(Tuple, getIterator)
	REGISTER_CLASS(Tuple, Iterator);
END_EHLC()

EH_METHOD(Tuple, initialize) {
	return ehi->to_tuple(args, obj);
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

START_EHLC(Tuple_Iterator)
	EHLC_ENTRY(Tuple_Iterator, initialize)
	EHLC_ENTRY(Tuple_Iterator, hasNext)
	EHLC_ENTRY(Tuple_Iterator, next)
END_EHLC()

EH_METHOD(Tuple_Iterator, initialize) {
	ASSERT_TYPE(args, tuple_e, "Tuple.Iterator.initialize");
	Tuple_Iterator *it = new Tuple_Iterator(args);
	return ehretval_t::make_resource(static_cast<LibraryBaseClass *>(it));
}
EH_METHOD(Tuple_Iterator, hasNext) {
	ASSERT_TYPE(args, null_e, "Tuple.Iterator.hasNext");
	Tuple_Iterator *data = static_cast<Tuple_Iterator *>(obj->get_resourceval());
	return ehretval_t::make_bool(data->has_next());
}
EH_METHOD(Tuple_Iterator, next) {
	ASSERT_TYPE(args, null_e, "Tuple.Iterator.next");
	Tuple_Iterator *data = static_cast<Tuple_Iterator *>(obj->get_resourceval());
	if(!data->has_next()) {
		throw_EmptyIterator(ehi);
	}
	return data->next();
}
