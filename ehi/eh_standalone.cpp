// Implementations of some EHI methods that are shared by ehi and libeh but not ehphp

#include "eh.hpp"
#include "std_lib/UnknownCommandError.hpp"

ehval_p EHI::execute_cmd(const char *name, eh_map_t *paras) {
	throw_UnknownCommandError(name, this);
	return nullptr;
}
char *EHI::eh_getline() {
	if(this->buffer == nullptr) {
		this->buffer = new char[512];
	}
	if(get_interactivity() == cli_prompt_e) {
		printf("> ");
	}
	return fgets(buffer, 511, stdin);
}
