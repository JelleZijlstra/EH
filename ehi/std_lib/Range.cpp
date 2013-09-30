#include "Range.hpp"

#include "ArgumentError.hpp"
#include "Array.hpp"
#include "EmptyIterator.hpp"

ehval_p Range::make(ehval_p min, ehval_p max, EHInterpreter *parent) {
	t *r = new t(min, max);
	return parent->allocate<Range>(r);
}

EH_INITIALIZER(Range) {
	REGISTER_CONSTRUCTOR(Range);
	REGISTER_METHOD(Range, min);
	REGISTER_METHOD(Range, max);
	REGISTER_METHOD_RENAME(Range, operator_arrow, "operator->");
	REGISTER_METHOD(Range, toString);
	REGISTER_METHOD(Range, toArray);
	REGISTER_METHOD(Range, toRange);
	REGISTER_METHOD(Range, compare);
	REGISTER_METHOD(Range, getIterator);
	REGISTER_CLASS(Range, Iterator);
}

EH_METHOD(Range, operator_colon) {
	return ehi->call_method_typed<Range>(args, "toRange", nullptr, obj);
}
EH_METHOD(Range, min) {
	ASSERT_NULL_AND_TYPE(Range, "Range.min");
	return obj->get<Range>()->min;
}
EH_METHOD(Range, max) {
	ASSERT_NULL_AND_TYPE(Range, "Range.max");
	return obj->get<Range>()->max;
}
EH_METHOD(Range, operator_arrow) {
	ASSERT_OBJ_TYPE(Range, "Range.operator->");
	args->assert_type<Integer>("Range.operator->", ehi);
	int index = args->get<Integer>();
	if(index == 0) {
		return obj->get<Range>()->min;
	} else if(index == 1) {
		return obj->get<Range>()->max;
	} else {
		throw_ArgumentError_out_of_range("Range.operator->", args, ehi);
		return nullptr;
	}
}
EH_METHOD(Range, toString) {
	ASSERT_NULL_AND_TYPE(Range, "Range.toString");
	Range::t *range = obj->get<Range>();
	ehval_p str1 = ehi->toString(range->min, obj);
	ehval_p str2 = ehi->toString(range->max, obj);
	size_t len1 = strlen(str1->get<String>());
	size_t len2 = strlen(str2->get<String>());
	size_t len = len1 + 4 + len2 + 1;
	char *out = new char[len]();
	strncpy(out, str1->get<String>(), len1);
	strncpy(out + len1, " to ", 4);
	strncpy(out + len1 + 4, str2->get<String>(), len2);
	out[len1 + 4 + len2] = '\0';
	return String::make(out);
}
EH_METHOD(Range, toArray) {
	ASSERT_NULL_AND_TYPE(Range, "Range.toArray");
	ehval_p out = Array::make(ehi->get_parent(), 2);
	Array::t *arr = out->get<Array>();
	arr->insert(0, obj->get<Range>()->min);
	arr->insert(1, obj->get<Range>()->max);
	return out;
}
EH_METHOD(Range, toRange) {
	ASSERT_NULL_AND_TYPE(Range, "Range.toRange");
	return obj;
}
EH_METHOD(Range, compare) {
	ASSERT_OBJ_TYPE(Range, "Range.compare");
	args->assert_type<Range>("Range.compare", ehi);
	ehval_p lhs_min = obj->get<Range>()->min;
	ehval_p rhs_min = args->get<Range>()->min;
	if(!lhs_min->equal_type(rhs_min)) {
		return Integer::make(intcmp(lhs_min->get_type_id(ehi->get_parent()), rhs_min->get_type_id(ehi->get_parent())));
	}
	/*
	 * This method returns:
	 * -1 if lhs.min < rhs.min and lhs.max < rhs.max or lhs is fully inside rhs
	 *    (lhs.min > rhs.min, lhs.max < rhs.max),
	 * 0 if lhs.min == rhs.min and lhs.max == rhs.max,
	 * 1 if lhs.max > rhs.max, lhs.min > rhs.min or lhs.min < rhs.min,
	 *    lhs.max > rhs.max
	 */
	ehval_p lhs_max = obj->get<Range>()->max;
	ehval_p rhs_max = args->get<Range>()->max;
	ehval_p min_cmp_p = ehi->call_method_typed<Integer>(lhs_min, "compare", rhs_min, obj);
	int min_cmp = min_cmp_p->get<Integer>();
	ehval_p max_cmp_p = ehi->call_method_typed<Integer>(lhs_max, "compare", rhs_max, obj);
	int max_cmp = max_cmp_p->get<Integer>();
	if(min_cmp == 0) {
		return max_cmp_p;
	} else if(max_cmp == 0) {
		return min_cmp_p;
	} else if((min_cmp < 0 && max_cmp < 0) || (min_cmp > 0 && max_cmp < 0)) {
		return Integer::make(-1);
	} else {
		return Integer::make(1);
	}
}

EH_METHOD(Range, getIterator) {
	ASSERT_NULL_AND_TYPE(Range, "Range.getIterator");
	ehval_p class_member = obj->get_type_object(ehi->get_parent())->get_property("Iterator", obj, ehi);
	return ehi->call_method(class_member, "new", obj, obj);
}

EH_INITIALIZER(Range_Iterator) {
	REGISTER_CONSTRUCTOR(Range_Iterator);
	REGISTER_METHOD(Range_Iterator, hasNext);
	REGISTER_METHOD(Range_Iterator, next);
}

ehval_p Range_Iterator::make(ehval_p range, EHInterpreter *parent) {
	return parent->allocate<Range_Iterator>(new t(range));
}

bool Range_Iterator::t::has_next(EHI *ehi) {
	ehval_p max = this->range->get<Range>()->max;
	ehval_p result = ehi->call_method_typed<Bool>(this->current, "operator<=", max, ehi->global());
	return result->get<Bool>();
}
ehval_p Range_Iterator::t::next(EHI *ehi) {
	assert(this->has_next(ehi));
	ehval_p out = this->current;
	this->current = ehi->call_method(out, "operator+", Integer::make(1), ehi->global());
	return out;
}

EH_METHOD(Range_Iterator, operator_colon) {
	args->assert_type<Range>("Range.Iterator()", ehi);
	return Range_Iterator::make(args, ehi->get_parent());
}
EH_METHOD(Range_Iterator, hasNext) {
	args->assert_type<Null>("Range.Iterator.hasNext", ehi);
	ASSERT_RESOURCE(Range_Iterator, "Range.Iterator.hasNext");
	return Bool::make(data->has_next(ehi));
}
EH_METHOD(Range_Iterator, next) {
	args->assert_type<Null>("Range.Iterator.next", ehi);
	ASSERT_RESOURCE(Range_Iterator, "Range.Iterator.next");
	if(!data->has_next(ehi)) {
		throw_EmptyIterator(ehi);
	}
	return data->next(ehi);
}
