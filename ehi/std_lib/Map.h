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
#include "std_lib_includes.h"

class Map : public LibraryBaseClass {
public:
	typedef std::map<ehretval_p, ehretval_p> eh_map;
	typedef eh_map::iterator iterator;

	size_t size() const {
		return map.size();
	}
	ehretval_p get(ehretval_p index) const {
		return map.at(index);
	}
	void set(ehretval_p index, ehretval_p value) {
		map[index] = value;		
	}
	bool has(ehretval_p index) const {
		return map.count(index);
	}
	Map() : map() {}
	~Map() {}
private:
	eh_map map;
	Map(const Map&);
	Map operator=(const Map&);

	friend class Map_Iterator;
};
EH_METHOD(Map, initialize);
EH_METHOD(Map, operator_arrow);
EH_METHOD(Map, operator_arrow_equals);
EH_METHOD(Map, has);
EH_METHOD(Map, size);
EH_METHOD(Map, getIterator);

EH_INITIALIZER(Map);

class Map_Iterator : public LibraryBaseClass {
public:
	Map_Iterator(Map *map);
	~Map_Iterator() {}
	bool has_next() const;
	ehretval_p next(EHI *ehi);
	ehretval_p peek(EHI *ehi) const;
private:
	Map::iterator begin;
	Map::iterator end;
	Map_Iterator(const Map_Iterator&);
	Map_Iterator operator=(const Map_Iterator&);
};
EH_METHOD(Map_Iterator, initialize);
EH_METHOD(Map_Iterator, hasNext);
EH_METHOD(Map_Iterator, next);
EH_METHOD(Map_Iterator, peek);

EH_INITIALIZER(Map_Iterator);
