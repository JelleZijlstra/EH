#include "File.h"

EH_INITIALIZER(File) {
	REGISTER_METHOD(File, initialize);
	REGISTER_METHOD(File, open);
	REGISTER_METHOD(File, getc);
	REGISTER_METHOD(File, gets);
	REGISTER_METHOD(File, puts);
	REGISTER_METHOD(File, close);
	REGISTER_METHOD(File, toBool);
	REGISTER_METHOD(File, finalize);
}

EH_METHOD(File, initialize) {
	return ehretval_t::make_resource((LibraryBaseClass *)new File());
}
EH_METHOD(File, open) {
	File *selfptr = (File *) obj->get_resourceval();

	ehretval_p filename = ehi->to_string(args, obj);
	ASSERT_TYPE(filename, string_e, "File.open");
	FILE *mfile = fopen(filename->get_stringval(), "r+");
	if(mfile == NULL) {
		return NULL;
	}
	selfptr->descriptor = mfile;
	return ehretval_t::make_bool(true);
}
EH_METHOD(File, getc) {
	ASSERT_TYPE(args, null_e, "File.getc");
	File *selfptr = (File *) obj->get_resourceval();

	if(selfptr->descriptor == NULL) {
		return NULL;
	}
	int c = fgetc(selfptr->descriptor);
	if(c == -1) {
		return NULL;
	}
	char *out = new char[2];
	out[0] = c;
	out[1] = '\0';
	return ehretval_t::make_string(out);
}
EH_METHOD(File, gets) {
	ASSERT_TYPE(args, null_e, "File.gets");
	File *selfptr = (File *) obj->get_resourceval();
	if(selfptr->descriptor == NULL) {
		return NULL;
	}
	
	char *out = new char[512];

	char *ptr = fgets(out, 511, selfptr->descriptor);
	if(ptr == NULL) {
		delete[] out;
		return NULL;
	}
	return ehretval_t::make_string(out);
}
EH_METHOD(File, puts) {
	File *selfptr = (File *) obj->get_resourceval();
	if(selfptr->descriptor == NULL) {
		return NULL;
	}
	
	ASSERT_TYPE(args, string_e, "File.puts");

	int count = fputs(args->get_stringval(), selfptr->descriptor);
	
	if(count == EOF) {
		return ehretval_t::make_bool(false);
	} else {
		return ehretval_t::make_bool(true);
	}
}
EH_METHOD(File, close) {
	ASSERT_TYPE(args, null_e, "File.close");
	File *selfptr = (File *) obj->get_resourceval();
	if(selfptr->descriptor == NULL) {
		return NULL;
	}
	fclose(selfptr->descriptor);
	selfptr->descriptor = NULL;
	return NULL;
}
EH_METHOD(File, toBool) {
	ASSERT_TYPE(args, null_e, "File.toBool");
	File *selfptr = (File *)obj->get_resourceval();
	return ehretval_t::make_bool(selfptr->descriptor != NULL);
}
EH_METHOD(File, finalize) {
	File *selfptr = (File *)obj->get_resourceval();
	if(selfptr->descriptor != NULL) {
		fclose(selfptr->descriptor);
		selfptr->descriptor = NULL;
	}
	return NULL;
}
