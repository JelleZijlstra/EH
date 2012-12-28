/*
 * eh_files.h
 *
 * Wrapper functions for file system operations.
 */

#ifndef EH_FILES_H_
#define EH_FILES_H_

const std::string eh_getcwd();

const std::string eh_full_path(const std::string &filename);

const std::string eh_dirname(const std::string &name);

#endif /* EH_FILES_H_ */
