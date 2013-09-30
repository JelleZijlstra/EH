/*
 * Type repository; used to keep track of types.
 */

class type_repository {
private:
    class type_info {
    public:
        const bool is_inbuilt;
        const std::string name;
        ehval_p type_object;

        type_info(bool ii, const std::string &n, ehval_p to) : is_inbuilt(ii), name(n), type_object(to) {}
    };

    std::unordered_map<std::type_index, unsigned int> inbuilt_types;

    std::vector<type_info> types;

public:
    template<class T>
    unsigned int register_inbuilt_class(ehval_p object) {
        const unsigned int type_id = static_cast<unsigned int>(register_class(ehval_t::name<T>(), object, true));
        inbuilt_types[std::type_index(typeid(T))] = type_id;
        return type_id;
    }

    template<class T>
    ehval_p get_primitive_class() const {
        return types.at(get_primitive_id<T>()).type_object;
    }

    template<class T>
    unsigned int get_primitive_id() const {
        return inbuilt_types.at(std::type_index(typeid(T)));;
    }

    unsigned int get_type_id(const ehval_p obj) const {
        return inbuilt_types.at(std::type_index(typeid(*obj.operator->())));
    }

    unsigned int register_class(const std::string &name, const ehval_p value, const bool is_inbuilt = false) {
        const unsigned int type_id = static_cast<unsigned int>(types.size());
        types.push_back(type_info(is_inbuilt, name, value));
        return type_id;
    }

    const std::string &get_name(const unsigned int type_id) const {
        return types.at(type_id).name;
    }

    ehval_p get_object(const unsigned int type_id) const {
        return types.at(type_id).type_object;
    }

    type_repository() : inbuilt_types(), types() {}
};
