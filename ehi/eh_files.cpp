/*
 * EH wrappers for file system functions.
 *
 * These implementations are inherently system-specific. Perhaps I should
 * use a library like Boost instead of trying to achieve portability myself.
 */
#include <libgen.h>
#include <stdlib.h>
#include <unistd.h>
#include <string>
#include <string.h>

#include <sstream>
#include <iostream>

#ifdef __linux__
#include <linux/limits.h>
#endif

#include "eh_files.hpp"

class FILE_p {
private:
	FILE *file;
public:
	FILE_p(FILE *f) : file(f) {}

	FILE *operator->() {
		return file;
	}

	~FILE_p() {
		pclose(file);
	}

	bool null() const {
		return file == nullptr;
	}
};

const std::string eh_getcwd() {
	const char *cwd = getcwd(nullptr, 0);
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

const std::string eh_shell_exec(const std::string &command) {
	FILE_p p = popen(command.c_str(), "r");
	if(p.null()) {
		throw eh_files_exception("Could not execute command");
	}
	char buffer[512];
	std::ostringstream output;
	while(fgets(buffer, 511, p.operator->()) != nullptr) {
		output << buffer;
	}
	std::cout.flush();
	return output.str();
}
