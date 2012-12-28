/*
 * eh_files.h
 *
 * Wrapper functions for file system operations.
 */

#ifndef EH_FILES_H_
#define EH_FILES_H_

#include <exception>

class eh_files_exception : public std::exception {
private:
	const std::string message;
public:
	eh_files_exception(const std::string &msg) : message(msg) {}

	virtual const char *what() const throw() {
		return message.c_str();
	}

	virtual ~eh_files_exception() throw() {}
};

const std::string eh_getcwd();

const std::string eh_full_path(const std::string &filename);

const std::string eh_dirname(const std::string &name);

const std::string eh_shell_exec(const std::string &command);

#endif /* EH_FILES_H_ */
