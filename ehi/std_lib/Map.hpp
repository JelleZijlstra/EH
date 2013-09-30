/*
 * Map library class.
 *
 * This class provides a mapping from unique keys of any type to values. The
 * mapping uses ehretval_t::naive_compare, however, which only compares
 * primitive types by value while others are compared by reference. As a
 * consequence, code like:
 *	m = Map.new()
 *	m->[] = 42
 *	echo m->[]
 * will not print 42.
 */
#include "../eh.hpp"

#ifndef EH_MAP_H_
#define EH_MAP_H_

#include "std_lib_includes.hpp"

EH_CLASS(Map) {
	struct Comparator : std::binary_function<ehval_p, ehval_p, bool> {
	private:
		mutable EHI *ehi;
	public:
		Comparator(EHI *e) : ehi(e) {}

		bool operator()(const ehval_p &l, const ehval_p &r) const;
	};

public:
	class t {
	public:
		typedef std::map<ehval_p, ehval_p, Comparator> eh_map;
		typedef eh_map::iterator iterator;

		size_t size() const {
			return map.size();
		}
		ehval_p get(ehval_p index) const {
			return map.at(index);
		}
		ehval_p safe_get(ehval_p index) const {
			if(has(index)) {
				return get(index);
			} else {
				return nullptr;
			}
		}
		void set(ehval_p index, ehval_p val) {
			map[index] = val;
		}
		bool has(ehval_p index) const {
			return map.count(index);
		}
		t(EHI *ehi);
		~t() {}

		eh_map map;
	private:
		t(const t&);
		t operator=(const t&);

		friend class Map_Iterator;

		friend EH_METHOD(Map, compare);
	};

	typedef t *type;
	type value;

	virtual ~Map() {
		delete value;
	}

	Map(type val) : value(val) {}

	virtual bool belongs_in_gc() const {
		return true;
	}

	virtual std::list<ehval_p> children() const override {
		std::list<ehval_p> out;
		for(auto &i : value->map) {
			out.push_back(i.first);
			out.push_back(i.second);
		}
		assert(out.size() == 2 * value->size());
		return out;
	}

	virtual void printvar(printvar_set &set, int level, EHI *ehi) override {
		void *ptr = static_cast<void *>(value);
		if(set.count(ptr) == 0) {
			set.insert(ptr);
			std::cout << "@map [" << std::endl;
			for(auto &i : value->map) {
				add_tabs(std::cout, level + 1);
				i.first->printvar(set, level + 1, ehi);
				add_tabs(std::cout, level + 1);
				std::cout << " => ";
				i.second->printvar(set, level + 1, ehi);
			}
			add_tabs(std::cout, level);
			std::cout << "]" << std::endl;
		} else {
			std::cout << "(recursion)" << std::endl;
		}
	}

	static ehval_p make(EHI *ehi);
};
EH_METHOD(Map, operator_colon);
EH_METHOD(Map, compare);
EH_METHOD(Map, operator_arrow);
EH_METHOD(Map, operator_arrow_equals);
EH_METHOD(Map, has);
EH_METHOD(Map, size);
EH_METHOD(Map, getIterator);

EH_INITIALIZER(Map);

EH_CHILD_CLASS(Map, Iterator) {
public:
	class t {
	public:
		t(ehval_p map);
		~t() {}
		bool has_next() const;
		ehval_p next(EHI *ehi);
		ehval_p peek(EHI *ehi) const;

		ehval_p map;
	private:
		Map::t::iterator begin;
		Map::t::iterator end;
		t(const t&);
		t operator=(const t&);
	};

	typedef t *type;
	type value;

	Map_Iterator(type val) : value(val) {}

	virtual ~Map_Iterator() {
		delete value;
	}

	virtual bool belongs_in_gc() const {
		return true;
	}

	virtual std::list<ehval_p> children() const override {
		return { value->map };
	}

	static ehval_p make(ehval_p map, EHInterpreter *parent);
};
EH_METHOD(Map_Iterator, operator_colon);
EH_METHOD(Map_Iterator, hasNext);
EH_METHOD(Map_Iterator, next);
EH_METHOD(Map_Iterator, peek);

EH_INITIALIZER(Map_Iterator);

#endif
