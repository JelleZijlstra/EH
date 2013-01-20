/*
 * File
 * Class used for file I/O. The interface is similar to C's stdio.h, but
 * currently more restrictive.
 */
#include "File.hpp"
// #include "../eh_files.hpp"

#include <string>
#include <fstream>
#include <sstream>

EH_INITIALIZER(File) {
	REGISTER_METHOD(File, initialize);
	REGISTER_METHOD(File, open);
	REGISTER_METHOD(File, getc);
	REGISTER_METHOD(File, gets);
	REGISTER_METHOD(File, puts);
	REGISTER_METHOD(File, close);
	REGISTER_METHOD(File, finalize);
	REGISTER_METHOD(File, readFile);
	REGISTER_METHOD(File, isOpen);
	REGISTER_METHOD(File, temporary);
}

/*
 * @description Initializer. If an argument is given, this method will attempt
 * to open the file named by the argument.
 * @argument String or none
 * @returns N/A
 */
EH_METHOD(File, initialize) {
	ehval_p new_obj = static_cast<ehval_t *>(new File());
	if(!args->is_a<Null>()) {
		ehlm_File_open(new_obj, args, ehi);
	}
	return new_obj;
}

/*
 * @description Open a file.
 * @argument File to open
 * @returns True on success, false on failure
 */
EH_METHOD(File, open) {
	args->assert_type<String>("File.open", ehi);
	ASSERT_RESOURCE(File, "File.open");
	File *f = static_cast<File *>(obj.operator->());
	// close any open file
	if(data != nullptr) {
		fclose(data);
		f->value = nullptr;
	}

	// and open the new one
	FILE *mfile = fopen(args->get<String>(), "r+");
	if(mfile == nullptr) {
		return Bool::make(false);
	}
	f->value = mfile;
	return Bool::make(true);
}

/*
 * @description Read a character from a file. Returns null on failure or EOF.
 * @argument None
 * @returns Character read or null
 */
EH_METHOD(File, getc) {
	args->assert_type<Null>("File.getc", ehi);
	ASSERT_RESOURCE(File, "File.getc");

	if(data == nullptr) {
		return nullptr;
	}
	int c = fgetc(data);
	if(c == EOF) {
		return nullptr;
	}
	char *out = new char[2];
	out[0] = c;
	out[1] = '\0';
	return String::make(out);
}

/*
 * @description Read a line from the input file.
 * @argument None
 * @returns The line read
 */
EH_METHOD(File, gets) {
	args->assert_type<Null>("File.gets", ehi);
	ASSERT_RESOURCE(File, "File.gets");
	if(data == nullptr) {
		return nullptr;
	}

	char *out = new char[512];

	char *ptr = fgets(out, 511, data);
	if(ptr == nullptr) {
		delete[] out;
		return nullptr;
	}
	return String::make(out);
}

/*
 * @description Write a line to a file.
 * @argument Line to write
 * @returns True on success, false on failure, null if file is closed
 */
EH_METHOD(File, puts) {
	args->assert_type<String>("File.puts", ehi);
	ASSERT_RESOURCE(File, "File.puts");
	if(data == nullptr) {
		return nullptr;
	}

	int count = fputs(args->get<String>(), data);
	return Bool::make(count != EOF);
}

/*
 * @description Class method; reads an entire file
 * @argument Name of file to read
 * @returns Content of the file
 */
EH_METHOD(File, readFile) {
	args->assert_type<String>("File.readFile", ehi);
	const char *file = args->get<String>();
	// http://stackoverflow.com/questions/2602013/read-whole-ascii-file-into-c-stdstring
	std::ifstream stream(file);
	if(!stream.is_open()) {
		throw_ArgumentError("Could not read file", "File.readFile", args, ehi);
	}
	std::stringstream buffer;
	buffer << stream.rdbuf();
	char *str = strdup(buffer.str().c_str());
	return String::make(str);
}

/*
 * @description Close the file
 * @argument None
 * @returns null
 */
EH_METHOD(File, close) {
	args->assert_type<Null>("File.close", ehi);
	File *f = static_cast<File *>(obj->data().operator->());
	ASSERT_RESOURCE(File, "File.close");
	if(data == nullptr) {
		return nullptr;
	}
	fclose(data);
	f->value = nullptr;
	return nullptr;
}

/*
 * @description Destructor (currently not actually supported)
 * @argument None
 * @returns N/A
 */
EH_METHOD(File, finalize) {
	ehi->call_method(obj, "close", nullptr, obj);
	return nullptr;
}

/*
 * @description Check whether this File object is associated with an open file.
 * @argument None
 * @returns Bool
 */
EH_METHOD(File, isOpen) {
	args->assert_type<Null>("File.isOpen", ehi);
	ASSERT_RESOURCE(File, "File.isOpen");
	return Bool::make(data != nullptr);
}

/*
 * @description Returns a temporary file suitable for use as a text file.
 * @argument None
 * @returns File name
 */
EH_METHOD(File, temporary) {
	ASSERT_TYPE(args, Null, "File.temporary");
	const std::string name = eh_temp_file();
	return String::make(strdup(name.c_str()));
}
