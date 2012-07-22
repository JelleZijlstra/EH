#include "eh.h"
#include "eh_libclasses.h"

/*
 * ehretval_t
 */
void ehretval_t::print() {
	switch(this->type()) {
		case string_e:
			printf("%s", this->get_stringval());
			break;
		case int_e:
			printf("%d", this->get_intval());
			break;
		case bool_e:
			if(this->get_boolval()) {
				printf("(true)");
			} else {
				printf("(false)");
			}
			break;
		case null_e:
			printf("(null)");
			break;
		case float_e:
			printf("%f", this->get_floatval());
			break;
		case range_e:
			this->get_rangeval()->min->print();
			printf(" to ");
			this->get_rangeval()->max->print();
			break;
		default:
			eh_error_type("echo operator", this->type(), enotice_e);
			break;
	}
	return;
}
std::list<ehretval_p> ehretval_t::children() {
	std::list<ehretval_p> out;
	switch(this->type()) {
		case object_e: {
			ehobj_t *o = this->get_objectval();
			OBJECT_FOR_EACH(o, i) {
				out.push_back(i->second->value);
			}
			out.push_back(o->parent);
			if(o->real_parent != NULL) {
				out.push_back(o->real_parent);
			}
			break;
		}
		case weak_object_e:
			// ignored for GC purposes
			break;
		case func_e:
			break;
		case array_e:
			ARRAY_FOR_EACH_INT(this->get_arrayval(), i) {
				out.push_back(i->second);
			}
			ARRAY_FOR_EACH_STRING(this->get_arrayval(), i) {
				out.push_back(i->second);
			}
			break;
		case range_e:
			out.push_back(this->get_rangeval()->min);
			out.push_back(this->get_rangeval()->max);
			break;
		case binding_e:
			out.push_back(this->get_bindingval()->method);
			out.push_back(this->get_bindingval()->object_data);
			break;
		case hash_e: {
			ehhash_t *f = this->get_hashval();
			HASH_FOR_EACH(f, i) {
				out.push_back(i->second);
			}
		}
		default:
			// no components
			break;
	}
	return out;
}
bool ehretval_t::equals(ehretval_p rhs) {
	if(this->type() != rhs->type()) {
		return false;
	}
	switch(this->type()) {
		case int_e:
			return (this->get_intval() == rhs->get_intval());
		case string_e:
			return strcmp(this->get_stringval(), rhs->get_stringval()) == 0;
		case bool_e:
			return (this->get_boolval() == rhs->get_boolval());
		case null_e:
			// null always equals null
			return true;
		case float_e:
			return (this->get_floatval() == rhs->get_floatval());
		case range_e:
			return this->get_rangeval()->min->equals(rhs->get_rangeval()->min)
				&& this->get_rangeval()->max->equals(rhs->get_rangeval()->max);
		case resource_e:
			return this->get_resourceval() == rhs->get_resourceval();
		default:
			// TODO: array comparison
			return false;
	}
}
bool ehretval_t::is_a(int in) {
	if(this->type() == in) {
		return true;
	} else {
		return this->type() == object_e && this->get_objectval()->type_id == in;
	}
}
ehretval_t::~ehretval_t() {
	switch(_type) {
		// Simple types; nothing to do
		case int_e:
		case bool_e:
		case float_e:
		case type_e:
		case null_e:
		case attribute_e:
		case attributestr_e:
		case base_object_e:
			break;
		case op_e:
			delete this->opval;
			break;
		case string_e:
			delete[] this->stringval;
			break;
		// Delete object. An ehretval_t owns the object pointed to.
		case range_e:
			delete this->rangeval;
			break;
		case object_e:
			delete this->objectval;
			break;
		case func_e:
			delete this->funcval;
			break;
		case array_e:
			delete this->arrayval;
			break;
		case weak_object_e:
			// we don't own the object
			break;
		case resource_e:
			delete this->resourceval;
			break;
		case binding_e:
			delete this->bindingval;
			break;
		case hash_e:
			delete this->hashval;
			break;
	}
}

/*
 * eharray_t
 */
ehretval_p &eharray_t::operator[](ehretval_p index) {
	switch(index->type()) {
		case int_e:
			return int_indices[index->get_intval()];
		case string_e:
			return string_indices[index->get_stringval()];
		default:
			eh_error_type("array index", index->type(), enotice_e);
			throw unknown_value_exception();
	}
}
void eharray_t::insert_retval(ehretval_p index, ehretval_p value) {
	// Inserts a member into an array.
	switch(index->type()) {
		case int_e:
			this->int_indices[index->get_intval()] = value;
			break;
		case string_e:
			this->string_indices[index->get_stringval()] = value;
			break;
		default:
			eh_error_type("array index", index->type(), enotice_e);
			break;
	}
}
/*
 * ehobj_t
 */
