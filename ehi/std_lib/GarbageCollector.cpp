#include "GarbageCollector.hpp"

EH_INITIALIZER(GarbageCollector) {
	REGISTER_METHOD(GarbageCollector, run);
	REGISTER_METHOD(GarbageCollector, stats);
}

EH_METHOD(GarbageCollector, run) {
	ehi->get_parent()->gc.do_collect();
	return nullptr;
}
EH_METHOD(GarbageCollector, stats) {
	ehi->get_parent()->gc.print_stats();
	return nullptr;
}
