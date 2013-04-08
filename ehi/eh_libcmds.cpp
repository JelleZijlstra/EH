/*
 * Code for library commands.
 */
#include "eh.hpp"
#include "eh_libcmds.hpp"
#include "eh_libclasses.hpp"
#include "std_lib/GlobalObject.hpp"
#include "std_lib/Map.hpp"

EH_LIBCMD(quit) {
	throw quit_exception();
}

EH_LIBCMD(echo) {
	paras->assert_type<Map>("echo", ehi);
	return ehlm_GlobalObject_echo(obj, paras->get<Map>()->safe_get(Integer::make(0)), ehi);
}

EH_LIBCMD(put) {
	paras->assert_type<Map>("put", ehi);
	return ehlm_GlobalObject_put(obj, paras->get<Map>()->safe_get(Integer::make(0)), ehi);
}
