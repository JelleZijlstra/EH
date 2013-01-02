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
#include "std_lib_includes.hpp"

EH_CLASS(Map) {
public:
	class t {
	public:
		typedef std::map<ehval_p, ehval_p> eh_map;
		typedef eh_map::iterator iterator;

		size_t size() const {
			return map.size();
		}
		ehval_p get(ehval_p index) const {
			return map.at(index);
		}
		void set(ehval_p index, ehval_p value) {
			map[index] = value;
		}
		bool has(ehval_p index) const {
			return map.count(index);
		}
		t() : map() {}
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

	~Map() {
		delete value;
	}

	Map(type val) : value(val) {}

	virtual bool belongs_in_gc() const {
		return true;
	}

	virtual std::list<ehval_p> children() {
		std::list<ehval_p> out;
		for(auto &i : value->map) {
			out.push_back(i.first);
			out.push_back(i.second);
		}
		return out;
	}

	static ehval_p make(EHInterpreter *parent) {
		return parent->allocate<Map>(new t());
	}
};
EH_METHOD(Map, compare);
EH_METHOD(Map, initialize);
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

	~Map_Iterator() {
		delete value;
	}

	virtual bool belongs_in_gc() const {
		return true;
	}

	virtual std::list<ehval_p> children() {
		return { value->map };
	}

	static ehval_p make(ehval_p map, EHInterpreter *parent) {
		return parent->allocate<Map_Iterator>(new t(map));
	}
};
EH_METHOD(Map_Iterator, initialize);
EH_METHOD(Map_Iterator, hasNext);
EH_METHOD(Map_Iterator, next);
EH_METHOD(Map_Iterator, peek);

EH_INITIALIZER(Map_Iterator);
