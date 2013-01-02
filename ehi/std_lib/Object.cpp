#include <set>

#include "Object.hpp"
#include "SuperClass.hpp"

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
	ehval_p ret = ehi->get_parent()->instantiate(obj);
	ret->get<Object>()->object_data = ehi->call_method(ret, "initialize", args, obj);
	return ret;
}
EH_METHOD(Object, inherit) {
	args->assert_type<Object>("Object.inherit", ehi);
	obj->get<Object>()->inherit(args);
	return SuperClass::make(args, ehi->get_parent());
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
	if(!args->is_a<Object>()) {
		args = ehi->get_parent()->repo.get_object(args);
	}
	type_repository &repo = ehi->get_parent()->repo;
	ehval_p type_object = repo.get_object(obj);
	if(type_object == args) {
		return Bool::make(true);
	} else if(obj->is_a<Object>()) {
		return Bool::make(obj->get<Object>()->inherits(args));
	} else {
		return Bool::make(type_object->get<Object>()->inherits(args));
	}
}

EH_METHOD(Object, operator_compare) {
	int lhs_type = obj->get_type_id(ehi->get_parent());
	int rhs_type = args->get_type_id(ehi->get_parent());
	int comparison = intcmp(lhs_type, rhs_type);
	if(comparison != 0) {
		return Integer::make(comparison);
	} else {
		return ehi->call_method(obj, "compare", args, obj);
	}
}
EH_METHOD(Object, compare) {
	ehval_p lhs = obj->data();
	if(lhs->is_a<Null>()) {
		lhs = obj;
	}
	ehval_p rhs = args->data();
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
	std::string name = ehi->get_parent()->repo.get_name(obj);
	return String::make(strdup(name.c_str()));
}
EH_METHOD(Object, typeId) {
	return Integer::make(obj->get_type_id(ehi->get_parent()));
}
// return all members in the class
EH_METHOD(Object, members) {
	ehval_p reference_retainer = obj;
	if(!obj->is_a<Object>()) {
		obj = ehi->get_parent()->repo.get_object(obj);
	}
	std::set<std::string> members = obj->get<Object>()->member_set();
	ehval_p out = Array::make(ehi->get_parent());
	Array::t *arr = out->get<Array>();
	int index = 0;
	for(std::set<std::string>::iterator i = members.begin(), end = members.end(); i != end; i++, index++) {
		arr->int_indices[index] = String::make(strdup((*i).c_str()));
	}
	return out;
}

// TODO: make these private methods. I'm pretty sure you can crash ehi with these.
EH_METHOD(Object, data) {
	if(obj->is_a<Object>()) {
		return obj->get<Object>()->object_data;
	} else {
		return obj;
	}
}

EH_METHOD(Object, setData) {
	// impossible to set object_data on primitive
	if(!obj->is_a<Object>()) {
		throw_TypeError("setData", obj, ehi);
	}
	obj->get<Object>()->object_data = args;
	// enable chaining
	return obj;
}
