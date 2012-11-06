#include <set>

#include "Object.h"
#include "SuperClass.h"

EH_INITIALIZER(Object) {
	REGISTER_METHOD(Object, new);
	REGISTER_METHOD(Object, inherit);
	REGISTER_METHOD(Object, initialize);
	REGISTER_METHOD(Object, toString);
	REGISTER_METHOD(Object, finalize);
	REGISTER_METHOD(Object, isA);
	REGISTER_METHOD_RENAME(Object, operator_compare, "operator<=>");
	REGISTER_METHOD(Object, compare);
	REGISTER_METHOD_RENAME(Object, operator_equals, "operator==");
	REGISTER_METHOD_RENAME(Object, operator_ne, "operator!=");
	REGISTER_METHOD_RENAME(Object, operator_gt, "operator>");
	REGISTER_METHOD_RENAME(Object, operator_gte, "operator>=");
	REGISTER_METHOD_RENAME(Object, operator_lt, "operator<");
	REGISTER_METHOD_RENAME(Object, operator_lte, "operator<=");
	REGISTER_METHOD(Object, type);
	REGISTER_METHOD(Object, typeId);
	REGISTER_METHOD(Object, members);
//	REGISTER_METHOD(Object, data);
//	REGISTER_METHOD(Object, setData);
}

EH_METHOD(Object, new) {
	ehretval_p ret = ehi->get_parent()->instantiate(obj);
	ret->get_objectval()->object_data = ehi->call_method(ret, "initialize", args, obj);
	return ret;
}
EH_METHOD(Object, inherit) {
  	ASSERT_TYPE(args, object_e, "Object.inherit");
  	obj->get_objectval()->inherit(args);
  	return ehi->get_parent()->make_super_class(new ehsuper_t(args));
}
EH_METHOD(Object, initialize) {
	return NULL;
}
EH_METHOD(Object, toString) {
	ASSERT_TYPE(args, null_e, "Object.toString");
	const size_t len = sizeof(void *) * 2 + 3;
	char *out = new char[len]();
	snprintf(out, len, "%p", reinterpret_cast<void *>(obj.operator->()));
	return ehretval_t::make_string(out);
}
EH_METHOD(Object, finalize) {
	return NULL;
}
EH_METHOD(Object, isA) {
  int type = args->type();
  if(type == object_e) {
    type = args->get_objectval()->type_id;
  }
  return ehretval_t::make_bool(obj->inherited_is_a(type));
}

static ehretval_p get_data(ehretval_p in) {
	if(in->is_object()) {
		return in->get_objectval()->object_data;
	} else {
		return in;
	}

}
EH_METHOD(Object, operator_compare) {
	int lhs_type = obj->get_full_type();
	int rhs_type = args->get_full_type();
	int comparison = intcmp(lhs_type, rhs_type);
	if(comparison != 0) {
		return ehretval_t::make_int(comparison);
	} else {
		return ehi->call_method(obj, "compare", args, obj);
	}
}
EH_METHOD(Object, compare) {
	int lhs_type = obj->get_full_type();
	int rhs_type = args->get_full_type();
	if(lhs_type != rhs_type) {
		throw_TypeError("Arguments to Object.compare must have the same type", rhs_type, ehi);
	}
	ehretval_p lhs = get_data(obj);
	if(lhs->type() == null_e) {
		lhs = obj;
	}
	ehretval_p rhs = get_data(args);
	return ehretval_t::make_int(lhs->naive_compare(rhs));	
}
#define CALL_COMPARE() \
	ehretval_p comparison_p = ehi->call_method(obj, "operator<=>", args, obj); \
	if(comparison_p->type() != int_e) { \
		throw_TypeError("operator<=> must return an Integer", comparison_p->type(), ehi); \
	} \
	int comparison = comparison_p->get_intval();

EH_METHOD(Object, operator_equals) {
	CALL_COMPARE();
	return ehretval_t::make_bool(comparison == 0);
}
EH_METHOD(Object, operator_ne) {
	CALL_COMPARE();
	return ehretval_t::make_bool(comparison != 0);
}
EH_METHOD(Object, operator_gt) {
	CALL_COMPARE();
	return ehretval_t::make_bool(comparison > 0);
}
EH_METHOD(Object, operator_gte) {
	CALL_COMPARE();
	return ehretval_t::make_bool(comparison >= 0);
}
EH_METHOD(Object, operator_lt) {
	CALL_COMPARE();
	return ehretval_t::make_bool(comparison < 0);
}
EH_METHOD(Object, operator_lte) {
	CALL_COMPARE();
	return ehretval_t::make_bool(comparison <= 0);
}
EH_METHOD(Object, type) {
	int type = obj->get_full_type();
	std::string name = ehi->get_parent()->repo.get_name(type);
	return ehretval_t::make_string(strdup(name.c_str()));
}
EH_METHOD(Object, typeId) {
	return ehretval_t::make_int(obj->get_full_type());
}
// return all members in the class
EH_METHOD(Object, members) {
	ehretval_p reference_retainer = obj;
	if(obj->type() != object_e) {
		obj = ehi->get_parent()->get_primitive_class(obj->type());
	}
	std::set<std::string> members = obj->get_objectval()->member_set();
	eharray_t *out = new eharray_t();
	int index = 0;
	for(std::set<std::string>::iterator i = members.begin(), end = members.end(); i != end; i++, index++) {
		out->int_indices[index] = ehretval_t::make_string(strdup((*i).c_str()));
	}
	return ehi->get_parent()->make_array(out);
}

// TODO: make these private methods. I'm pretty sure you can crash ehi with these.
EH_METHOD(Object, data) {
	if(obj->type() == object_e) {
		return obj->get_objectval()->object_data;
	} else {
		return obj;
	}
}

EH_METHOD(Object, setData) {
	// impossible to set object_data on primitive
	if(!obj->type() == object_e) {
		throw_TypeError("setData", obj->type(), ehi);
	}
	obj->get_objectval()->object_data = args;
	// enable chaining
	return obj;
}
