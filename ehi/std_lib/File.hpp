/*
 * File library class
 */
#include "std_lib_includes.hpp"

class File : public LibraryBaseClass {
public:
	FILE *descriptor;
	File() : descriptor(NULL) {}
	~File() {}
private:
	File(const File&);
	File operator=(const File&);
};
EH_METHOD(File, initialize);
EH_METHOD(File, open);
EH_METHOD(File, getc);
EH_METHOD(File, gets);
EH_METHOD(File, puts);
EH_METHOD(File, close);
EH_METHOD(File, finalize);
EH_METHOD(File, readFile);
EH_METHOD(File, isOpen);

EH_INITIALIZER(File);
