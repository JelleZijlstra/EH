#include "eh.h"

int EHI::eh_interactive(interactivity_enum interactivity) {
	ehretval_p ret;

	EHParser parser(interactivity, this);
	char *cmd = eh_getline(&parser);
	if(!cmd) {
		return eh_outer_exit(0);
	}
	// if a syntax error occurs, stop parsing and return -1
	try {
		ret = parser.parse_string(cmd);
	} catch(...) {
		// do nothing
	}
	return ret->intval;
}
ehretval_p EHI::parse_file(const char *name) {
	FILE *infile = fopen(name, "r");
	if(!infile) {
		fprintf(stderr, "Could not open input file\n");
		return NULL;
	}
	EHParser parser(end_is_end_e, this);
	// if a syntax error occurs, stop parsing and return -1
	try {
		return parser.parse_file(infile);
	} catch(...) {
		// TODO: actually do something useful with exceptions
		return NULL;
	}
}
void EHI::init_eval_parser() {
	if(eval_parser == NULL) {
		eval_parser = new EHParser(end_is_end_e, this);
	}
}
ehretval_p EHI::parse_string(const char *cmd) {
	init_eval_parser();
	return eval_parser->parse_string(cmd);
}
ehretval_t::~ehretval_t() {
	switch(_type) {
		// Simple types; nothing to do
		case int_e:
		case bool_e:
		case float_e:
		case type_e:
		case null_e:
		case accessor_e:
		case attribute_e:
		case attributestr_e:
			break;
		case op_e:
			delete this->opval;
			break;
		// TODO
		case string_e:
			//delete[] this->stringval;
			break;
		// Delete object. An ehretval_t owns the object pointed to.
		case range_e:
			delete this->rangeval;
			break;
		case object_e:
		case func_e:
			delete this->objectval;
			break;
		case array_e:
			delete this->arrayval;
			break;
		case weak_object_e:
			// we don't own the object
			break;
	}
}

