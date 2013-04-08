#include "Map.hpp"

#include "ArgumentError.hpp"
#include "EmptyIterator.hpp"

EH_INITIALIZER(Map) {
	REGISTER_METHOD(Map, initialize);
	REGISTER_METHOD_RENAME(Map, operator_arrow, "operator->");
	REGISTER_METHOD_RENAME(Map, operator_arrow_equals, "operator->=");
	REGISTER_METHOD(Map, has);
	REGISTER_METHOD(Map, compare);
	REGISTER_METHOD(Map, size);
	REGISTER_METHOD(Map, getIterator);
	REGISTER_CLASS(Map, Iterator);
}

ehval_p Map::make(EHI* ehi) {
	return ehi->get_parent()->allocate<Map>(new t(ehi));
}

bool Map::Comparator::operator()(const ehval_p &l, const ehval_p &r) const {
	return ehi->call_method_typed<Bool>(l, "operator<", r, l)->get<Bool>();
}

Map::t::t(EHI *ehi) : map(Comparator(ehi)) {}

// Map::t::t(EHI *ehi) : map([ehi](const ehval_p &l, const ehval_p &r) {
// 	return ehi->call_method_typed<Bool>(l, "operator<", r, l)->get<Bool>();
// }) {}


EH_METHOD(Map, initialize) {
	return Map::make(ehi);
}

EH_METHOD(Map, operator_arrow) {
	ASSERT_RESOURCE(Map, "Map.operator->");
	if(data->has(args)) {
		return data->get(args);
	} else {
		return nullptr;
	}
}

EH_METHOD(Map, operator_arrow_equals) {
	ASSERT_NARGS(2, "Map.operator->=");
	ASSERT_RESOURCE(Map, "Map.operator->=");
	const Tuple::t *tuple = args->get<Tuple>();
	ehval_p key = tuple->get(0);
	ehval_p value = tuple->get(1);
	data->set(key, value);
	return value;
}

EH_METHOD(Map, size) {
	ASSERT_NULL("Map.size");
	ASSERT_RESOURCE(Map, "Map.size");
	return Integer::make(data->size());
}

EH_METHOD(Map, has) {
	ASSERT_RESOURCE(Map, "Map.operator->");
	return Bool::make(data->has(args));
}

EH_METHOD(Map, getIterator) {
	ASSERT_OBJ_TYPE(Map, "Map.getIterator");
	ehval_p map_class = ehi->get_parent()->repo.get_object(obj);
	ehval_p class_member = map_class->get_property("Iterator", obj, ehi);
	return ehi->call_method(class_member, "new", obj, obj);
}

/*
 * @description Compare two maps.
 * @argument Map to compare to.
 * @returns Integer (as specified for Object.compare)
 */
EH_METHOD(Map, compare) {
	ASSERT_RESOURCE(Map, "Map.compare");
	ASSERT_TYPE(args, Map, "Map.compare");
	Map::t *lhs = data;
	Map::t *rhs = args->get<Map>();

	Map::t::iterator lhs_it = lhs->map.begin();
	Map::t::iterator rhs_it = rhs->map.begin();
	Map::t::iterator lhs_end = lhs->map.end();
	Map::t::iterator rhs_end = rhs->map.end();
	while(true) {
		// check whether we've reached the end
		if(lhs_it == lhs_end) {
			if(rhs_it == rhs_end) {
				break;
			} else {
				return Integer::make(-1);
			}
		} else if(rhs_it == rhs_end) {
			return Integer::make(1);
		}

		// compare keys
		int key_cmp = ehi->compare(lhs_it->first, rhs_it->first, obj);
		if(key_cmp != 0) {
			return Integer::make(key_cmp);
		}

		// compare values
		int value_cmp = ehi->compare(lhs_it->second, rhs_it->second, obj);
		if(value_cmp != 0) {
			return Integer::make(value_cmp);
		}

		// continue iteration
		lhs_it++;
		rhs_it++;
	}
	return Integer::make(0);
}

EH_INITIALIZER(Map_Iterator) {
	REGISTER_METHOD(Map_Iterator, initialize);
	REGISTER_METHOD(Map_Iterator, hasNext);
	REGISTER_METHOD(Map_Iterator, next);
	REGISTER_METHOD(Map_Iterator, peek);
}

ehval_p Map_Iterator::make(ehval_p map, EHInterpreter *parent) {
	return parent->allocate<Map_Iterator>(new t(map));
}

Map_Iterator::t::t(ehval_p _map) : map(_map) {
	Map::t *data = _map->get<Map>();
	begin = data->map.begin();
	end = data->map.end();
}
bool Map_Iterator::t::has_next() const {
	return begin != end;
}
ehval_p Map_Iterator::t::next(EHI *ehi) {
	ehval_p out = peek(ehi);
	begin++;
	return out;
}
ehval_p Map_Iterator::t::peek(EHI *ehi) const {
	assert(this->has_next());
	ehval_p tuple[2];
	tuple[0] = begin->first;
	tuple[1] = begin->second;
	return Tuple::make(2, tuple, ehi->get_parent());
}

EH_METHOD(Map_Iterator, initialize) {
	ASSERT_TYPE(args, Map, "Map.Iterator.initialize");
	return Map_Iterator::make(args, ehi->get_parent());
}
EH_METHOD(Map_Iterator, hasNext) {
	args->assert_type<Null>("Map.Iterator.hasNext", ehi);
	ASSERT_RESOURCE(Map_Iterator, "Map.Iterator.hasNext");
	return Bool::make(data->has_next());
}
EH_METHOD(Map_Iterator, next) {
	args->assert_type<Null>("Map.Iterator.next", ehi);
	ASSERT_RESOURCE(Map_Iterator, "Map.Iterator.hasNext");
	if(!data->has_next()) {
		throw_EmptyIterator(ehi);
	}
	return data->next(ehi);
}
EH_METHOD(Map_Iterator, peek) {
	args->assert_type<Null>("Map.Iterator.peek", ehi);
	ASSERT_RESOURCE(Map_Iterator, "Map.Iterator.peek");
	if(!data->has_next()) {
		throw_EmptyIterator(ehi);
	}
	return data->peek(ehi);
}
