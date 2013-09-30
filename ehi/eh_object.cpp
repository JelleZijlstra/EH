
// TODO: fix this
// std::set<std::string> ehobj_t::member_set(const EHInterpreter *interpreter_parent) const {
//     std::set<std::string> out;
//     for(auto &i : this->members) {
//         out.insert(i.first);
//     }
//     for(auto &super_class : super) {
//         auto member_set = super_class->get<Object>()->member_set(nullptr);
//         out.insert(member_set.begin(), member_set.end());
//     }
//     if(interpreter_parent != nullptr) {
//         ehobj_t *base_object = interpreter_parent->base_object->get<Object>();
//         auto object_set = base_object->member_set(nullptr);
//         out.insert(object_set.begin(), object_set.end());
//     }
//     return out;
// }

// bool ehobj_t::context_compare(const ehcontext_t &key, const class EHI *ehi) const {
//     // in global context, we never have access to private stuff
//     if(key.object->is_a<Null>()) {
//         return false;
//     } else if(key.scope->get<Object>() == this) {
//         return true;
//     } else if(key.object->is_a<Object>()) {
//         // this may fail when the key.object is not an Object (i.e., )
//         ehobj_t *key_obj = key.object->get<Object>();
//         return (type_id == key_obj->type_id) || this->context_compare(ehcontext_t(key_obj->parent, key_obj->parent), ehi);
//     } else {
//         const unsigned int key_id = ehi->get_parent()->repo.get_type_id(key.object);
//         return type_id == key_id;
//     }
// }
