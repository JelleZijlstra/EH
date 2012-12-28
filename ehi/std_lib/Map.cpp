#include "Map.hpp"

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

EH_METHOD(Map, initialize) {
	LibraryBaseClass *m = static_cast<LibraryBaseClass *>(new Map());
	return ehretval_t::make_resource(obj->get_full_type(), m);
}

EH_METHOD(Map, operator_arrow) {
	ASSERT_RESOURCE(Map, "Map.operator->");
	if(data->has(args)) {
		return data->get(args);
	} else {
		return NULL;
	}
}

EH_METHOD(Map, operator_arrow_equals) {
	ASSERT_NARGS_AND_TYPE(2, resource_e, "Map.operator->=");
	const ehtuple_t *tuple = args->get_tupleval();
	ehretval_p key = tuple->get(0);
	ehretval_p value = tuple->get(1);

	Map *m = static_cast<Map *>(obj->get_resourceval());
	m->set(key, value);
	return value;
}

EH_METHOD(Map, size) {
	ASSERT_NULL_AND_TYPE(resource_e, "Map.size");
	const Map *m = static_cast<Map *>(obj->get_resourceval());
	return ehretval_t::make_int(m->size());
}

EH_METHOD(Map, has) {
	ASSERT_OBJ_TYPE(resource_e, "Map.operator->");
	const Map *m = static_cast<Map *>(obj->get_resourceval());
	return ehretval_t::make_bool(m->has(args));
}

EH_METHOD(Map, getIterator) {
	ASSERT_OBJ_TYPE(resource_e, "Map.getIterator");
	unsigned int map_type = obj->get_full_type();
	ehretval_p map_class = ehi->get_parent()->repo.get_object(map_type);
	ehretval_p class_member = ehi->get_property(map_class, "Iterator", obj);
	return ehi->call_method(class_member, "new", obj, obj);
}

/*
 * @description Compare two maps.
 * @argument Map to compare to.
 * @returns Integer (as specified for Object.compare)
 */
EH_METHOD(Map, compare) {
	ASSERT_OBJ_TYPE(resource_e, "Map.compare");
	ASSERT_TYPE(args, resource_e, "Map.compare");
	Map *lhs = static_cast<Map *>(obj->get_resourceval());
	Map *rhs = static_cast<Map *>(args->get_resourceval());

	Map::iterator lhs_it = lhs->map.begin();
	Map::iterator rhs_it = rhs->map.begin();
	Map::iterator lhs_end = lhs->map.end();
	Map::iterator rhs_end = rhs->map.end();
	while(true) {
		// check whether we've reached the end
		if(lhs_it == lhs_end) {
			if(rhs_it == rhs_end) {
				break;
			} else {
				return ehretval_t::make_int(-1);
			}
		} else if(rhs_it == rhs_end) {
			return ehretval_t::make_int(1);
		}

		// compare keys
		int key_cmp = ehi->compare(lhs_it->first, rhs_it->first, obj);
		if(key_cmp != 0) {
			return ehretval_t::make_int(key_cmp);
		}

		// compare values
		int value_cmp = ehi->compare(lhs_it->second, rhs_it->second, obj);
		if(value_cmp != 0) {
			return ehretval_t::make_int(value_cmp);
		}

		// continue iteration
		lhs_it++;
		rhs_it++;
	}
	return ehretval_t::make_int(0);
}

EH_INITIALIZER(Map_Iterator) {
	REGISTER_METHOD(Map_Iterator, initialize);
	REGISTER_METHOD(Map_Iterator, hasNext);
	REGISTER_METHOD(Map_Iterator, next);
	REGISTER_METHOD(Map_Iterator, peek);
}

Map_Iterator::Map_Iterator(Map *map) {
	begin = map->map.begin();
	end = map->map.end();
}
bool Map_Iterator::has_next() const {
	return begin != end;
}
ehretval_p Map_Iterator::next(EHI *ehi) {
	ehretval_p out = peek(ehi);
	begin++;
	return out;
}
ehretval_p Map_Iterator::peek(EHI *ehi) const {
	assert(this->has_next());
	ehretval_p tuple[2];
	tuple[0] = begin->first;
	tuple[1] = begin->second;
	return ehi->get_parent()->make_tuple(new ehtuple_t(2, tuple));
}

EH_METHOD(Map_Iterator, initialize) {
	ASSERT_TYPE(args, resource_e, "Map.Iterator.initialize");
	Map *m = static_cast<Map *>(args->get_resourceval());
	Map_Iterator *data = new Map_Iterator(m);
	return ehretval_t::make_resource(obj->get_full_type(), data);
}
EH_METHOD(Map_Iterator, hasNext) {
	ASSERT_TYPE(args, null_e, "Map.Iterator.hasNext");
	ASSERT_RESOURCE(Map_Iterator, "Map.Iterator.hasNext");
	return ehretval_t::make_bool(data->has_next());
}
EH_METHOD(Map_Iterator, next) {
	ASSERT_TYPE(args, null_e, "Map.Iterator.next");
	ASSERT_RESOURCE(Map_Iterator, "Map.Iterator.hasNext");
	if(!data->has_next()) {
		throw_EmptyIterator(ehi);
	}
	return data->next(ehi);
}
EH_METHOD(Map_Iterator, peek) {
	ASSERT_TYPE(args, null_e, "Map.Iterator.peek");
	ASSERT_RESOURCE(Map_Iterator, "Map.Iterator.peek");
	if(!data->has_next()) {
		throw_EmptyIterator(ehi);
	}
	return data->peek(ehi);
}
