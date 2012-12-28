/*
 * File
 * Class used for file I/O. The interface is similar to C's stdio.h, but
 * currently more restrictive.
 */
#include "File.hpp"

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
}

/*
 * @description Initializer. If an argument is given, this method will attempt
 * to open the file named by the argument.
 * @argument String or none
 * @returns N/A
 */
EH_METHOD(File, initialize) {
	ehretval_p new_obj = ehretval_t::make_resource(obj->get_full_type(), static_cast<LibraryBaseClass *>(new File()));
	if(args->type() != null_e) {
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
	ASSERT_TYPE(args, string_e, "File.open");
	ASSERT_RESOURCE(File, "File.open");
	// close any open file
	if(data->descriptor != NULL) {
		fclose(data->descriptor);
		data->descriptor = NULL;
	}

	// and open the new one
	FILE *mfile = fopen(args->get_stringval(), "r+");
	if(mfile == NULL) {
		return ehretval_t::make_bool(false);
	}
	data->descriptor = mfile;
	return ehretval_t::make_bool(true);
}

/*
 * @description Read a character from a file. Returns null on failure or EOF.
 * @argument None
 * @return Character read or null
 */
EH_METHOD(File, getc) {
	ASSERT_TYPE(args, null_e, "File.getc");
	ASSERT_RESOURCE(File, "File.getc");

	if(data->descriptor == NULL) {
		return NULL;
	}
	int c = fgetc(data->descriptor);
	if(c == EOF) {
		return NULL;
	}
	char *out = new char[2];
	out[0] = c;
	out[1] = '\0';
	return ehretval_t::make_string(out);
}

/*
 * @description Read a line from the input file.
 * @argument None
 * @return The line read.
 */
EH_METHOD(File, gets) {
	ASSERT_TYPE(args, null_e, "File.gets");
	ASSERT_RESOURCE(File, "File.gets");
	if(data->descriptor == NULL) {
		return NULL;
	}

	char *out = new char[512];

	char *ptr = fgets(out, 511, data->descriptor);
	if(ptr == NULL) {
		delete[] out;
		return NULL;
	}
	return ehretval_t::make_string(out);
}

/*
 * @description Write a line to a file.
 * @argument Line to write
 * @returns True on success, false on failure, null if file is closed
 */
EH_METHOD(File, puts) {
	ASSERT_TYPE(args, string_e, "File.puts");
	ASSERT_RESOURCE(File, "File.puts");
	if(data->descriptor == NULL) {
		return NULL;
	}

	int count = fputs(args->get_stringval(), data->descriptor);
	return ehretval_t::make_bool(count != EOF);
}

/*
 * @description Class method; reads an entire file
 * @argument Name of file to read
 * @returns Content of the file
 */
EH_METHOD(File, readFile) {
	ASSERT_TYPE(args, string_e, "File.readFile");
	const char *file = args->get_stringval();
	// http://stackoverflow.com/questions/2602013/read-whole-ascii-file-into-c-stdstring
	std::ifstream stream(file);
	if(!stream.is_open()) {
		throw_ArgumentError("Could not read file", "File.readFile", args, ehi);
	}
	std::stringstream buffer;
	buffer << stream.rdbuf();
	char *str = strdup(buffer.str().c_str());
	return ehretval_t::make_string(str);
}

/*
 * @description Close the file
 * @argument None
 * @return null
 */
EH_METHOD(File, close) {
	ASSERT_TYPE(args, null_e, "File.close");
	ASSERT_RESOURCE(File, "File.close");
	if(data->descriptor == NULL) {
		return NULL;
	}
	fclose(data->descriptor);
	data->descriptor = NULL;
	return NULL;
}

/*
 * @description Destructor (currently not actually supported)
 * @argument None
 * @return N/A
 */
EH_METHOD(File, finalize) {
	ehi->call_method(obj, "close", NULL, obj);
	return NULL;
}

/*
 * @description Check whether this File object is associated with an open file.
 * @argument None
 * @return Bool
 */
EH_METHOD(File, isOpen) {
	ASSERT_TYPE(args, null_e, "File.isOpen");
	ASSERT_RESOURCE(File, "File.isOpen");
	return ehretval_t::make_bool(data->descriptor != NULL);
}