ehmember_p ehobj_t::insert_retval(const char *name, attributes_t attribute, ehretval_p value) {
	if(this->has(name)) {
		// We may want to re-enable this check once inheritance is revamped
		//eh_error("object member already set", enotice_e);
		//return NULL;
		if(members[name]->isconst()) {
			eh_error("Attempt to write to constant variable", eerror_e);
			return NULL;
		}
	}
	// insert a member into a class
	ehmember_p member;
	member->attribute = attribute;
	member->value = value;

	// insert into object
	members[name] = member;
	return member;
}
ehmember_p ehobj_t::get(const char *name, const ehcontext_t context, int token) {
	if(this->has(name)) {
		ehmember_p out = this->members[name];
		if(out->attribute.visibility == private_e && !this->context_compare(context)) {
			eh_error_unknown("object member", name, enotice_e);
			return NULL;
		} else if(token == T_LVALUE_SET && out->attribute.isconst == const_e) {
			eh_error("Attempt to write to constant variable", eerror_e);
			return NULL;
		} else {
			return out;
		}
	} else if(token == T_LVALUE_SET) {
		ehmember_p member;
		// force pointer target to be created
		member->isstatic();
		this->insert(name, member);
		return this->members[name];
	} else {
		// HACK until all objects inherit from Object: ignore absent initializer
		if(strcmp(name, "initialize") != 0) {
			eh_error_unknown("object member", name, enotice_e);
		}
		return NULL;
	}
}
ehmember_p ehobj_t::get_recursive(const char *name, ehcontext_t context, int token) {
	ehmember_p currvar = this->get_recursive_helper(name, context);
	if(token == T_LVALUE_SET) {
		if(currvar == NULL) {
			if(!this->has(name)) {
				ehmember_p newvar;
				// force pointer target to be created
				newvar->isstatic();
				this->insert(name, newvar);
				return this->members[name];
			} else {
				throw unknown_value_exception();
			}
		} else if(currvar->attribute.isconst == const_e) {
			eh_error("Attempt to write to constant variable", eerror_e);
			throw unknown_value_exception();
		}
	}
	return currvar;
}
ehmember_p ehobj_t::get_recursive_helper(const char *name, const ehcontext_t context) {
	if(this->has(name)) {
		return this->members[name];
	}
	if(this->real_parent == NULL) {
		if(this->parent != NULL) {
			return this->get_parent()->get_recursive_helper(name, context);
		} else {
			return NULL;
		}
	} else {
		if(this->parent != NULL && this->get_parent()->has(name)) {
			return this->get_parent()->members[name];
		} else {
			return this->get_real_parent()->get_recursive_helper(name, context);
		}
	}
}
void ehobj_t::copy_member(obj_iterator &classmember, bool set_real_parent, ehretval_p ret, EHI *ehi) {
	ehmember_p newmember;
	if(classmember->first.compare("this") == 0) {
		// handle $this pointer
		newmember->attribute = classmember->second->attribute;
		newmember->value = ehi->make_weak_object(this);
	} else if(classmember->first.compare("scope") == 0) {
		// handle $scope pointer
		newmember->attribute = classmember->second->attribute;
		newmember->value = ret;
	} else if(classmember->second->isstatic() || (classmember->second->isconst() && !classmember->second->value->is_a(func_e))) {
		// we can safely share static members, as well as const members that are not functions
		newmember = classmember->second;
	} else {
		newmember->attribute = classmember->second->attribute;
		if(classmember->second->value->is_a(func_e)) {
			ehobj_t *oldobj = classmember->second->value->get_objectval();
			ehobj_t *obj = new ehobj_t();
			obj->type_id = func_e;
			obj->object_data = oldobj->object_data;
			newmember->value = ehi->make_object(obj);
			obj->parent = ret;
			if(set_real_parent && oldobj->real_parent == NULL) {
				obj->real_parent = oldobj->get_parent()->parent;
			} else {
				obj->real_parent = oldobj->real_parent;
			}
		} else {
			newmember->value = classmember->second->value;
		}
	}
	this->members[classmember->first] = newmember;
}
bool ehobj_t::context_compare(const ehcontext_t key) const {
	// in global context, we never have access to private stuff
	if(ehretval_p::null(key) || key->get_objectval()->get_parent() == NULL) {
		return false;
	} else {
		if(this->type_id == key->get_objectval()->type_id) {
			return true;
		} else {
			return this->context_compare(key->get_objectval()->parent);
		}
	}
}
ehobj_t::~ehobj_t() {
	// Commenting out for now until I figure out how to get it working.
	//ehi->call_method_obj(this, "finalize", 0, NULL, NULL);
}
// set with default attributes
void ehobj_t::set(const char *name, ehretval_p value) {
	if(this->has(name)) {
		ehmember_p member = this->members[name];
		if(member->attribute.isconst == const_e) {
			eh_error("Attempt to write to constant variable", eerror_e);
		} else {
			this->members[name]->value = value;
		}
	} else {
		ehmember_p newmember;
		newmember->attribute = attributes_t::make(public_e, nonstatic_e, nonconst_e);
		newmember->value = value;
		this->members[name] = newmember;
	}
}
