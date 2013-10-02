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

    bool inherits(ehval_p superclass);

    void register_method(const std::string &name, const ehlibmethod_t method, const attributes_t attributes, EHInterpreter *interpreter_parent);
};
