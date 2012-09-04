#include "GarbageCollector.h"

START_EHLC(GarbageCollector)
EHLC_ENTRY(GarbageCollector, run)
EHLC_ENTRY(GarbageCollector, stats)
END_EHLC()

EH_METHOD(GarbageCollector, run) {
	ehi->gc.do_collect(ehi->global_object);
	return NULL;
}
EH_METHOD(GarbageCollector, stats) {
	ehi->gc.print_stats();
	return NULL;
}
