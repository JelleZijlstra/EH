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

struct MapComparator : std::binary_function<ehval_p, ehval_p, bool> {
private:
	// We include a pointer to the encompassing Map::t object here so we have access to an ehi object
	// that can be mutated by outside code. Initially, this would simply keep a reference to the
	// EHI object active when the map was created, but this would fail horribly if the map is used
	// after that EHI object is destroyed (for example, if the map is created in an include'ed file).
	// Instead, every Map::t method now needs to be passed an EHI object, ensuring that the Comparator
	// has a usable EHI object that it can use to call operator< on objects.

	// This is still bad in a broader way, because we rely on the EHI object for the location of files
	// to be include'ed. Therefore, include'ing a file in a function that is called from a different
	// file is doomed to failure. Fixing this requires a more radical rethinking of the EHI/EHInterpreter
	// structure.
	mutable class eh_map_t *map_obj;
public:
	MapComparator(class eh_map_t *m) : map_obj(m) {}

	bool operator()(const ehval_p &l, const ehval_p &r) const;
};

class eh_map_t {
public:
	typedef std::map<ehval_p, ehval_p, MapComparator> eh_map;
	typedef eh_map::iterator iterator;

	size_t size() const {
		return map.size();
	}
	ehval_p get(ehval_p index, EHI *ehi) const {
		current_ehi = ehi;
		return map.at(index);
	}
	ehval_p safe_get(ehval_p index, EHI *ehi) const {
		current_ehi = ehi;
		if(has(index, ehi)) {
			return get(index, ehi);
		} else {
			return nullptr;
		}
	}
	void set(ehval_p index, ehval_p val, EHI *ehi) {
		current_ehi = ehi;
		map[index] = val;
	}
	bool has(ehval_p index, EHI *ehi) const {
		current_ehi = ehi;
		return map.count(index) != 0;
	}
	eh_map_t(EHI *ehi);
	~eh_map_t() {}

	mutable EHI *current_ehi;
	eh_map map;
private:
	eh_map_t(const eh_map_t&) = delete;
	eh_map_t operator=(const eh_map_t&) = delete;

	friend class Map_Iterator;

	friend EH_METHOD(Map, compare);
};

EH_CLASS(Map) {

public:
	typedef eh_map_t *type;
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
		eh_map_t::iterator begin;
		eh_map_t::iterator end;
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
