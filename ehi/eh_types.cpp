#include "eh.h"
#include "eh_libclasses.h"

/*
 * ehretval_t
 */
void ehretval_t::print() {
	std::cout << get_typestring(this->type()) << std::endl;
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
			printf("(cannot print value)");
			break;
	}
	std::cout << std::endl;
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
			break;
		}
		case tuple_e: {
			ehtuple_t *t = this->get_tupleval();
			int size = t->size();
			for(int i = 0; i < size; i++) {
				out.push_back(t->get(i));
			}
			break;
		}
		case super_class_e:
			out.push_back(this->get_super_classval()->content());
			break;
		default:
			// nothing to see here
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
int ehretval_t::get_full_type() const  {
	int out = this->type();
	if(out == object_e) {
		return this->get_objectval()->type_id;
	} else {
		return out;
	}
}
const std::string &ehretval_t::type_string(EHI *ehi) const {
	int type = this->get_full_type();
	return ehi->repo.get_name(type);
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
			break;
		case super_class_e:
			delete this->super_classval;
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
		case resource_e:
			delete this->resourceval;
			break;
		case binding_e:
			delete this->bindingval;
			break;
		case hash_e:
			delete this->hashval;
			break;
		case tuple_e:
		 	delete this->tupleval;
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
			// callers should make sure type is right
			assert(false);
			throw "impossible";
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
			// callers should make sure type is right
			assert(false);
	}
}
/*
 * ehobj_t
 */
ehmember_p ehobj_t::get_recursive(const char *name, const ehcontext_t context) {
	if(this->has(name)) {
		return this->members[name];
	} else if(this->real_parent == NULL) {
		if(this->parent != NULL) {
			return this->get_parent()->get_recursive(name, context);
		} else {
			return NULL;
		}
	} else {
		if(this->parent != NULL && this->get_parent()->has(name)) {
			return this->get_parent()->members[name];
		} else {
			return this->get_real_parent()->get_recursive(name, context);
		}
	}
}
bool ehobj_t::inherited_has(const std::string &key) const {
	if(this->has(key)) {
		return true;
	}
	for(std::list<ehretval_p>::const_iterator i = super.begin(), end = super.end(); i != end; i++) {
		if((*i)->get_objectval()->inherited_has(key)) {
			return true;
		}
	}
	return false;
}
ehmember_p ehobj_t::inherited_get(const std::string &key) {
	if(this->has(key)) {
		return this->get_known(key);
	}
	for(std::list<ehretval_p>::const_iterator i = super.begin(), end = super.end(); i != end; i++) {
		ehmember_p result = (*i)->get_objectval()->inherited_get(key);
		if(!ehmember_p::null(result)) {
			return result;
		}
	}
	return NULL;
}
bool ehobj_t::context_compare(const ehcontext_t key) const {
	// in global context, we never have access to private stuff
	if(ehretval_p::null(key.object) || key.object->get_objectval()->get_parent() == NULL) {
		return false;
	} else {
		if(this->type_id == key.object->get_objectval()->type_id) {
			return true;
		} else {
			return this->context_compare(key.object->get_objectval()->parent);
		}
	}
}
ehobj_t::~ehobj_t() {
	// Commenting out for now until I figure out how to get it working.
	//ehi->call_method_obj(this, "finalize", 0, NULL, NULL);
}
