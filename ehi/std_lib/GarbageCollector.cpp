#include "GarbageCollector.h"

START_EHLC(GarbageCollector)
EHLC_ENTRY(GarbageCollector, run)
EHLC_ENTRY(GarbageCollector, stats)
END_EHLC()

EH_METHOD(GarbageCollector, run) {
	ehi->get_parent()->gc.do_collect(ehi->global());
	return NULL;
}
EH_METHOD(GarbageCollector, stats) {
	ehi->get_parent()->gc.print_stats();
	return NULL;
}
