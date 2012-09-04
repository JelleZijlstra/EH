#include "CountClass.h"

START_EHLC(CountClass)
EHLC_ENTRY(CountClass, initialize)
EHLC_ENTRY(CountClass, docount)
EHLC_ENTRY(CountClass, setcount)
END_EHLC()

EH_METHOD(CountClass, initialize) {
	return ehretval_t::make_resource((LibraryBaseClass *)new CountClass());
}
EH_METHOD(CountClass, docount) {
	ASSERT_TYPE(args, null_e, "CountClass.docount");
	CountClass *selfptr = (CountClass *)obj->get_resourceval();
	return ehretval_t::make_int(++selfptr->count);
}
EH_METHOD(CountClass, setcount) {
	CountClass *selfptr = (CountClass *)obj->get_resourceval();
	ASSERT_TYPE(args, int_e, "CountClass.setcount");
	selfptr->count = args->get_intval();
	return ehretval_t::make_bool(true);
}
