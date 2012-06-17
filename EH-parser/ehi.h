/*
 * ehi.h
 * Jelle Zijlstra, December 2011
 */
#include "eh.bison.hpp"

/*
 * Flex and Bison
 */
int yylex(YYSTYPE *, void *);
int yylex_init_extra(struct EHParser *, void **);
int yylex_destroy(void *);
struct yy_buffer_state *yy_scan_string ( const char *str );

typedef enum {
	cli_prompt_e,
	cli_no_prompt_e,
	end_is_end_e
} interactivity_enum;

class EHI {
private:
	class EHParser *eval_parser;
	void init_eval_parser();
public:
	int eh_interactive(interactivity_enum interactivity = cli_prompt_e);
	ehretval_t parse_string(const char *cmd);
	ehretval_t parse_file(const char *name);
	EHI() : eval_parser(NULL) {}

	virtual ehretval_t *execute_cmd(const char *rawcmd, ehvar_t **paras);
	virtual char *eh_getline(class EHParser *parser = NULL);
	virtual ~EHI();
};

/*
 * The EH parser
 */
class EHParser {
public:
	interactivity_enum interactivity() {
		return _interactivity;
	}
	char *getline() {
		return _parent->eh_getline(this);
	}

	ehretval_t parse_file(FILE *infile);
	ehretval_t parse_string(const char *cmd);

	EHParser(interactivity_enum inter, EHI *parent) :  _parent(parent), _interactivity(inter) {
		yylex_init_extra(this, &_scanner);
	}
	~EHParser(void) {
		yylex_destroy(_scanner);
	}
private:
	void *_scanner;
	EHI *_parent;
	interactivity_enum _interactivity;
	struct yy_buffer_state *buffer;

};
