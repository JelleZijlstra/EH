/*
 * File library class
 */
#include "std_lib_includes.hpp"

EH_CLASS(File) {
public:
    class t {
    public:
        FILE *fd;
        bool open;

        t() : fd(nullptr), open(false) {}

        ~t() {
            if(open) {
                fclose(fd);
            }
        }
    };
	typedef t *type;
	type value;

	File(type val) : value(val) {}
	virtual ~File() {
        delete value;
    }

	virtual bool belongs_in_gc() const {
		return false;
	}
private:
	File(const File&);
	File operator=(const File&);
};

EH_METHOD(File, operator_colon);
EH_METHOD(File, open);
EH_METHOD(File, getc);
EH_METHOD(File, gets);
EH_METHOD(File, puts);
EH_METHOD(File, close);
EH_METHOD(File, finalize);
EH_METHOD(File, readFile);
EH_METHOD(File, isOpen);
EH_METHOD(File, temporary);
EH_METHOD(File, dirname);
EH_METHOD(File, fullPath);

EH_INITIALIZER(File);
