/*
 * EH objects and class objects
 */

typedef std::map<const std::string, ehmember_p> obj_map;

// EH object
class ehobj_t {
public:
    typedef void (*initializer)(class ehclass_t *cls, EHInterpreter *parent);

    obj_map members;
    ehval_w cls;

    ehobj_t() : members(), cls() {}
    virtual ~ehobj_t() {}

    ehobj_t(const ehobj_t&) = delete;
    ehobj_t operator=(const ehobj_t&) = delete;

    // inline methods
    size_t size() const {
        return members.size();
    }

    ehmember_p get_known(const std::string &key) const {
        assert(this->has(key));
        return members.at(key);
    }

    bool has(const std::string &key) const {
        return members.count(key);
    }

    void insert(const std::string &name, ehmember_p value) {
        members[name] = value;
    }

    void register_static_method(const std::string &name, const ehlibmethod_t method, const attributes_t attributes, EHInterpreter *interpreter_parent);
    void register_value(const std::string &name, ehval_p value, const attributes_t attributes);
    unsigned int register_member_class(const char *name, const initializer init_func, const attributes_t attributes, EHInterpreter *interpreter_parent);

    template<class T>
    unsigned int register_member_class(const ehobj_t::initializer init_func, const char *name, const attributes_t attributes, EHInterpreter *interpreter_parent, ehval_p the_class = nullptr);

    ///////

    /*
     * const methods
     */
    std::set<std::string> member_set(const EHInterpreter *interpreter_parent) const;

    // bool has(const char *key) const {
    //     return has(std::string(key));
    // }

    /*
     * non-const methods
     */

    // void insert(const char *name, ehmember_p value) {
    //     const std::string str(name);
    //     this->insert(str, value);
    // }
};

// EH class
class ehclass_t : public ehobj_t {
public:
    obj_map instance_members;
    unsigned int type_id;
    // used for scoping
    ehval_w parent;
    // inherited classes. Invariant: all must be instances of Class
    std::list<ehval_w> super;

    // constructor
    ehclass_t() : ehobj_t(), instance_members(), type_id(0), parent(), super() {}

    ehmember_p get_instance_member_current_object(const char *name, ehcontext_t context, class EHI *ehi);

    void inherit(ehval_p superclass);

    bool inherits(ehval_p superclass, EHInterpreter *parent);

    void register_method(const std::string &name, const ehlibmethod_t method, const attributes_t attributes, EHInterpreter *interpreter_parent);
};

EH_CLASS(Object) {
public:
    typedef ehobj_t *const type;
    type value;

    Object(ehobj_t *val) : value(val) {}

    virtual ~Object() {
        delete value;
    }

    virtual bool belongs_in_gc() const {
        return true;
    }

    virtual std::list<ehval_p> children() const override {
        std::list<ehval_p> out;
        for(auto &kv : value->members) {
            out.push_back(kv.second->value);
        }
        out.push_back(value->cls);
        assert(out.size() == value->members.size() + 1);
        return out;
    }

    virtual void printvar(printvar_set &set, int level, EHI *ehi) override;

    static ehval_p make(ehobj_t *obj, EHInterpreter *parent);

    // template<class T>
    // bool inherits() const {
    //     for(const auto &i : value->super) {
    //         if(i->inherited_is_a<T>()) {
    //             return true;
    //         }
    //     }
    //     return false;
    // }

    unsigned int get_type_id(const class EHInterpreter *parent) override {
        return value->cls->get_type_id(parent);
    }

    virtual ehval_p get_type_object(const EHInterpreter *parent) override {
        return value->cls;
    }

    virtual ehmember_p get_property_current_object(const char *name, ehcontext_t context, class EHI *ehi);

    virtual void set_member_directly(const char *name, ehmember_p value, ehcontext_t context, class EHI *ehi);
};

EH_CLASS(Class) {
public:
    typedef ehclass_t *const type;
    type value;

    Class(type val) : value(val) {}

    virtual std::list<ehval_p> children() const override {
        std::list<ehval_p> out;
        for(auto &kv : value->members) {
            out.push_back(kv.second->value);
        }
        for(auto &kv : value->instance_members) {
            out.push_back(kv.second->value);
        }
        out.push_back(value->parent);
        for(auto &i : value->super) {
            out.push_back(i);
        }
        assert(out.size() == value->members.size() + value->instance_members.size() + 1 + value->super.size());
        return out;
    }

    virtual unsigned int get_type_id(const class EHInterpreter *parent) override {
        return value->type_id;
    }

    virtual ~Class() {
        delete value;
    }

    virtual bool belongs_in_gc() const {
        return true;
    }

    virtual bool can_create_property() {
        // direct property creation is allowed only on a class object
        return true;
    }

    virtual ehmember_p get_property_current_object(const char *name, ehcontext_t context, class EHI *ehi);

    virtual void set_member_directly(const char *name, ehmember_p value, ehcontext_t context, class EHI *ehi);

    virtual void set_instance_member_directly(const char *name, ehmember_p new_value, ehcontext_t context, class EHI *ehi) {
        value->instance_members[name] = new_value;
    }

    virtual void printvar(printvar_set &set, int level, EHI *ehi) override;

    virtual ehval_p get_parent_scope();

    static ehval_p make(ehclass_t *obj, EHInterpreter *parent);

    virtual ehmember_p get_instance_member_current_object(const char *name, ehcontext_t context, class EHI *ehi) {
        return value->get_instance_member_current_object(name, context, ehi);
    }

    virtual const std::list<ehval_w> get_super_classes() {
        return value->super;
    }
    virtual bool has_instance_members() const {
        return true;
    }
};
