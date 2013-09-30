#include <set>

#include "Array.hpp"
#include "Object.hpp"
#include "SuperClass.hpp"

EH_INITIALIZER(Object) {
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

EH_METHOD(Object, initialize) {
	return nullptr;
}
EH_METHOD(Object, toString) {
	args->assert_type<Null>("Object.toString", ehi);
	const size_t len = sizeof(void *) * 2 + 3;
	char *out = new char[len]();
	snprintf(out, len, "%p", reinterpret_cast<void *>(obj.operator->()));
	return String::make(out);
}
EH_METHOD(Object, finalize) {
	return nullptr;
}
EH_METHOD(Object, isA) {
	args->assert_type<Class>("Object.isA", ehi);
	ehval_p _obj = obj;
	ehval_p type_object = obj->get_type_object(ehi->get_parent());
	if(type_object == args) {
		return Bool::make(true);
	} else {
		return Bool::make(type_object->get<Class>()->inherits(args, ehi->get_parent()));
	}
}

EH_METHOD(Object, operator_compare) {
	unsigned int lhs_type = obj->get_type_id(ehi->get_parent());
	unsigned int rhs_type = args->get_type_id(ehi->get_parent());
	int comparison = intcmp(lhs_type, rhs_type);
	if(comparison != 0) {
		return Integer::make(comparison);
	} else {
		return ehi->call_method(obj, "compare", args, obj);
	}
}
EH_METHOD(Object, compare) {
	ehval_p lhs = obj;
	if(lhs->is_a<Null>()) {
		lhs = obj;
	}
	ehval_p rhs = args;
	if(!lhs->equal_type(rhs)) {
		throw_TypeError("Arguments to Object.compare must have the same type", rhs, ehi);
	}
	return Integer::make(lhs->naive_compare(rhs));
}
#define CALL_COMPARE() \
	ehval_p comparison_p = ehi->call_method_typed<Integer>(obj, "operator<=>", args, obj); \
	int comparison = comparison_p->get<Integer>();

EH_METHOD(Object, operator_equals) {
	CALL_COMPARE();
	return Bool::make(comparison == 0);
}
EH_METHOD(Object, operator_ne) {
	CALL_COMPARE();
	return Bool::make(comparison != 0);
}
EH_METHOD(Object, operator_gt) {
	CALL_COMPARE();
	return Bool::make(comparison > 0);
}
EH_METHOD(Object, operator_gte) {
	CALL_COMPARE();
	return Bool::make(comparison >= 0);
}
EH_METHOD(Object, operator_lt) {
	CALL_COMPARE();
	return Bool::make(comparison < 0);
}
EH_METHOD(Object, operator_lte) {
	CALL_COMPARE();
	return Bool::make(comparison <= 0);
}
EH_METHOD(Object, type) {
	unsigned int type_id = obj->get_type_id(ehi->get_parent());
	return ehi->get_parent()->repo.get_object(type_id);
}
EH_METHOD(Object, typeId) {
	return Integer::make(static_cast<Integer::type>(obj->get_type_id(ehi->get_parent())));
}
// return all members in the class
EH_METHOD(Object, members) {
	// TODO
	// ehval_p reference_retainer = obj;
	// obj = obj->get_underlying_object(ehi->get_parent());
	// std::set<std::string> members = obj->get<Object>()->member_set(ehi->get_parent());
	ehval_p out = Array::make(ehi->get_parent());
	// Array::t *arr = out->get<Array>();
	// for(std::set<std::string>::iterator i = members.begin(), end = members.end(); i != end; i++) {
	// 	arr->append(String::make(strdup((*i).c_str())));
	// }
	return out;
}
