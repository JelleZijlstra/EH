#include <libgen.h>
#include <stdlib.h>
#include <unistd.h>
#include <string>

#include "eh_files.h"

const std::string eh_getcwd() {
	const char *cwd = getcwd(NULL, 0);
	const std::string out(cwd);
	free((void *)cwd);
	return out;
}

const std::string eh_full_path(const std::string &filename) {
	char *tmp = strdup(filename.c_str());
	const char *my_dirname = dirname(tmp);
	char *resolved_name = new char[PATH_MAX];
	const char *my_fullpath = realpath(my_dirname, resolved_name);
	const std::string out(resolved_name);
	free((void *)tmp);
	delete[] my_fullpath;
	return out;
}

const std::string eh_dirname(const std::string &name) {
	char *tmp = strdup(name.c_str());
	std::string out(dirname(tmp));
	free((void *)tmp);
	return out;
}
