#include <sstream>

#include "../Array.hpp"

class json_scanner {
public:
	void *inner_scanner;
	std::ostringstream str_str;
	EHI *ehi;
	ehval_p result;

	json_scanner(EHI *_ehi);

	~json_scanner();

	ehval_p parse(const char *str);
};

#define YYSTYPE ehval_p
