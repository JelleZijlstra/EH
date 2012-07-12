#include "eh.h"
#include "eh_error.h"

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
			printf("%d to %d", this->get_rangeval()->min, this->get_rangeval()->max);
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
		case func_e: {
			ehobj_t *f = this->get_funcval();
			OBJECT_FOR_EACH(f, i) {
				out.push_back(i->second->value);
			}
			out.push_back(f->parent);
			if(f->real_parent != NULL) {
				out.push_back(f->real_parent);
			}
			break;
		}
		case array_e:
			ARRAY_FOR_EACH_INT(this->get_arrayval(), i) {
				out.push_back(i->second);
			}
			ARRAY_FOR_EACH_STRING(this->get_arrayval(), i) {
				out.push_back(i->second);
			}
			break;
		default:
			// no components
			break;
	}
	return out;
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
		eh_error("object member already set", enotice_e);
		return NULL;
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
	} else if(classmember->second->isstatic() || (classmember->second->isconst() && classmember->second->value->type() != func_e)) {
		// we can safely share static members, as well as const members that are not functions
		newmember = classmember->second;
	} else {
		newmember->attribute = classmember->second->attribute;
		if(classmember->second->value->type() == func_e) {
			ehobj_t *oldobj = classmember->second->value->get_funcval();
			ehobj_t *f = new ehobj_t(oldobj->classname);
			newmember->value = ehi->make_func(f);
			f->parent = ret;
			if(set_real_parent && oldobj->real_parent == NULL) {
				f->real_parent = oldobj->get_parent()->parent;
			} else {
				f->real_parent = oldobj->real_parent;
			}
			f->function = oldobj->function;
			f->members = oldobj->members;
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
		if(this->classname.compare(key->get_objectval()->classname) == 0) {
			return true;
		} else {
			return this->context_compare(key->get_objectval()->parent);
		}
	}
}
