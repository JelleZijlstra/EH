#ifndef EH_ENUM_H_
#define EH_ENUM_H_

#include <unordered_map>

#include "../eh_libclasses.hpp"

EH_CLASS(Enum) {
public:
	class member_info {
	public:
		std::string name;
		std::vector<std::string> members;

		member_info(const std::string &_name, const std::vector<std::string> &_members) : name(_name), members(_members) {}

		member_info() : name(), members() {}

		const std::string to_string() const;
	};

	class t {
	private:
		size_t nmembers;
		unsigned int next_id;

	public:
		std::unordered_map<unsigned int, member_info> member_map;
		const std::string name;

		const std::string to_string() const;

		size_t size() const {
			return nmembers;
		}

		t(const std::string &_name) : nmembers(0), next_id(0), member_map(), name(_name) {}

		unsigned int add_member(const std::string &member_name, const std::vector<std::string> &members, unsigned int id = 0) {
			member_info member(member_name, members);

			if(id == 0) {
				id = next_id;
			}
			member_map[id] = member;

			next_id = id + 1;
			nmembers++;
			return id;
		}
	};

	typedef t *type;
	type value;

	virtual bool belongs_in_gc() const {
		return false;
	}

	virtual ~Enum() {
		delete value;
	}

	Enum(type val) : value(val) {}

	static ehval_p make(const std::string name) {
		return static_cast<ehval_t *>(new Enum(new t(name)));
	}

	static ehval_p make_enum_class(const char *name, ehval_p scope, EHInterpreter *parent);
};

EH_CHILD_CLASS(Enum, Instance) {
public:
	class t {
	public:
		const unsigned int type_id;

		const unsigned int member_id;

		const unsigned int nmembers;

		ehval_p *members;

		t(unsigned int type, unsigned int member, unsigned int n, ehval_p *args) : type_id(type), member_id(member), nmembers(n), members(args) {}

		virtual ~t() {
			delete[] members;
		}

		int type_compare(Enum_Instance::t *rhs);

		int compare(Enum_Instance::t *rhs, EHI *ehi, ehcontext_t context);

		ehval_p get(unsigned int i) {
			assert(i < nmembers);
			return members[i];
		}

		const ehval_p get_parent_enum(EHI *ehi) const {
			return ehi->get_parent()->repo.get_object(type_id)->data();
		}

		const Enum::member_info &get_member_info(EHI *ehi) const {
			Enum::t *parent_enum = get_parent_enum(ehi)->get<Enum>();
			return parent_enum->member_map[member_id];
		}

		bool is_constructor() const {
			return nmembers > 0 && members == nullptr;
		}

		virtual std::string decompile(int level) const {
			return "";
		}
	private:
		t(const t &) =delete;
		t operator=(const t &) =delete;
	};

	typedef t *type;
	type value;

	virtual bool belongs_in_gc() const override {
		return true;
	}

	virtual std::list<ehval_p> children() const override {
		std::list<ehval_p> out;
		if(value->members != nullptr) {
			const unsigned int size = value->nmembers;
			for(unsigned int i = 0; i < size; i++) {
				out.push_back(value->get(i));
			}
		}
		assert(out.size() == ((value->members == nullptr) ? 0 : value->nmembers));
		return out;
	}

	virtual void printvar(printvar_set &set, int level, EHI *ehi) override;

	virtual ~Enum_Instance() {
		delete value;
	}

	Enum_Instance(type val) : value(val) {
		assert(value != nullptr);
	}

	static ehval_p make(type val, EHInterpreter *parent) {
		return parent->allocate<Enum_Instance>(val);
	}

	virtual std::string decompile(int level) const override {
		return value->decompile(level);
	}
private:
	Enum_Instance(const Enum_Instance &) =delete;
	Enum_Instance operator=(const Enum_Instance &) =delete;
};

EH_INITIALIZER(Enum);

EH_METHOD(Enum, new);
EH_METHOD(Enum, toString);
EH_METHOD(Enum, operator_colon);
EH_METHOD(Enum, compare);
EH_METHOD(Enum, typeId);
EH_METHOD(Enum, type);
EH_METHOD(Enum, isConstructor);
EH_METHOD(Enum, operator_arrow);
EH_METHOD(Enum, map);

#endif /* EH_ENUM_H_ */
