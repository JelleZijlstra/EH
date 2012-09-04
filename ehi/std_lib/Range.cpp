#include "Range.h"

START_EHLC(Range)
EHLC_ENTRY(Range, min)
EHLC_ENTRY(Range, max)
EHLC_ENTRY_RENAME(Range, operator_arrow, "operator->")
EHLC_ENTRY(Range, toString)
EHLC_ENTRY(Range, toArray)
EHLC_ENTRY(Range, toRange)
EHLC_ENTRY(Range, compare)
END_EHLC()

EH_METHOD(Range, initialize) {
	return ehi->to_range(args, obj);
}
EH_METHOD(Range, min) {
	ASSERT_NULL_AND_TYPE(range_e, "Range.min");
	return obj->get_rangeval()->min;
}
EH_METHOD(Range, max) {
	ASSERT_NULL_AND_TYPE(range_e, "Range.max");
	return obj->get_rangeval()->max;
}
EH_METHOD(Range, operator_arrow) {
	ASSERT_OBJ_TYPE(range_e, "Range.operator->");
	ASSERT_TYPE(args, int_e, "Range.operator->");
	int index = args->get_intval();
	if(index == 0) {
		return obj->get_rangeval()->min;
	} else if(index == 1) {
		return obj->get_rangeval()->max;
	} else {
		throw_ArgumentError_out_of_range("Range.operator->", args, ehi);
		return NULL;		
	}
}
EH_METHOD(Range, toString) {
	ASSERT_NULL_AND_TYPE(range_e, "Range.toString");
	ehrange_t *range = obj->get_rangeval();
	ehretval_p str1 = ehi->to_string(range->min, obj);
	ehretval_p str2 = ehi->to_string(range->max, obj);
	size_t len1 = strlen(str1->get_stringval());
	size_t len2 = strlen(str2->get_stringval());
	size_t len = len1 + 4 + len2 + 1;
	char *out = new char[len]();
	strncpy(out, str1->get_stringval(), len1);
	strncpy(out + len1, " to ", 4);
	strncpy(out + len1 + 4, str2->get_stringval(), len2);
	out[len1 + 4 + len2] = '\0';
	return ehretval_t::make_string(out);
}
EH_METHOD(Range, toArray) {
	ASSERT_NULL_AND_TYPE(range_e, "Range.toArray");
	ehretval_p array = ehi->make_array(new eharray_t);
	array->get_arrayval()->int_indices[0] = obj->get_rangeval()->min;
	array->get_arrayval()->int_indices[1] = obj->get_rangeval()->max;
	return array;
}
EH_METHOD(Range, toRange) {
	ASSERT_NULL_AND_TYPE(range_e, "Range.toRange");
	return obj;
}
EH_METHOD(Range, compare) {
	ASSERT_OBJ_TYPE(range_e, "Range.compare");
	ASSERT_TYPE(args, range_e, "Range.compare");
	ehretval_p lhs_min = obj->get_rangeval()->min;
	ehretval_p rhs_min = args->get_rangeval()->min;
	int lhs_type = lhs_min->get_full_type();
	int rhs_type = rhs_min->get_full_type();
	if(lhs_type != rhs_type) {
		return ehretval_t::make_int(intcmp(lhs_type, rhs_type));
	}
	// returns -1 if lhs.min < rhs.min and lhs.max < rhs.max or lhs is fully inside rhs (lhs.min > rhs.min, lhs.max < rhs.max),
	// 0 if lhs.min == rhs.min and lhs.max == rhs.max,
	// 1 if lhs.max > rhs.max, lhs.min > rhs.min or lhs.min < rhs.min, lhs.max > rhs.max
	ehretval_p lhs_max = obj->get_rangeval()->max;
	ehretval_p rhs_max = args->get_rangeval()->max;
	ehretval_p min_cmp_p = ehi->call_method(lhs_min, "compare", rhs_min, obj);
	if(min_cmp_p->type() != int_e) {
		throw_TypeError("compare must return an Integer", min_cmp_p->type(), ehi);
	}
	int min_cmp = min_cmp_p->get_intval();
	ehretval_p max_cmp_p = ehi->call_method(lhs_max, "compare", rhs_max, obj);
	if(max_cmp_p->type() != int_e) {
		throw_TypeError("compare must return an Integer", max_cmp_p->type(), ehi);
	}
	int max_cmp = max_cmp_p->get_intval();
	if(min_cmp == 0) {
		return max_cmp_p;
	} else if(max_cmp == 0) {
		return min_cmp_p;
	} else if((min_cmp < 0 && max_cmp < 0) || (min_cmp > 0 && max_cmp < 0)) {
		return ehretval_t::make_int(-1);
	} else {
		return ehretval_t::make_int(1);
	}
}
